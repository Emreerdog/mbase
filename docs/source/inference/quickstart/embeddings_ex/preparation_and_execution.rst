===============================
Input Preperation and Execution
===============================

Now, it is time to read user input through stdin.
Then, make the inference engine work to generate embeddings.

Here are the steps that should be taken to achieve that result:

* Read the input from the terminal by the time the client is registered.
* Tokenize the user input
* Execute the input.
* Normalize the generated embeddings and display the result.

---------------------
Processing User Input
---------------------

In :code:`ClientObject`'s :code:`on_register` method, let's read a string from the terminal:

.. code-block:: cpp
    :caption: main.cpp

    void on_register(mbase::InfProcessorBase* out_processor) override 
    {
        std::cout << "Client registered!" << std::endl;
        std::cout << "Enter your query: ";
        mbase::string clientInput = mbase::get_line();
    }

Now, we will tokenize our input and generate a token vector from that.
Only the token vectors can be supplied to the inference engine to work.

After the tokenization is complete, we will execute our input:

.. code-block:: cpp
    :caption: main.cpp

    void on_register(mbase::InfProcessorBase* out_processor) override 
    {
        std::cout << "Client registered!" << std::endl;
        std::cout << "Enter your query: ";
        mbase::string clientInput = mbase::get_line();

        mbase::InfEmbedderProcessor* tmpProcessor = static_cast<mbase::InfEmbedderProcessor*>(out_processor);
        mbase::inf_text_token_vector tokVec;
        tmpProcessor->tokenize_input(clientInput, tokVec);
        tmpProcessor->execute_input({tokVec});
    }

The call to :code:`execute_input` will not block your main thread but instead it will send a signal to the processor's
parallel update loop. This way, the expensive computation will not block your thread.

After the batch processing is finished, :code:`on_batch_processed` method will be called indicating that the processor computed the 
embeddings.

In order to see the generated embeddings, you need to call the :code:`next` method in the :code:`on_batch_processed` as follows:

.. code-block:: cpp
    :caption: main.cpp

    void on_batch_processed(mbase::InfEmbedderProcessor* out_processor, const uint32_t& out_proc_batch_length) override
    {
        mbase::InfEmbedderProcessor* tmpProcessor = static_cast<mbase::InfEmbedderProcessor*>(out_processor);
        tmpProcessor->next();
    }

This will call the :code:`on_write` method so that you can display the embeddings of the user query:

.. code-block:: cpp
    :caption: main.cpp

    void on_write(mbase::InfEmbedderProcessor* out_processor, float* out_embeddings, const uint32_t& out_cursor, bool out_is_finished) override
    {
        uint32_t embdLength = out_processor->get_embedding_length();
        for(uint32_t i = 0; i < embdLength; i++)
        {
            std::cout << out_embeddings[i] << ", ";
        }
        std::cout << std::endl;
    }

Now, let's normalize the embeddings that are generated by the user query:

.. code-block:: cpp
    :caption: main.cpp

    class ClientObject : public mbase::InfClientEmbedder {
    public:
        ...
        void on_write(mbase::InfEmbedderProcessor* out_processor, float* out_embeddings, const uint32_t& out_cursor, bool out_is_finished) override
        {
            uint32_t embdLength = out_processor->get_embedding_length();
            mbase::inf_common_embd_normalize(out_embeddings, out_embeddings, embdLength);
            for(uint32_t i = 0; i < embdLength; i++)
            {
                std::cout << out_embeddings[i] << ", ";
            }
            std::cout << std::endl;
        }
        ...
    };

Now congratulations! You have used the inference engine to get a response from the LLM.

Here is the total main.cpp file:

