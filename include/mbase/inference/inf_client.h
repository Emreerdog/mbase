#ifndef MBASE_INF_CLIENT_H
#define MBASE_INF_CLIENT_H

#include <mbase/common.h>
#include <mbase/behaviors.h>
#include <mbase/string.h>
#include <mbase/inference/inf_t2t_processor.h>
#include <mbase/unordered_map.h>
#include <mbase/framework/handler_base.h>

MBASE_BEGIN

// inheriting from InfClientBase seemed unneccesary, so I removed it.

class MBASE_API InfClientTextToText {
public:	
	using chat_history_map = mbase::unordered_map<U32, context_line>;
	using size_type = SIZE_T;

	enum flags : U8 {
		INF_CLIENT_SUCCESS,
		INF_CLIENT_ERR_MSG_ID_MISMATCH,
		INF_CLIENT_ERR_MISSING_INPUT,
		INF_CLIENT_ERR_NOT_REGISTERED
	};


	InfClientTextToText();
	InfClientTextToText(const InfClientTextToText& in_rhs);
	~InfClientTextToText();

	InfClientTextToText& operator=(const InfClientTextToText& in_rhs);

	bool is_registered() const;
	bool has_message(const U32& in_msg_id) const;
	flags get_host_processor(InfProcessorBase*& out_processor);
	flags get_message(const U32& in_msg_id, context_line& out_message);
	flags get_message_array(PTRU32 in_msg_ids, size_type in_id_count, mbase::vector<context_line>& out_messages);
	
	#ifdef MBASE_INTERNAL_API
	GENERIC _on_register(InfProcessorBase* in_processor);
	GENERIC _on_unregister();
	#endif
	
	virtual GENERIC on_register(InfProcessorBase* out_processor) = 0;
	virtual GENERIC on_embedding_data(const F32* out_data, size_type out_size);
	virtual GENERIC on_write(const inf_text_token_vector& out_token, bool out_is_finish) = 0;
	virtual GENERIC on_finish(size_type out_total_token_size, InfTextToTextProcessor::finish_state out_finish_state) = 0;
	virtual GENERIC on_unregister() = 0;

	flags add_message(CBYTEBUFFER in_data, size_type in_size, context_role in_role, U32& out_message_id);
	flags add_message(const mbase::string& in_data, context_role in_role, U32& out_message_id);
	flags add_message(const mbase::wstring& in_data, context_role in_role, U32& out_message_id);
	flags remove_messages(const mbase::vector<U32>& in_msg_ids = mbase::vector<U32>());

	GENERIC clear_chat_history(); // clears the chat map
protected:
	
	InfProcessorBase* mBaseProcessor;
	chat_history_map mChatHistory;
	U32 mMessageIndexer;
};

