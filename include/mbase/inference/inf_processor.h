#ifndef MBASE_INF_PROCESSOR_H
#define MBASE_INF_PROCESSOR_H

#include <mbase/common.h>
#include <mbase/vector.h>
#include <mbase/list.h>
#include <mbase/string.h>
#include <mbase/set.h>
#include <mbase/synchronization.h>
#include <mbase/behaviors.h>
#include <mbase/inference/inf_model.h>
#include <mbase/inference/inf_context_line.h>
#include <mbase/inference/inf_sampling.h>
#include <mbase/inference/inf_sampling_set.h>
#include <mbase/framework/logical_processing.h>
#include <mbase/pc/pc_diagnostics.h>
#include <mutex>
#include <llama.h>

MBASE_BEGIN

static const U32 gProcessorMinimumTokenCount = 32;

class InfClientTextToText;
class InfProgram;

class MBASE_API InfProcessorBase : public mbase::logical_processor {
public:
	using size_type = SIZE_T;

	enum class flags : U8 {
		INF_PROC_SUCCESS,
		INF_PROC_ERR_UNREGISTERED_PROCESSOR,
		INF_PROC_ERR_ALREADY_INITIALIZED,
		INF_PROC_ERR_ALREADY_PROCESSING,
		INF_PROC_ERR_BELONGS_TO_ANOTHER_PROCESSOR,
		INF_PROC_ERR_MODEL_IS_NOT_INITIALIZED,
		INF_PROC_ERR_TOKEN_LIMIT_IS_TOO_LOW,
		INF_PROC_ERR_INPUT_EXCEED_TOKEN_LIMIT,
		INF_PROC_ERR_UNABLE_TO_TOKENIZE_INPUT,
		INF_PROC_ERR_INPUT_IS_EMPTY,
		INF_PROC_ERR_MISSING_CLIENT,
		INF_PROC_ERR_SAMPLER_NAME_MISMATCH,
		INF_PROC_INFO_INITIALIZING,
		INF_PROC_INFO_DESTROYING,
		INF_PROC_INFO_HALTED,
		INF_PROC_INFO_CONTINUE,
		INF_PROC_INFO_NEED_UPDATE
	};

	InfProcessorBase();

	bool is_registered() const;
	bool is_running();
	bool signal_state_initializing() const;
	bool signal_state_destroying() const;
	bool signal_initializing() const;
	bool signal_destroying() const;

	U32 get_context_size();
	InfModelBase* get_processed_model();
	U32 get_inactivity_threshold();
	processor_signal& get_initialize_signal();
	processor_signal& get_destroy_signal();
	const mbase::string& get_context_identifier();

	GENERIC set_inactivity_threshold(U32 in_threshold);
	GENERIC halt();
	GENERIC resume();

protected:
	InfModelBase* mTargetModel_md_model;
	bool mIsRunning;
	bool mIsRegistered;
	processor_signal mInitializeSignal;
	processor_signal mDestroySignal;

	U32 mContextLength;
	U32 mInactivityThreshold;
	mbase::string mContextIdentifier;
}; // TODO: speech-to-text(whisper.cpp), text-to-text(llama.cpp), text-to-speech<EXPERIMENTAL>(bark.cpp)

class MBASE_API InfTextToTextProcessor : public mbase::InfProcessorBase {
public:
	using inf_token = llama_token;
	using inf_token_candidates = mbase::vector<llama_token_data>;
	using inf_token_vector = mbase::vector<inf_token>;

	enum class finish_state {
		FINISHED,
		CONTINUE,
		ABANDONED,
		TOKEN_LIMIT_REACHED
	};

	enum class init_fail_code {
		MODEL_NOT_INITIALIZED,
		NOT_ENOUGH_MEMORY
	};

	InfTextToTextProcessor();
	~InfTextToTextProcessor();

	init_fail_code get_last_fail_code() const;
	bool is_available() const;
	bool signal_state_input_process() const;
	bool signal_state_decode_process() const;
	bool signal_input_process() const;
	bool signal_decode_process() const;
	bool signal_token_generated() const;
	bool signal_init_method() const;
	bool signal_destroy_method() const;
	bool signal_init_fail_method() const;
	#ifdef MBASE_INTERNAL_API
		inf_token_candidates& get_token_candidates();
	#endif // MBASE_INTERNAL_API
	U32 get_max_token_length();
	InfClientTextToText* get_assigned_client();
	bool has_sampler(InfSamplerDescription::SAMPLER in_sampler_type, InfSamplerDescription& out_sampler);
	GENERIC get_available_samplers(inf_sampling_set& out_samplers);
	bool has_client() const;
	flags get_processor_status() const;

	flags tokenize_input(CBYTEBUFFER in_data, size_type in_size, inf_token_vector& out_tokens);
	flags tokenize_input(context_line* in_lines, size_type in_count, inf_token_vector& out_tokens, bool in_append_assistant_token = true);
	flags execute_input(const inf_token_vector& in_tokens, bool in_abandon = false);
	flags next();
	flags set_inference_client(InfClientTextToText* in_client);
	flags initialize(
		InfModelTextToText* in_model, 
		const U32& in_context_length, 
		const mbase::string& in_context_id,
		const U32& in_batch_size,
		const U32& in_thread_count,
		const U32& in_batch_thread_count,
		const bool& in_flash_attention,
		const inf_sampling_set& in_sampler_set
	);
	flags initialize_sync(
		InfModelTextToText* in_model, 
		const U32& in_context_length, 
		const mbase::string& in_context_id,
		const U32& in_batch_size,
		const U32& in_thread_count,
		const U32& in_batch_thread_count,
		const bool& in_flash_attention,
		const inf_sampling_set& in_sampler_set
	);
	flags destroy();
	flags destroy_sync();
	GENERIC release_inference_client();
	GENERIC release_inference_client_stacked();
	#ifdef MBASE_INTERNAL_API
		
	#endif // MBASE_INTERNAL_API
	GENERIC clear_token_candidates();
	GENERIC clear_samplers();
	GENERIC update() override;
	GENERIC update_t() override;

	virtual GENERIC on_initializing();
	virtual GENERIC on_initialize_fail(init_fail_code out_code);
	virtual GENERIC on_destroying();
	virtual GENERIC on_initialize() = 0;
	virtual GENERIC on_destroy() = 0;

private:
	GENERIC _decode_input();
	GENERIC _decode_next();
	GENERIC _initialize_context();
	GENERIC _destroy_context();

	llama_sampler* mSamplerChain;
	llama_context* mModelContext;
	llama_batch mInputBatch;
	inf_token_candidates mPresetCandidates;
	inf_token_vector mTokenizedInput;
	inf_sampling_set mSamplerDescriptions;
	inf_token mGeneratedToken;
	U32 mContextCursor; // -----> if it exceeds the context size, stop generating
	U32 mBatchSize;
	U32 mThreadCount;
	U32 mBatchThreadCount;
	processor_signal mInputSignal;
	processor_signal mTokenGeneratedSignal;
	processor_signal mDecodeSignal;
	processor_signal mInitializeMethodSignal;
	processor_signal mDestroyMethodSignal;
	processor_signal mInitializeFailSignal;
	finish_state mFinishState;
	InfClientTextToText* mAssignedClient;
	PcDiagnostics mDiagnostics;
	init_fail_code mLastFailCode;	
	bool mFlashAttention;

};

// class MBASE_API InfTextToTextEmbedder : public InfProcessorBase {
//public:
//private:
// };

MBASE_END

#endif // !MBASE_INF_PROCESSOR_H
