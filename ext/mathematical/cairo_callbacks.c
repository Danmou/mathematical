#include <cairo_callbacks.h>

cairo_status_t cairoSvgSurfaceCallback (void *closure, const unsigned char *data, unsigned int length)
{
  PyObject *self = (PyObject *) closure;
  if (PyObject_GetAttrString(self, "svg") == Py_None) {
    PyObject_SetAttrString(self, "svg", PyUnicode_FromString(""));
  }

  PyObject_SetAttrString(self, "png", PyUnicode_Concat(PyObject_GetAttrString(self, "svg"), PyUnicode_FromStringAndSize((char *) data, length)));

  return CAIRO_STATUS_SUCCESS;
}

cairo_status_t cairoPngSurfaceCallback (void *closure, const unsigned char *data, unsigned int length)
{
  PyObject *self = (PyObject *) closure;
  if (PyObject_GetAttrString(self, "png") == Py_None) {
    PyObject_SetAttrString(self, "png", PyUnicode_FromString(""));
  }

  PyObject_SetAttrString(self, "png", PyUnicode_Concat(PyObject_GetAttrString(self, "png"), PyUnicode_FromStringAndSize((char *) data, length)));

  return CAIRO_STATUS_SUCCESS;
}
