#include <mbase/inference/inf_t2t_processor.h>
#include <mbase/inference/inf_t2t_model.h>
#include <mbase/inference/inf_t2t_client.h>
#include <chrono>

MBASE_BEGIN

#define MBASE_INF_T2T_PROC_RETURN_UNREGISTERED \
if (this->signal_destroying())\
{\
	return flags::INF_PROC_INFO_DESTROYING;\
}\
if(this->signal_initializing())\
{\
	return flags::INF_PROC_INFO_INITIALIZING;\
}\
if(!this->is_registered())\
{\
	return flags::INF_PROC_ERR_UNREGISTERED_PROCESSOR;\
}

#define MBASE_INF_T2T_PROC_RETURN_HALTED \
if(!this->is_registered())\
{\
	return flags::INF_PROC_ERR_UNREGISTERED_PROCESSOR;\
}\
if(!this->is_running())\
{\
	return flags::INF_PROC_ERR_HALTED;\
}

InfProcessorTextToText::InfProcessorTextToText():
	mSamplerChain(NULL),
	mModelContext(NULL),
	mPresetCandidates(),
	mContextCursor(0),
	mBatchSize(0),
	mThreadCount(0),
	mBatchProcessThreadCount(0),
	mProcessedBatchLength(0),
	mLogitStartIndex(0),
	mPromptStartIndex(0),
	mFinishState(finish_state::FINISHED),
	mLastFailCode(last_fail_code::MODEL_NOT_INITIALIZED),
	mFlashAttention(false),
	mIsInitializeFailed(false),
	mIsManualCaching(false),
	mIsBenchmarkOn(false),
	mCacheMode(cache_mode::AUTO_LOGIT_STORE_MODE)
{
	mModelCategory = inf_model_category::TEXT_TO_TEXT;
}

InfProcessorTextToText::~InfProcessorTextToText()
{
	if(mModelContext)
	{
		stop_processor();
		llama_free(mModelContext);
		this->release_object_watcher();

		if(mAssignedClient)
		{
			mAssignedClient->on_unregister(this);
			mAssignedClient = NULL;
		}
	}
}

InfProcT2TDiagnostics& InfProcessorTextToText::get_diagnostics()
{
	return mDiagnostics;
}

InfProcessorTextToText::last_fail_code InfProcessorTextToText::get_last_fail_code() const
{
	return mLastFailCode;
}

InfProcessorTextToText::cache_mode InfProcessorTextToText::get_manual_cache_mode() const
{
	return mCacheMode;
}

bool InfProcessorTextToText::is_benchmark() const
{
	return mIsBenchmarkOn;
}

bool InfProcessorTextToText::is_update_required() const
{
	if(signal_state_initializing() || signal_state_destroying() || signal_state_input_process() || signal_state_decode_process() || signal_state_kv_locked_process())
	{
		return true;
	}
	return false;
}

bool InfProcessorTextToText::is_init_failed() const
{
	return mIsInitializeFailed;
}

bool InfProcessorTextToText::is_available() const
{
	if (signal_initializing() || signal_destroying() || signal_input_process() || signal_decode_process() || signal_kv_locked_process())
	{
		return false;
	}

	return true;
}

bool InfProcessorTextToText::is_manual_caching() const
{
	return mIsManualCaching;
}

bool InfProcessorTextToText::signal_state_lora_operate() const
{
	return mLoraOperationSignal.get_signal_state();
}

bool InfProcessorTextToText::signal_state_input_process() const
{
	return mInputSignal.get_signal_state();
}

bool InfProcessorTextToText::signal_state_decode_process() const
{
	return mDecodeSignal.get_signal_state();
}

bool InfProcessorTextToText::signal_state_kv_locked_process() const
{
	return mInputKvLockedSignal.get_signal_state();
}

bool InfProcessorTextToText::signal_lora_operate_process() const
{
	return mLoraOperationSignal.get_signal();
}

bool InfProcessorTextToText::signal_input_process() const
{
	return mInputSignal.get_signal();
}

bool InfProcessorTextToText::signal_decode_process() const
{
	return mDecodeSignal.get_signal();
}

bool InfProcessorTextToText::signal_kv_locked_process() const
{
	return mInputKvLockedSignal.get_signal();
}

const U32& InfProcessorTextToText::get_batch_size() const
{
	return mBatchSize;
}

const U32& InfProcessorTextToText::get_max_token_length() const
{
	return mContextLength;
}

const U32& InfProcessorTextToText::get_context_cursor_position() const
{
	return mContextCursor;
}

I32 InfProcessorTextToText::get_cache_token_count() const
{
	return llama_kv_self_n_tokens(mModelContext);
}

I32 InfProcessorTextToText::get_batch_thread_count() const
{
	return llama_n_threads_batch(mModelContext);
}
I32 InfProcessorTextToText::get_thread_count() const
{
	return llama_n_threads(mModelContext);
}

