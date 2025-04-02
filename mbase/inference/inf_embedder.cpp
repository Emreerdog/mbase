#include <mbase/inference/inf_embedder.h>
#include <mbase/inference/inf_t2t_model.h>
#include <mbase/inference/inf_client.h>
#include <mbase/inference/inf_embedder_client.h>

MBASE_BEGIN


#define MBASE_INF_EMBEDDER_PROC_RETURN_UNREGISTERED \
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

InfEmbedderProcessor::InfEmbedderProcessor() noexcept:
    mOperationProcedure(NULL),
    mModelContext(NULL),
    mEmbeddingLength(0),
    mBatchSize(0),
    mThreadCount(0),
    mSequenceEmbeddingCursor(0),
    mProcessedBatchLength(0),
    mFinishState(finish_state::FINISHED),
    mIsInitializeFailed(false)
{
    mModelCategory = inf_model_category::EMBEDDING;
}

InfEmbedderProcessor::~InfEmbedderProcessor() noexcept
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

bool InfEmbedderProcessor::is_init_failed() const
{
    return mIsInitializeFailed;
}

bool InfEmbedderProcessor::is_available() const
{
    if(signal_embedding_process() || signal_state_embedding_process())
    {
        return false;
    }

    return true;
}

InfEmbedderProcessor::last_fail_code InfEmbedderProcessor::get_last_fail_code() const
{
    return last_fail_code::INVALID_MODEL_TYPE;
}

bool InfEmbedderProcessor::signal_state_embedding_process() const
{
    return mEmbeddingSignal.get_signal_state();
}

bool InfEmbedderProcessor::signal_embedding_process() const
{
    return mEmbeddingSignal.get_signal();
}

const U32& InfEmbedderProcessor::get_embedding_length() const
{
    return mEmbeddingLength;
}

const U32& InfEmbedderProcessor::get_max_token_length() const
{
    return mContextLength;
}

const I32& InfEmbedderProcessor::get_batch_thread_count() const
{
    return llama_n_threads_batch(mModelContext);
}

InfEmbedderProcessor::flags InfEmbedderProcessor::get_processor_status() const
{
	if(signal_initializing())
	{
		return flags::INF_PROC_INFO_INITIALIZING;
	}

	if(signal_destroying())
	{
		return flags::INF_PROC_INFO_DESTROYING;
	}

	if(signal_state_embedding_process())
	{
		return flags::INF_PROC_INFO_INITIAL_INPUT_PROCESSING;
	}

	return flags::INF_PROC_INFO_NEED_UPDATE;
}

InfEmbedderProcessor::flags InfEmbedderProcessor::tokenize_input(CBYTEBUFFER in_data, size_type in_size, inf_text_token_vector& out_tokens)
{
    MBASE_INF_EMBEDDER_PROC_RETURN_UNREGISTERED;

    if(!in_size)
    {
        return flags::INF_PROC_ERR_INPUT_IS_EMPTY;
    }

    inf_text_token_vector tokenizedInput(in_size * 4);
    InfModelTextToText* t2tModel = static_cast<InfModelTextToText*>(this->mTargetModel_md_model);
    const llama_vocab* tmpVocab = llama_model_get_vocab(t2tModel->get_raw_model());
	I32 tokenCount = llama_tokenize(tmpVocab, in_data, static_cast<I32>(in_size), tokenizedInput.data(), static_cast<I32>(tokenizedInput.capacity()), false, true);

    if(tokenCount == -1)
    {
        return flags::INF_PROC_ERR_UNABLE_TO_TOKENIZE_INPUT;
    }

    tokenizedInput.resize_on_preset(tokenCount);
    out_tokens = std::move(tokenizedInput);
    return flags::INF_PROC_SUCCESS;
}

InfEmbedderProcessor::flags InfEmbedderProcessor::tokenize_input(const mbase::string& in_string, inf_text_token_vector& out_tokens)
{
    return tokenize_input(in_string.c_str(), in_string.size(), out_tokens);
}

InfEmbedderProcessor::flags InfEmbedderProcessor::tokenize_input(const mbase::wstring& in_string, inf_text_token_vector& out_tokens)
{
    return tokenize_input(mbase::to_utf8(in_string), out_tokens);
}

