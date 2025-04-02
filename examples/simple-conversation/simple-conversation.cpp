#include <mbase/inference/inf_t2t_model.h>
#include <mbase/inference/inf_t2t_processor.h>
#include <mbase/inference/inf_t2t_client.h>
#include <mbase/argument_get_value.h>
#include <mbase/filesystem.h>
#include <math.h>
#include <chrono>
#include <signal.h>
#include <locale.h>

#define MBASE_SIMPLE_CONVERSATION_VERSION "v0.1.0"

using namespace mbase;

class ConversationModel;
class ConversationProcessor;
class ConversationClient;

struct program_parameters {
    program_parameters()
    {
        mSeed.mSamplerType = InfSamplerDescription::SAMPLER::RNG;
        mTkSampler.mSamplerType = InfSamplerDescription::SAMPLER::TOP_K;
        mTpSampler.mSamplerType = InfSamplerDescription::SAMPLER::TOP_P;
        mMpSampler.mSamplerType = InfSamplerDescription::SAMPLER::MIN_P;
        mPenalty.mSamplerType = InfSamplerDescription::SAMPLER::REPETITION;
        mTmp.mSamplerType = InfSamplerDescription::SAMPLER::TEMP;

        srand(time(NULL));
        mSeed.mRng = rand();
        mTkSampler.mTopK = 40;
        mTpSampler.mTopP = 1.0f;
        mMpSampler.mMinP = 0.2f;
        mPenalty.mRepetition.mPenaltyN = 64;
        mPenalty.mRepetition.mRepeatPenalty = 1.15f;
        mPenalty.mRepetition.mPenaltyLinefeed = true;
        mPenalty.mRepetition.mPenaltyEos = false;
        mTmp.mTemp = 0.1f;
    }
    mbase::string mModelFile;
    mbase::string mSystemPromptFile;
    mbase::string mSystemPrompt;
    I32 mThreadCount = 16;
    I32 mBatchThreadCount = 8;
    I32 mContextLength = 8192;
    I32 mBatchLength = 4096;
    I32 mGpuLayer = 999;
    bool mIsGreeady = false;
    InfSamplerDescription mTkSampler;
    InfSamplerDescription mTpSampler;
    InfSamplerDescription mMpSampler;
    InfSamplerDescription mPenalty;
    InfSamplerDescription mTmp;
    InfSamplerDescription mSeed;
};

mbase::vector<InfDeviceDescription> deviceDescription;
program_parameters gSampleParams;
bool gIsProgramRunning = true;
ConversationProcessor* gGlobalProcessor = NULL;

GENERIC catching_interrupt_signal(I32 out_sig_id);
GENERIC print_usage();
GENERIC print_usage()
{
    printf("========================================\n");
    printf("#Program name:      mbase_simple_conversation\n");
    printf("#Version:           %s\n", MBASE_SIMPLE_CONVERSATION_VERSION);
    printf("#Type:              Example\n");
    printf("#Further docs:      https://docs.mbasesoftware.com/inference/programs/simple-conversation/about\n");
    printf("***** DESCRIPTION *****\n");
    printf("An example program for creating a conversation with the given TextToText LLM model.\n");
    printf("The given implementation is stable and shows the basics of non-blocking LLM inference.\n");
    printf("The code section here will mostly be used by other example programs.\n");
    printf("========================================\n\n");
    printf("Usage: mbase_simple_conversation <model_path> *[<option> [<value>]]\n");
    printf("       mbase_simple_conversation model.gguf\n");
    printf("       mbase_simple_conversation model.gguf -gl 80\n");
    printf("       mbase_simple_conversation model.gguf -gl 80 -sys 'You are a helpful assistant.'\n");
    printf("Options: \n\n");
    printf("--help                            Print usage.\n");
    printf("-v, --version                     Shows program version.\n");
    printf("-sys, --system-prompt <str>       LLM system prompt (if this option is given after -fsys, it will overwrite it).\n");
    printf("-fsys, --system-prompt-file <str> LLM system prompt file (if this option is given after -sys, it will overwrite it).\n");
    printf("-t, --thread-count <int>          Amount of threads to use for output processing (default=16).\n");
    printf("-bt, --batch-thread-count <int>   Amount of threads to use for initial batch processing (default=8).\n");
    printf("-c, --context-length <int>        Total context length (default=8192).\n");
    printf("-b, --batch-length <int>          Batch length (default=4096).\n");
    printf("-gl, --gpu-layers <int>           GPU layers to offload to (default=999).\n");
    printf("-tk, --top-k <int>                Top k tokens to pick from (default=20, min=1, max=<model_vocabulary_size>).\n");
    printf("-tp, --top-p <float>              Token probability at most (default=1.0, min=0.1, max=1.0).\n");
    printf("-mp, --min-p <float>              Token probability at least (default=0.3), min=0.1, max=1.0.\n");
    printf("-pn, --penatly-n <int>            Apply repetition penalty on last 'n' tokens (default=64).\n");
    printf("-pr, --penalty-repeat <float>     Discourages repeating exact tokens based on their past presence (default=1.15, min=1.0, max=2.0).\n");
    printf("-temp, --temperature <float>      Higher values increase the randomness (default=0.1, min=0.1, max=1.4).\n");
    printf("-s, --seed <int>                  If it is 0, the program will generate an arbitrary number (default=0).\n");
    printf("-gr, --greedy                     Ignore all sampling techniques, pick the most probable token. (default=false).\n\n");
}

