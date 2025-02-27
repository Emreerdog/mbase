#include <mbase/inference/inf_embedder.h>
#include <mbase/inference/inf_embedder_client.h>
#include <mbase/inference/inf_t2t_model.h>
#include <mbase/argument_get_value.h>
#include <mbase/filesystem.h>
#include <mbase/io_file.h>
#include <mbase/json/json.h>
#include <iostream>

#define MBASE_EMBEDDING_SIMPLE_VERSION "v0.1.0"

using namespace mbase;

class EmbedderModel;
class EmbedderProcessor;
class EmbedderClient;

struct program_parameters {
    mbase::string mModelFile; // direct argument
    mbase::string mPromptFile; // -pf, --prompt-file
    mbase::string mPrompt; // -p , --prompt
    mbase::string mSeperator = "<embd_sep>"; // -sp, --seperator == prompt content seperator (default=<mbembd_sep>)
    mbase::string mJout; // -jout, --json-output-path
    I32 mThreadCount = 16; // -t, --thread-count
    I32 mGpuLayer = 999;
};

mbase::vector<mbase::vector<F32>> gAllGeneratedEmbeddings;
mbase::vector<mbase::string> gLoadedPrompts;
mbase::vector<mbase::string>::iterator gPromptIt;
program_parameters gSampleParams;
mbase::string gModelName;
I32 gEmbeddingIndex = 0;
I32 gTotalPromptTokenLength = 0; // For openai_embeddings.json

GENERIC print_usage();

GENERIC print_usage()
{
    printf("========================================\n");
    printf("#Program name:      mbase_embedding_simple\n");
    printf("#Version:           %s\n", MBASE_EMBEDDING_SIMPLE_VERSION);
    printf("#Type:              Example\n");
    printf("#Further docs:      https://docs.mbasesoftware.com/inference/programs/embedding/about\n");
    printf("***** DESCRIPTION *****\n");
    printf("An example program for generating the embeddings of the given prompt or prompts.\n");
    printf("The given implementation is stable and shows the basics of how to generate embeddings using MBASE embedding procesor.\n");
    printf("========================================\n\n");
    printf("Usage: mbase_embedding_simple <model_path> *[<option> [<value>]]\n");
    printf("       mbase_embedding_simple model.gguf -gl 80 -p 'What is life?'\n");
    printf("       mbase_embedding_simple model.gguf -gl 80 -pf prompt1.txt -pf prompt2.txt\n");
    printf("Options: \n\n");
    printf("-h, --help                      Print usage.\n");
    printf("-v, --version                   Shows program version.\n");
    printf("-pf, --prompt-file <str>        File containing prompt or prompts seperated by the seperator (default=''). If prompt is given, prompt file will be ignored\n");
    printf("-p, --prompt <str>              Prompt or prompts seperated by the seperator (default=''). This will be used even if the prompt file is supplied\n");
    printf("-sp, --seperator <str>          Prompt seperator (default=\"<embd_sep>\").\n");
    printf("-t, --thread-count <int>        Threads used to compute embeddings (default=16).\n");
    printf("-gl, --gpu-layers <int>         GPU layers to offload to (default=999).\n");
    printf("-jout, --json-output-path <str> If the json output path is specified, result will be written there in file(openai_embeddings.json) (default='').\n\n");
}

class EmbedderModel : public InfModelTextToText {
public:
    GENERIC on_initialize_fail([[maybe_unused]] init_fail_code out_fail_code) override
    {
        printf("ERR: Failed loading model.\n");
        exit(1);
    }
    GENERIC on_initialize() override
    {
        gModelName = this->get_model_name();
    }
    GENERIC on_destroy() override{}
private:
};

class EmbedderProcessor : public InfEmbedderProcessor {
public:
    GENERIC on_initialize() override{}
    GENERIC on_destroy() override{}
private:

};

class EmbedderClient : public InfClientEmbedder {
public:
    GENERIC on_register(InfProcessorBase* out_processor) override
    {
        gPromptIt = gLoadedPrompts.begin();
        InfEmbedderProcessor* hostProc = static_cast<InfEmbedderProcessor*>(out_processor);
        mbase::inf_text_token_vector tokVec;
        hostProc->tokenize_input(*gPromptIt, tokVec);
        gTotalPromptTokenLength += tokVec.size();
        if(hostProc->execute_input({tokVec}) == InfEmbedderProcessor::flags::INF_PROC_ERR_INPUT_EXCEED_TOKEN_LIMIT)
        {
            printf("ERR: Given prompt's token length is greater than embedder context length.\n");
            exit(1);
        }
        ++gPromptIt;
    }

	GENERIC on_unregister([[maybe_unused]] InfProcessorBase* out_processor) override
    {
    }

    GENERIC on_batch_processed(InfEmbedderProcessor* out_processor, [[maybe_unused]] const U32& out_proc_batch_length) override
    {
        InfEmbedderProcessor* hostProc = static_cast<InfEmbedderProcessor*>(out_processor);
        hostProc->next();
    }

    GENERIC on_write(InfEmbedderProcessor* out_processor, PTRF32 out_embeddings, [[maybe_unused]] const U32& out_cursor, [[maybe_unused]] bool out_is_finished) override
    {
        printf("Index %d: ", gEmbeddingIndex);
        mbase::vector<F32> generatedEmbeddings;
        inf_common_embd_normalize(out_embeddings, out_embeddings, out_processor->get_embedding_length());
        I32 printLength = 8;
        for(U32 i = 0; i < out_processor->get_embedding_length(); i++)
        {
            generatedEmbeddings.push_back(out_embeddings[i]);
            if(printLength)
            {
                printf("%f ", out_embeddings[i]);
                --printLength;
            }
        }
        gAllGeneratedEmbeddings.push_back(generatedEmbeddings);
        printf("... (%d more)\n", out_processor->get_embedding_length() - 8);
    }