bool InfProcessorTextToText::has_sampler(InfSamplerDescription::SAMPLER in_sampler_type, InfSamplerDescription& out_sampler)
{
	for(inf_sampling_set::iterator It = mSamplerDescriptions.begin(); It != mSamplerDescriptions.end(); ++It)
	{
		InfSamplerDescription smpBase = *It;
		if(smpBase.mSamplerType == in_sampler_type)
		{
			out_sampler = smpBase;
			return true;
		}
	}
	return false;
}

GENERIC InfProcessorTextToText::get_available_samplers(inf_sampling_set& out_samplers)
{
	out_samplers = mSamplerDescriptions;
}

InfProcessorTextToText::flags InfProcessorTextToText::get_processor_status() const
{
	if(signal_initializing())
	{
		return flags::INF_PROC_INFO_INITIALIZING;
	}

	if(signal_destroying())
	{
		return flags::INF_PROC_INFO_DESTROYING;
	}

	if(signal_input_process())
	{
		return flags::INF_PROC_INFO_INITIAL_INPUT_PROCESSING;
	}

	if(signal_decode_process())
	{
		return flags::INF_PROC_INFO_DECODING;
	}

	if(is_update_required())
	{
		return flags::INF_PROC_INFO_NEED_UPDATE;
	}

	return flags::INF_PROC_SUCCESS;
}

InfProcessorTextToText::flags InfProcessorTextToText::token_to_description(const inf_text_token& in_token, inf_token_description& out_description)
{
	MBASE_INF_T2T_PROC_RETURN_UNREGISTERED;
	InfModelTextToText* t2tModel = static_cast<InfModelTextToText*>(this->mTargetModel_md_model);

	IBYTE tokenString[64] = {0};
	const llama_vocab* tmpVocab = llama_model_get_vocab(t2tModel->get_raw_model());
	I32 tokenLength = llama_token_to_piece(tmpVocab, in_token, tokenString, 64, false, true);

	if(!tokenLength)
	{
		// UNKNOWN ERROR
		// GO BACK HERE LATER
	}

	out_description.mTokenString = mbase::string(tokenString, tokenLength);
	
	if(t2tModel->is_token_control(in_token) == InfModelTextToText::flags::INF_MODEL_SUCCESS)
	{
		out_description.mIsSpecial = true;
	}

	return flags::INF_PROC_SUCCESS;
}

InfProcessorTextToText::flags InfProcessorTextToText::tokens_to_description_vector(const mbase::vector<inf_text_token>& in_tokens, mbase::vector<inf_token_description>& out_descriptions)
{
	MBASE_INF_T2T_PROC_RETURN_UNREGISTERED;
	InfModelTextToText* t2tModel = static_cast<InfModelTextToText*>(this->mTargetModel_md_model);

	for(mbase::vector<inf_text_token>::const_iterator cIt = in_tokens.cbegin(); cIt != in_tokens.cend(); ++cIt)
	{
		const inf_text_token& cvTokenRef = *cIt;
		IBYTE tokenString[64] = {0};
		const llama_vocab* tmpVocab = llama_model_get_vocab(t2tModel->get_raw_model());
		I32 tokenLength = llama_token_to_piece(tmpVocab, cvTokenRef, tokenString, 64, false, true);
		if(t2tModel->is_token_control(cvTokenRef) == InfModelTextToText::flags::INF_MODEL_SUCCESS)
		{
			out_descriptions.push_back({mbase::string(tokenString, tokenLength), true});
			continue;
		}
		out_descriptions.push_back({mbase::string(tokenString, tokenLength), false});
	}

	return flags::INF_PROC_SUCCESS;
}

InfProcessorTextToText::flags InfProcessorTextToText::tokenize_input(CBYTEBUFFER in_data, size_type in_size, inf_text_token_vector& out_tokens)
{
	MBASE_INF_T2T_PROC_RETURN_UNREGISTERED;
	
	if(!in_size)
	{
		return flags::INF_PROC_ERR_INPUT_IS_EMPTY;
	}
	inf_text_token_vector tokenizedInput(in_size * 4);
	InfModelTextToText* t2tModel = static_cast<InfModelTextToText*>(this->mTargetModel_md_model);
	try
	{
		const llama_vocab* tmpVocab = llama_model_get_vocab(t2tModel->get_raw_model());
		I32 tokenCount = llama_tokenize(tmpVocab, in_data, static_cast<I32>(in_size), tokenizedInput.data(), static_cast<I32>(tokenizedInput.capacity()), false, true);
		if(tokenCount == -1)
		{
			return flags::INF_PROC_ERR_UNABLE_TO_TOKENIZE_INPUT;
		}

		tokenizedInput.resize_on_preset(tokenCount);
	}
	catch([[maybe_unused]] const std::exception& e)
	{
		return flags::INF_PROC_ERR_UNABLE_TO_TOKENIZE_INPUT;
	}
	
	out_tokens = std::move(tokenizedInput);
	return flags::INF_PROC_SUCCESS;
}