class ConversationModel : public InfModelTextToText {
public:
    GENERIC on_initialize_fail([[maybe_unused]] init_fail_code out_fail_code) override{}
    GENERIC on_initialize() override{}
    GENERIC on_destroy() override{}
private:
};

class ConversationProcessor : public InfProcessorTextToText {
public:
    GENERIC on_initialize() override{}
    GENERIC on_initialize_fail([[maybe_unused]] last_fail_code out_code) override
    {
        fflush(stdout);
        printf("ERR: Context initialization failed.\n");
        printf("ERR: Insufficent memory.\n");
        exit(1);
    }
    GENERIC on_destroy() override{}
private:
};

class ConversationClient : public InfClientTextToText {
public:
    ConversationClient(const mbase::string& in_system_prompt)
    {
        mSystemPromptString = in_system_prompt;
    }

    GENERIC start_conversation(ConversationProcessor* in_processor)
    {
		#ifdef MBASE_PLATFORM_WINDOWS
			SetConsoleCP(CP_UTF8);
			SetConsoleOutputCP(CP_UTF8);
		#endif		
        printf("User >> ");
	
        mbase::string userPrompt = mbase::get_line();
		
        mbase::inf_text_token_vector tokenVector;
        context_line ctxLine;
        ctxLine.mRole = mbase::context_role::USER;
        ctxLine.mMessage = userPrompt;
        in_processor->tokenize_input(&ctxLine, 1, tokenVector);
        if(in_processor->execute_input(tokenVector) == ConversationProcessor::flags::INF_PROC_ERR_INPUT_EXCEED_TOKEN_LIMIT)
        {   
        }
    }

    GENERIC on_register(InfProcessorBase* out_processor) override
    {
        ConversationProcessor* hostProcessor = static_cast<ConversationProcessor*>(out_processor);
        gGlobalProcessor = hostProcessor;
        hostProcessor->set_manual_caching(true, ConversationProcessor::cache_mode::KV_LOCK_MODE);
        printf("System >> %s\n", mSystemPromptString.c_str());
        mbase::context_line ctxLine;
        ctxLine.mRole = mbase::context_role::SYSTEM;
        ctxLine.mMessage = mSystemPromptString;

        inf_text_token_vector tokenVector;
        hostProcessor->tokenize_input(&ctxLine, 1, tokenVector);
        hostProcessor->execute_input(tokenVector, true);
    }

    GENERIC on_unregister([[maybe_unused]] InfProcessorBase* out_processor) override{}
    
    GENERIC on_batch_processed(InfProcessorTextToText* out_processor, [[maybe_unused]] const U32& out_proc_batch_length, [[maybe_unused]] const bool& out_is_kv_locked) override
    {
        ConversationProcessor* hostProcessor = static_cast<ConversationProcessor*>(out_processor);
        if(out_is_kv_locked)
        {
            hostProcessor->set_manual_caching(true, ConversationProcessor::cache_mode::AUTO_LOGIT_STORE_MODE);
            start_conversation(hostProcessor);
        }

        else
        {
            mbase::decode_behavior_description dbd;
            dbd.mHaltDelay = 1;
            dbd.mTokenAtMost = 1;
            dbd.mHaltOnWrite = false;
            hostProcessor->next(dbd);
        }
    }

