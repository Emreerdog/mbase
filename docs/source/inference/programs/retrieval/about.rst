=================
Retrieval Program
=================

----------------------
Program Identification
----------------------

.. |pi_author| replace:: M\. Emre Erdog
.. |pi_maintainer| replace:: M\. Emre Erdog
.. |pi_email| replace:: erdog@mbasesoftware.com
.. |pi_name| replace:: mbase_retrieval
.. |pi_version| replace:: v0.1.0
.. |pi_type| replace:: Example
.. |pi_net_usage| replace:: No
.. |pi_lib_depends| replace:: mbase-std mbase-inference
.. |pi_repo_location| replace:: https://github.com/Emreerdog/mbase/tree/main/examples/retrieval

.. include:: ../../../program_identification.rst

--------
Synopsis
--------

.. code-block:: bash

    mbase_retrieval model_path *[option [value]]
    mbase_retrieval model.gguf -q 'What is MBASE' -pf file1.txt -pf file2.txt -gl 80

-----------
Description
-----------

An example for calculating the distance between the
given query and multiple text files our documents in this context and applying retrieval operation on multiple texts seperated by <embd_sep>.

What this program does is that it takes a query from the user and creates the embeddings of the user input and all other texts.
After the embeddings are generated, it applies the cosine similarity function to all embeddings and shows the distance of the query to each text.

-------
Options
-------

.. option:: -h, --help

    Print program information.

.. option:: -v, --version

    Shows program version.

.. option:: -pf file_path, --prompt-file file_path

    File containing prompt or prompts seperated by the seperator (default='').
    To give multiple prompt files, call this option multiple times.

.. option:: -q string, --query string

    User provided query.

.. option:: -sp seperator, --seperator seperator

    Prompt seperator (default="<embd_sep>").

.. option:: -t count, --thread-count count

    Threads used to compute embeddings (default=16).

.. option:: -gl count, --gpu-layers count

    Number of layers too offload to GPU.
    Ignored if there is no GPU is present. (default=999)
