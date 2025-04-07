.. include:: links.rst

=================
Inference Library
=================

Welcome to the MBASE inference library documentation!

Github link: https://github.com/Emreerdog/mbase

-------------------
Document Navigation
-------------------

The document is structured into four chapters where each chapter has a distinct purpose.
Those chapters are as follows:

* :doc:`Quickstart <quick_start>`: This chapter starts by explaining how to setup your environment, and goes with system requirements identification, library installations and simple project examples to show how to use the inference SDK.

* :doc:`Programs <programs/index>`: This chapter explains the programs that are developed using the inference library. The programs in that chapter are complete executable programs.

* :doc:`Information Reference <info-reference/index>`: This chapter is for explaining the concepts, inference and SDK in detail. It is your goto if you struggle to understand some parts of the code, or struggling to understand the SDK usage, or examples in quickstart chapter. 

.. toctree::
    :hidden:

    Quickstart <quick_start>
    Programs <programs/index>
    Information Reference <info-reference/index>

-----
About
-----

MBASE inference library is a high-level non-blocking LLM inference library written on top
of the `llama.cpp <llama.cpp_>`_ library to provide the necessary tools and APIs to allow developers
to integrate popular LLMs into their applications with minimal performance loss and development time.

The inference SDK will allow developers the utilize the LLMs and create their own solutions,
tooling mechanisms etc.

When the phrase "local LLM inference" is thrown around,
it usually means that hosting an Openai API compatible HTTP server and using the completions API locally.
The MBASE inference library is expected to change this notion by providing you the LLM inference capability 
through its low-level objects and procedures so that 
you can integrate and embedd LLM into your high-performance applications such as games, server applications and many more.

Also you still have the option of hosting Openai server using the :doc:`mbase_openai_server <programs/openai-server/about>` program or you can
code a similar one yourself!

There also is a benchmarking tool developed for testing the performance of the LLM but more specifically,
the impact of the inference operation on your main program loop. For further details refer to :doc:`benchmark <programs/benchmark-t2t/about>` documentation.

--------
Features
--------

* Non-blocking TextToText LLM inference SDK.
* Non-blocking Embedder model inference SDK.
* GGUF file meta-data manipulation SDK.
* Openai server program supporting both TextToText and Embedder endpoints with system prompt caching support which implies significant performance boost.
* Hosting multiple models in a single Openai server program.
* Using `llama.cpp <llama.cpp_>`_ as an inference backend so that models that are supported by the `llama.cpp <llama.cpp_>`_ library are supported by default.
* Benchmark application for measuring the impact of LLM inference on your application.
* Plus anything `llama.cpp <llama.cpp_>`_ supports.

----------------
Supported Models
----------------

Since the MBASE SDK uses `llama.cpp <llama.cpp_>`_ as a backend inference engine,
the models that are supported by `llama.cpp <llama.cpp_>`_ are supported by default
which includes major models such as **Phi**, **Deepseek**, **Llama** and **Qwen**.

You can see the full list `here <llama.cpp_>`_.

---------------------
Implementation Matrix
---------------------

+------------+--------------------------------------------------+--------------------------------------------------+---------------------------+
| Type       | SDK Support                                      | Openai API Support                               | Engine                    |
+============+==================================================+==================================================+===========================+
| TextToText | :octicon:`check-circle-fill;1em;sd-text-success` | :octicon:`check-circle-fill;1em;sd-text-success` | `llama.cpp <llama.cpp_>`_ |
+------------+--------------------------------------------------+--------------------------------------------------+---------------------------+
| Embedder   | :octicon:`check-circle-fill;1em;sd-text-success` | :octicon:`check-circle-fill;1em;sd-text-success` | `llama.cpp <llama.cpp_>`_ |
+------------+--------------------------------------------------+--------------------------------------------------+---------------------------+

-------------------
Supported Platforms
-------------------

* Mac OS
* Linux
* Windows

-----------------------
Download and Setting up
-----------------------

Download page: :doc:`quickstart/setup/download`

SDK setup and compiling from source: :doc:`quickstart/setup/setting_up`

------------------
SDK Usage Examples
------------------

