.. include:: links.rst

==========
Quickstart
==========

.. tip::
    It is highly suggested that you read the sections in order since they are not randomly laid out in the document.
    For example, the :doc:`dialogue example<quickstart/dialogue_ex/about>` and :doc:`single-prompt example<quickstart/single_prompt_ex/about>` is inter-connected where we are doing
    improvements to the :doc:`single-prompt example<quickstart/single_prompt_ex/about>`.

-----
About
-----

In this chapter, we will cover everything to install, run and develop using MBASE library.
Even if the setting up section covers everything to setup the MBASE library,
the concepts that are implemented in the examples section may require further clarification.

For that reason, when you have confusion at some parts, make sure to refer to its :doc:`info-reference/index`
and get the detailed explanation.

---------
Structure
---------

This chapter is composed of four sections which cover topics as follows:

* :doc:`Setting-up<quickstart/setup/setting_up>`: Setting up the environment, installing `llama.cpp <llama.cpp_>`_ and `MBASE <mbase_main_>`_.

* :doc:`Single-Prompt Example<quickstart/single_prompt_ex/about>`: An application example where we read stdin from user and send it to the LLM and get a response.

* :doc:`Dialogue Example<quickstart/dialogue_ex/about>`: An application example where we have a dialogue with the LLM in terminal. 

* :doc:`Embeddings Example<quickstart/embeddings_ex/about>`: The third one is a vector embedding generator which is generally used by RAG programs and more. At the end of the example, the user will supply an input and vector embeddings will be generated by using embedder LLM model.

In every example, the complete source code and the cmake file will be provided for ease of referencing.

.. toctree::
    :hidden:

    Setting-up <quickstart/setup/setting_up>
    Single-Prompt Example <quickstart/single_prompt_ex/about>
    Dialogue Example <quickstart/dialogue_ex/about>
    Embeddings Example <quickstart/embeddings_ex/about>