    GENERIC on_finish(InfEmbedderProcessor* out_processor, [[maybe_unused]] const size_type& out_total_processed_embeddings) override
    {
        ++gEmbeddingIndex;
        InfEmbedderProcessor* hostProc = static_cast<InfEmbedderProcessor*>(out_processor);
        if(gPromptIt != gLoadedPrompts.end())
        {
            mbase::inf_text_token_vector tokVec;
            hostProc->tokenize_input(*gPromptIt, tokVec);
            gTotalPromptTokenLength += tokVec.size();
            if(hostProc->execute_input({tokVec}) == InfEmbedderProcessor::flags::INF_PROC_ERR_INPUT_EXCEED_TOKEN_LIMIT)
            {
                printf("ERR: Given prompt's token length is greater than embedder context length.\n");
                exit(1);
            }
            ++gPromptIt;
        }
        else
        {
            if(gSampleParams.mJout.size())
            {
                mbase::io_file iof;
                gSampleParams.mJout += "openai_embeddings.json";
                iof.open_file(mbase::from_utf8(gSampleParams.mJout));
                if(!iof.is_file_open())
                {
                    printf("ERR: Unable to write embeddings to openai_embeddings.json\n");
                    exit(1);
                }
                mbase::Json openaiJson;
                openaiJson.setObject();
                openaiJson["object"] = "list";
                openaiJson["model"] = gModelName;
                openaiJson["usage"]["prompt_tokens"] = gTotalPromptTokenLength;
                openaiJson["usage"]["total_tokens"] = gTotalPromptTokenLength;
                openaiJson["data"].setArray();
                
                for(size_type i = 0; i < gAllGeneratedEmbeddings.size(); ++i)
                {
                    mbase::Json currentData;

                    currentData["object"] = "embedding",
                    currentData["index"] = i;
                    
                    mbase::Json embeddingArray;
                    embeddingArray.setArray();
                    
                    mbase::vector<F32> &activeVector = gAllGeneratedEmbeddings[i];
                    for(size_type j = 0; j < activeVector.size(); ++j)
                    {
                        currentData["embedding"][j] = activeVector[j];
                    }
                    openaiJson["data"][i] = currentData;
                }
                iof.write_data(openaiJson.toStringPretty());
            }
            exit(0);
        }
    }
private:
};

int main(int argc, char** argv)
{   
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
        printf("MBASE Embedding Simple %s\n", MBASE_EMBEDDING_SIMPLE_VERSION);
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
        if(argumentString == "--help" || argumentString == "-h")
        {
            print_usage();
            return 1;
        }

        else if(argumentString == "-v" || argumentString == "--version")
        {
            printf("MBASE Embedding Simple %s\n", MBASE_EMBEDDING_SIMPLE_VERSION);
            return 0;
        }

        else if(argumentString == "-pf" || argumentString == "--prompt-file")
        {
            mbase::argument_get<mbase::string>::value(i, argc, argv, gSampleParams.mPromptFile);
        }

        else if(argumentString == "-p" || argumentString == "--prompt")
        {
            mbase::argument_get<mbase::string>::value(i, argc, argv, gSampleParams.mPrompt);
        }

        else if(argumentString == "-sp" || argumentString == "--seperator")
        {
            mbase::argument_get<mbase::string>::value(i, argc, argv, gSampleParams.mSeperator);
        }

        else if(argumentString == "-t" || argumentString == "--thread-count")
        {
            mbase::argument_get<I32>::value(i, argc, argv, gSampleParams.mThreadCount);
        }

        else if(argumentString == "-gl" || argumentString == "--gpu-layers")
        {
            mbase::argument_get<I32>::value(i, argc, argv, gSampleParams.mGpuLayer);
        }

        else if(argumentString == "-jout" || argumentString == "--json-output-path")
        {
            mbase::argument_get<mbase::string>::value(i, argc, argv, gSampleParams.mJout);
        }
    }

    if(!gSampleParams.mPrompt.size())
    {
        if(!gSampleParams.mPromptFile.size())
        {
            printf("ERR: Prompt is missing.\n");
            return 1;
        }

        if(mbase::is_file_valid(mbase::from_utf8(gSampleParams.mPromptFile)))
        {
            printf("ERR: Unable to open prompt file: %s\n", gSampleParams.mPromptFile.c_str());
            return 1;
        }

        gSampleParams.mPrompt = mbase::read_file_as_string(mbase::from_utf8(gSampleParams.mPromptFile));
        if(!gSampleParams.mPrompt.size())
        {
            printf("ERR: Prompt is missing.\n");
            return 1;
        }
    }

    gSampleParams.mPrompt.split_full(gSampleParams.mSeperator, gLoadedPrompts);

    EmbedderModel embdModel;
    EmbedderProcessor embdProcessor;
    EmbedderClient embdClient;
    embdModel.initialize_model_sync(mbase::from_utf8(gSampleParams.mModelFile), 32000, -1);
    embdModel.update();

    if(!embdModel.is_embedding_model())
    {
        printf("ERR: Loaded model is not embedding model.\n");
        printf("INFO: Embedding simple program does not generate token embeddings\n");
        return 1;
    }

    U32 ctxLength = embdModel.get_max_embedding_context();
    embdModel.register_context_process(&embdProcessor, ctxLength, gSampleParams.mThreadCount);

    while(1)
    {
        embdProcessor.set_inference_client(&embdClient);
        embdProcessor.update();    
        mbase::sleep(2);
    }

    return 0;
}