InfProcessorTextToText::flags InfProcessorTextToText::tokenize_input(context_line* in_lines, size_type in_count, inf_text_token_vector& out_tokens, bool in_append_assistant_token)
{
	MBASE_INF_T2T_PROC_RETURN_UNREGISTERED;
	
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
			roleString = t2tModel->get_sys_start();
			endString = t2tModel->get_sys_end();
		}

		else if(tmpLine->mRole == context_role::ASSISTANT)
		{
			roleString = t2tModel->get_assistant_start();
			endString = t2tModel->get_assistant_end();
		}

		else if(tmpLine->mRole == context_role::USER)
		{
			roleString = t2tModel->get_usr_start();
			endString = t2tModel->get_usr_end();
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
				totalMessage += t2tModel->get_assistant_start();
			}
		}
	}
	
	return tokenize_input(totalMessage.data(), totalMessage.size(), out_tokens);
}

InfProcessorTextToText::flags InfProcessorTextToText::execute_input(const inf_text_token_vector& in_tokens, bool in_kv_locked)
{
	MBASE_INF_T2T_PROC_RETURN_UNREGISTERED;

	if(!in_tokens.size())
	{
		return flags::INF_PROC_ERR_INPUT_IS_EMPTY;
	}

	if(is_manual_caching())
	{
		if(in_tokens.size() + get_cache_token_count() > mContextLength)
		{
			return flags::INF_PROC_ERR_INPUT_EXCEED_TOKEN_LIMIT;
		}
	}

	else
	{
		if(in_tokens.size() > mContextLength)
		{
			return flags::INF_PROC_ERR_INPUT_EXCEED_TOKEN_LIMIT;
		}
	}

	if (!is_running())
	{
		return flags::INF_PROC_INFO_HALTED;
	}

	if(!is_available())
	{
		return flags::INF_PROC_ERR_ALREADY_PROCESSING;
	}

	mTokenizedInput = in_tokens;
	if(in_kv_locked)
	{
		if(is_manual_caching() && get_manual_cache_mode() == cache_mode::KV_LOCK_MODE)
		{
			mInputKvLockedSignal.set_signal();
		}

		else
		{
			return flags::INF_PROC_ERR_OPERATION_NOT_SUPPORTED;
		}
	}

	else
	{
		mInputSignal.set_signal();
	}

	start_processor();
	release_synchronizer();
	return flags::INF_PROC_SUCCESS;
}

InfProcessorTextToText::flags InfProcessorTextToText::execute_input(inf_text_token* in_tokens, size_type in_size, bool in_kv_locked)
{
	inf_text_token_vector tokenVector;
	for(size_type i = 0; i < in_size; i++)
	{
		tokenVector.push_back(in_tokens[i]);
	}

	return execute_input(tokenVector, in_kv_locked);
}

InfProcessorTextToText::flags InfProcessorTextToText::execute_input_sync(const inf_text_token_vector& in_tokens, bool in_kv_locked)
{
	MBASE_INF_T2T_PROC_RETURN_UNREGISTERED;
	flags outResult = execute_input(in_tokens, in_kv_locked);
	if(outResult == flags::INF_PROC_SUCCESS || outResult == flags::INF_PROC_ERR_ALREADY_PROCESSING)
	{
		if(in_kv_locked)
		{
			while(signal_kv_locked_process())
			{
				mbase::sleep(2);
			}
		}
		else
		{
			while(signal_input_process())
			{
				mbase::sleep(2);
			}
		}
		return flags::INF_PROC_INFO_NEED_UPDATE;
	}

	return outResult;
}

InfProcessorTextToText::flags InfProcessorTextToText::next(const decode_behavior_description& in_description)
{
	MBASE_INF_T2T_PROC_RETURN_UNREGISTERED;

	if(signal_decode_process())
	{
		// If it is already processing, ignore the request
		return flags::INF_PROC_SUCCESS;
	}

	if(signal_state_decode_process())
	{
		return flags::INF_PROC_INFO_NEED_UPDATE;
	}

	if(!is_running())
	{
		return flags::INF_PROC_INFO_HALTED;
	}

	mGeneratedTokenVector.clear();
	mDecodeBehavior = in_description;
	mDecodeSignal.set_signal();

	start_processor();
	release_synchronizer();
	return flags::INF_PROC_SUCCESS;
}

InfProcessorTextToText::flags InfProcessorTextToText::next_sync(const decode_behavior_description& in_description)
{
	MBASE_INF_T2T_PROC_RETURN_UNREGISTERED;

	flags nextResult = next(in_description);
	if(nextResult == flags::INF_PROC_SUCCESS)
	{
		while(signal_decode_process()){ mbase::sleep(2); }
		return flags::INF_PROC_INFO_NEED_UPDATE;
	}

	return nextResult;
}

InfProcessorTextToText::flags InfProcessorTextToText::clear_response()
{
	MBASE_INF_T2T_PROC_RETURN_UNREGISTERED;
	if(!mLogitStartIndex)
	{
		return flags::INF_PROC_SUCCESS;
	}

	llama_kv_self_seq_rm(mModelContext, 0, mLogitStartIndex - 1, -1);
	llama_kv_self_update(mModelContext);
	return flags::INF_PROC_SUCCESS;
}