* :doc:`Single-Prompt <quickstart/single_prompt_ex/about>`: Simple prompt and answer example. At the end of the example, the user will supply a prompt in the terminal and LLM will give the response.
* :doc:`Dialogue-Example <quickstart/dialogue_ex/about>`: More complex dialogue based prompt and answer example. At the end of the example, the user will be able to have a dialogue with LLM using terminal.
* :doc:`Embedding-Example <quickstart/embeddings_ex/about>` :Vector embedding generator which is generally used by RAG programs and more. At the end of the example, the user will supply an input and vector embeddings will be generated by using embedder LLM model.

------------------------------
What do I mean by Non-blocking
------------------------------

**Since there are multiple questions arised from people asking what non-blocking means in this context, this title is addressing those questions**

Let's don't think about the LLMs for a second and think about IO management in the program.

IO and network operations are expensive operations. Their performance are limited by the read/write speed or the network operations highly influenced by your network environment speec etc.

In IO scenario, when you want to write multiple Gigs of data into a disk, you should write a mechanism in your program so that the write won't block your main application logic.
You may do this by writing data to a file by specifying a threshold let's say writing 1KB every iteration. Or you may do your IO operations in seperate thread and write a synchronization mechanism based-off of your needs.
Or, you can use async io libraries that can do this for you.

In my opinion, LLM inference deserve its own non-blocking terminology because the operations such as language model initialization(llama_model_load_from_file), destruction(llama_model_free), context creation(llama_init_from_model), encoder/decoder(llama_encode/llama_decode) methods are extremelly expensive which makes them really difficult to integrate into your main application logic.

Even with high-end GPU, the amount your program halts on llama_model_load_from_file, llama_init_from_model, llama_encode/llama_decode prevents people from integrating LLMs into their applications.

This SDK apply those operations in a non-blocking manner or in other words, the model initialization, destruction, context creation, encoder/decoder methods doesn't block your main thread and synchronization is handled by the MBASE SDK.

Using this, you will be able to load/unload multiple models, create contexts, and apply encode/decoder operations all at the same time while not blocking your main application thread because MBASE will handle all those
operations in parallel and will provide you the synchronized callbacks so that you won't need to consider issues arise from parallel programming.

---------------
Useful Programs
---------------

^^^^^^^^^^^^^
Openai Server
^^^^^^^^^^^^^

Detailed documentation: :doc:`programs/openai-server/about`

An Openai API compatible HTTP/HTTPS server for serving LLMs.
This program provides chat completion API for TextToText models and embeddings API For embedder models.

**Usage:**

.. code-block:: bash

    mbase_openai_server *[option [value]]
    mbase_openai_server --hostname "127.0.0.1" -jsdesc description.json
    mbase_openai_server --hostname "127.0.0.1" --port 8080 -jsdesc description.json
    mbase_openai_server --hostname "127.0.0.1" --port 8080 --ssl-pub public_key_file --ssl-key private_key_file -jsdesc description.json

^^^^^^^^^^^^^
Benchmark T2T
^^^^^^^^^^^^^

Detailed documentation: :doc:`programs/benchmark-t2t/about`

It is a program written to measure the performance
of the given T2T LLM and its impact on your main application logic.

**Usage:**

.. code-block:: bash

    mbase_benchmark_t2t model_path *[option [value]]
    mbase_benchmark_t2t model.gguf -uc 1 -fps 500 -jout .
    mbase_benchmark_t2t model.gguf -uc 1 -fps 500 -jout . -mdout .

^^^^^^^^^
Embedding
^^^^^^^^^

Detailed documentation: :doc:`programs/embedding/about`

An example program for generating the embeddings of the given prompt or prompts.

**Usage:**

.. code-block:: bash

    mbase_embedding_simple model_path *[option [value]]
    mbase_embedding_simple model.gguf -gl 80 -p 'What is life?'
    mbase_embedding_simple model.gguf -gl 80 -pf prompt1.txt -pf prompt2.txt

^^^^^^^^^
Retrieval
^^^^^^^^^

Detailed documentation: :doc:`programs/retrieval/about`

An example for calculating the distance between the
given query and multiple text files/documents and applying a retrieval operation.

**Usage:**

.. code-block:: bash

    mbase_retrieval model_path *[option [value]]
    mbase_retrieval model.gguf -q 'What is MBASE' -pf file1.txt -pf file2.txt -gl 80


^^^^^^^^^^^^^^^^^^^
Simple Conversation
^^^^^^^^^^^^^^^^^^^

Detailed documentation: :doc:`programs/simple-conversation/about`

It is a simple executable program where you are having
a dialogue with the LLM you provide. It is useful for examining the answer of the LLM since the system prompt 
and sampler values can be altered.

