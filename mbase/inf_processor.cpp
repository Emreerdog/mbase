#include <mbase/inference/inf_processor.h>
#include <mbase/inference/inf_client.h>
#include <mbase/inference/inf_model.h>
#include <mbase/inference/inf_sampling.h>
#include <mbase/framework/timer_loop.h>
#include <mbase/framework/timers.h>
#include <common/common.h>

MBASE_BEGIN

#define MBASE_INF_PROC_RETURN_UNREGISTERED \
if(this->signal_state_initializing())\
{\
	return flags::INF_PROC_INFO_INITIALIZING;\
}\
if(!this->is_registered())\
{\
	return flags::INF_PROC_ERR_UNREGISTERED_PROCESSOR;\
}\
if (this->signal_state_destroying())\
{\
	return flags::INF_PROC_INFO_DESTROYING;\
}

#define MBASE_INF_PROC_RETURN_HALTED \
if(!this->is_registered())\
{\
	return flags::INF_PROC_ERR_UNREGISTERED_PROCESSOR;\
}\
if(!this->is_running())\
{\
	return flags::INF_PROC_ERR_HALTED;\
}

static U32 gInfProcMaxTokenLength = 128;

InfProcessorBase::InfProcessorBase() :
	mTargetModel_md_model(NULL),
	mIsRunning(false),
	mIsRegistered(false),
	mContextSize_md_model(0),
	mProcessorId_md_model(0),
	mInactivityThreshold(0)
{
}

bool InfProcessorBase::is_registered() const
{
	return mIsRegistered;
}

bool InfProcessorBase::is_running()
{
	return mIsRunning;
}

bool InfProcessorBase::signal_state_initializing() const
{
	return mInitializeSignal.get_signal_state();
}

bool InfProcessorBase::signal_state_destroying() const
{
	return mDestroySignal.get_signal_state();
}

bool InfProcessorBase::signal_initializing() const
{
	return mInitializeSignal.get_signal();
}

bool InfProcessorBase::signal_destroying() const
{
	return mDestroySignal.get_signal();
}

InfProcessorBase::flags InfProcessorBase::get_context_size(U32& out_size)
{
	MBASE_INF_PROC_RETURN_UNREGISTERED;
	out_size = mContextSize_md_model;
	return flags::INF_PROC_SUCCESS;
}

InfModelBase* InfProcessorBase::get_processed_model()
{
	return mTargetModel_md_model;
}

U32 InfProcessorBase::get_inactivity_threshold()
{
	return mInactivityThreshold;
}

processor_signal& InfProcessorBase::get_initialize_signal()
{
	return mInitializeSignal;
}

processor_signal& InfProcessorBase::get_destroy_signal()
{
	return mDestroySignal;
}

GENERIC InfProcessorBase::set_inactivity_threshold(U32 in_threshold)
{
	mInactivityThreshold = in_threshold;
}

GENERIC InfProcessorBase::halt()
{
	mIsRunning = false;
}

GENERIC InfProcessorBase::resume()
{
	mIsRunning = true;
}

InfTextToTextProcessor::InfTextToTextProcessor():
	mModelContext(NULL),
	//mPresetCandidates(),
	mGeneratedToken(0),
	mContextLength(0),
	mContextCursor(0),
	mTokenizedInput(),
	mContextState(context_state::AWAITING_FOR_INPUT),
	mFinishState(finish_state::FINISHED),
	mAssignedClient(NULL),
	mSamplerChain(NULL),
	mIsSamplerSet(false)
{
	auto sparams = llama_sampler_chain_default_params();
	mSamplerChain = llama_sampler_chain_init(sparams);
}

InfTextToTextProcessor::~InfTextToTextProcessor()
{
	destroy_sync();
}

bool InfTextToTextProcessor::is_available() const
{
	if (signal_input_process() || signal_decode_process() || signal_state_decode_process())
	{
		return false;
	}

	return true;
}

bool InfTextToTextProcessor::signal_state_input_process() const
{
	return mInputSignal.get_signal_state();
}

bool InfTextToTextProcessor::signal_state_decode_process() const
{
	return mDecodeSignal.get_signal_state();
}