InfProcessorTextToText::flags InfProcessorTextToText::set_inference_client(InfClientBase* in_client)
{
	MBASE_INF_T2T_PROC_RETURN_UNREGISTERED;

	if(get_assigned_client())
	{
		return flags::INF_PROC_ERR_ALREADY_PROCESSING;
	}

	if(!in_client)
	{
		return flags::INF_PROC_ERR_MISSING_CLIENT;
	}
	
	if(mAssignedClient == in_client)
	{
		return flags::INF_PROC_SUCCESS;
	}

	release_inference_client();

	mAssignedClient = in_client;
	mAssignedClient->on_register(this);

	return flags::INF_PROC_SUCCESS;
}

InfProcessorTextToText::flags InfProcessorTextToText::declare_lora_assign(const inf_lora_adapter& in_adapter)
{
	MBASE_INF_T2T_PROC_RETURN_UNREGISTERED;
	return flags::INF_PROC_SUCCESS;
}

InfProcessorTextToText::flags InfProcessorTextToText::declare_lora_remove(const inf_lora_adapter& in_adapter)
{
	MBASE_INF_T2T_PROC_RETURN_UNREGISTERED;
	return flags::INF_PROC_SUCCESS;
}


InfProcessorTextToText::flags InfProcessorTextToText::initialize(
	InfModelTextToText* in_model, 
	const U32& in_context_length, 
	const mbase::string& in_context_id,
	const U32& in_batch_size,
	const U32& in_thread_count,
	const U32& in_batch_thread_count,
	const bool& in_flash_attention,
	const inf_sampling_set& in_sampler_set
)
{
	if (signal_initializing())
	{
		return flags::INF_PROC_INFO_INITIALIZING;
	}

	if (signal_destroying())
	{
		return flags::INF_PROC_INFO_DESTROYING;
	}

	if(is_registered())
	{
		return flags::INF_PROC_ERR_ALREADY_INITIALIZED;
	}

	mTargetModel_md_model = in_model;
	mContextLength = in_context_length;
	mContextIdentifier = in_context_id;
	mBatchSize = in_batch_size;
	mThreadCount = in_thread_count;
	mBatchProcessThreadCount = in_batch_thread_count;
	mSamplerDescriptions = in_sampler_set;
	mFlashAttention = in_flash_attention;

	mInitializeSignal.set_signal();
	start_processor();
	release_synchronizer();
	return flags::INF_PROC_INFO_INITIALIZING;
}

InfProcessorTextToText::flags InfProcessorTextToText::initialize_sync(
	InfModelTextToText* in_model, 
	const U32& in_context_length, 
	const mbase::string& in_context_id,
	const U32& in_batch_size,
	const U32& in_thread_count,
	const U32& in_batch_thread_count,
	const bool& in_flash_attention,
	const inf_sampling_set& in_sampler_set
)
{
	initialize(
		in_model, 
		in_context_length, 
		in_context_id,
		in_batch_size,
		in_thread_count,
		in_batch_thread_count,
		in_flash_attention,
		in_sampler_set
	);
	while(signal_initializing())
	{
		mbase::sleep(2);
	}
	
	return flags::INF_PROC_INFO_NEED_UPDATE;
}

InfProcessorTextToText::flags InfProcessorTextToText::destroy()
{
	if(!is_registered())
	{
		return flags::INF_PROC_SUCCESS;
	}

	if(signal_destroying())
	{
		return flags::INF_PROC_INFO_DESTROYING;
	}

	release_inference_client();

	mDestroySignal.set_signal();
	
	start_processor();
	release_synchronizer();
	return flags::INF_PROC_INFO_DESTROYING;
}

InfProcessorTextToText::flags InfProcessorTextToText::destroy_sync()
{
	destroy();
	while(signal_destroying())
	{
		mbase::sleep(2);
	}

	return flags::INF_PROC_INFO_NEED_UPDATE;
}

GENERIC InfProcessorTextToText::set_benchmark(bool in_is_on)
{
	mIsBenchmarkOn = in_is_on;
}

GENERIC InfProcessorTextToText::clear_token_candidates()
{
	mPresetCandidates.clear();
}

GENERIC InfProcessorTextToText::clear_samplers()
{	
	if(mSamplerChain)
	{
		llama_sampler_free(mSamplerChain);
		mSamplerDescriptions.clear();
		mSamplerChain = NULL;
	}
}

GENERIC InfProcessorTextToText::clear_kv_cache()
{
	mLogitStartIndex = 0;
	mLogitTokenVector.clear();
	llama_kv_self_clear(mModelContext);
}

GENERIC InfProcessorTextToText::set_manual_caching(bool in_manual_cache, cache_mode in_cache_mode)
{
	mIsManualCaching = in_manual_cache;
	mCacheMode = in_cache_mode;
}