    GENERIC on_write(InfProcessorTextToText* out_processor, [[maybe_unused]] const inf_text_token_vector& out_token, bool out_is_finish) override
    {
        ConversationProcessor* hostProcessor = static_cast<ConversationProcessor*>(out_processor);
        mbase::vector <mbase::inf_token_description> tokenDesc;
        mbase::decode_behavior_description dbd;
        dbd.mHaltDelay = 1;
        dbd.mTokenAtMost = 1;
        dbd.mHaltOnWrite = false;
        hostProcessor->tokens_to_description_vector(out_token, tokenDesc);
        for(auto& n : tokenDesc)
        {
            if(n.mIsSpecial)
            {
                if(out_is_finish)
                {
                    return;
                }
            }
            fflush(stdout);
            printf("%s", n.mTokenString.c_str());
        }
        hostProcessor->next(dbd);
    }

    GENERIC on_finish(InfProcessorTextToText* out_processor, [[maybe_unused]] size_type out_total_token_size, [[maybe_unused]] InfProcessorTextToText::finish_state out_finish_state) override
    {
        printf("\n\n");
        ConversationProcessor* hostProcessor = static_cast<ConversationProcessor*>(out_processor);
        start_conversation(hostProcessor);
    }
private:
    mbase::string mSystemPromptString;
};

GENERIC catching_interrupt_signal([[maybe_unused]] I32 out_sig_id)
{
    printf("Program interrupted\n");
    
    if(gGlobalProcessor)
    {
        InfProcT2TDiagnostics& t2tDiag = gGlobalProcessor->get_diagnostics();
        printf("\n==== Processor diagnostics ====\n");
        printf("| Context load delay         | %lu ms\n", t2tDiag.loadTimeInMilliseconds);
        printf("| Prompt processing rate(pp) | %f tokens/sec\n", t2tDiag.ppTokensPerSecond);
        printf("| Token generation rate(tg)  | %f tokens/sec\n", t2tDiag.evalTokensPerSecond); 
    }
    exit(0);
    gIsProgramRunning = false;
}

