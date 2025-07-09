# mbasepy

This directory contains experimental Python bindings for the C++ MBASE SDK.

`mbasepy` exposes a very small portion of the inference API to Python. The
bindings are implemented using the Python C API and require the C++ sources to
be built. Currently only `get_sys_name_total` and `cosine_similarity` from the
inference library are available.

To build a wheel locally:

```bash
python -m pip install wheel
python setup.py bdist_wheel
```

The build requires a valid MBASE C++ build environment. Missing dependencies
(such as the `llama.cpp` submodule) will cause the build to fail.
