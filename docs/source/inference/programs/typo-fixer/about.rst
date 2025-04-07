==================
Typo Fixer Program
==================

----------------------
Program Identification
----------------------

.. |pi_author| replace:: M\. Emre Erdog
.. |pi_maintainer| replace:: M\. Emre Erdog
.. |pi_email| replace:: erdog@mbasesoftware.com
.. |pi_name| replace:: mbase_typo_fixer
.. |pi_version| replace:: v0.1.0
.. |pi_type| replace:: Utility, Example
.. |pi_net_usage| replace:: No
.. |pi_lib_depends| replace:: mbase-std mbase-inference
.. |pi_repo_location| replace:: https://github.com/Emreerdog/mbase/tree/main/examples/typo-fixer

.. include:: ../../../program_identification.rst

--------
Synopsis
--------

.. code-block:: bash

    mbase_typo_fixer model_path *[option [value]]
    mbase_typo_fixer model.gguf -gl 80 -s typo.txt -o fixed.txt

-----------
Description
-----------

This is an applied example use case of the MBASE library.
The program is reading a user-supplied text file and fixing the typos.

Fixed text will be written to a file specified by option :code:`-o` or :code:`--output-file`.

-------
Options
-------

.. option:: --help

  Print program information.

.. option:: -v, --version

  Shows program version.

.. option:: -t count, --thread-count count

  Amount of threads to use for token generation. (default=16)

.. option:: -bt count, --batch-thread-count count 

  Amount of thread to use for initial batch processing. (default=8)

.. option:: -gl count, --gpu-layers count

  Number of layers too offload to GPU.
  Ignored if there is no GPU is present. (default=999)

.. option:: -s file_path, --source-file file_path

    File containing the text with typos.

.. option:: -o, file_path, --output-file file_path

    Output file where fixed text will be written to.
