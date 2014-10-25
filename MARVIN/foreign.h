#ifndef __FOREIGN_H__
#define __FOREIGN_H__

#ifndef __STDLIB_H__
#define __STDLIB_H__
#include <stdlib.h>
#endif

#ifndef __STDIO_H__
#define __STDIO_H__
#include <stdio.h>
#endif

#ifndef __STRING_H__
#define __STRING_H__
#include <string.h>
#endif

#ifndef __PYTHON_H__
#define __PYTHON_H__
#include <Python.h>
#endif

#ifndef __AI_H__
#include "ai.h"
#endif

PyObject * MarvinAI_initialize(PyObject *, PyObject *);
PyObject * MarvinAI_evaluate(PyObject *, PyObject *);

static
PyMethodDef MarvinAI_Methods[] = {
      { "evaluate", MarvinAI_evaluate, METH_OLDARGS, "Evalute board position." },
      { "initialize", MarvinAI_initialize, METH_OLDARGS, "Initialize tables." },
      { NULL, NULL, 0, NULL }
};

PyMODINIT_FUNC
initMarvinAI(void)
{
  (void) Py_InitModule("MarvinAI", MarvinAI_Methods);
}

#endif
