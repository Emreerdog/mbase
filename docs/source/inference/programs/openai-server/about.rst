=============
Openai Server
=============

----------------------
Program Identification
----------------------

.. |pi_author| replace:: M\. Emre Erdog
.. |pi_maintainer| replace:: M\. Emre Erdog
.. |pi_email| replace:: erdog@mbasesoftware.com
.. |pi_name| replace:: mbase_openai_server
.. |pi_version| replace:: v0.1.0
.. |pi_type| replace:: Example, Utility
.. |pi_net_usage| replace:: Yes
.. |pi_lib_depends| replace:: mbase-std mbase-inference
.. |pi_repo_location| replace:: https://github.com/Emreerdog/mbase/tree/main/examples/openai-server

.. include:: ../../../program_identification.rst

--------
Synopsis
--------

.. code-block:: bash

    mbase_openai_server *[option [value]]
    mbase_openai_server --hostname "127.0.0.1" -jsdesc description.json
    mbase_openai_server --hostname "127.0.0.1" --port 8080 -jsdesc description.json
    mbase_openai_server --hostname "127.0.0.1" --port 8080 --ssl-pub public_key_file --ssl-key private_key_file -jsdesc description.json

-----------
Description
-----------

An Openai API compatible HTTP/HTTPS server for serving LLMs.
This program provides chat completion API for TextToText models and embeddings API for embedder models.

The :code:`mbase_openai_server` can host either single or multiple models and serve its clients simultaneusly 
which is specified by the key :code:`processor_count` in the provided JSON description file.

In order to properly use the :code:`mbase_openai_server` program, you should supply a json file, describing
the behavior of the openai server program.

Along with the program description json, you can specify the hostname(default=127.0.0.1) to listen to and the port(default=8080).
The specified hostname and port must be configured properly so that the application may listen.

----------------------
JSON Description Usage
----------------------

User must write a JSON description file and specify its path in the option :code:`-jsdesc`:

.. code-block:: bash

    mbase_openai_server --hostname "127.0.0.1" -jsdesc description.json

In the description file, user will specify multiple parameters such as the source of the model file,
amount of concurrent users, samplers etc. 

^^^^^^^^^^^^^^^^^^^^^
Format and Parameters
^^^^^^^^^^^^^^^^^^^^^

JSON file will contain an array of objects that contains the given keys and values:

- :code:`model_path`: Path of the model. It must be a valid GGUF file.
- :code:`processor_count` (default=4): Amount of users that the server will concurrently serve the LLM to.
- :code:`thread_count` (default=8): Amount of generation threads to use for inference engine to generate tokens.
- :code:`batch_thread_count` (default=8): Amount of threads to use for initial batch processing.
- :code:`context_length` (default=2048): Context length of each processor. The inference engine will allocate a context for each processor.
- :code:`batch_length` (default=512): User's input will be processed in batches by the inference engine. Higher the number, better the performance but significant increase on RAM usage. This number can't exceed the context size.
- :code:`gpu_layers` (default=80): Number of layers to be offloaded to GPU if there are any GPU devices in your system. Ignored if there are no GPUs.

If you are hosting a TextToText model, the following samplers may also be specified.

- :code:`samplers.top_k`
- :code:`samplers.top_p`
- :code:`samplers.min_p`
- :code:`temp`
- :code:`mirostat_v2.tau`
- :code:`mirostat_v2_eta`
- :code:`repetition.penalty_n`
- :code:`repetition.penalty_repeat` 

If you don't specify any sampling parameters, the greedy sampling will be applied by default.

----------------------------
Single Model Hosting Example
----------------------------

.. code-block:: json
    :caption: description.json

    [
        {
            "model_path" : "model.gguf" 
        }
    ]

For 8 concurrent access with 4096 context length each:

.. code-block:: json
    :caption: description.json

    [
        {
            "model_path" : "model.gguf",
            "processor_count" : 8,
            "context_length" : 4096
        }
    ]

Specifying all parameters and some samplers:

.. code-block:: json
    :caption: description.json

    [
        {
            "model_path" : "model.gguf",
            "processor_count" : 8,
            "context_length" : 4096,
            "thread_count" : 8,
            "batch_thread_count" : 8,
            "batch_length" : 512,
            "gpu_layers" : 80,
            "samplers" :
            {
                "top_k" : 40,
                "top_p" : 1.0,
                "min_p" : 0.3,
                "temp" : 0.8,
                "repetition" :
                {
                    "penalty_n" : 64,
                    "penalty_repeat" : 1.2
                }
            }
        }
    ]

---------------------------
Multi Model Hosting Example
---------------------------

.. code-block:: json
    :caption: description.json

    [
        {
            "model_path" : "model.gguf"
        },
        {
            "model_path" : "model1.gguf"
        },
        {
            "model_path" : "model2.gguf"
        }
    ]

----------------------
REST API Usage Example
----------------------

After you create your JSON description file and run the openai-server program, you can request the server using the Openai API.

^^^^^^^^^^^^^^^
Chat Completion
^^^^^^^^^^^^^^^

.. important::

    You can observe the model names by sending a GET request to the :code:`/v1/models` endpoint.

**Using CURL:**

.. code-block:: bash

    curl "http://localhost:8080/v1/chat/completions" \
    -H "Content-Type: application/json" \
    -H "Authorization: Bearer $OPENAI_API_KEY" \
    -d '{
        "model": "$MODEL_NAME",
        "messages": [
            {
                "role": "developer",
                "content": "You are a helpful assistant."
            },
            {
                "role": "user",
                "content": "How are you?"
            }
        ]
    }'

**Using Python:**

.. code-block:: python

    from openai import OpenAI

    client = OpenAI(
            base_url="http://localhost:8080/v1",
            api_key="OPENAI_API_KEY"
    )

    completion = client.chat.completions.create(
        model="MODEL_NAME",
        messages=[
            {"role": "developer", "content": "You are a helpful assistant."},
            {
                "role": "user",
                "content": "How are you?"
            }
        ]
    )

    print(completion.choices[0].message)


-------
Options
-------

.. option:: --help

    Print program information.

.. option:: -v, --version

    Shows program version.

.. option:: --api-key key

    API key to be checked by the server.

.. option:: -h host, --hostname host

    Hostname to listen to. (default=127.0.0.1)

.. option:: -p port, --port port

    Port to assign to. (default=8080)

.. option:: --ssl-public file

    SSL public file for HTTPS support.

.. option:: --ssl-key file
    
    SSL private key file for HTTPS support.

.. option:: -jsdesc description_file

    JSON description file for the openai server program.