bool InfTextToTextProcessor::signal_input_process() const
{
	return mInputSignal.get_signal();
}

bool InfTextToTextProcessor::signal_decode_process() const
{
	return mDecodeSignal.get_signal();
}

bool InfTextToTextProcessor::signal_token_generated() const
{
	return mTokenGeneratedSignal.get_signal();
}

bool InfTextToTextProcessor::signal_init_method() const
{
	return mInitializeMethodSignal.get_signal();
}

bool InfTextToTextProcessor::signal_destroy_method() const
{
	return mDestroyMethodSignal.get_signal();
}

//typename InfTextToTextProcessor::inf_token_candidates& InfTextToTextProcessor::get_token_candidates()
//{
//	return mPresetCandidates;
//}

U32 InfTextToTextProcessor::get_max_token_length()
{
	return mContextLength;
}

InfClientTextToText* InfTextToTextProcessor::get_assigned_client()
{
	return mAssignedClient;
}

bool InfTextToTextProcessor::has_sampler(const mbase::string& in_sampler_name, InfSamplerMeta& out_sampler)
{
	for(mbase::vector<InfSamplerMeta>::iterator It = mSamplingOrder.begin(); It != mSamplingOrder.end(); ++It)
	{
		InfSamplerMeta smpBase = *It;
		if(smpBase.mSamplerName == in_sampler_name)
		{
			out_sampler = smpBase;
			return true;
		}
	}
	return false;
}

GENERIC InfTextToTextProcessor::get_available_samplers(mbase::vector<InfSamplerMeta>& out_samplers)
{
	out_samplers = mSamplingOrder;
}

bool InfTextToTextProcessor::has_client() const
{
	return mAssignedClient != NULL;
}

InfTextToTextProcessor::flags InfTextToTextProcessor::get_processor_status() const
{
	if(signal_state_initializing())
	{
		return flags::INF_PROC_INFO_INITIALIZING;
	}

	if(!is_registered())
	{
		return flags::INF_PROC_ERR_UNREGISTERED_PROCESSOR;
	}

	if(signal_state_destroying())
	{
		return flags::INF_PROC_INFO_DESTROYING;
	}

	return flags::INF_PROC_SUCCESS;
}

InfTextToTextProcessor::flags InfTextToTextProcessor::tokenize_input(CBYTEBUFFER in_data, size_type in_size, mbase::vector<inf_token>& out_tokens)
{
	MBASE_INF_PROC_RETURN_UNREGISTERED;
	
	if(!in_size)
	{
		return flags::INF_PROC_ERR_INPUT_IS_EMPTY;
	}

	mbase::vector<inf_token> tokenizedInput(in_size * 4);
	InfModelTextToText* t2tModel = static_cast<InfModelTextToText*>(this->mTargetModel_md_model);
	I32 tokenCount = llama_tokenize(t2tModel->get_raw_model(), in_data, in_size, tokenizedInput.data(), in_size * 4, false, true);

	if(tokenCount == -1)
	{
		return flags::INF_PROC_ERR_UNABLE_TO_TOKENIZE_INPUT;
	}

	tokenizedInput.resize(tokenCount);
	out_tokens = std::move(tokenizedInput);
	return flags::INF_PROC_SUCCESS;
}

InfTextToTextProcessor::flags InfTextToTextProcessor::tokenize_input(context_line* in_lines, size_type in_count, mbase::vector<inf_token>& out_tokens, bool in_append_assistant_token)
{
	MBASE_INF_PROC_RETURN_UNREGISTERED;
	
	if(!in_lines || !in_count)
	{
		return flags::INF_PROC_ERR_INPUT_IS_EMPTY;
	}

	mbase::string totalMessage;
	InfModelTextToText* t2tModel = static_cast<InfModelTextToText*>(this->mTargetModel_md_model);

	for(size_type i = 0; i < in_count; ++i)
	{
		context_line* tmpLine = in_lines + i;
		mbase::string roleString;
		mbase::string endString;
		if(tmpLine->mRole == context_role::SYSTEM)
		{
			t2tModel->get_sys_start(roleString);
			t2tModel->get_sys_end(endString);
		}

		else if(tmpLine->mRole == context_role::ASSISTANT)
		{
			t2tModel->get_assistant_start(roleString);
			t2tModel->get_assistant_end(endString);
		}

		else if(tmpLine->mRole == context_role::USER)
		{
			t2tModel->get_usr_start(roleString);
			t2tModel->get_usr_end(endString);
		}

		totalMessage += (roleString + tmpLine->mMessage + endString);
	}
	
	if(totalMessage.size())
	{
		context_line* lastLine = in_lines + (in_count - 1);
		if(lastLine->mRole == context_role::USER)
		{
			if (in_append_assistant_token)
			{
				mbase::string assistantToken;
				t2tModel->get_assistant_start(assistantToken);
				totalMessage += assistantToken;
			}
		}
	}

	return tokenize_input(totalMessage.data(), totalMessage.size(), out_tokens);
}

