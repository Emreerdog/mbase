======================
Complete Project Setup
======================

From now on, we will use the **CMakeLists.txt** and **main.cpp** files as a base
for every demonstration example we build together.

There are three simple inference examples implemented on the next three chapters.

* :doc:`Single-Prompt <single_prompt_ex/about>`: First one is a simple prompt and answer example. At the end of the example, the user will supply a prompt in the terminal and LLM will give the response.
* :doc:`Dialogue-Example <dialogue_ex/about>`: The second is a more complex dialogue based prompt and answer example. At the end of the example, the user will be able to have a dialogue with LLM using terminal.
* :doc:`Embedding-Example <embeddings_ex/about>`: The third one is a vector embedding generator which is generally used by RAG programs and more. At the end of the example, the user will supply an input and vector embeddings will be generated by using embedder LLM model.

.. important::
    Unlike third example, the first two examples are inter-connected so that when you finish the first example, you will use that result as a base to implement the second example.

Total CMakeLists.txt file:

.. code-block:: cmake
    :caption: CMakeLists.txt

    cmake_minimum_required(VERSION 3.15...3.31)
    project("mbase_simple_project" LANGUAGES CXX)

    add_executable(simple_project main.cpp)

    find_package(mbase.libs REQUIRED COMPONENTS std inference)

    target_compile_features(simple_project PUBLIC cxx_std_17)
    target_link_libraries(simple_project PRIVATE mbase-std mbase-inference)
    target_include_directories(simple_project PUBLIC mbase-std mbase-inference)

Total main.cpp file:

.. code-block:: cpp
    :caption: main.cpp

    #include <mbase/inference/inf_device_desc.h>
    #include <mbase/vector.h>
    #include <iostream>

    int main()
    {
        mbase::vector<mbase::InfDeviceDescription> deviceDesc = mbase::inf_query_devices();

        for(mbase::vector<mbase::InfDeviceDescription>::iterator It = deviceDesc.begin(); It != deviceDesc.end(); It++)
        {
            std::cout << It->get_device_description() << std::endl;
        }

        return 0;
    }