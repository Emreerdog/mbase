#ifndef MBASE_INF_PROGRAM_H
#define MBASE_INF_PROGRAM_H

#include <mbase/common.h>
#include <mbase/vector.h>
#include <mbase/string.h>
#include <mbase/list.h>
#include <mbase/unordered_map.h>
#include <mbase/traits.h>
#include <mbase/pc/pc_net_manager.h>
#include <mbase/inference/inf_model.h>
#include <mbase/inference/inf_client.h>

MBASE_BEGIN

#define MBASE_MAIP_CL_AUTH \
const U64& in_csid, const mbase::string& in_clid\

struct InfAcceptedClient;

class MBASE_API InfMaipTunedClient : public mbase::InfClient {
public:
	InfMaipTunedClient();
	InfMaipTunedClient(InfAcceptedClient& in_client);

	GENERIC on_register() override;
	GENERIC on_write(CBYTEBUFFER out_data, size_type out_size) override;
	GENERIC on_finish(size_type out_total_token_size) override;
	GENERIC on_unregister() override;
private:
	InfAcceptedClient* mManagerClient;
};

struct MBASE_API InfAcceptedClient {
	std::shared_ptr<mbase::PcNetPeerClient> mPeer;
	mbase::vector<mbase::string> mAcceptedModels;
	mbase::unordered_map<U64, InfMaipTunedClient> mChatSessions;
	mbase::string mClid;
	U64 mCsId;
	U64 mChatSessionIdCounter;
};

class MBASE_API InfProgram {
public:
	enum class flags : U8 {
		INF_PROGRAM_SUCCESS,
		INF_PROGRAM_ERR_MODEL_ALREADY_BEING_HOSTED,
		INF_PROGRAM_ERR_MODEL_IS_NOT_INITIALIZED,
		INF_PROGRAM_ERR_MODEL_NAME_MISMATCH
	};

	enum class maip_err_code : U16 {
		INF_SUCCESS = 2000,
		INF_MAXIMUM_CLIENTS,
		INF_CLIENT_ID_MISMATCH,
		INF_UNAUTHORIZED_ACCESS,
		INF_MODEL_NAME_MISMATCH,
		INF_CONTEXT_ID_MISMATCH,
		INF_CONTEXT_LIMIT_REACHED,
		INF_UNABLE_TO_FIND_SUITABLE_PROCESSOR,
		INF_UNKNOWN_STATUS,
		INF_CLIENT_UNREGISTERING,
		INF_CLIENT_NOT_REGISTERED,
		EXEC_SUCCESS = 3000,
		EXEC_PROCESSING,
		EXEC_MESSAGE_ID_MISMATCH,
		EXEC_MISSING_MESSAGE,
		EXEC_TOKENIZATION_FAILED,
		EXEC_TOKEN_LIMIT_EXCEEDED,
		EXEC_MESSAGE_CONTINUE,
		EXEC_MESSAGE_FINISH
	};

	bool is_session_match(MBASE_MAIP_CL_AUTH);

	maip_err_code inf_create_session(const mbase::string& in_clid, U64& out_csid, mbase::string& out_clid);
	maip_err_code inf_destroy_client(MBASE_MAIP_CL_AUTH);
	maip_err_code inf_get_acquired_models(MBASE_MAIP_CL_AUTH, mbase::vector<mbase::string>& out_models);
	maip_err_code inf_get_created_context_ids(MBASE_MAIP_CL_AUTH, mbase::vector<mbase::string>& out_contexts);
	maip_err_code inf_create_context(MBASE_MAIP_CL_AUTH, const mbase::string& in_model, const U32& in_ctsize); // CTSIZE : Context size
	maip_err_code inf_destroy_context(MBASE_MAIP_CL_AUTH, const U64& in_ctxId);
	// get_context_params
	maip_err_code inf_acquire_model(MBASE_MAIP_CL_AUTH, const mbase::string& in_model);
	maip_err_code inf_release_model(MBASE_MAIP_CL_AUTH, const mbase::string& in_model);
	maip_err_code inf_get_models(MBASE_MAIP_CL_AUTH, mbase::vector<mbase::string>& out_models);
	maip_err_code inf_get_model_params(MBASE_MAIP_CL_AUTH, const mbase::string& in_model);

	maip_err_code exec_set_input(MBASE_MAIP_CL_AUTH, const U64& in_ctxId, InfClient::input_role in_role, const mbase::string& in_input, U32& out_msgid);
	maip_err_code exec_execute_input(MBASE_MAIP_CL_AUTH, const U64& in_ctxId, const mbase::vector<U32>& in_msgid);
	maip_err_code exec_next(MBASE_MAIP_CL_AUTH, const U64& in_ctxId);
	maip_err_code exec_terminate_generation(MBASE_MAIP_CL_AUTH, const U64& in_ctxId);

	flags host_model(InfModel& in_model);
	flags release_model(const mbase::string& in_model_name);

	static maip_err_code proc_err_to_maip(InfProcessor::flags in_flag);
	static maip_err_code client_err_to_maip(InfClient::flags in_flag);

private:
	mbase::unordered_map<U64, InfAcceptedClient> mActiveClients;
	mbase::unordered_map<mbase::string, InfModel*> mRegisteredModels;
	U64 mClientSessionIdCounter;
};

MBASE_END

#endif // !MBASE_INF_PROGRAM_H