InfTextToTextProcessor::flags InfTextToTextProcessor::execute_input(const mbase::vector<inf_token>& in_tokens, bool in_abandon)
{
	MBASE_INF_PROC_RETURN_UNREGISTERED;

	if(!in_tokens.size())
	{
		return flags::INF_PROC_ERR_INPUT_IS_EMPTY;
	}

	if(in_tokens.size() > mContextLength)
	{
		return flags::INF_PROC_ERR_INPUT_EXCEED_TOKEN_LIMIT;
	}

	if (!is_running())
	{
		return flags::INF_PROC_INFO_HALTED;
	}

	if(!in_abandon)
	{
		if(!is_available())
		{
			return flags::INF_PROC_ERR_ALREADY_PROCESSING;
		}
	}

	else
	{
		mInputSignal.reset_signal_with_state();
		mDecodeSignal.reset_signal_with_state();
	}

	if(!mIsSamplerSet)
	{
		I32 seedValue = 3261246819; // TODO, GET THIS FROM APPLICATION CONFIGURATION
		InfModelTextToText* t2tModel = static_cast<InfModelTextToText*>(this->mTargetModel_md_model);
		I32 modelVocab = 0;
		inf_token eotId = 0;
		inf_token lineFeedId = 0;

		t2tModel->get_vocab_count(modelVocab);
		t2tModel->get_eot_token(eotId);
		t2tModel->get_lf_token(lineFeedId);

		llama_sampler_chain_add(mSamplerChain, llama_sampler_init_softmax());
		llama_sampler_chain_add(mSamplerChain, llama_sampler_init_dist(seedValue));
		llama_sampler_chain_add(mSamplerChain, llama_sampler_init_mirostat_v2(seedValue, 5.0, 0.100));

		mIsSamplerSet = true;
	}
	mTokenizedInput = in_tokens;
	mInputSignal.set_signal();
	
	return flags::INF_PROC_SUCCESS;
}

InfTextToTextProcessor::flags InfTextToTextProcessor::next()
{
	MBASE_INF_PROC_RETURN_UNREGISTERED;

	if(is_available())
	{
		return flags::INF_PROC_ERR_INPUT_IS_EMPTY;
	}

	mDecodeSignal.set_signal();
	if (!is_running())
	{
		return flags::INF_PROC_INFO_HALTED;
	}
	
	return flags::INF_PROC_SUCCESS;
}

InfTextToTextProcessor::flags InfTextToTextProcessor::set_inference_client(InfClientTextToText* in_client, bool in_reset_on_set)
{
	MBASE_INF_PROC_RETURN_UNREGISTERED;

	if(get_assigned_client())
	{
		return flags::INF_PROC_ERR_ALREADY_PROCESSING;
	}

	if(!in_client)
	{
		return flags::INF_PROC_ERR_MISSING_CLIENT;
	}

	if(in_client->is_registered())
	{
		InfTextToTextProcessor* clientProc = NULL;
		in_client->get_host_processor(clientProc);
		if(clientProc != this)
		{
			return flags::INF_PROC_ERR_BELONGS_TO_ANOTHER_PROCESSOR;
		}
		else
		{
			return flags::INF_PROC_SUCCESS;
		}
	}

	mAssignedClient = in_client;
	mAssignedClient->_on_register(this);

	return flags::INF_PROC_SUCCESS;
}