.. code-block:: cpp
    :caption: main.cpp

    #include <mbase/inference/inf_device_desc.h>
    #include <mbase/inference/inf_t2t_model.h>
    #include <mbase/inference/inf_embedder.h>
    #include <mbase/inference/inf_embedder_client.h>
    #include <iostream>
    #include <mbase/vector.h>

    bool gIsRunning = true;

    class ModelObject;
    class ProcessorObject;
    class ClientObject;

    class ClientObject : public mbase::InfClientEmbedder {
    public:
        void on_register(mbase::InfProcessorBase* out_processor) override 
        {
            std::cout << "Client registered!" << std::endl;
            std::cout << "Enter your query: ";
            mbase::string clientInput = mbase::get_line();

            mbase::InfEmbedderProcessor* tmpProcessor = static_cast<mbase::InfEmbedderProcessor*>(out_processor);
            mbase::inf_text_token_vector tokVec;
            tmpProcessor->tokenize_input(clientInput, tokVec);
            tmpProcessor->execute_input({tokVec});
        }

        void on_unregister(mbase::InfProcessorBase* out_processor) override {}

        void on_batch_processed(mbase::InfEmbedderProcessor* out_processor, const uint32_t& out_proc_batch_length) override
        {
            std::cout << "Embedding calculation is finished!!" << std::endl;
            mbase::InfEmbedderProcessor* tmpProcessor = static_cast<mbase::InfEmbedderProcessor*>(out_processor);

            tmpProcessor->next();
        }

        void on_write(mbase::InfEmbedderProcessor* out_processor, float* out_embeddings, const uint32_t& out_cursor, bool out_is_finished) override
        {
            uint32_t embdLength = out_processor->get_embedding_length();
            mbase::inf_common_embd_normalize(out_embeddings, out_embeddings, embdLength);
            
            for(uint32_t i = 0; i < embdLength; i++)
            {
                std::cout << out_embeddings[i] << ", " << std::endl;
            }
            std::cout << std::endl;
        }

        void on_finish(mbase::InfEmbedderProcessor* out_processor, const size_type& out_total_processed_embeddings) override {}
    };

    class ProcessorObject : public mbase::InfEmbedderProcessor {
    public:
        void on_initialize_fail(last_fail_code out_code) override
        {
            std::cout << "Processor initialization failed." << std::endl;
            gIsRunning = false;
        }

        void on_initialize() override
        {
            std::cout << "Processor is initialized." << std::endl;
            this->set_inference_client(&clientObject); // registering the client
        }

        void on_destroy() override
        {

        }
    private:
        ClientObject clientObject;
    };

    class ModelObject : public mbase::InfModelTextToText {
    public:
        void on_initialize_fail(init_fail_code out_fail_code) override
        {
            std::cout << "Model initialization failed." << std::endl;
            gIsRunning = false;
        }

        void on_initialize() override
        {
            std::cout << "Model is initialized." << std::endl;

            uint32_t contextSize = 0;
            uint32_t procThreadCount = 16;
            this->get_max_embedding_context(contextSize);
            ModelObject::flags registerationStatus = this->register_context_process(
                &processorObject,
                contextSize,
                procThreadCount
            );

            if(registerationStatus != ModelObject::flags::INF_MODEL_INFO_REGISTERING_PROCESSOR)
            {
                std::cout << "Registration unable to proceed." << std::endl;
                gIsRunning = false;
            }
        }
        void on_destroy() override{}
    private:
        ProcessorObject processorObject;
    };

    int main()
    {
        mbase::vector<mbase::InfDeviceDescription> deviceDesc = mbase::inf_query_devices();
        for(mbase::vector<mbase::InfDeviceDescription>::iterator It = deviceDesc.begin(); It != deviceDesc.end(); It++)
        {
            std::cout << It->get_device_description() << std::endl;
        }

        ModelObject modelObject;

        uint32_t totalContextLength = 32000;
        int32_t gpuLayersToUse = 80;
        bool isMmap = true;
        bool isMLock = true;

        if (modelObject.initialize_model_ex(
            L"<path_to_your_model>",
            totalContextLength,
            gpuLayersToUse,
            isMmap,
            isMLock,
            deviceDesc
        ) != ModelObject::flags::INF_MODEL_INFO_INITIALIZING_MODEL)
        {
            std::cout << "Unable to start initializing the model." << std::endl;
            return 1;
        }

        while(gIsRunning)
        {
            modelObject.update();
            mbase::sleep(2);
        }

        return 0;
    }