GENERIC InfProcessorTextToText::on_lora_operate(const mbase::vector<inf_lora_adapter>& out_adapters)
{
	
}

GENERIC InfProcessorTextToText::on_initializing()
{
	
}

GENERIC InfProcessorTextToText::on_initialize_fail([[maybe_unused]] last_fail_code out_code)
{

}

GENERIC InfProcessorTextToText::on_destroying()
{

}

GENERIC InfProcessorTextToText::_decode_cached_logits()
{
	if(!mLogitStartIndex)
	{
		return;
	}

	if(get_manual_cache_mode() == cache_mode::KV_LOCK_MODE)
	{
		llama_kv_self_seq_rm(mModelContext, 0, mPromptStartIndex, -1);
		return;
	}
	else
	{
		llama_kv_self_seq_rm(mModelContext, 0, mLogitStartIndex - 1, -1);
	}
	
	llama_kv_self_update(mModelContext);
	
	llama_batch tempBatch = llama_batch_init(mBatchSize, 0, 1);
	U32 tmpBatchCursor = 0;
	for(size_type i = 0; i < mLogitTokenVector.size(); i++)
	{
		++tmpBatchCursor;
		inf_common_batch_add(tempBatch, mLogitTokenVector[i], mLogitStartIndex, {0}, false);
		if(tmpBatchCursor == mBatchSize)
		{
			llama_decode(mModelContext, tempBatch);
			tempBatch.n_tokens = 0;
			tmpBatchCursor = 0;
		}
		++mLogitStartIndex;
	}

	if(tmpBatchCursor)
	{
		llama_decode(mModelContext, tempBatch);
	}
	
	llama_batch_free(tempBatch);

	mLogitStartIndex = 0;
	mLogitTokenVector.clear();
}

GENERIC InfProcessorTextToText::_decode_kv_locked_input()
{
	_decode_cached_logits();
	I32 totalPosition = get_cache_token_count();
	U32 tmpBatchCursor = 0;
	mProcessedBatchLength = 0;
	llama_batch tempBatch = llama_batch_init(mBatchSize, 0, 1);
	for(size_type i = 0; i < mTokenizedInput.size(); i++)
	{
		++mProcessedBatchLength;
		++tmpBatchCursor;
		inf_common_batch_add(tempBatch, mTokenizedInput[i], totalPosition, {0}, false);
		if(tmpBatchCursor == mBatchSize)
		{
			llama_decode(mModelContext, tempBatch);
			tempBatch.n_tokens = 0;
			tmpBatchCursor = 0;
		}
		++totalPosition;
	}

	if(tmpBatchCursor)
	{
		llama_decode(mModelContext, tempBatch);
	}
	
	mPromptStartIndex = get_cache_token_count();
	llama_batch_free(tempBatch);
	mInputKvLockedSignal.set_signal_finished();
}

GENERIC InfProcessorTextToText::_decode_input()
{
	I32 totalPosition = get_cache_token_count();
	if(!is_manual_caching())
	{
		clear_kv_cache();
		totalPosition = 0;
	}
	else
	{
		_decode_cached_logits();
	}
	totalPosition = get_cache_token_count();
	llama_sampler_reset(mSamplerChain);
	std::chrono::high_resolution_clock::time_point beginTime;
	std::chrono::high_resolution_clock::time_point endTime;
	mProcessedBatchLength = 0;
	U32 tmpBatchCursor = 0;
	I64 msPassed = 0;
	llama_batch tempBatch = llama_batch_init(mBatchSize, 0, 1);
	for(size_type i = 0; i < mTokenizedInput.size() - 1; i++)
	{
		++tmpBatchCursor;
		inf_common_batch_add(tempBatch, mTokenizedInput[i], totalPosition, {0}, false);
		if(tmpBatchCursor == mBatchSize)
		{
			beginTime = std::chrono::high_resolution_clock::now();
			llama_decode(mModelContext, tempBatch);
			endTime = std::chrono::high_resolution_clock::now();
			msPassed += std::chrono::duration_cast<std::chrono::milliseconds>(endTime - beginTime).count();
			tempBatch.n_tokens = 0;
			tmpBatchCursor = 0;
		}
		++totalPosition;
	}

	inf_common_batch_add(tempBatch, mTokenizedInput.back(), totalPosition, {0}, true);
	beginTime = std::chrono::high_resolution_clock::now();
	llama_decode(mModelContext, tempBatch);
	mContextCursor = get_cache_token_count();
	mProcessedBatchLength = mContextCursor;
	mLogitTokenVector.clear();
	mLogitStartIndex = mContextCursor - 1;
	if(is_manual_caching())
	{
		mLogitTokenVector.push_back(mTokenizedInput.back());
	}

	endTime = std::chrono::high_resolution_clock::now();
	msPassed += std::chrono::duration_cast<std::chrono::milliseconds>(endTime - beginTime).count();
	
	F32 secondsPassed = (F32)msPassed / 1000.0f;
	mDiagnostics.ppTokensPerSecond = mContextCursor / secondsPassed;
	llama_batch_free(tempBatch);
	mInputSignal.set_signal_finished();
	mFinishState = finish_state::CONTINUE;
}