InfTextToTextProcessor::flags InfTextToTextProcessor::initialize(InfModelTextToText* in_model, U32 in_context_length)
{
	if(is_registered())
	{
		return flags::INF_PROC_SUCCESS;
	}

	if(signal_state_initializing())
	{
		return flags::INF_PROC_INFO_INITIALIZING;
	}

	if(signal_state_destroying())
	{
		return flags::INF_PROC_INFO_DESTROYING;
	}
	mTargetModel_md_model = in_model;
	mContextLength = in_context_length;
	mDiagnostics.log(PcDiagnostics::flags::LOGTYPE_INFO, PcDiagnostics::flags::LOGIMPORTANCE_HIGH, "Initializing %d", 15);
	mInitializeSignal.set_signal_with_state();
	start_processor();
	return flags::INF_PROC_INFO_INITIALIZING;
}

InfTextToTextProcessor::flags InfTextToTextProcessor::initialize_sync(InfModelTextToText* in_model, U32 in_context_length)
{
	initialize(in_model, in_context_length);
	while(signal_state_initializing())
	{

	}
	if(!is_registered())
	{
		return flags::INF_PROC_ERR_UNREGISTERED_PROCESSOR;
	}
	return flags::INF_PROC_SUCCESS;
}

InfTextToTextProcessor::flags InfTextToTextProcessor::destroy()
{
	clear_samplers();
	if(!is_registered())
	{
		return flags::INF_PROC_SUCCESS;
	}

	release_inference_client();

	if(signal_state_destroying())
	{
		return flags::INF_PROC_INFO_DESTROYING;
	}

	mDestroySignal.set_signal_with_state();
	return flags::INF_PROC_INFO_DESTROYING;
}

InfTextToTextProcessor::flags InfTextToTextProcessor::destroy_sync()
{
	destroy();
	while(signal_state_destroying())
	{
		// block until operation finishes
	}

	return flags::INF_PROC_INFO_NEED_UPDATE;
}

GENERIC InfTextToTextProcessor::release_inference_client()
{
	InfClientTextToText* assignedClient = get_assigned_client();
	if(assignedClient)
	{
		assignedClient->_on_unregister();
		mAssignedClient = NULL;
	}
}

GENERIC InfTextToTextProcessor::clear_token_candidates()
{
	mPresetCandidates.clear();
}

GENERIC InfTextToTextProcessor::clear_samplers()
{
	// TODO: Do not allow clearing when processing tokens
	// TODO: Delete the return when the sampler interface is properly working
	
	if(mSamplerChain)
	{
		llama_sampler_free(mSamplerChain);
		mIsSamplerSet = false;
		mSamplingOrder.clear();
		mSamplerChain = NULL;
	}
}

GENERIC InfTextToTextProcessor::on_initializing()
{

}

GENERIC InfTextToTextProcessor::on_initialize_fail()
{

}

GENERIC InfTextToTextProcessor::on_destroying()
{

}

