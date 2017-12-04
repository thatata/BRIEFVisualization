// BRIEF :: Thomas J. Magnan :: GW Department of Computer Science :: Senior Design Project 2017

// Pull in the Python API for extending the OpenCV functionality to easy Python interface
#include <Python.h>

// C function that will be called C function that will be called when the Python expression spam.system(string) is evaluated
// Extend this function to call opencv logic
static PyObject *
spam_system(PyObject *self, PyObject *args)
{
    const char *command;
    int sts;

    if (!PyArg_ParseTuple(args, "s", &command))
        return NULL;
    sts = system(command);
    return Py_BuildValue("i", sts);
}