InfEmbedderProcessor::flags InfEmbedderProcessor::execute_input(const mbase::vector<inf_text_token_vector>& in_tokens, bool in_abandon)
{
    MBASE_INF_EMBEDDER_PROC_RETURN_UNREGISTERED;

    if(!in_tokens.size())
	{
		return flags::INF_PROC_ERR_INPUT_IS_EMPTY;
	}

    size_type totalTokenCount = 0;
    
    for(auto& tokenVector : in_tokens)
    {
        totalTokenCount += tokenVector.size();
        if(totalTokenCount > mContextLength)
        {
            return flags::INF_PROC_ERR_INPUT_EXCEED_TOKEN_LIMIT;
        }
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
        mEmbeddingSignal.reset_signal_with_state();
        stop_processor();
    }

    mSequenceEmbeddingCursor = 0;
    mTokenizedInput = in_tokens;
    mEmbeddingSignal.set_signal();
    start_processor();

    return flags::INF_PROC_SUCCESS;
}

InfEmbedderProcessor::flags InfEmbedderProcessor::next()
{
    MBASE_INF_EMBEDDER_PROC_RETURN_UNREGISTERED;
    
    if(signal_embedding_process())
    {
        // If it is already processing, ignore the request
        return flags::INF_PROC_ERR_ALREADY_PROCESSING;
    }

    if(signal_state_embedding_process())
    {
        return flags::INF_PROC_INFO_NEED_UPDATE;
    }

    if(!is_running())
    {
        return flags::INF_PROC_INFO_HALTED;
    }

    if(mFinishState != finish_state::CONTINUE)
    {
        return flags::INF_PROC_ERR_INPUT_IS_EMPTY;
    }
    
    PTRF32 embeddingsOut = llama_get_embeddings_seq(mModelContext, mSequenceEmbeddingCursor++);
    if(mAssignedClient)
    {
        bool tmpIsFinished = false;
        if(mSequenceEmbeddingCursor == mTokenizedInput.size())
        {
            mFinishState = finish_state::FINISHED;
            tmpIsFinished = true;
        }
        InfClientEmbedder* embedderClient = static_cast<InfClientEmbedder*>(mAssignedClient);
        //inf_common_embd_normalize(embeddingsOut, embeddingsOut, mEmbeddingLength);
        embedderClient->on_write(this, embeddingsOut, mSequenceEmbeddingCursor - 1, tmpIsFinished);
        if(tmpIsFinished)
        {
            embedderClient->on_finish(this, mEmbeddingLength * mSequenceEmbeddingCursor);
        }
    }

    return flags::INF_PROC_SUCCESS; 
}

InfEmbedderProcessor::flags InfEmbedderProcessor::set_inference_client(InfClientBase* in_client)
{
    MBASE_INF_EMBEDDER_PROC_RETURN_UNREGISTERED;

    if (get_assigned_client())
    {
        return flags::INF_PROC_ERR_ALREADY_PROCESSING;
    }

    if (!in_client)
    {
        return flags::INF_PROC_ERR_MISSING_CLIENT;
    }

    if (mAssignedClient == in_client)
    {
        return flags::INF_PROC_SUCCESS;
    }

    release_inference_client();

    mAssignedClient = in_client;
    mAssignedClient->on_register(this);

    return flags::INF_PROC_SUCCESS;
}

InfEmbedderProcessor::flags InfEmbedderProcessor::initialize(
    InfModelTextToText* in_model,
    const mbase::string& in_context_id,
    const U32& in_context_length,
    const U32& in_thread_count
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
    mContextIdentifier = in_context_id;
    mContextLength = in_context_length;
    mBatchSize = mContextLength;
    mThreadCount = in_thread_count;
    
    mInitializeSignal.set_signal();
    on_initializing();
    start_processor();
    return flags::INF_PROC_INFO_INITIALIZING;
}

InfEmbedderProcessor::flags InfEmbedderProcessor::initialize_sync(
    InfModelTextToText* in_model,
    const mbase::string& in_context_id,
    const U32& in_context_length,
    const U32& in_thread_count
)
{
    initialize(
        in_model,
        in_context_id,
        in_context_length,
        in_thread_count
    );

    while(signal_initializing())
    {
        mbase::sleep(2);
    }

    return flags::INF_PROC_INFO_NEED_UPDATE;
}

InfEmbedderProcessor::flags InfEmbedderProcessor::destroy()
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
	on_destroying();

	start_processor();
	mDestroySignal.set_signal();
	return flags::INF_PROC_INFO_DESTROYING;
}

InfEmbedderProcessor::flags InfEmbedderProcessor::destroy_sync()
{
    destroy();
	while(signal_destroying())
	{
        mbase::sleep(2);
		// block until operation finishes
	}

	return flags::INF_PROC_INFO_NEED_UPDATE;
}

