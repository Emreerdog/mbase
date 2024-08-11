#ifndef MBASE_INF_MODEL_H
#define MBASE_INF_MODEL_H

#include <mbase/common.h>
#include <mbase/string.h>
#include <mbase/list.h>
#include <mbase/vector.h>
#include <mbase/unordered_map.h>
#include <llama.h>

MBASE_BEGIN

static const U32 gInfProcessorDefaultCtxLength = 4096;
static const U32 gInfProcessorDefaultBatchSize = 4096;
static const U32 gInfProcessorDefaultMaxSeq = 32;
static const U32 gInfProcessorDefaultThreadCount = 32;

static const U32 gInfProcessorMinCtxLength = 64;
static const U32 gInfProcessorMinBatchSize = 64;
static const U32 gInfProcessorMinSeq = 1;
static const U32 gInfProcessorMinThreadCount = 1;

class InfModel;
class InfProcessor;

struct MBASE_API InfProcInitParams {
	U32 mContextLength = gInfProcessorDefaultCtxLength;
	U32 mBatchSize = gInfProcessorDefaultBatchSize;
	U32 mMaxSequence = gInfProcessorDefaultMaxSeq;
	U32 mThreadCount = gInfProcessorDefaultThreadCount;
};

class MBASE_API InfModel {
public:
	using inf_token = llama_token;
	using processor_list = mbase::list<InfProcessor*>;
	using size_type = SIZE_T;

	enum class flags : U8 {
		INF_MODEL_SUCCESS,
		INF_MODEL_ERR_CANT_LOAD_MODEL,
		INF_MODEL_ERR_MISSING_MODEL,
		INF_MODEL_ERR_NO_SENTENCE,
		INF_MODEL_ERR_PROCESSOR_ALREADY_REGISTERED,
		INF_MODEL_ERR_PROCESSOR_NOT_FOUND,
		INF_MODEL_ERR_PROCESSOR_BELONGS_TO_ANOTHER_MODEL,
		INF_MODEL_ERR_UNABLE_REGISTER_PROCESSOR,
		INF_MODEL_ERR_NOT_INITIALIZED
	};

	InfModel();
	~InfModel();

	bool is_initialized() const;
	llama_model* get_raw_model();
	flags get_special_tokens(mbase::vector<inf_token>& out_tokens);
	flags get_special_tokens(mbase::vector<mbase::string>& out_tokens);
	flags get_model_name(mbase::string& out_name);
	flags get_vocabulary_type(mbase::string& out_type);
	flags get_architecture(mbase::string& out_architecture);
	flags get_finetune_type(mbase::string& out_type);
	flags get_embedding_length(I32& out_length);
	flags get_model_base_name(mbase::string& out_name);
	flags get_rope_type(mbase::string& out_type);
	flags get_sys_start(mbase::string& out_start);
	flags get_usr_start(mbase::string& out_start);
	flags get_sys_end(mbase::string& out_end);
	flags get_usr_end(mbase::string& out_end);
	flags get_vocab_count(I32& out_count);
	flags get_model_param_count(size_type& out_count);
	flags get_model_params(mbase::unordered_map<mbase::string, mbase::string>& out_params);
	flags get_size(size_type& out_size);
	bool is_token_eof_generation(inf_token in_token);
	flags is_token_control(inf_token in_token);
	flags get_metadata_count(size_type& out_count);

	flags initialize(const mbase::string& in_model_path, const I32& in_gpu_layers = -1);
	flags load_model(const mbase::string& in_model_path, const I32& in_gpu_layers = -1); // same with initialize
	flags unload_model();
	flags register_processor(InfProcessor& out_processor, InfProcInitParams in_params = InfProcInitParams());
	flags unregister_processor(InfProcessor& in_processor);

private:
	llama_model* mModel;
	processor_list mRegisteredProcessors;
	mbase::unordered_map<mbase::string, mbase::string> mModelKvals;
	mbase::string mModelName;
	mbase::string mEndOfTokenString;
	mbase::string mUsrStart;
	mbase::string mSystemStart;
	inf_token mEndOfToken;
};

MBASE_END

#endif // !MBASE_INF_MODEL_H