InfTextToTextProcessor::flags InfTextToTextProcessor::add_sampler(const InfSamplingInput& in_sampling)
{
	// TODO: Do not allow adding samplers while clearing

	InfSamplerMeta tmpSampler;
	if(has_sampler(in_sampling.mSamplerName, tmpSampler))
	{
		return flags::INF_PROC_SUCCESS;
	}
	
	llama_sampler* newSampler = NULL;
	
	if(in_sampling.mSamplerName == "TEMP")
	{
		newSampler = llama_sampler_init_temp(in_sampling.mSamplerValue);
		llama_sampler_chain_add(mSamplerChain, newSampler);
		mSamplingOrder.push_back({ "TEMP", newSampler, true });
	}
	
	else if(in_sampling.mSamplerName == "TOP_K")
	{
		I32 kValue = in_sampling.mSamplerValue;
		newSampler = llama_sampler_init_top_k((I32)in_sampling.mSamplerValue);
		llama_sampler_chain_add(mSamplerChain, newSampler);
		mSamplingOrder.push_back({ "TOP_K", newSampler, true });
	}

	else if(in_sampling.mSamplerName == "TYPICAL_P")
	{
		newSampler = llama_sampler_init_typical(in_sampling.mSamplerValue, 1);
		llama_sampler_chain_add(mSamplerChain, newSampler);
		mSamplingOrder.push_back({ "TYPICAL_P", newSampler, true });
	}

	else if(in_sampling.mSamplerName == "TOP_P")
	{
		newSampler = llama_sampler_init_top_p(in_sampling.mSamplerValue, 1);
		llama_sampler_chain_add(mSamplerChain, newSampler);
		mSamplingOrder.push_back({ "TOP_P", newSampler, true });
	}
	
	else if(in_sampling.mSamplerName == "MIN_P")
	{
		newSampler = llama_sampler_init_min_p(in_sampling.mSamplerValue, 1);
		llama_sampler_chain_add(mSamplerChain, newSampler);
		mSamplingOrder.push_back({ "MIN_P", newSampler, true });
	}

	else if(in_sampling.mSamplerName == "TFZ")
	{
		newSampler = llama_sampler_init_tail_free(in_sampling.mSamplerValue, 1);
		llama_sampler_chain_add(mSamplerChain, newSampler);
		mSamplingOrder.push_back({ "TFZ", newSampler, true });
	}
	else
	{
		return flags::INF_PROC_ERR_SAMPLER_NAME_MISMATCH;
	}

	return flags::INF_PROC_SUCCESS;
}

GENERIC InfTextToTextProcessor::update()
{
	if(is_registered())
	{
		if(signal_init_method())
		{
			mInitializeMethodSignal.reset_signal_with_state();
			on_initialize();
		}

		if(signal_token_generated())
		{
			// do things with the generated token
			InfModelTextToText* t2tModel = static_cast<InfModelTextToText*>(this->mTargetModel_md_model);
			mTokenGeneratedSignal.reset_signal_with_state();
			InfClientTextToText* t2tClient = get_assigned_client();
			if(!t2tClient)
			{
				// means client is abandoned the context
				return;
			}

			bool isTokenControl = false;
			bool isFinish = false;
			if(t2tModel->is_token_control(mGeneratedToken) == InfModelTextToText::flags::INF_MODEL_SUCCESS)
			{
				isTokenControl = true;
			}
			
			if(mFinishState != finish_state::CONTINUE)
			{
				isFinish = true;
			} 

			IBYTE tokenString[64] = { 0 };
			I32 tokenLength = llama_token_to_piece(t2tModel->get_raw_model(), mGeneratedToken, tokenString, 64, false, true);
			mbase::string outString(tokenString, tokenLength);
			t2tClient->on_write(outString.c_str(), outString.size(), mGeneratedToken, isTokenControl, isFinish);

			if(isFinish)
			{
				t2tClient->on_finish(mContextCursor, mFinishState);
			}
		}
	}
	else
	{
		if(signal_destroy_method())
		{
			mDestroyMethodSignal.reset_signal_with_state();
			on_destroy();
		}
	}
}

GENERIC InfTextToTextProcessor::_decode_input()
{
	// if the input signal is set, process
	if(llama_get_kv_cache_used_cells(mModelContext))
	{
		llama_kv_cache_clear(mModelContext);
	}

	llama_batch_clear(mInputBatch);
	for (size_type i = 0; i < mTokenizedInput.size(); ++i)
	{
		llama_batch_add(mInputBatch, mTokenizedInput[i], i, { 0 }, false);
	}

	mInputBatch.logits[mInputBatch.n_tokens - 1] = true;
	mContextCursor = mInputBatch.n_tokens;
	mContextState = context_state::DECODING_INPUT;
	mFinishState = finish_state::CONTINUE;
	
	int decodeResult = llama_decode(mModelContext, mInputBatch);
	mInputSignal.reset_signal_with_state();
	mDecodeSignal.set_signal_state();
}