GENERIC InfEmbedderProcessor::_initialize_context()
{
    llama_context_params ctxParams = llama_context_default_params();
    ctxParams.n_ctx = mContextLength;
    ctxParams.n_batch = mBatchSize;
    ctxParams.n_seq_max = 1;
    ctxParams.n_threads = mThreadCount;
    ctxParams.n_threads_batch = mThreadCount;
    ctxParams.n_ubatch = mBatchSize;
    ctxParams.embeddings = true;

    InfModelTextToText* t2tModel = static_cast<InfModelTextToText*>(this->mTargetModel_md_model);

	if(!t2tModel || !t2tModel->is_initialized())
	{
		mLastFailCode = last_fail_code::MODEL_NOT_INITIALIZED;
		mIsInitializeFailed = true;
		mInitializeSignal.set_signal_finished();
		return;
	}

    llama_model* rawModel = t2tModel->get_raw_model();
    mModelContext = llama_init_from_model(rawModel, ctxParams);
    if(!mModelContext)
    {
        mLastFailCode = last_fail_code::NOT_ENOUGH_MEMORY;
		mIsInitializeFailed = true;
		mInitializeSignal.set_signal_finished();
		return;
    }

    mEmbeddingLength = t2tModel->get_embedding_length();

    if(llama_model_has_encoder(rawModel) && !llama_model_has_decoder(rawModel))
    {
        mOperationProcedure = llama_encode;
    }
    
    else if(!llama_model_has_encoder(rawModel) && llama_model_has_decoder(rawModel))
    {
        mOperationProcedure = llama_decode;
    }

	mInitializeSignal.set_signal_finished();
}

GENERIC InfEmbedderProcessor::_destroy_context()
{
    // EMBEDDER PROCESSOR FACTORY RESET

    llama_free(mModelContext);
    mModelContext = NULL;
    mTokenizedInput.clear();
    mBatchSize = 0;
	mThreadCount = 0;
    mSequenceEmbeddingCursor = 0;
    mIsRunning = false;
    mFinishState = finish_state::FINISHED;
    mIsInitializeFailed = false;
    mAssignedClient = NULL;
	mLastFailCode = last_fail_code::MODEL_NOT_INITIALIZED;

    mTargetModel_md_model = NULL;
	mIsRegistered = false;
	mDestroySignal.set_signal_finished();
}

GENERIC InfEmbedderProcessor::_calculate_embeddings()
{
    I32 sequenceCounter = 0;
    mProcessedBatchLength = 0;
    
    //llama_kv_cache_clear(mModelContext);
    llama_kv_self_clear(mModelContext);
    llama_batch embedderBatch = llama_batch_init(mBatchSize, 0, 1);
    for(inf_text_token_vector& tmpTokenVector : mTokenizedInput)
    {
        I32 tmpBatchCursor = 0;
        mFinishState = finish_state::CONTINUE;
        for(size_type i = 0; i < tmpTokenVector.size(); ++i)
        {
            inf_common_batch_add(embedderBatch, tmpTokenVector[i], static_cast<I32>(i), {sequenceCounter}, true);
            ++tmpBatchCursor;
        }
        ++sequenceCounter;
    }
    I32 opResult = mOperationProcedure(mModelContext, embedderBatch);
    if(opResult)
    {
        // It should NEVER happen
        // Handle later
    }
    mProcessedBatchLength += embedderBatch.n_tokens;
    llama_batch_free(embedderBatch);
    mEmbeddingSignal.set_signal_finished();
}

GENERIC InfEmbedderProcessor::update()
{
    if(signal_destroying())
    {
        return;
    }

    if(signal_state_destroying())
    {
        stop_processor();
		reset_base_signals();
        mEmbeddingSignal.reset_signal_with_state();
		this->release_object_watcher();
		on_destroy();
    }

    if(signal_state_initializing())
	{
		mInitializeSignal.reset_signal_state();
		stop_processor();
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
	}

    if(signal_state_embedding_process())
    {
        mEmbeddingSignal.reset_signal_state();
        // Which means that the embeddings are generated

        InfClientBase* assignedClient = get_assigned_client();
        if(assignedClient)
        {
            InfClientEmbedder* embedderClient = static_cast<InfClientEmbedder*>(assignedClient);
            embedderClient->on_batch_processed(this, mProcessedBatchLength);
        }
    }
}

GENERIC InfEmbedderProcessor::update_t() 
{
    if(is_registered())
    {
        if(signal_destroying())
        {
            _destroy_context();
        }

        if(is_running())
        {
            if(signal_embedding_process())
            {
                _calculate_embeddings();
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
}

GENERIC InfEmbedderProcessor::on_initializing()
{

}

GENERIC InfEmbedderProcessor::on_initialize_fail([[maybe_unused]] last_fail_code out_code)
{

}

GENERIC InfEmbedderProcessor::on_destroying()
{

}


MBASE_END