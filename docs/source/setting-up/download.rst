.. include:: ../inference/links.rst

========
Download
========

For github release page, refer to: https://github.com/Emreerdog/mbase/releases/tag/v0.3.1

MBASE SDK v0.3.1 download page. 

-------
Windows
-------

^^^^^^^^^^^^^^^^^^^^
Executable Installer
^^^^^^^^^^^^^^^^^^^^

`mbase-bundled-cuda-0.3.1-amd64-windows.exe <https://github.com/Emreerdog/mbase/releases/download/v0.3.1/mbase-bundled-cuda-0.3.1-amd64-windows.exe>`_: 64-bit NSIS windows installer. It installs the MBASE SDK and llama.cpp with CUDA support.

`mbase-bundled-0.3.1-amd64-windows.exe <https://github.com/Emreerdog/mbase/releases/download/v0.3.1/mbase-bundled-0.3.1-amd64-windows.exe>`_: 64-bit NSIS windows installer. It installs the MBASE SDK and llama.cpp.

^^^^^^^^^^^^^^^^^^^^
Precompiled Binaries
^^^^^^^^^^^^^^^^^^^^

`mbase-bundled-cuda-0.3.1-amd64-windows.tar.gz <https://github.com/Emreerdog/mbase/releases/download/v0.3.1/mbase-bundled-cuda-0.3.1-amd64-windows.tar.gz>`_: MBASE SDK and llama.cpp (DLLs and executables) and CUDA DLLs as ZIP archive.

`mbase-bundled-cuda-0.3.1-amd64-windows.zip <https://github.com/Emreerdog/mbase/releases/download/v0.3.1/mbase-bundled-cuda-0.3.1-amd64-windows.zip>`_: MBASE SDK and llama.cpp (DLLs and executables) and CUDA DLLs as TGZ archive.

`mbase-bundled-0.3.1-amd64-windows.zip <https://github.com/Emreerdog/mbase/releases/download/v0.3.1/mbase-bundled-0.3.1-amd64-windows.zip>`_: MBASE SDK and llama.cpp DLLs and executables as ZIP archive.

`mbase-bundled-0.3.1-amd64-windows.tar.gz <mbase-bundled-0.3.1-amd64-windows.tar.gz>`_: MBASE SDK and llama.cpp DLLs and executables as TGZ archive.

-----
Linux
-----

^^^^^^^^^^^^^^^^^^^^^^
APT Repository Install
^^^^^^^^^^^^^^^^^^^^^^

.. important::

    For CUDA support to be enabled, make sure you have installed the CUDA toolkit: https://developer.nvidia.com/cuda-downloads?target_os=Linux

Setting up MBASE apt repository:

.. code-block:: bash
    
    sudo curl -fsSL https://repo.mbasesoftware.com/apt/pgp-key.public -o /etc/apt/keyrings/mbase-apt.public
    echo "deb [arch=amd64 signed-by=/etc/apt/keyrings/mbase-apt.public] https://repo.mbasesoftware.com/apt/ stable main" | sudo tee /etc/apt/sources.list.d/mbase.list > /dev/null
    sudo apt-get update

MBASE SDK with CUDA support:

.. code-block:: bash

    sudo apt-get -y install mbase-bundled-cuda

MBASE SDK without CUDA support:

.. code-block:: bash

    sudo apt-get -y install mbase-bundled

^^^^^^^^^^^^^^^^^^^^
Precompiled Binaries
^^^^^^^^^^^^^^^^^^^^

`mbase-bundled-cuda_0.3.1_amd64.deb <https://github.com/Emreerdog/mbase/releases/download/v0.3.1/mbase-bundled-cuda_0.3.1_amd64.deb>`_: MBASE SDK and llama.cpp binaries with CUDA support as .deb package.

`mbase-bundled-cuda-0.3.1-x86_64-linux.rpm <https://github.com/Emreerdog/mbase/releases/download/v0.3.1/mbase-bundled-cuda-0.3.1-x86_64-linux.rpm>`_: MBASE SDK and llama.cpp with CUDA support as .rpm package.

`mbase-bundled-cuda-0.3.1-x86_64-linux.tar.gz <https://github.com/Emreerdog/mbase/releases/download/v0.3.1/mbase-bundled-cuda-0.3.1-x86_64-linux.tar.gz>`_: MBASE SDK and llama.cpp binaries with CUDA support as TGZ archive.

`mbase-bundled-cuda-0.3.1-x86_64-linux.zip <https://github.com/Emreerdog/mbase/releases/download/v0.3.1/mbase-bundled-cuda-0.3.1-x86_64-linux.zip>`_: MBASE SDK and llama.cpp with CUDA support as ZIP archive.

`mbase-bundled_0.3.1_amd64.deb <https://github.com/Emreerdog/mbase/releases/download/v0.3.1/mbase-bundled_0.3.1_amd64.deb>`_: MBASE SDK and llama.cpp binaries as .deb package.

`mbase-bundled-0.3.1-x86_64-linux.rpm <https://github.com/Emreerdog/mbase/releases/download/v0.3.1/mbase-bundled-0.3.1-x86_64-linux.rpm>`_: MBASE SDK and llama.cpp binaries as .rpm package.

`mbase-bundled-0.3.1-x86_64-linux.tar.gz <https://github.com/Emreerdog/mbase/releases/download/v0.3.1/mbase-bundled-0.3.1-x86_64-linux.tar.gz>`_: MBASE SDK and llama.cpp binaries as TGZ archive.

`mbase-bundled-0.3.1-x86_64-linux.zip <https://github.com/Emreerdog/mbase/releases/download/v0.3.1/mbase-bundled-0.3.1-x86_64-linux.zip>`_: MBASE SDK and llama.cpp binaries as ZIP archive.

------------------------
Mac OS X (Apple Silicon)
------------------------

.. important::

    For macOS, until the MBASE is installable through :code:`brew`, refer to :doc:`cfs`

-----------
Source Code
-----------

`mbase-0.3.1.zip <https://github.com/Emreerdog/mbase/archive/refs/tags/v0.3.1.zip>`_

`mbase-0.3.1.tar.gz <https://github.com/Emreerdog/mbase/archive/refs/tags/v0.3.1.tar.gz>`_