GENERIC InfTextToTextProcessor::_decode_next()
{
	mDecodeSignal.reset_signal();
	// if the decode signal state is set, process
	mContextState = context_state::GENERATING_OUTPUT;
	I32 modelVocab = 0;
	InfModelTextToText* t2tModel = static_cast<InfModelTextToText*>(this->mTargetModel_md_model);
	t2tModel->get_vocab_count(modelVocab);

	clear_token_candidates();
	PTRF32 logits = llama_get_logits_ith(mModelContext, mInputBatch.n_tokens - 1);
	for (llama_token token_id = 0; token_id < modelVocab; ++token_id)
	{
		mPresetCandidates.emplace_back(llama_token_data{ token_id, logits[token_id], 0.0f });
	}
	llama_token_data_array tokenCandidates = { mPresetCandidates.data(), mPresetCandidates.size(), false};
	llama_sampler_reset(mSamplerChain);
	llama_sampler_apply(mSamplerChain, &tokenCandidates);
	mGeneratedToken = llama_sampler_sample(mSamplerChain, mModelContext, -1);
	//llama_sampler_reset(greedySampler);
	llama_sampler_accept(mSamplerChain, mGeneratedToken);
	//llama_sampler_accept(mSamplerChain, mGeneratedToken);
	


	//llama_token_data_array tokenCandidates = { mPresetCandidates.data(), mPresetCandidates.size(), false };
	//mGeneratedToken = llama_sampler_sample(mGreedy, mModelContext, -1);
	//llama_sampler_accept(mSamplerChain, mGeneratedToken);
	
	llama_batch_clear(mInputBatch);
	
	if (llama_token_is_eog(t2tModel->get_raw_model(), mGeneratedToken))
	{
		// means end of generation
		//mSamplingOrder.clear();
		mFinishState = finish_state::FINISHED;
		llama_kv_cache_clear(mModelContext);
		clear_token_candidates();
		mDecodeSignal.reset_signal_with_state();
		mTokenGeneratedSignal.set_signal();
	}

	else
	{
		clear_token_candidates();

		if (mContextCursor == mContextLength)
		{
			// means token limit is reached
			//mSamplingOrder.clear();
			mContextState = context_state::AWAITING_FOR_CURSOR_ALIGNMENT;
			mFinishState = finish_state::TOKEN_LIMIT_REACHED;
			llama_kv_cache_clear(mModelContext);
			mDecodeSignal.reset_signal_with_state();
		}

		else
		{
			llama_batch_add(mInputBatch, mGeneratedToken, mContextCursor, { 0 }, true);
			mContextCursor++;
			llama_decode(mModelContext, mInputBatch);
		}
		mTokenGeneratedSignal.set_signal();
	}
}

