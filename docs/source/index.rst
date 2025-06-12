=======================
MBASE SDK Documentation
=======================

Github link: https://github.com/Emreerdog/mbase

Welcome to the MBASE SDK documentation.

MBASE SDK is a set of mostly LLM related libraries actively being maintained and developed by M. Emre Erdog
and the `MBASE Yazılım A.Ş. <https://mbasesoftware.com>`_. 

The main purpose of the MBASE SDK is to supply the developer with necessary tools and procedures to easily integrate LLM capabilities into their
C++ applications.

Here is a list of libraries in the MBASE SDK:

* :doc:`Inference Library <inference/about>`: An LLM inference library built over https://github.com/ggerganov/llama.cpp library for integrating LLMs into programs.
* :doc:`Model Context Protocol Library <inference/about>`: A C++ MCP client/server library that includes all fundamental features, with support for both STDIO and StreamableHTTP transport methods.
* :doc:`Standard Library <std/index>` : A standard library containing STL compatible containers with default serialization support and useful utilities such as built-in uuid generation, timers etc.
* :doc:`JSON Library <json/index>`: A light-weight json library. 

-------------------
Supported Platforms
-------------------

* macOS
* Linux
* Windows

----------
Setting-Up
----------

In order the setup the MBASE SDK, see: :doc:`setting-up <setting-up/about>`

.. toctree::
   :caption: Contents:
   
   Setting-up <setting-up/about>
   Inference Library <inference/about>
   Model Context Protocol Library <mcp/about>
   Standard Library <std/index>
   JSON Library <json/index>
   Copyright <copyright>