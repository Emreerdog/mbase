#include <Python.h>
#include <mbase/inference/inf_common.h>
#include <vector>

static PyObject* py_get_sys_name_total(PyObject*, PyObject*) {
    mbase::string result = mbase::inf_get_sys_name_total();
    return PyUnicode_FromString(result.c_str());
}

static PyObject* py_cosine_similarity(PyObject*, PyObject* args) {
    PyObject* list1; PyObject* list2;
    if (!PyArg_ParseTuple(args, "OO", &list1, &list2))
        return NULL;
    if (!PySequence_Check(list1) || !PySequence_Check(list2)) {
        PyErr_SetString(PyExc_TypeError, "arguments must be sequences");
        return NULL;
    }
    Py_ssize_t len1 = PySequence_Size(list1);
    Py_ssize_t len2 = PySequence_Size(list2);
    if (len1 != len2) {
        PyErr_SetString(PyExc_ValueError, "vectors must be same length");
        return NULL;
    }
    std::vector<float> v1(len1), v2(len2);
    for (Py_ssize_t i = 0; i < len1; ++i) {
        PyObject* item1 = PySequence_GetItem(list1, i);
        PyObject* item2 = PySequence_GetItem(list2, i);
        if (!item1 || !item2) {
            Py_XDECREF(item1);
            Py_XDECREF(item2);
            PyErr_SetString(PyExc_RuntimeError, "invalid sequence element");
            return NULL;
        }
        v1[i] = static_cast<float>(PyFloat_AsDouble(item1));
        v2[i] = static_cast<float>(PyFloat_AsDouble(item2));
        Py_DECREF(item1);
        Py_DECREF(item2);
    }
    float res = mbase::inf_common_cosine_similarity(v1.data(), v2.data(), (int)len1);
    return PyFloat_FromDouble(res);
}

static PyMethodDef Methods[] = {
    {"get_sys_name_total", py_get_sys_name_total, METH_NOARGS, "Return library name"},
    {"cosine_similarity", py_cosine_similarity, METH_VARARGS, "Compute cosine similarity"},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef moduledef = {
    PyModuleDef_HEAD_INIT,
    "_core",
    "MBASE Python bindings",
    -1,
    Methods,
    NULL, NULL, NULL, NULL
};

PyMODINIT_FUNC PyInit__core(void) {
    return PyModule_Create(&moduledef);
}