GENERIC InfTextToTextProcessor::_initialize_context()
{
	llama_context_params ctxParams = llama_context_default_params();
	ctxParams.n_ctx = mContextLength;
	ctxParams.n_batch = mContextLength;
	ctxParams.n_seq_max = 1;
	ctxParams.n_threads = 32;
	ctxParams.n_threads_batch = 32;
	ctxParams.n_ubatch = mContextLength / 8;

	InfModelTextToText* t2tModel = static_cast<InfModelTextToText*>(this->mTargetModel_md_model);

	mModelContext = llama_new_context_with_model(t2tModel->get_raw_model(), ctxParams);
	if (!mModelContext)
	{
		mInitializeSignal.reset_signal_with_state();
		return;
	}
	
	mInputBatch = llama_batch_init(mContextLength, 0, 1);
	mContextCursor = 0;
	mIsRunning = true;
	mInitializeSignal.reset_signal_with_state();
	mIsRegistered = true;
	
	/*I32 modelVocabCount = 0;
	t2tModel->get_vocab_count(modelVocabCount);*/
	
	//llama_sampler_chain_add(mSamplerChain, llama_sampler_init_greedy());
	//llama_sampler_chain_add(mSamplerChain, llama_sampler_init_penalties(modelVocabCount, ));
	
	I32 modelVocabCount = 0;
	inf_token eotToken = 0;
	inf_token nlToken = 0;
	// List /usr file
	t2tModel->get_vocab_count(modelVocabCount);
	t2tModel->get_eot_token(eotToken);
	t2tModel->get_lf_token(nlToken);
	I32 seedValue = 1048204757;

	llama_sampler* penaltySampler = llama_sampler_init_penalties(modelVocabCount, eotToken, nlToken, 64, 1.5, 0.5, 0.5, false, false);
	llama_sampler_chain_add(mSamplerChain, penaltySampler);
	//llama_sampler_chain_add(mSamplerChain, llama_sampler_init_temp(0.1));
	//llama_sampler_chain_add(mSamplerChain, llama_sampler_init_min_p(0.5, 1));
	//llama_sampler_chain_add(mSamplerChain, llama_sampler_init_top_p(0.95, 1));
	//llama_sampler_chain_add(mSamplerChain, llama_sampler_init_top_k(50));
	//llama_sampler_chain_add(mSamplerChain, llama_sampler_init_softmax());
	//llama_sampler_chain_add(mSamplerChain, llama_sampler_init_dist(seedValue));
	//llama_sampler_chain_add(mSamplerChain, llama_sampler_init_mirostat_v2(seedValue, 4.0, 0.100));	
	
	//llama_sampler_chain_add(mSamplerChain, llama_sampler_init_penalties(modelVocabCount, 128009, 128, 64, 1.5, 0.0, 0.0, false, true));

	/*;
	llama_sampler_chain_add(mSamplerChain, llama_sampler_init_penalties(modelVocabCount, 128009, 128, 64, 1.5, 0.0, 0.0, true, true));
	llama_sampler_chain_add(mSamplerChain, llama_sampler_init_top_k(5));
	llama_sampler_chain_add(mSamplerChain, llama_sampler_init_tail_free(1.0, 0));
	llama_sampler_chain_add(mSamplerChain, llama_sampler_init_typical(1.0, 0));
	llama_sampler_chain_add(mSamplerChain, llama_sampler_init_top_p(0.95, 0));
	llama_sampler_chain_add(mSamplerChain, llama_sampler_init_min_p(0.050, 0));
	llama_sampler_chain_add(mSamplerChain, llama_sampler_init_temp(0.8));
	llama_sampler_chain_add(mSamplerChain, llama_sampler_init_softmax());
	llama_sampler_chain_add(mSamplerChain, llama_sampler_init_dist(seedValue))
	*/

	mInitializeMethodSignal.set_signal_with_state();
}

GENERIC InfTextToTextProcessor::_destroy_context()
{
	llama_batch_free(mInputBatch);
	llama_free(mModelContext);
	mContextCursor = 0;
	mModelContext = NULL;
	mIsRunning = false;
	//mPresetCandidates.clear();
	mContextLength = 0;
	mContextCursor = 0;
	mTokenizedInput.clear();
	mPenaltyList.clear();
	mSamplingOrder.clear();
	mContextState = context_state::AWAITING_FOR_INPUT;
	mFinishState = finish_state::FINISHED;

	clear_samplers();

	mInitializeSignal.reset_signal_with_state();
	mTokenGeneratedSignal.reset_signal_with_state();
	mDecodeSignal.reset_signal_with_state();
	mDestroySignal.reset_signal_with_state();
	mTargetModel_md_model = NULL;
	mIsRegistered = false;

	mDestroyMethodSignal.set_signal_with_state();
}

GENERIC InfTextToTextProcessor::update_t()
{
	while(is_processor_running())
	{
		if(is_registered())
		{
			if (is_running())
			{
				if (signal_input_process())
				{
					_decode_input(); /* SIG_SET(decode), SIGW_STATE_RESET(input) */
				}

				if (signal_decode_process())
				{
					_decode_next();
					/*
					* IF TOKEN END OF GENERATION --> SIG_SET(token_generated), SIGW_STATE_RESET(decode)
					* IF NOT TOKEN END OF GENERATION --> SIG_SET(token_generated), SIG_STATE_RESET(decode)
					*
					*
					*/
				}
				else
				{
					// decode is not present
					//mbase::sleep(30); // slow down the loop
				}
			}

			if(signal_destroying())
			{
				_destroy_context();
			}

			else
			{
				//mbase::sleep(30); // Since the context is halted, we may slow down the loop
			}
		}
		else
		{
			if(signal_initializing())
			{
				_initialize_context();
			}
			else
			{
				//mbase::sleep(30); // unregistered context, we may slow down the loop
			}
		}
	}
}

MBASE_END
