#include <mbase/inference/inf_processor.h>
#include <mbase/inference/inf_client.h>
#include <mbase/inference/inf_model.h>
#include <mbase/inference/inf_sampling.h>
#include <mbase/framework/timer_loop.h>
#include <mbase/framework/timers.h>
#include <common/common.h>

MBASE_BEGIN

#define MBASE_INF_PROC_RETURN_UNREGISTERED \
if(!this->is_registered())\
{\
	return flags::INF_PROC_ERR_UNREGISTERED_PROCESSOR;\
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

InfInactiveClientCleaner::InfInactiveClientCleaner(InfProcessor & in_host) : mHostProcessor(&in_host)
{

}

GENERIC InfInactiveClientCleaner::on_call(user_data in_data)
{
	if(mHostProcessor)
	{
		mHostProcessor->update_inactivity();
	}
}

InfProcessor::InfProcessor() :
	mProcessedHandlers(),
	mRegisteredClients(),
	mProcessedModel(NULL),
	mPresetCandidates(),
	mModelContext(NULL),
	mProcessorId(0),
	mContextIdCounter(0),
	mClientsMutex(),
	mMaxClients(gInfProcessorDefaultMaxSeq),
	mRegisteredBatchSize(0),
	mInactivityThreshold(gProcessorMinimumInactivityThreshold),
	mIsRunning(false),
	mClientCleaner(*this)
{
	mClientCleaner.set_target_time(1000);
	mClientCleaner.set_tick_limit(100000);
}

InfProcessor::~InfProcessor()
{
	this->destroy();
}

typename InfProcessor::iterator InfProcessor::begin() noexcept
{
	return mRegisteredClients.begin();
}

typename InfProcessor::iterator InfProcessor::end() noexcept
{
	return mRegisteredClients.end();
}

typename InfProcessor::const_iterator InfProcessor::begin() const noexcept
{
	return mRegisteredClients.begin();
}

typename InfProcessor::const_iterator InfProcessor::end() const noexcept
{
	return mRegisteredClients.end();
}

typename InfProcessor::const_iterator InfProcessor::cbegin() const noexcept
{
	return mRegisteredClients.cbegin();
}

typename InfProcessor::const_iterator InfProcessor::cend() const noexcept
{
	return mRegisteredClients.cend();
}

typename InfProcessor::reverse_iterator InfProcessor::rbegin() noexcept
{
	return mRegisteredClients.rbegin();
}

typename InfProcessor::reverse_iterator InfProcessor::rend() noexcept
{
	return mRegisteredClients.rend();
}

typename InfProcessor::const_reverse_iterator InfProcessor::crbegin() const noexcept
{
	return mRegisteredClients.crbegin();
}

typename InfProcessor::const_reverse_iterator InfProcessor::crend() const noexcept
{
	return mRegisteredClients.crend();
}

bool InfProcessor::is_registered() const
{
	return (mModelContext != NULL);
}

bool InfProcessor::is_running() const
{
	return mIsRunning;
}

InfProcessor::flags InfProcessor::get_context_size(U32& out_size)
{
	MBASE_INF_PROC_RETURN_UNREGISTERED;
	out_size = llama_n_ctx(mModelContext);
	return flags::INF_PROC_SUCCESS;
}

InfProcessor::flags InfProcessor::get_client_count(U32& out_size)
{
	MBASE_INF_PROC_RETURN_UNREGISTERED;
	out_size = mRegisteredClients.size();
	return flags::INF_PROC_SUCCESS;
}

InfProcessor::flags InfProcessor::get_max_clients(U32& out_size) 
{
	MBASE_INF_PROC_RETURN_UNREGISTERED;
	out_size = mMaxClients;
	return flags::INF_PROC_SUCCESS;
}

InfProcessor::flags InfProcessor::get_process_thread_count(U32& out_count)
{
	MBASE_INF_PROC_RETURN_UNREGISTERED;
	out_count = llama_n_threads(mModelContext);
	return flags::INF_PROC_SUCCESS;
}

InfProcessor::flags InfProcessor::get_max_batch_size(U32& out_size)
{
	MBASE_INF_PROC_RETURN_UNREGISTERED;
	out_size = llama_n_batch(mModelContext);
	return flags::INF_PROC_SUCCESS;
}

InfModel* InfProcessor::get_processed_model()
{
	return mProcessedModel;
}

U32 InfProcessor::get_inactivity_threshold()
{
	return mInactivityThreshold;
}

InfInactiveClientCleaner* InfProcessor::get_client_cleaner()
{
	return &mClientCleaner;
}

InfProcessor::flags InfProcessor::destroy()
{
	MBASE_INF_PROC_RETURN_UNREGISTERED;
	mClientsMutex.acquire();
	for (client_list::iterator It = mRegisteredClients.begin(); It != mRegisteredClients.end(); ++It)
	{
		InfClient* myClient = *It;
		unregister_client(*myClient);
	}
	resume();
	update_t();
	mClientsMutex.release();
	update();

	// CLEANING PROCESSOR
	mProcessedHandlers.clear();
	mPresetCandidates.clear();
	llama_free(mModelContext);
	mModelContext = NULL;
	mProcessorId = 0;
	mContextIdCounter = 0;
	mMaxClients = gInfProcessorDefaultMaxSeq;
	mRegisteredBatchSize = 0;
	InfModel::iterator tmpIt;
	mProcessedModel->_unregister_processor(*this, tmpIt);
	mProcessedModel = NULL;
	mIsRunning = false;
	// CLEANING PROCESSOR

	return flags::INF_PROC_SUCCESS;
}

InfProcessor::flags InfProcessor::_destroy(InfModel::iterator& _out_it)
{
	MBASE_INF_PROC_RETURN_UNREGISTERED;
	mClientsMutex.acquire();
	for(client_list::iterator It = mRegisteredClients.begin(); It != mRegisteredClients.end(); ++It)
	{
		InfClient* myClient = *It;
		unregister_client(*myClient);
	}
	resume();
	update_t();
	mClientsMutex.release();
	update();

	// CLEANING PROCESSOR
	mProcessedHandlers.clear();
	mPresetCandidates.clear();
	llama_free(mModelContext);
	mModelContext = NULL;
	mProcessorId = 0;
	mContextIdCounter = 0;
	mMaxClients = gInfProcessorDefaultMaxSeq;
	mRegisteredBatchSize = 0;
	InfModel::iterator tmpIt;
	mProcessedModel->_unregister_processor(*this, _out_it);
	mProcessedModel = NULL;
	mIsRunning = false;
	// CLEANING PROCESSOR

	return flags::INF_PROC_SUCCESS;
}

InfProcessor::flags InfProcessor::tokenize_input(CBYTEBUFFER in_data, size_type in_size, mbase::vector<inf_token>& out_tokens)
{
	MBASE_INF_PROC_RETURN_UNREGISTERED;

	if(!in_size)
	{
		return flags::INF_PROC_ERR_INVALID_TOKEN_SIZE;
	}

	mbase::vector<inf_token> tokenizedInput(in_size * 3);
	I32 tokenCount = llama_tokenize(get_processed_model()->get_raw_model(), in_data, in_size, tokenizedInput.data(), in_size * 3, false, true);
	if(tokenCount == -1)
	{
		return flags::INF_PROC_ERR_INVALID_TOKEN_INPUT;
	}

	tokenizedInput.resize(tokenCount);
	out_tokens = std::move(tokenizedInput);
	return flags::INF_PROC_SUCCESS;
}

InfProcessor::flags InfProcessor::register_client(const mbase::vector<inf_token>& in_data, InfClient& out_client, U32 in_token_limit)
{
	MBASE_INF_PROC_RETURN_HALTED;

	if (out_client.is_registered())
	{
		return flags::INF_PROC_ERR_CLIENT_ALREADY_REGISTERED;
	}

	if (mRegisteredClients.size() == mMaxClients)
	{
		return flags::INF_PROC_ERR_CLIENT_LIMIT_REACHED;
	}

	if (in_token_limit < gProcessorMinimumTokenCount) // minimum token count
	{
		return flags::INF_PROC_ERR_TOKEN_LIMIT_IS_TOO_LOW;
	}

	U32 contextBatchSize = 0;
	U32 totalContextSize = 0;
	get_max_batch_size(contextBatchSize);
	get_context_size(totalContextSize);

	if(in_token_limit > contextBatchSize || in_data.size() > in_token_limit)
	{
		return flags::INF_PROC_ERR_EXCEED_TOKEN_LIMIT;
	}

	if(mRegisteredBatchSize + in_token_limit > totalContextSize)
	{
		return flags::INF_PROC_ERR_CONTEXT_IS_FULL;
	}

	if (in_data.size())
	{
		out_client.mParsedTokens = in_data;
		out_client.mIsDataSet = true;
	}

	out_client.mfrBatchCursor = 0;
	out_client.mfrHostProcessor = this;
	out_client.mSequenceId = ++this->mContextIdCounter;
	out_client.mInactivityCounter = 0;
	out_client.mfrMaxTokenCount = in_token_limit;
	out_client.mFs = InfClient::finish_state::INF_FINISH_STATE_CONTINUE;
	out_client.mfrBatch = llama_batch_init(in_token_limit, 0, 1);

	mRegisteredBatchSize += out_client.mfrMaxTokenCount;
	mRegisteredClients.push_back(&out_client);
	out_client.mfrSelfIter = mRegisteredClients.end_node();

	out_client.on_register();
	return flags::INF_PROC_SUCCESS;
}

InfProcessor::flags InfProcessor::register_client(CBYTEBUFFER in_data, size_type in_size, InfClient& out_client, U32 in_token_limit)
{
	mbase::vector<inf_token> parsedTokens;
	flags tokenizeResult = tokenize_input(in_data, in_size, parsedTokens);

	if(tokenizeResult != flags::INF_PROC_SUCCESS)
	{
		return tokenizeResult;
	}
	
	return register_client(std::move(parsedTokens), out_client, in_token_limit);
}

InfProcessor::flags InfProcessor::register_client(const mbase::string& in_string, InfClient& out_client, U32 in_token_limit)
{
	return register_client(in_string.c_str(), in_string.size(), out_client, in_token_limit);
}

InfProcessor::flags InfProcessor::register_client(InfClient& out_client, U32 in_token_limit)
{
	MBASE_INF_PROC_RETURN_HALTED;

	if (out_client.is_registered())
	{
		return flags::INF_PROC_ERR_CLIENT_ALREADY_REGISTERED;
	}

	if (mRegisteredClients.size() == mMaxClients)
	{
		return flags::INF_PROC_ERR_CLIENT_LIMIT_REACHED;
	}

	U32 contextBatchSize = 0;
	U32 totalContextSize = 0;
	get_max_batch_size(contextBatchSize);
	get_context_size(totalContextSize);

	if(in_token_limit > contextBatchSize)
	{
		return flags::INF_PROC_ERR_EXCEED_TOKEN_LIMIT;
	}

	if(mRegisteredBatchSize + in_token_limit > totalContextSize)
	{
		return flags::INF_PROC_ERR_CONTEXT_IS_FULL;
	}

	out_client.mfrBatchCursor = 0;
	out_client.mfrHostProcessor = this;
	out_client.mSequenceId = ++this->mContextIdCounter;
	out_client.mInactivityCounter = 0;
	out_client.mfrMaxTokenCount = in_token_limit;
	out_client.mFs = InfClient::finish_state::INF_FINISH_STATE_CONTINUE;
	out_client.mfrBatch = llama_batch_init(in_token_limit, 0, 1);
	
	mRegisteredBatchSize += out_client.mfrMaxTokenCount;
	mRegisteredClients.push_back(&out_client);
	out_client.mfrSelfIter = mRegisteredClients.end_node();

	out_client.on_register();
	return flags::INF_PROC_SUCCESS;
}

InfProcessor::flags InfProcessor::unregister_client(InfClient& in_client)
{
	MBASE_INF_PROC_RETURN_UNREGISTERED;

	if (!in_client.is_registered() || in_client.is_unregistering())
	{
		return flags::INF_PROC_SUCCESS;
	}

	if(in_client.mfrHostProcessor != this)
	{
		return flags::INF_PROC_ERR_BELONGS_TO_ANOTHER_PROCESSOR;
	}

	in_client.mIsUnregistering = true;
	return flags::INF_PROC_SUCCESS;
}

GENERIC InfProcessor::set_max_client_count(U32 in_max_clients)
{
	mMaxClients = in_max_clients;
}

GENERIC InfProcessor::set_inactivity_threshold(U32 in_threshold)
{
	mInactivityThreshold = in_threshold;
}

GENERIC InfProcessor::update_inactivity()
{
	if (this->is_registered())
	{
		for (InfProcessor::client_list::iterator It = begin(); It != end(); ++It)
		{
			InfClient* tmpClient = *It;

			if (tmpClient->get_inactivity_counter() == get_inactivity_threshold())
			{
				unregister_client(*tmpClient);
				continue;
			}
			++tmpClient->mInactivityCounter;
		}
	}
	mClientCleaner.reset_tick_counter();
}

GENERIC InfProcessor::halt()
{
	mIsRunning = false;
}

GENERIC InfProcessor::resume()
{
	mIsRunning = true;
}

GENERIC InfProcessor::update()
{
	if(!this->is_registered() || !this->is_running())
	{
		return;
	}

	mbase::lock_guard mRg(mProcHandlerMutex);
	for(logic_handlers::iterator It = mProcessedHandlers.begin(); It != mProcessedHandlers.end(); ++It)
	{
		InfClient* myClient = *It;
		myClient->mInactivityCounter = 0;
		mbase::char_stream& myCharacterStream = myClient->mfrGeneratedToken;
		if(!myClient->is_registered())
		{
			mRegisteredBatchSize -= myClient->mfrMaxTokenCount;
			if(myClient->mFs == mbase::InfClient::finish_state::INF_FINISH_STATE_ABANDONED)
			{
				myCharacterStream.set_cursor_front();
				myClient->on_finish(myClient->mfrBatchCursor);
			}
			myClient->_reset_client();
			myClient->on_unregister();
			continue;
		}

		if(!myClient->is_processing())
		{
			myClient->mFs = mbase::InfClient::finish_state::INF_FINISH_STATE_SUCCESS;
			myClient->mSamplingOrder.clear();
			myClient->on_write(myCharacterStream.get_buffer(), myCharacterStream.get_pos());
			myCharacterStream.set_cursor_front();
			myClient->on_finish(myClient->mfrBatchCursor);
			myClient->mFs = mbase::InfClient::finish_state::INF_FINISH_STATE_CONTINUE;
			continue;
		}
		
		if(!myClient->is_data_set())
		{
			// means token limit reached or execution abandoned
			myClient->mIsProcessing = false;
			myClient->mSamplingOrder.clear();
			myCharacterStream.set_cursor_front();
			myClient->on_finish(myClient->mfrBatchCursor);
			myClient->mFs = mbase::InfClient::finish_state::INF_FINISH_STATE_CONTINUE;
			continue;
		}

		myClient->on_write(myCharacterStream.get_buffer(), myCharacterStream.get_pos());
		myCharacterStream.set_cursor_front();
	}
	mProcessedHandlers.clear();
}

GENERIC InfProcessor::update_t()
{
	if (!this->is_registered() || !this->is_running())
	{
		return;
	}

	mbase::lock_guard lgClient(mClientsMutex);
	for (client_list::iterator It = mRegisteredClients.begin(); It != mRegisteredClients.end(); ++It)
	{
		if (!this->is_registered() || !this->is_running())
		{
			return;
		}
		InfClient* myClient = *It;
		if(myClient->is_unregistering())
		{
			mbase::lock_guard procMutex(mProcHandlerMutex);
			if(myClient->is_processing())
			{
				myClient->mFs = InfClient::finish_state::INF_FINISH_STATE_ABANDONED;
			}

			llama_kv_cache_seq_rm(mModelContext, myClient->mSequenceId, -1, -1);
			myClient->mfrHostProcessor = NULL;
			It = mRegisteredClients.erase(It);
			mProcessedHandlers.push_back(myClient);
			if (It == mRegisteredClients.end())
			{
				break;
			}
			continue;
		}

		if (!myClient->is_logic_processed())
		{
			continue;
		}

		if (!myClient->is_data_set())
		{
			continue;
		}

		if(!myClient->is_processing())
		{
			mbase::vector<InfProcessor::inf_token>& generatedTokens = myClient->mParsedTokens;
			U32 parsedTokenSize = generatedTokens.size();
			if (!parsedTokenSize)
			{
				continue;
			}

			for (size_type i = 0; i < generatedTokens.size(); i++)
			{
				I32 seqId = myClient->mSequenceId;
				llama_batch_add(myClient->mfrBatch, generatedTokens[i], i, { seqId }, false);
			}

			//generatedTokens.clear();
			myClient->mfrBatch.logits[myClient->mfrBatch.n_tokens - 1] = true;
			myClient->mIsProcessing = true;
			myClient->mfrBatchCursor = myClient->mfrBatch.n_tokens;
		}
		else
		{
			
			if(myClient->mfrBatchCursor >= myClient->mfrMaxTokenCount)
			{
				myClient->mIsDataSet = false;
				myClient->mFs = InfClient::finish_state::INF_FINISH_STATE_TOKEN_LIMIT_REACHED;
				llama_batch_clear(myClient->mfrBatch);
				llama_kv_cache_seq_rm(mModelContext, myClient->mSequenceId, -1, -1);
				//mClientsMutex.release();
				mbase::lock_guard procMutex(mProcHandlerMutex);
				mProcessedHandlers.push_back(myClient);
				continue;
			}

			llama_decode(mModelContext, myClient->mfrBatch);

			I32 modelVocab = 0;
			I32 seqId = myClient->mSequenceId;
			get_processed_model()->get_vocab_count(modelVocab);
			float* logits = llama_get_logits_ith(mModelContext, myClient->mfrBatch.n_tokens - 1);

			for (llama_token token_id = 0; token_id < modelVocab; ++token_id)
			{ 
				mPresetCandidates.emplace_back(llama_token_data{ token_id, logits[token_id], 0.0f });
			}

			// TODO: implement an object that will provide a token sampling interface
			llama_token_data_array tokenCandidates = { mPresetCandidates.data(), mPresetCandidates.size(), false };

			llama_sample_repetition_penalties(mModelContext, &tokenCandidates, myClient->mParsedTokens.data(), myClient->mParsedTokens.size(), 1.0, 0.0, 0.0);

			mbase::vector<InfSamplingBase*> tmpSamplers;
			myClient->get_sampling_order(tmpSamplers);

			for(mbase::vector<InfSamplingBase*>::iterator It = tmpSamplers.begin(); It != tmpSamplers.end(); ++It)
			{
				InfSamplingBase* activeSampler = *It;
				activeSampler->set_context(mModelContext);
				activeSampler->set_token_array(&tokenCandidates);
				activeSampler->apply_sampling();
			}
			
			llama_token generatedToken;

			generatedToken = llama_sample_token_greedy(mModelContext, &tokenCandidates);
			// TODO: Handle unknown token generation
			
			if (llama_token_is_eog(get_processed_model()->get_raw_model(), generatedToken)) // || batch_cursor >= predictMax
			{
				I32 resultLength = llama_token_to_piece(get_processed_model()->get_raw_model(), generatedToken, myClient->mfrGeneratedToken.get_buffer(), 128, 0, true);
				mbase::string strGeneratedToken(myClient->mfrGeneratedToken.get_buffer(), resultLength);
				myClient->mfrGeneratedToken.set_cursor_pos(resultLength);
				myClient->mIsDataSet = false;
				myClient->mIsProcessing = false;
				myClient->mFs = InfClient::finish_state::INF_FINISH_STATE_SUCCESS;
				llama_batch_clear(myClient->mfrBatch);
				mPresetCandidates.clear();
				llama_kv_cache_seq_rm(mModelContext, myClient->mSequenceId, -1, -1);
				//mClientsMutex.release();
				mbase::lock_guard procMutex(mProcHandlerMutex);
				mProcessedHandlers.push_back(myClient);
				continue;
			}

			I32 resultLength = llama_token_to_piece(get_processed_model()->get_raw_model(), generatedToken, myClient->mfrGeneratedToken.get_buffer(), 128, 0, true);
			llama_batch_clear(myClient->mfrBatch);
			llama_batch_add(myClient->mfrBatch, generatedToken, myClient->mfrBatchCursor, { seqId }, true);
			mbase::string strGeneratedToken(myClient->mfrGeneratedToken.get_buffer(), resultLength);
			// TODO: CHECK IF THE GENERATED TOKEN IS SPECIAL
			myClient->mfrGeneratedToken.set_cursor_pos(resultLength);

			myClient->mfrBatchCursor++;
			mPresetCandidates.clear();
			myClient->mIsLogicProcessed = false;
			//mClientsMutex.release();

			mbase::lock_guard procMutex(mProcHandlerMutex);
			mProcessedHandlers.push_back(myClient);
		}
	}
}

MBASE_END