GENERIC InfProcessorTextToText::_decode_next()
{
	// Main Decode loop
	I64 totalMilliseconds = 1;
	I64 totalGeneratedTokens = 0;
	llama_batch tempBatch = llama_batch_init(mDecodeBehavior.mTokenAtMost, 0, 1);
	
	for(U32 i = 0; i < mDecodeBehavior.mTokenAtMost; i++)
	{
		std::chrono::high_resolution_clock::time_point beginTime = std::chrono::high_resolution_clock::now();
		InfModelTextToText* t2tModel = static_cast<InfModelTextToText*>(this->mTargetModel_md_model);
		const llama_vocab* tmpVocab = llama_model_get_vocab(t2tModel->get_raw_model());
		
		inf_text_token tmpGeneratedToken = llama_sampler_sample(mSamplerChain, mModelContext, -1);
		if(is_benchmark())
		{
			tmpGeneratedToken = llama_vocab_n_tokens(tmpVocab) / 2; // the token selection is arbitrary. it literally has no meaning
		}
		else
		{	
			tmpGeneratedToken = llama_sampler_sample(mSamplerChain, mModelContext, -1);
		}
		
		//llama_sampler_accept(mSamplerChain, tmpGeneratedToken);
		mGeneratedTokenVector.push_back(tmpGeneratedToken);
		if(is_manual_caching())
		{
			if(get_manual_cache_mode() == cache_mode::AUTO_LOGIT_STORE_MODE)
			{
				mLogitTokenVector.push_back(tmpGeneratedToken);
			}
		}
		
		clear_token_candidates();
		if (llama_vocab_is_eog(tmpVocab, tmpGeneratedToken))
		{
			// means end of generation
			llama_sampler_reset(mSamplerChain);
			if(is_manual_caching())
			{
				_decode_cached_logits();
			}
			//llama_kv_cache_clear(mModelContext);
			mFinishState = finish_state::FINISHED;
			break;
		}
		
		else
		{
			if (mContextCursor == mContextLength)
			{
				// means token limit is reached
				llama_sampler_reset(mSamplerChain);
				if(is_manual_caching())
				{
					_decode_cached_logits();
				}
				//llama_kv_cache_clear(mModelContext);
				mFinishState = finish_state::TOKEN_LIMIT_REACHED;
				break;
			}

			else
			{
				tempBatch.n_tokens = 0;
				inf_common_batch_add(tempBatch, tmpGeneratedToken, mContextCursor++, {0}, true);
				llama_decode(mModelContext, tempBatch); // Handle error here
				totalGeneratedTokens++;
				std::chrono::high_resolution_clock::time_point endTime = std::chrono::high_resolution_clock::now();
				totalMilliseconds += std::chrono::duration_cast<std::chrono::milliseconds>(endTime - beginTime).count();
			}
		}
	}
	llama_batch_free(tempBatch);
	if(totalGeneratedTokens)
	{
		F32 secondsPassed = (F32)totalMilliseconds / 1000.0f;
		mDiagnostics.evalTokensPerSecond = totalGeneratedTokens / secondsPassed;
	}

	mDecodeSignal.set_signal_finished();
}

GENERIC InfProcessorTextToText::_lora_operate()
{
	for(mbase::vector<inf_lora_adapter>::iterator It = mDeclaredAdapters.begin(); It != mDeclaredAdapters.end(); ++It)
	{
		if(!llama_set_adapter_lora(mModelContext, It->mAdapterHandle, It->mLoraScale))
		{
			// means success
			mAssignedAdapters.push_back(*It);
		}
	}

	mDeclaredAdapters.clear();

	mbase::vector<inf_lora_adapter> newAssignedAdapters;

	for(mbase::vector<inf_lora_adapter>::iterator It = mAssignedAdapters.begin(); It != mAssignedAdapters.end(); ++It)
	{
		if(mbase::find(mRemoveAdapters.begin(), mRemoveAdapters.end(), *It) != mRemoveAdapters.end())
		{
			llama_rm_adapter_lora(mModelContext, It->mAdapterHandle);
		}

		else
		{
			newAssignedAdapters.push_back(*It);
		}
	}

	mRemoveAdapters.clear();
	mAssignedAdapters = newAssignedAdapters;

	mLoraOperationSignal.set_signal_finished();
}

GENERIC InfProcessorTextToText::_internal_adapter_remove(mbase::vector<inf_lora_adapter>& in_adapters_to_remove)
{
	mbase::vector<inf_lora_adapter> newAssignedAdapters;
	for(mbase::vector<inf_lora_adapter>::iterator It = mAssignedAdapters.begin(); It != mAssignedAdapters.end(); ++It)
	{
		if(mbase::find(in_adapters_to_remove.begin(), in_adapters_to_remove.end(), *It) != in_adapters_to_remove.end())
		{
			llama_rm_adapter_lora(mModelContext, It->mAdapterHandle);
		}

		else
		{
			newAssignedAdapters.push_back(*It);
		}
	}
	mAssignedAdapters = newAssignedAdapters;
}

