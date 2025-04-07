===========================
Simple Conversation Program
===========================

----------------------
Program Identification
----------------------

.. |pi_author| replace:: M\. Emre Erdog
.. |pi_maintainer| replace:: M\. Emre Erdog
.. |pi_email| replace:: erdog@mbasesoftware.com
.. |pi_name| replace:: mbase_simple_conversation
.. |pi_version| replace:: v0.1.0
.. |pi_type| replace:: Example, Utility
.. |pi_net_usage| replace:: No
.. |pi_lib_depends| replace:: mbase-std mbase-inference
.. |pi_repo_location| replace:: https://github.com/Emreerdog/mbase/tree/main/examples/simple-conversation

.. include:: ../../../program_identification.rst

--------
Synopsis
--------

.. code-block:: bash

    mbase_simple_conversation model_path *[option [value]]
    mbase_simple_conversation model.gguf
    mbase_simple_conversation model.gguf -gl 80
    mbase_simple_conversation model.gguf -gl 80 -sys 'You are a helpful assistant.'

-----------
Description
-----------

This is a simple conversation program implemented to demonstrate 
what can be implemented using MBASE and show the capabilities.

The program is a simple executable where you are having a dialogue
with the LLM you provide. 

You can adjust the sampling parameters, thread count, or supply 
a system prompt either from file or from string using the options.

At the end of the program, it will print useful information about
the performance of the program such as, pp and tg rates as well as
model load delay.

-------
Options
-------

.. option:: --help

    Print program information.

.. option:: -v, --version

    Shows program version.

.. option:: -sys prompt, --system prompt

    LLM system prompt.
    If this option is given after -fsys, it will overwrite it. (default="")

.. option:: -fsys file, --system-prompt-file file

    Text file that contains the LLM system prompt.
    If this option is given after -sys, it will overwrite it. (default="")

.. option:: -t count, --thread-count count

    Amount of threads to use for token generation. (default=16)

.. option:: -bt count, --batch-thread-count count
    
    Amount of thread to use for initial batch processing. (default=8)

.. option:: -c length, --context-length length 

    Total context length of the conversation which includes
    the special tokens and the response of the LLM. (default=8192)

.. option:: -b length, --batch-length length 

    The input is executed in batches in processor decode loop.
    This is the maximum batch length to be processed in single iteration. (default=4096)

.. option:: -gl count, --gpu-layers count

    Number of layers too offload to GPU.
    Ignored if there is no GPU is present. (default=999)

.. option:: -tk k, --top-k k

    Top k most tokens to pick from, during the sampling phase. (default=20, min=1, max=<model_vocabulary>)

.. option:: -tp p, --top-p p

    Token probability at most during the sampling phase with
    values between (0.0, 1.0] where the higher the 'p', the bigger the pool.
    (default=1.0)

.. option:: -mp p, --min-p p

    Token probability at most during the sampling phase with
    values between (0.0, 1.0] where the higher the 'p', the smaller the pool.
    (default=0.3)

.. option:: -pn n, --penalty-n n

    Apply repetition penalty on last 'n' tokens.
    (default=64)

.. option:: -pr frequency, --penalty-repeat frequency

    Discourages repeating exact tokens based on their past presence.
    The higher the frequency, the lower the repetition.
    (default=1.3, min=1.0, max=2.0)

.. option:: -temp n, --temperature n

    Higher values increase the randomness.
    (default=0.1, min.01, max 1.4)

.. option:: -gr, --greedy

    Ignore all sampling techniques, pick the most probable token.
    In other words, apply greedy. (default=false)

------------
Contribution
------------
