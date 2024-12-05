#ifndef MBASE_CLIENT_T2T_H
#define MBASE_CLIENT_T2T_H

#include <mbase/inference/inf_common.h>
#include <mbase/inference/inf_client.h>
#include <mbase/inference/inf_context_line.h>
#include <mbase/inference/inf_t2t_processor.h>

MBASE_BEGIN

class MBASE_API InfClientTextToText : public InfClientBase {
public:
	using chat_history_map = mbase::unordered_map<U32, context_line>;

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

	bool has_message(const U32& in_msg_id) const;
	flags get_message(const U32& in_msg_id, context_line& out_message);
	flags get_message_array(PTRU32 in_msg_ids, size_type in_id_count, mbase::vector<context_line>& out_messages);
	flags add_message(CBYTEBUFFER in_data, size_type in_size, context_role in_role, U32& out_message_id);
	flags add_message(const mbase::string& in_data, context_role in_role, U32& out_message_id);
	flags add_message(const mbase::wstring& in_data, context_role in_role, U32& out_message_id);
	flags remove_messages(const mbase::vector<U32>& in_msg_ids = mbase::vector<U32>());
	GENERIC clear_chat_history(); // clears the chat map
	
	//virtual GENERIC on_embedding_data(InfProcessorBase* out_procesor, const F32* out_data, size_type out_size)
	virtual GENERIC on_write(InfTextToTextProcessor* out_processor, const inf_text_token_vector& out_token, bool out_is_finish) = 0;
	virtual GENERIC on_finish(InfTextToTextProcessor* out_processor, size_type out_total_token_size, InfTextToTextProcessor::finish_state out_finish_state) = 0;
protected:
	chat_history_map mChatHistory;
	U32 mMessageIndexer;
};

MBASE_END

#endif // !MBASE_CLIENT_T2T_H