GENERIC InfProcessorTextToText::_initialize_context()
{
	llama_context_params ctxParams = llama_context_default_params();
	ctxParams.n_ctx = mContextLength;
	ctxParams.n_batch = mBatchSize;
	ctxParams.n_seq_max = 1;
	ctxParams.n_threads = mThreadCount;
	ctxParams.n_threads_batch = mBatchProcessThreadCount;
	ctxParams.n_ubatch = mBatchSize / 4;
	ctxParams.flash_attn = mFlashAttention;

	InfModelTextToText* t2tModel = static_cast<InfModelTextToText*>(this->mTargetModel_md_model);

	if(!t2tModel || !t2tModel->is_initialized())
	{
		clear_samplers();
		mLastFailCode = last_fail_code::MODEL_NOT_INITIALIZED;
		mIsInitializeFailed = true;
		mInitializeSignal.set_signal_finished();
		return;
	}

	std::chrono::high_resolution_clock::time_point beginTime = std::chrono::high_resolution_clock::now();
	
	mModelContext = llama_init_from_model(t2tModel->get_raw_model(), ctxParams);
	if (!mModelContext)
	{
		clear_samplers();
		mLastFailCode = last_fail_code::NOT_ENOUGH_MEMORY;
		mIsInitializeFailed = true;
		mInitializeSignal.set_signal_finished();
		return;
	}
	
	std::chrono::high_resolution_clock::time_point endTime = std::chrono::high_resolution_clock::now();
	
	mDiagnostics.loadTimeInMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - beginTime).count();

	mContextCursor = 0;
	srand(time(NULL));
	U32 seedValue = static_cast<U32>(rand()); // Default random seed if no rng is specified.

	mSamplerChain = llama_sampler_chain_init(llama_sampler_chain_default_params());

	if(!mSamplerDescriptions.size())
	{
		// If no sampler is present, apply greedy
		llama_sampler_chain_add(mSamplerChain, llama_sampler_init_greedy());
	}

	else
	{
		for(inf_sampling_set::iterator It = mSamplerDescriptions.begin(); It != mSamplerDescriptions.end(); ++It)
		{
			if(It->mSamplerType == InfSamplerDescription::SAMPLER::RNG)
			{
				seedValue = It->mRng;
			}

			if(It->mSamplerType == InfSamplerDescription::SAMPLER::REPETITION)
			{
				InfSamplingRepetition repeatSampler = It->mRepetition;
				llama_sampler_chain_add(
					mSamplerChain, 
					llama_sampler_init_penalties(
						repeatSampler.mPenaltyN, 
						repeatSampler.mRepeatPenalty, 
						repeatSampler.mPenaltyFrequency, 
						repeatSampler.mPenaltyPresent
					)
				);
			}

			else if(It->mSamplerType == InfSamplerDescription::SAMPLER::TOP_K)
			{
				U32 kValue = It->mTopK;
				llama_sampler_chain_add(
					mSamplerChain,
					llama_sampler_init_top_k(kValue)
				);
			}

			else if(It->mSamplerType == InfSamplerDescription::SAMPLER::TOP_P)
			{
				F32 topValue = It->mTopP;
				llama_sampler_chain_add(
					mSamplerChain,
					llama_sampler_init_top_p(topValue, 1)
				);
			}

			else if(It->mSamplerType == InfSamplerDescription::SAMPLER::MIN_P)
			{
				F32 minValue = It->mMinP;
				llama_sampler_chain_add(
					mSamplerChain,
					llama_sampler_init_min_p(minValue, 1)
				);
			}

			else if(It->mSamplerType == InfSamplerDescription::SAMPLER::TYPICAL_P)
			{
				F32 typicalValue = It->mTypicalP;
				llama_sampler_chain_add(
					mSamplerChain,
					llama_sampler_init_typical(typicalValue, 1)
				);
			}

			else if(It->mSamplerType == InfSamplerDescription::SAMPLER::TEMP)
			{
				F32 temperature = It->mTemp;
				llama_sampler_chain_add(
					mSamplerChain,
					llama_sampler_init_temp(temperature)
				);
			}
			else if(It->mSamplerType == InfSamplerDescription::SAMPLER::MIROSTAT_V2)
			{
				InfSamplingMirostatV2 mirostatObject = It->mMiroV2;
				llama_sampler_chain_add(
					mSamplerChain,
					llama_sampler_init_mirostat_v2(
						seedValue,
						mirostatObject.mTau,
						mirostatObject.mEta
					)	
				);
			}
		}

		llama_sampler_chain_add(
			mSamplerChain,
			llama_sampler_init_dist(seedValue)
		);
	}

	mInitializeSignal.set_signal_finished();
}

