#include <mbase/inference/inf_client.h>
#include <mbase/inference/inf_model.h>

MBASE_BEGIN

#define MBASE_INF_CLIENT_USUAL_CHECK \
if(!is_registered())\
{\
	return flags::INF_CLIENT_ERR_NOT_REGISTERED;\
}\
if(is_processing() || is_data_set())\
{\
	return flags::INF_CLIENT_ERR_PROCESSING;\
}

static U32 gSeqIdCounter = 0;

InfClient::InfClient() :
	mfrHostProcessor(NULL),
	mfrSelfIter(NULL),
	mIsProcessing(false),
	mIsLogicProcessed(true),
	mIsDataSet(false),
	mParsedTokens(),
	mChatHistory(),
	mSequenceId(0),
	mfrBatchCursor(0),
	mMessageIndexer(0),
	mfrMaxTokenCount(0),
	mfrBatch(),
	mfrGeneratedToken(128),
	mFs(finish_state::INF_FINISH_STATE_CONTINUE)
{
}

InfClient::~InfClient()
{
	if(this->is_registered())
	{
		mfrHostProcessor->unregister_client(*this);
	}
}

bool InfClient::is_processing() const
{
	return mIsProcessing;
}

bool InfClient::is_registered() const
{
	return (mfrHostProcessor != NULL);
}

bool InfClient::is_data_set() const
{
	return mIsDataSet;
}

bool InfClient::is_logic_processed() const
{
	return mIsLogicProcessed;
}

InfClient::flags InfClient::get_generated_token_count(size_type& out_token_count)
{
	MBASE_INF_CLIENT_USUAL_CHECK;
	out_token_count = mfrBatchCursor;
	return flags::INF_CLIENT_SUCCESS;
}

InfClient::flags InfClient::get_context_id(U32& out_context_id)
{
	if(!is_registered())
	{
		return flags::INF_CLIENT_ERR_NOT_REGISTERED;
	}
	out_context_id = mSequenceId;
	return flags::INF_CLIENT_SUCCESS;
}

InfClient::flags InfClient::get_message_context(U32 in_msg_id, context_line& out_context_line)
{
	if (!is_registered())
	{
		return flags::INF_CLIENT_ERR_NOT_REGISTERED;
	}

	chat_history_map::iterator foundLine = mChatHistory.find(in_msg_id);
	if(foundLine == mChatHistory.end())
	{
		return flags::INF_CLIENT_ERR_MSG_ID_MISMATCH;
	}

	out_context_line = foundLine->second;

	return flags::INF_CLIENT_SUCCESS;
}

InfClient::flags InfClient::set_input(CBYTEBUFFER in_data, size_type in_size, input_role in_role, U32 &out_message_id)
{
	MBASE_INF_CLIENT_USUAL_CHECK;

	mbase::string lineString(in_data, in_size);
	mbase::string roleString;
	mbase::string totalInput = lineString;
	if(in_role == input_role::INF_ROLE_SYSTEM)
	{
		mbase::string endString;
		mfrHostProcessor->get_processed_model()->get_sys_start(roleString);
		mfrHostProcessor->get_processed_model()->get_sys_end(endString);
		totalInput = roleString + lineString + endString;
	}
	else if(in_role == input_role::INF_ROLE_USR)
	{
		mbase::string endString;
		mfrHostProcessor->get_processed_model()->get_usr_start(roleString);
		mfrHostProcessor->get_processed_model()->get_sys_end(endString);
		totalInput = roleString + lineString + endString;
	}
	printf("%s\n", totalInput.c_str());
	context_line tempContextLine{ in_role, std::move(totalInput), ++mMessageIndexer };
	mChatHistory.insert(mbase::pair<U32, context_line>(mMessageIndexer, tempContextLine));
	out_message_id = mMessageIndexer;

	return flags::INF_CLIENT_SUCCESS;
}

InfClient::flags InfClient::set_input(const mbase::string& in_data, input_role in_role, U32& out_message_id)
{
	return set_input(in_data.c_str(), in_data.size(), in_role, out_message_id);
}

InfClient::flags InfClient::execute_prompt(const mbase::vector<U32>& in_msg_ids)
{
	MBASE_INF_CLIENT_USUAL_CHECK;

	mbase::string totalPrompt;
	if(in_msg_ids.size())
	{
		for (mbase::vector<U32>::const_iterator cIt = in_msg_ids.cbegin(); cIt != in_msg_ids.cend(); ++cIt)
		{
			chat_history_map::iterator myIt = mChatHistory.find(*cIt);
			if (myIt == mChatHistory.end())
			{
				return flags::INF_CLIENT_ERR_MSG_ID_MISMATCH;
			}

			totalPrompt += myIt->second.mInput;
		}
	}
	else
	{
		return flags::INF_CLIENT_ERR_MISSING_CHAT;
	}

	if(!totalPrompt.size())
	{
		return flags::INF_CLIENT_ERR_MISSING_CHAT;
	}

	if(mfrHostProcessor->tokenize_input(totalPrompt.c_str(), totalPrompt.size(), mParsedTokens) != InfProcessor::flags::INF_PROC_SUCCESS)
	{
		return flags::INF_CLIENT_ERR_TOKENIZATION_FAILED;
	}

	if(mParsedTokens.size() > mfrMaxTokenCount)
	{
		return flags::INF_CLIENT_ERR_TOKEN_LIMIT_EXCEEDED;
	}

	mIsDataSet = true; // SIGNALS THE UPDATE_T TO START EXECUTION

	return flags::INF_CLIENT_SUCCESS;
}

InfClient::flags InfClient::remove_messages(const mbase::vector<U32>& in_msg_ids)
{
	if(in_msg_ids.size())
	{
		for(auto&n : in_msg_ids)
		{
			mChatHistory.erase(n);
		}
	}
	else
	{
		mChatHistory.clear();
	}
	return flags::INF_CLIENT_SUCCESS;
}

GENERIC InfClient::next()
{
	mIsLogicProcessed = true;
}

GENERIC InfClient::abandon()
{
	mIsDataSet = false;
}

GENERIC InfClient::_reset_client()
{
	if(!this->is_registered())
	{
		return;
	}
	llama_batch_free(mfrBatch);
	mfrHostProcessor = NULL;
	mfrSelfIter = NULL;
	mIsProcessing = false;
	mIsLogicProcessed = true;
	mIsDataSet = false;
	mParsedTokens.clear();
	mSequenceId = 0;
	mfrBatchCursor = 0;
}

MBASE_END