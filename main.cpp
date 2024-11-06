﻿#include <iostream>
#include <mbase/mbase_std.h>
#include <mbase/pc/pc_program.h>
#include <mbase/pc/pc_state.h>
#include <mbase/pc/pc_config.h>
#include <mbase/pc/pc_diagnostics.h>
#include <mbase/inference/inf_client.h>
#include <mbase/inference/inf_processor.h>
#include <mbase/inference/inf_program.h>
#include <mbase/inference/inf_maip_server.h>
#include <mbase/inference/inf_gguf_metadata_configurator.h>
#include <vector>
#include <set>
#include <unordered_map>
#include <signal.h>

using namespace mbase;

class my_client : public mbase::InfClientTextToText {
public:
    GENERIC on_register(InfTextToTextProcessor* out_processor) override 
    {
        std::cout << "Client is registered" << std::endl;
        U32 outMsgId = 0;
        this->add_message(L"You are a helpful assistant.", context_role::SYSTEM, outMsgId);
        msgIds.push_back(outMsgId);
        mbase::io_file iof;
        iof.open_file(L"./cmake_install.cmake", mbase::io_file::access_mode::READ_ACCESS, mbase::io_file::disposition::OPEN);

        mbase::deep_char_stream dcs(iof.get_file_size());
        iof.read_data(dcs);

        if(iof.is_file_open())
        {
            std::cout << "File is open" << std::endl;
        }
 
        mbase::string cmakeData(dcs.get_buffer(), dcs.buffer_length());

        std::cout << cmakeData << std::endl;

        std::cout << (I32)this->add_message(dcs.get_buffer(), dcs.buffer_length(), context_role::USER, outMsgId) << std::endl;
        msgIds.push_back(outMsgId);
        
        mbase::vector<context_line> txtMessages;
        this->get_message_array(msgIds.data(), msgIds.size(), txtMessages);
        
        InfTextToTextProcessor* txtOut;
        get_host_processor(txtOut);

        InfClientTextToText::token_vector tv;
        std::cout << (I32)txtOut->tokenize_input(txtMessages.data(), txtMessages.size(), tv) << std::endl;
        std::cout << (I32)txtOut->execute_input(tv, true) << std::endl;
        txtOut->next();
    }

    GENERIC on_write(CBYTEBUFFER out_data, size_type out_size, InfTextToTextProcessor::inf_token out_token, bool out_is_special, bool out_is_finish) override 
    {        
        fflush(stdout);
        printf("%s", out_data);
        totalMessage += out_data;
        InfTextToTextProcessor* txtOut;
        get_host_processor(txtOut);
        txtOut->next();
    }

    GENERIC on_finish(size_type out_total_token_size, InfTextToTextProcessor::finish_state out_finish_state) override 
    {
        
    }
    
    GENERIC on_unregister() override 
    {

    }
private:
    mbase::vector<U32> msgIds;
    mbase::string totalMessage;
};

class my_context : public mbase::InfTextToTextProcessor {
public:
    GENERIC on_initializing() override
    {
        std::cout << "WE ARE INITIALZINING!!" << std::endl;
    }

    GENERIC on_initialize_fail(init_fail_code out_code) override
    {
        std::cout << "INITIALIZE FAILED ;(" << std::endl;
    }

    GENERIC on_initialize() override
    {
        std::cout << "eeeee" << std::endl;
        this->set_inference_client(&myCl);
    }
    GENERIC on_destroy() override
    {
        std::cout << "I am dead" << std::endl;
    }
private:
    my_client myCl;
};

class my_model : public mbase::InfModelTextToText {
public:
    void on_initialize() override 
    {
        InfSamplerDescription topK;
        InfSamplerDescription topP;
        InfSamplerDescription minP;
        InfSamplerDescription temperature;
        InfSamplerDescription mirostat;
        InfSamplerDescription repeatControl;

        topK.mTopK = 40;
        topK.mSamplerType = InfSamplerDescription::SAMPLER::TOP_K;

        topP.mTopP = 0.9;
        topP.mSamplerType = InfSamplerDescription::SAMPLER::TOP_P;

        minP.mMinP = 0.1;
        minP.mSamplerType = InfSamplerDescription::SAMPLER::MIN_P;

        temperature.mTemp = 0.2;
        temperature.mSamplerType = InfSamplerDescription::SAMPLER::TEMP;

        InfSamplingRepetition repeatSampler;
        repeatSampler.mPenaltyLinefeed = true;
        repeatSampler.mRepeatPenalty = 1.5f;
        repeatSampler.mPenaltyFrequency = 1.0f;
        repeatSampler.mPenaltyPresent = 1.0f;

        repeatControl.mSamplerType = InfSamplerDescription::SAMPLER::REPETITION;
        repeatControl.mRepetition = repeatSampler;

        this->register_context_process(
            &myContext, 
            64000, 
            4096, 
            16, 
            4, 
            true,
            {topK, topP, minP, temperature, mirostat}
        );
    }
    void on_destroy() override {}
private:
    my_context myContext;
};

using namespace mbase;

int main()
{
    while(1)
    {
        int i = 0;
        my_model mm;
        mm.initialize_model(L"./Llama-3.2-1B-Instruct-Q4_K_M.gguf", 512000, 999);
        while(1)
        {
            mm.update();
        }
    
    }

    /*InfProgram mainProgram;
    mbase::InfProgramInformation programInformation;

    programInformation.mProgramInformation.mMbaseVersion = "1.0.0";
    programInformation.mProgramInformation.mProductAuthor = "Mustafa Emre Erdog";
    programInformation.mProgramInformation.mProductId = "12345";
    programInformation.mProgramInformation.mProductLicence = "MIT";
    programInformation.mProgramInformation.mProductName = "MBASE Inference Engine";
    programInformation.mProgramInformation.mProductVersion = "0.1.0";
    programInformation.mProgramInformation.mVersionMajor = 0;
    programInformation.mProgramInformation.mVersionMinor = 1;
    programInformation.mProgramInformation.mVersionPatch = 0;

    programInformation.mDataPath = L"./";
    programInformation.mExecutionPath = L"./";
    programInformation.mTempPath = L"./";

    mainProgram.initialize(programInformation);
    mbase::string outAccToken;
    mainProgram.create_user("admin", 9, 4096, true, false, "yumniye99", MAIP_ADAPTER_LOAD_UNLOAD, true, outAccToken);
    mainProgram.create_user("emreerdog", 3, 4096, false, false, "159753", MAIP_MODEL_LOAD_UNLOAD | MAIP_ADAPTER_LOAD_UNLOAD, true, outAccToken);
    mainProgram.create_user("qwen2-tooler", 1, 4096, false, false, "1df215a571", MAIP_MODEL_LOAD_UNLOAD | MAIP_ADAPTER_LOAD_UNLOAD, true, outAccToken);
    mainProgram.create_user("llama3", 5, 4096, false, false, "yumniye99", MAIP_MODEL_LOAD_UNLOAD | MAIP_ADAPTER_LOAD_UNLOAD, true, outAccToken);

    mainProgram.authorize_user_on_model("emreerdog", "qwen2-7b-instruct-q3_k_m.gguf");

    mbase::InfMaipDefaultServer defaultMaipServer(mainProgram);
    mainProgram.get_net_manager()->create_server("127.0.0.1", 4553, defaultMaipServer);
    defaultMaipServer.listen();
    while(1)
    {
        defaultMaipServer.update();
        mainProgram.update();
    }*/

    getchar();
    return 0;
}