**Usage:**

.. code-block:: bash

    mbase_simple_conversation model_path *[option [value]]
    mbase_simple_conversation model.gguf
    mbase_simple_conversation model.gguf -gl 80
    mbase_simple_conversation model.gguf -gl 80 -sys 'You are a helpful assistant.'

^^^^^^^^^^
Typo Fixer
^^^^^^^^^^

Detailed documentation: :doc:`programs/typo-fixer/about`

This is an applied example use case of the MBASE library.
The program is reading a user-supplied text file and fixing the typos.

**Usage:**

.. code-block:: bash

    mbase_typo_fixer model_path *[option [value]]
    mbase_typo_fixer model.gguf -gl 80 -s typo.txt -o fixed.txt

---------------
Finding the SDK
---------------

Detailed documentation: :doc:`info-reference/sdk-structure`

If you have installed the MBASE SDK, you can find the library using CMake find_package function with components specification.

In order to find the library using cmake, write the following:

.. code-block:: cmake

    find_package(mbase.libs REQUIRED COMPONENTS inference)

This will find the inference SDK. In order to set the include directories and link the libraries, write the following:

.. code-block:: cmake

    target_compile_features(<your_target> PUBLIC cxx_std_17)
    target_link_libraries(<your_target> PRIVATE mbase-inference)
    target_include_directories(<your_target> PUBLIC mbase-inference)

---------------------------------
GGUF, Displaying General Metadata
---------------------------------

Detailed documentation: :doc:`info-reference/gguf-files`

.. code-block:: cpp

    #include <mbase/inference/inf_gguf_metadata_configurator.h>
    #include <iostream>

    int main()
    {
        mbase::GgufMetaConfigurator metaConfigurator(L"<path_to_model>");

        if(!metaConfigurator.is_open())
        {
            std::cout << "Unable to open gguf file." << std::endl;
            return 1;
        }

        mbase::string modelArchitecture;
        mbase::string modelName;
        mbase::string modelAuthor;
        mbase::string modelVersion;
        mbase::string modelOrganization;
        mbase::string modelSizeLabel;
        mbase::string modelLicense;
        mbase::string modelLicenseName;
        mbase::string modelLicenseLink;
        mbase::string modelUuid;
        uint32_t modelFileType;

        metaConfigurator.get_key("general.architecture", modelArchitecture);
        metaConfigurator.get_key("general.name", modelName);
        metaConfigurator.get_key("general.author", modelAuthor);
        metaConfigurator.get_key("general.version", modelVersion);
        metaConfigurator.get_key("general.organization", modelOrganization);
        metaConfigurator.get_key("general.size_label", modelSizeLabel);
        metaConfigurator.get_key("general.license", modelLicense);
        metaConfigurator.get_key("general.license.name", modelLicenseName);
        metaConfigurator.get_key("general.license.link", modelLicenseLink);
        metaConfigurator.get_key("general.uuid", modelUuid);
        metaConfigurator.get_key("general.file_type", modelFileType);

        std::cout << "Architecture: " << modelArchitecture << std::endl;
        std::cout << "Name: " << modelName << std::endl;
        std::cout << "Author: " << modelAuthor << std::endl;
        std::cout << "Version: " << modelVersion << std::endl;
        std::cout << "Organization: " << modelOrganization << std::endl;
        std::cout << "Size label: " << modelSizeLabel << std::endl;
        std::cout << "License: " << modelLicense << std::endl;
        std::cout << "License name: " << modelLicenseName << std::endl;
        std::cout << "License link: " << modelLicenseLink << std::endl;
        std::cout << "Model UUID: " << modelUuid << std::endl;
        std::cout << "File type: " << modelFileType << std::endl;

        return 0;
    }

-----------------------
Project State and Goals
-----------------------

The MBASE SDK is in its baby-steps for now and it is expected
to err on some scenarios because not all cases are tested and the product
is not yet fully-polished.

The project has been developed only by me and I was planning to open-source
in the near future. However, the complication and the workload forced me to open-source the project much earlier
before being perfectly tested and polished. In other words, I am once again asking for your contribution to this project.

The company is being established to accelerate the development of the MBASE SDK and for creating both proprietary and open-source products
built using the MBASE SDK.

The goal of the MBASE SDK is expected to provide non-blocking LLM inference to both beginner and advanced users of the C++ library
and very useful tools and programs to non-programmer users.