int main(int argc, char** argv)
{
    std::setlocale(LC_ALL, "en_US.UTF-8"); 

    if(argc < 2)
    {
        printf("ERR: Model file is not supplied\n");
        print_usage();
        return 1;
    }

    mbase::argument_get<mbase::string>::value(0, argc, argv, gSampleParams.mModelFile);
    
    if(gSampleParams.mModelFile == "-h" || gSampleParams.mModelFile == "--help")
    {
        print_usage();
        return 1;
    }

    if(gSampleParams.mModelFile == "-v" || gSampleParams.mModelFile == "--version")
    {
        printf("MBASE Simple conversation %s\n", MBASE_SIMPLE_CONVERSATION_VERSION);
        return 0;
    }

    if(!mbase::is_file_valid(mbase::from_utf8(gSampleParams.mModelFile)))
    {
        printf("ERR: Can't open model file: %s\n", gSampleParams.mModelFile.c_str());
        return 1;
    }

    for(I32 i = 2; i < argc; i++)
    {
        mbase::string argumentString = argv[i];
        if(argumentString == "-v" || argumentString == "--version")
        {
            printf("MBASE Simple conversation %s\n", MBASE_SIMPLE_CONVERSATION_VERSION);
            return 0;
        }

        else if(argumentString == "-sys" || argumentString == "--system-prompt")
        {
            mbase::argument_get<mbase::string>::value(i, argc, argv, gSampleParams.mSystemPrompt);
        }

        else if(argumentString == "-fsys" || argumentString == "--system-prompt-file")
        {
            mbase::argument_get<mbase::string>::value(i, argc, argv, gSampleParams.mSystemPromptFile);
            if(!mbase::is_file_valid(mbase::from_utf8(gSampleParams.mSystemPromptFile)))
            {
                printf("ERR: Can't open system prompt file: %s\n", gSampleParams.mSystemPromptFile.c_str());
                return 1;
            }
            gSampleParams.mSystemPrompt = mbase::read_file_as_string(mbase::from_utf8(gSampleParams.mSystemPromptFile));
        }

        else if(argumentString == "--thread-count" || argumentString == "-t")
        {
            mbase::argument_get<I32>::value(i, argc, argv, gSampleParams.mThreadCount);
        }

        else if(argumentString == "--batch-thread-count" || argumentString == "-bt")
        {
            mbase::argument_get<I32>::value(i, argc, argv, gSampleParams.mBatchThreadCount);
        }

        else if(argumentString == "--context-length" || argumentString == "-c")
        {
            mbase::argument_get<I32>::value(i, argc, argv, gSampleParams.mContextLength);
        }

        else if(argumentString == "--batch-length" || argumentString == "-b")
        {
            mbase::argument_get<I32>::value(i, argc, argv, gSampleParams.mBatchLength);
        }

        else if(argumentString == "--gpu-layers" || argumentString == "-gl")
        {
            mbase::argument_get<I32>::value(i, argc, argv, gSampleParams.mGpuLayer);
        }

        else if(argumentString == "-tk" || argumentString == "--top-k")
        {
            mbase::argument_get<U32>::value(i, argc, argv, gSampleParams.mTkSampler.mTopK);
        }

        else if(argumentString == "-tp" || argumentString == "--top-p")
        {
            mbase::argument_get<F32>::value(i, argc, argv, gSampleParams.mTpSampler.mTopP);
        }

        else if(argumentString == "-mp" || argumentString == "--min-p")
        {
            mbase::argument_get<F32>::value(i, argc, argv, gSampleParams.mMpSampler.mMinP);
        }

        else if(argumentString == "-pn" || argumentString == "--penalty-n")
        {
            mbase::argument_get<U32>::value(i, argc, argv, gSampleParams.mPenalty.mRepetition.mPenaltyN);
        }

        else if(argumentString == "-pr" || argumentString == "--penalty-repeat")
        {
            mbase::argument_get<F32>::value(i, argc, argv, gSampleParams.mPenalty.mRepetition.mRepeatPenalty);
        }

        else if(argumentString == "-temp" || argumentString == "--temperature")
        {
            mbase::argument_get<F32>::value(i, argc, argv, gSampleParams.mTmp.mTemp);
        }

        else if(argumentString == "-s" || argumentString == "--seed")
        {
            mbase::argument_get<U32>::value(i, argc, argv, gSampleParams.mSeed.mRng);
        }

        else if(argumentString == "-gr" || argumentString == "--greedy")
        {
            gSampleParams.mIsGreeady = true;
        }
        
        else if(argumentString == "--help")
        {
            print_usage();
            return 1;
        }
    }

    if(gSampleParams.mThreadCount <= 0)
    {
        printf("ERR: Thread must be at least 1 but given: %d\n", gSampleParams.mThreadCount);
        return 1;   
    }
    
    if(gSampleParams.mBatchThreadCount <= 0)
    {
        printf("ERR: Batch thread must be at least 1 but given: %d\n", gSampleParams.mBatchThreadCount);
        return 1;
    }

    deviceDescription = inf_query_devices();

    ConversationModel cnvModel;
    ConversationProcessor cnvProcessor;
    ConversationClient cnvClient(gSampleParams.mSystemPrompt);
    
    cnvModel.initialize_model_ex_sync(mbase::from_utf8(gSampleParams.mModelFile), 1200000, gSampleParams.mGpuLayer, true, true, deviceDescription);
    
    if(!cnvModel.is_initialized())
    {
        printf("ERR: Unable to initialize model\n");
        return 1;
    }

    inf_sampling_set samplingSet;
    if(!gSampleParams.mIsGreeady)
    {
        samplingSet.insert(gSampleParams.mSeed);
        samplingSet.insert(gSampleParams.mTkSampler);
        samplingSet.insert(gSampleParams.mTpSampler);
        samplingSet.insert(gSampleParams.mMpSampler);
        samplingSet.insert(gSampleParams.mPenalty);
        samplingSet.insert(gSampleParams.mTmp);
    }

    cnvModel.register_context_process(
        &cnvProcessor,
        gSampleParams.mContextLength,
        gSampleParams.mBatchLength,
        gSampleParams.mThreadCount,
        gSampleParams.mBatchThreadCount,
        true,
        samplingSet
    );

    // Waiting for processor registration
    while(!cnvProcessor.is_registered())
    {
        cnvProcessor.update();
        mbase::sleep(2);
    }

    signal(SIGINT, catching_interrupt_signal);
    mbase::string modelName = cnvModel.get_model_name();
    printf("==== Session Information ====\n\n");
    printf("- Model name: %s\n", modelName.c_str());
    printf("- Context length: %u\n", gSampleParams.mContextLength);
    printf("- Batch size: %u\n", gSampleParams.mBatchLength);
    printf("- Batch processing threads: %u\n", gSampleParams.mBatchThreadCount);
    printf("- Generation threads: %u\n", gSampleParams.mThreadCount);
    printf("- Compute devices: \n");

    for(InfDeviceDescription& tmpDescription : deviceDescription)
    {
        // mbase::string typeString;
        // switch (tmpDescription.get_device_type())
        // {
        // case InfDeviceDescription::device_type::CPU:
        //     typeString = "CPU";
        //     break;
        // case InfDeviceDescription::device_type::GPU:
        //     typeString = "GPU";
        //     break;
        // case InfDeviceDescription::device_type::CUSTOM:
        //     typeString = "CUSTOM";
        //     break;
        // case InfDeviceDescription::device_type::UNKNOWN:
        //     typeString = "UNKNOWN";
        //     break;
        // default:
        //     break;
        // }
        //printf("\t%s ## Type: %s\n", tmpDescription.get_device_description().c_str(), typeString.c_str());
        printf("\t%s ## Type: %s\n", tmpDescription.get_device_description().c_str(), tmpDescription.get_device_name().c_str());
    }

    

    printf("- Samplers in order: \n");
    if(gSampleParams.mIsGreeady)
    {
        printf("\t Greedy\n");
    }
    else
    {
        for(auto& tmpSampler : samplingSet)
        {
            if(tmpSampler.mSamplerType == InfSamplerDescription::SAMPLER::TOP_K)
            {
                printf("\t Top k: %d\n", tmpSampler.mTopK);
            }
            else if(tmpSampler.mSamplerType == InfSamplerDescription::SAMPLER::TOP_P)
            {
                printf("\t Top p: %f\n", tmpSampler.mTopP);
            }
            else if(tmpSampler.mSamplerType == InfSamplerDescription::SAMPLER::MIN_P)
            {
                printf("\t Min p: %f\n", tmpSampler.mMinP);
            }
            else if(tmpSampler.mSamplerType == InfSamplerDescription::SAMPLER::REPETITION)
            {
                printf("\t Penalty N: %d, Penalty Repeat: %f\n", tmpSampler.mRepetition.mPenaltyN, tmpSampler.mRepetition.mRepeatPenalty);
            }
            else if(tmpSampler.mSamplerType == InfSamplerDescription::SAMPLER::TEMP)
            {
                printf("\t Temperature: %f\n", tmpSampler.mTemp);
            }
        }
    }

    while(gIsProgramRunning)
    {
        std::chrono::high_resolution_clock::time_point beginTime = std::chrono::high_resolution_clock::now();
        cnvProcessor.set_inference_client(&cnvClient);
        cnvModel.update();
        std::chrono::high_resolution_clock::time_point endTime = std::chrono::high_resolution_clock::now();
        
        if(std::chrono::duration_cast<std::chrono::milliseconds>(endTime - beginTime).count() < 2)
        {
            mbase::sleep(2);
        }
    }

    cnvProcessor.release_inference_client();
    InfProcT2TDiagnostics& t2tDiag = cnvProcessor.get_diagnostics();
    printf("\n==== Processor diagnostics ====\n");
    printf("| Context load delay         | %lu ms\n", t2tDiag.loadTimeInMilliseconds);
    printf("| Prompt processing rate(pp) | %f tokens/sec\n", t2tDiag.ppTokensPerSecond);
    printf("| Token generation rate(tg)  | %f tokens/sec\n", t2tDiag.evalTokensPerSecond);

    return 0;
}
