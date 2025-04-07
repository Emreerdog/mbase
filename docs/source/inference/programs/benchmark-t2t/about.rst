=============
Benchmark T2T
=============

----------------------
Program Identification
----------------------

.. |pi_author| replace:: M\. Emre Erdog
.. |pi_maintainer| replace:: M\. Emre Erdog
.. |pi_email| replace:: erdog@mbasesoftware.com
.. |pi_name| replace:: mbase_benchmark_t2t
.. |pi_version| replace:: v1.0.0
.. |pi_type| replace:: Utility
.. |pi_net_usage| replace:: No
.. |pi_lib_depends| replace:: mbase-std mbase-inference
.. |pi_repo_location| replace:: https://github.com/Emreerdog/mbase/tree/main/examples/benchmark

.. include:: ../../../program_identification.rst

--------
Synopsis
--------

.. code-block:: bash

  mbase_benchmark_t2t model_path *[option [value]]
  mbase_benchmark_t2t model.gguf -uc 1 -fps 500 -jout .
  mbase_benchmark_t2t model.gguf -uc 1 -fps 500 -jout . -mdout .

-----------
Description
-----------

This is a utility program to measure the performance of the given T2T LLM.

The program will do an inference based-off of the given context size, batch length,
and n predict, simultaneusly on multiple users at the same time.

While running multiple processors in parallel which is specified by the option :code:`-uc, --user-count`, program will have a main 
loop working idly in the rate specified by the option: :code:`-fps, --frame-per-second` and
it will actively display the FPS every second. With this, the user will be able to examine the effect of the inference operation
on the main application loop.

At the end of the inference, it will display the following metrics along
with the model and session information:

* Total elapsed time in seconds.
* Average FPS.
* For each processor:

  * The time it took to initialize the context in milliseconds.
  * Prompt processing tokens per second(pp t/s).
  * Token generation tokens per second(tg t/s).

----------------
Formatted Output
----------------

Along with the terminal display of benchmark output, user can specify an output directory 
by supplying options such as :code:`-jout, --json-output-path` and :code:`-mdout, --markdown-output-path` 
in which the program will output files given as :code:`mbase_bench.json` and :code:`mbase_bench.md`.

In the next two sections, you will see the benchmark result of
the model "Phi 3 Mini 128k Instruct Q4_0" on a given hardware with single user :code:`-uc 1`: 

CPU: **Intel(R) Core(TM) i7-9750HF**

GPU: **NVIDIA GeForce RTX 2060**

Program call:

.. code-block:: bash

  .\mbase_benchmark_t2t.exe path_to_model -jout . -mdout . -uc 1 -b 2048 -np 512

^^^^^^^^^^^^^^^^^^^
JSON Output Example
^^^^^^^^^^^^^^^^^^^

.. code-block:: json
  :caption: mbase_bench.json

  {
    "model_information": {
      "embedding_length": 3072,
      "head_count": 32,
      "layer_count": 32,
      "model_size_gb": 2.0260353,
      "name": "Phi 3 Mini 128k Instruct",
      "quantization": "Q4_0"
    },
    "processor_diagnostics": [{
      "load_delay_ms": 41,
      "pp tokens per sec": 1698.1758,
      "tg tokens per sec": 76.92307
    }],
    "session_information": {
      "batch_proc_threads": 8,
      "batch_size": 2048,
      "compute_devices": [{
        "device_name": "NVIDIA GeForce RTX 2060",
        "type": "GPU"
      }, {
        "device_name": "Intel(R) Core(TM) i7-9750HF CPU @ 2.60GHz",
        "type": "CPU"
      }],
      "context_length": 2048,
      "flash_attention": true,
      "generation_threads": 16,
      "gpu_layers": 999,
      "predict": 512,
      "prompt_length": 1024,
      "user_count": 1
    },
    "useful_metrics": {
      "average_fps": 456.16666,
      "total_elapsed_time_seconds": 9.748
    }
  }

^^^^^^^^^^^^^^^^^^^^^^^
Markdown Output Example
^^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: markdown
  :caption: mbase_bench.md

  ### Model Information
  __Name__: Phi 3 Mini 128k Instruct<br>
  __Model size__: 2.03 GB <br>
  __Quantization__: Q4_0<br>
  __Embedding length__: 3072<br>
  __Head count__: 32<br>
  __Layer count__: 32<br>
  ### Session Information
  __Context length__: 2048<br>
  __Batch size__: 2048<br>
  __Prompt length__: 1024<br>
  __Batch processing threads__: 8<br>
  __Generation threads__: 16<br>
  __User count__: 1<br>
  __Flash attention__: Enabled<br>
  __GPU offload layers__: 999<br>
  __N Predict__: 512<br>
  __Compute devices__:
  - NVIDIA GeForce RTX 2060
  - Intel(R) Core(TM) i7-9750HF CPU @ 2.60GHz
  ### Useful Metrics
  __Total elapsed time in seconds__: 9.75<br>
  __Average FPS__: 456<br>
  ### Performance Table
  | Load delay ms | pp t/s | tg t/s |
  | ------------- | ------ | ------ |
  | 41 | 1698.18 | 76.92 |

-------
Options
-------

.. option:: -h, --help

  Print program information.

.. option:: -v, --version

  Shows program version.

.. option:: -dfa, --disable-flash-attention

  Disables the flash attention, which is enabled by default.
  Disabling it may decrease the performance.

.. option:: -t count, --thread-count count

  Amount of threads to use for token generation. (default=16)

.. option:: -bt count, --batch-thread-count count 

  Amount of thread to use for initial batch processing. (default=8)

.. option:: -c length, --context-length length 

  Total context length of the conversation which includes
  the special tokens and the response of the LLM. (default=2048)

.. option:: -b length, --batch-length length

  The input is executed in batches in processor decode loop.
  This is the maximum batch length to be processed in single iteration. (default=1024)

.. option:: -pr length, --prompt-length length

  Pseudo prompt length. Can't exceed context length. (default=1024)
  Higher numbers may result in more precise pp t/s

.. option:: -gl count, --gpu-layers count

  Number of layers too offload to GPU.
  Ignored if there is no GPU is present. (default=999)

.. option:: -np n, --n-predict n

  Number of tokens to predict.
  Higher numbers may result in more precise tg t/s. (default=256)

.. option:: -uc count, --user-count count

  Amount of users to be processed in parallel. (default=1)

.. option:: -fps n, --frame-per-second n

  Max FPS of the main loop. This is for measuring the effects of inference engine on main application loop(default=500, min=10, max=1000).

.. option:: -jout directory_path, --json-output-path directory_path

  If the json output path is specified,
  result will be written there in file 
  "mbase_bench.json". (default="")

.. option:: -mdout directory_path, --markdown-output-path directory_path

  If the markdown output path is specified,
  result will be written there in file 
  "mbase_bench.md". (default="")