//class MBASE_API InfClient : public mbase::handler_base {
//public:
//	enum class flags : U8 {
//		INF_CLIENT_SUCCESS,
//		INF_CLIENT_ERR_NOT_REGISTERED,
//		INF_CLIENT_ERR_PROCESSING,
//		INF_CLIENT_ERR_MSG_ID_MISMATCH,
//		INF_CLIENT_ERR_MISSING_CHAT,
//		INF_CLIENT_ERR_TOKENIZATION_FAILED,
//		INF_CLIENT_ERR_TOKEN_LIMIT_EXCEEDED,
//		INF_CLIENT_ERR_UNKNOWN,
//		INF_CLIENT_ERR_UNREGISTERATION_IN_PROGRESS,
//		INF_CLIENT_ERR_MISSING_PROCESSOR,
//		INF_CLIENT_ERR_MISSING_INPUT,
//		INF_CLIENT_ERR_SAMPLER_MISMATCH
//	};
//
//	enum class finish_state : U8 {
//		INF_FINISH_STATE_SUCCESS,
//		INF_FINISH_STATE_CONTINUE,
//		INF_FINISH_STATE_TOKEN_LIMIT_REACHED,
//		INF_FINISH_STATE_ABANDONED
//	};
//
//	enum class input_role : U8 {
//		INF_ROLE_SYSTEM,
//		INF_ROLE_ASSISTANT,
//		INF_ROLE_USR,
//		INF_ROLE_NONE
//	};
//
//	struct context_line {
//		input_role mRole = input_role::INF_ROLE_NONE;
//		mbase::string mInput = "";
//		U32 mMessageIndex = 0;
//	};
//
//	using chat_history_map = mbase::unordered_map<U32, context_line>;
//	using sampler_map = mbase::unordered_map<mbase::string, InfSamplingBase*>;
//	using token_vector = mbase::vector<InfTextToTextProcessor::inf_text_token>;
//	using size_type = SIZE_T;
//
//	friend class InfTextToTextProcessor;
//
//	InfClient();
//	InfClient(const InfClient& in_rhs);
//	~InfClient();
//
//	InfClient& operator=(const InfClient& in_rhs);
//
//	bool is_processing() const;
//	bool is_registered() const;
//	bool is_unregistering() const;
//	bool is_data_set() const;
//	bool is_logic_processed() const;
//	bool has_sampler(const mbase::string& in_sampler_name);
//	bool get_sampler(const mbase::string& in_sampler_name, InfSamplingBase*& out_sampler);
//	GENERIC get_sampling_order(mbase::vector<InfSamplingBase*>& out_order);
//	flags get_message_context(U32 in_msg_id, context_line& out_context_line);
//	flags get_host_processor(InfTextToTextProcessor*& out_processor);
//	U32 get_inactivity_counter();
//
//	virtual GENERIC on_register() = 0;
//	virtual GENERIC on_write(CBYTEBUFFER out_data, size_type out_size) = 0;
//	virtual GENERIC on_finish(size_type out_total_token_size) = 0;
//	virtual GENERIC on_unregister() = 0;
//	flags set_input(CBYTEBUFFER in_data, size_type in_size, input_role in_role, U32& out_message_id);
//	flags set_input(const mbase::string& in_data, input_role in_role, U32& out_message_id);
//	flags set_input(const mbase::wstring& in_data, input_role in_role, U32& out_message_id);
//	flags execute_prompt(const mbase::vector<U32>& in_msg_ids);
//	flags remove_messages(const mbase::vector<U32>& in_msg_ids = mbase::vector<U32>());
//
//	template<typename SamplerType>
//	flags add_sampler() {
//		SamplerType* newSampler = new SamplerType(NULL, NULL);
//		mbase::string samplerName = newSampler->get_sampler_name();
//		if(has_sampler(samplerName))
//		{
//			delete newSampler;
//			return flags::INF_CLIENT_SUCCESS;
//		}
//
//		mSamplerMap[samplerName] = newSampler;
//		return flags::INF_CLIENT_SUCCESS;
//	}
//	bool add_to_sampling_order(InfSamplingBase* in_sampler); // MUST BE IN THE SAMPLING MAP
//	GENERIC next();
//	GENERIC abandon();
//	GENERIC clear_chat_history(); // clears the chat map
//	
//protected:
//	GENERIC _reset_client();
//
//	GENERIC _register_self_client_params(
//		InfTextToTextProcessor* in_processor
//	);
//
//	GENERIC _register_self_client_params(
//		InfTextToTextProcessor* in_processor,
//		const mbase::vector<InfTextToTextProcessor::inf_text_token>& in_token_vector
//	);
//
//	InfTextToTextProcessor* mfrHostProcessor;
//	bool mIsProcessing;
//	bool mIsUnregistering;
//	bool mIsLogicProcessed;
//	bool mIsDataSet;
//	mbase::vector<InfTextToTextProcessor::inf_text_token> mParsedTokens;
//	mbase::vector<InfSamplingBase*> mSamplingOrder;
//	mbase::unordered_map<U32, context_line> mChatHistory;
//	sampler_map mSamplerMap;
//	U32 mMessageIndexer;
//	U32 mInactivityCounter;
//	mbase::deep_char_stream mfrGeneratedToken;
//	finish_state mFs;
//};

MBASE_END

#endif // !MBASE_INF_CLIENT_H