GENERIC InfProcessorTextToText::_destroy_context()
{
	// CONTEXT FACTORY RESET

	llama_clear_adapter_lora(mModelContext); // if any
	llama_free(mModelContext);
	mModelContext = NULL;
	mPresetCandidates.clear();
	mTokenizedInput.clear();
	mSamplerDescriptions.clear();
	mGeneratedTokenVector.clear();
	mLogitTokenVector.clear();
	mDeclaredAdapters.clear();
	mRemoveAdapters.clear();
	mAssignedAdapters.clear();
	mContextCursor = 0;
	mBatchSize = 0;
	mThreadCount = 0;
	mProcessedBatchLength = 0;
	mLogitStartIndex = 0;
	mPromptStartIndex = 0;
	mFlashAttention = false;
	mIsRunning = false;
	mIsInitializeFailed = false;
	mIsManualCaching = false;
	mIsBenchmarkOn = false;
	mFinishState = finish_state::FINISHED;
	mAssignedClient = NULL;
	mCacheMode = cache_mode::AUTO_LOGIT_STORE_MODE;
	mLastFailCode = last_fail_code::MODEL_NOT_INITIALIZED;

	clear_samplers();
	
	mTargetModel_md_model = NULL;
	mIsRegistered = false;
	mDestroySignal.set_signal_finished();
}

GENERIC InfProcessorTextToText::update()
{
	if(signal_initializing())
	{
		on_initializing();
		return;
	}

	if(signal_destroying())
	{
		on_destroying();
		return;
	}

	if(signal_input_process() || signal_kv_locked_process())
	{
		InfClientTextToText* t2tClient = static_cast<InfClientTextToText*>(get_assigned_client());
		if(t2tClient)
		{
			t2tClient->on_batch_processing(this);
		}
		return;
	}

	if(signal_decode_process())
	{
		InfClientTextToText* t2tClient = static_cast<InfClientTextToText*>(get_assigned_client());
		if(t2tClient)
		{
			t2tClient->on_decoding(this);
		}
		return;
	}

	if(signal_state_destroying())
	{
		// Reset all signals on destruction
		reset_base_signals();
		mDecodeSignal.reset_signal_with_state();
		mInputSignal.reset_signal_with_state();
		mInputKvLockedSignal.reset_signal_with_state();
		mLoraOperationSignal.reset_signal_with_state();
		this->release_object_watcher();
		stop_processor();
		on_destroy();
		return;
	}

	if(signal_state_initializing())
	{
		acquire_synchronizer();
		mInitializeSignal.reset_signal_state();
		if(is_init_failed())
		{
			this->release_object_watcher();
			on_initialize_fail(get_last_fail_code());
		}
		else
		{
			mIsRunning = true;
			mIsRegistered = true;
			on_initialize();
		}
		return;
	}

	if(signal_state_lora_operate())
	{
		acquire_synchronizer();
		mLoraOperationSignal.reset_signal_state();
		on_lora_operate(mAssignedAdapters);
		return;
	}

	if(signal_state_input_process())
	{
		acquire_synchronizer();
		mInputSignal.reset_signal_state();
		InfClientTextToText* t2tClient = static_cast<InfClientTextToText*>(get_assigned_client());
		if(t2tClient)
		{
			t2tClient->on_batch_processed(this, mProcessedBatchLength, false);
		}
		return;
	}

	if(signal_state_kv_locked_process())
	{
		acquire_synchronizer();
		mInputKvLockedSignal.reset_signal_state();
		InfClientTextToText* t2tClient = static_cast<InfClientTextToText*>(get_assigned_client());
		if(t2tClient)
		{
			t2tClient->on_batch_processed(this, mProcessedBatchLength, true);
		}
		return;
	}

	if(signal_state_decode_process())
	{
		acquire_synchronizer();
		mDecodeSignal.reset_signal_state();		
		bool isFinish = false;

		if(mFinishState != finish_state::CONTINUE)
		{
			isFinish = true;
		}

		if(isFinish)
		{
			mInputSignal.reset_signal_with_state();
		}

		InfClientTextToText* t2tClient = static_cast<InfClientTextToText*>(get_assigned_client());

		if(t2tClient)
		{
			inf_text_token_vector tokenVector = mGeneratedTokenVector;
			t2tClient->on_write(this, tokenVector, isFinish);
			if(isFinish)
			{
				t2tClient->on_finish(this, mContextCursor, mFinishState);
			}
		}
	}
}

GENERIC InfProcessorTextToText::update_t()
{
	while(is_processor_running())
	{
		release_synchronizer();

		if(is_registered())
		{
			if (signal_destroying())
			{
				_destroy_context();
			}

			if (is_running())
			{
				if(signal_lora_operate_process())
				{
					_lora_operate();
				}

				if(signal_kv_locked_process())
				{
					_decode_kv_locked_input();
				}

				if (signal_input_process())
				{
					_decode_input();
				}
				
				if (signal_decode_process())
				{
					_decode_next();
				}
			}
		}
		else
		{
			if(signal_initializing())
			{
				_initialize_context();
			}
		}
		acquire_synchronizer();
	}
	release_synchronizer();
}

MBASE_END
