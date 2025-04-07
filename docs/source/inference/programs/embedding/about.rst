=================
Embedding Program
=================

----------------------
Program Identification
----------------------

.. |pi_author| replace:: M\. Emre Erdog
.. |pi_maintainer| replace:: M\. Emre Erdog
.. |pi_email| replace:: erdog@mbasesoftware.com
.. |pi_name| replace:: mbase_embedding_simple
.. |pi_version| replace:: v0.1.0
.. |pi_type| replace:: Utility, Example
.. |pi_net_usage| replace:: No
.. |pi_lib_depends| replace:: mbase-std mbase-inference
.. |pi_repo_location| replace:: https://github.com/Emreerdog/mbase/tree/main/examples/embedding

.. include:: ../../../program_identification.rst

--------
Synopsis
--------

.. code-block:: bash

    mbase_embedding_simple model_path *[option [value]]
    mbase_embedding_simple model.gguf -gl 80 -p 'What is life?'
    mbase_embedding_simple model.gguf -gl 80 -pf prompt1.txt -pf prompt2.txt

-----------
Description
-----------

An example program for generating the embeddings of the given prompt or prompts.
The given implementation is stable and shows the basics of how to generate embeddings using MBASE embedder processor :code:`InfEmbedderProcessor`.

-------
Options
-------

.. option:: -h, --help

    Print program information.

.. option:: -v, --version

    Shows program version.

.. option:: -pf file_path, --prompt-file file_path

    File containing prompt or prompts seperated by the seperator (default=''). If prompt is given, prompt file will be ignored.

.. option:: -p string, --prompt string

    Prompt or prompts seperated by the seperator (default=''). This will be used even if the prompt file is supplied.

.. option:: -sp seperator, --seperator seperator

    Prompt seperator (default="<embd_sep>").

.. option:: -t count, --thread-count count

    Threads used to compute embeddings (default=16).

.. option:: -gl count, --gpu-layers count

    Number of layers too offload to GPU.
    Ignored if there is no GPU is present. (default=999)

.. option:: -jout output_path, --json-output-path output_path

    If the json output path is specified, result will be written there in file "openai_embeddings.json" (default='').
