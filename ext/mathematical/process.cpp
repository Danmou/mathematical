/****************************************************************************
* Mathematical Copyright(c) 2014, Garen J. Torikian, All rights reserved.
* --------------------------------------------------------------------------
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
****************************************************************************/

#include <process.h>

// Raised when the size of the latex string is too large
static PyObject *PyExc_MaxsizeError;
// Raised when the contents could not be parsed
static PyObject *PyExc_ParseError;
// Raised when the SVG document could not be created
static PyObject *PyExc_DocumentCreationError;
// Raised when the SVG document could not be read
static PyObject *PyExc_DocumentReadError;

static PyObject *py_init(PyObject *module, PyObject *args)
{
  PyObject *self;
  PyObject *py_Options;

  if (!PyArg_ParseTuple(args, "OO!", &self, &PyDict_Type, &py_Options)) {
    return NULL;
  }
  PyObject *py_ppi, *py_zoom, *py_maxsize, *py_format, *py_delimiter;

  py_ppi = PyDict_GetItemString(py_Options, "ppi");
  py_zoom = PyDict_GetItemString(py_Options, "zoom");
  py_maxsize = PyDict_GetItemString(py_Options, "maxsize");
  py_format = PyDict_GetItemString(py_Options, "formatInt");
  py_delimiter = PyDict_GetItemString(py_Options, "delimiter");

  if (!(PyFloat_Check(py_ppi) && PyFloat_Check(py_zoom) && PyLong_Check(py_maxsize) && PyLong_Check(py_format) && PyLong_Check(py_delimiter))) {
    PyErr_SetString(PyExc_TypeError, "");
    return NULL;
  }

  PyObject_SetAttrString(self, "ppi", py_ppi);
  PyObject_SetAttrString(self, "zoom", py_zoom);
  PyObject_SetAttrString(self, "maxsize", py_maxsize);
  PyObject_SetAttrString(self, "format", py_format);
  PyObject_SetAttrString(self, "delimiter", py_delimiter);

  PyObject_SetAttrString(self, "png", Py_None);
  PyObject_SetAttrString(self, "svg", Py_None);

  return self;
}

static PyObject *process_rescue(PyObject *args)
{
  PyObject *rescue_dict = PyDict_New();

  PyDict_SetItemString(rescue_dict, "data", args);
  PyDict_SetItemString(rescue_dict, "exception", PyErr_Occurred());

  return rescue_dict;
}

static PyObject *process(PyObject *self, unsigned long maxsize, const char *latex_code, unsigned long latex_size, int delimiter, int parse_type)
{
  if (latex_size > maxsize) {
    PyErr_SetString(PyExc_MaxsizeError, "Size of latex string is greater than the maxsize");
  }

  PyObject *result_dict = PyDict_New();
  FileFormat format = (FileFormat) PyLong_AsLong(PyObject_GetAttrString(self, "format"));

  /* convert the TeX math to MathML */
  char * mathml = lsm_mtex_to_mathml(latex_code, latex_size, delimiter, parse_type);
  if (mathml == NULL) { PyErr_SetString(PyExc_ParseError, "Failed to parse mtex"); }

  if (format == FORMAT_MATHML || parse_type == TEXT_FILTER) {
    PyDict_SetItemString(result_dict, "data", PyUnicode_FromString(mathml));
    mtex2MML_free_string(mathml);
    return result_dict;
  }

  int mathml_size = strlen(mathml);

  LsmDomDocument *document;
  document = lsm_dom_document_new_from_memory(mathml, mathml_size, NULL);

  lsm_mtex_free_mathml_buffer(mathml);

  if (document == NULL) { PyErr_SetString(PyExc_DocumentCreationError, "Failed to create document"); }

  LsmDomView *view;

  double ppi = PyFloat_AsDouble(PyObject_GetAttrString(self, "ppi"));
  double zoom = PyFloat_AsDouble(PyObject_GetAttrString(self, "zoom"));

  view = lsm_dom_document_create_view (document);
  lsm_dom_view_set_resolution (view, ppi);

  double width_pt = 2.0, height_pt = 2.0;
  unsigned int height, width;

  lsm_dom_view_get_size (view, &width_pt, &height_pt, NULL);
  lsm_dom_view_get_size_pixels (view, &width, &height, NULL);

  width_pt *= zoom;
  height_pt *= zoom;

  cairo_t *cairo;
  cairo_surface_t *surface;

  if (format == FORMAT_SVG) {
    surface = cairo_svg_surface_create_for_stream (cairoSvgSurfaceCallback, self, width_pt, height_pt);
  } else { // format == FORMAT_PNG
    surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, width, height);
  }

  cairo = cairo_create (surface);
  cairo_scale (cairo, zoom, zoom);
  lsm_dom_view_render (view, cairo, 0, 0);

  switch (format) {
  case FORMAT_PNG: {
    cairo_surface_write_to_png_stream (cairo_get_target (cairo), cairoPngSurfaceCallback, self);
    break;
  }
  default: {
    break;
  }
  }

  cairo_destroy (cairo);
  cairo_surface_destroy (surface);
  g_object_unref (view);
  g_object_unref (document);

  switch (format) {
  case FORMAT_SVG: {
    if (PyObject_GetAttrString(self, "svg") == Py_None) { PyErr_SetString(PyExc_DocumentReadError, "Failed to read SVG contents"); }
    PyDict_SetItemString(result_dict, "data", PyObject_GetAttrString(self, "svg"));
    break;
  }
  case FORMAT_PNG: {
    if (PyObject_GetAttrString(self, "png") == Py_None) { PyErr_SetString(PyExc_DocumentReadError, "Failed to read PNG contents"); }
    PyDict_SetItemString(result_dict, "data", PyObject_GetAttrString(self, "png"));
    break;
  }
  default: {
    /* should be impossible, Python code prevents this */
    PyErr_SetString(PyExc_TypeError, "not valid format");
    break;
  }
  }

  PyDict_SetItemString(result_dict, "width",  PyLong_FromLong((long) width_pt));
  PyDict_SetItemString(result_dict, "height", PyLong_FromLong((long) height_pt));

  /* we need to clear out this key when attempting multiple calls. See http://git.io/i1hblQ */
  PyObject_SetAttrString(self, "svg", Py_None);
  PyObject_SetAttrString(self, "png", Py_None);

  return result_dict;
}

static PyObject *py_process(PyObject *module, PyObject *args)
{
  PyObject *self;
  PyObject *py_Input;
  int parse_type;

  if (!PyArg_ParseTuple(args, "OOi", &self, &py_Input, &parse_type)) {
    return NULL;
  }

  unsigned long maxsize = PyLong_AsUnsignedLong(PyObject_GetAttrString(self, "maxsize"));

  /* make sure that the passed latex string is not larger than the maximum value of
    a signed long (or the maxsize option) */
  if (maxsize == 0) {
    maxsize = LONG_MAX;
  }

  int delimiter = (int) PyLong_AsLong(PyObject_GetAttrString(self, "delimiter"));

#if !GLIB_CHECK_VERSION(2,36,0)
  g_type_init();
#endif

  const char *latex_code;
  unsigned long latex_size;

  PyObject *output;

  if (PyUnicode_Check(py_Input)) {
    latex_code = PyBytes_AsString(PyUnicode_AsASCIIString(py_Input)); // TODO: unicode support?
    latex_size = (unsigned long) strlen(latex_code);

    output = process(self, maxsize, latex_code, latex_size, delimiter, parse_type);
    if (PyErr_Occurred() != NULL) {
      output = process_rescue(PyUnicode_FromString(latex_code));
      // Clear error here?
    }
  } else if (PyList_Check(py_Input)) {
    Py_ssize_t i, length = PyList_Size(py_Input);
    PyObject *dict;
    output = PyList_New(length);

    for (i = 0; i < length; i++) {
      /* grab the ith element */
      PyObject *math = PyList_GetItem(py_Input, i);

      /* get the string and length */
      latex_code = PyBytes_AsString(PyUnicode_AsASCIIString(math)); // TODO: unicode support?
      latex_size = (unsigned long) strlen(latex_code);

      dict = process(self, maxsize, latex_code, latex_size, delimiter, parse_type);
      if (PyErr_Occurred() != NULL) {
        dict = process_rescue(PyUnicode_FromString(latex_code));
        // Clear error here?
      }

      PyList_SetItem(output, i, dict);
    }
  } else {
    /* should be impossible, Python code prevents this */
    PyErr_SetString(PyExc_TypeError, "not valid value");
    output = (PyObject *) NULL;
  }

  return output;
}

static PyMethodDef ProcessMethods[] = {
  {"initialize", py_init, METH_VARARGS, ""},
  {"process", py_process, METH_VARARGS, ""},
  {NULL, NULL, 0, NULL}        /* Sentinel */
};

static struct PyModuleDef processmodule = {
  PyModuleDef_HEAD_INIT,
  "process",   /* name of module */
  NULL, /* module documentation, may be NULL */
  -1,       /* size of per-interpreter state of the module,
                or -1 if the module keeps state in global variables. */
  ProcessMethods
};

PyMODINIT_FUNC PyInit_process(void)
{
  // Main module
  PyObject *module;

  module = PyModule_Create(&processmodule);
  if (module == NULL) {
    return NULL;
  }

  // Exception definitions
  PyExc_MaxsizeError = PyErr_NewException("process.MaxsizeError", NULL, NULL);
  PyExc_ParseError = PyErr_NewException("process.ParseError", NULL, NULL);
  PyExc_DocumentCreationError = PyErr_NewException("process.DocumentCreationError", NULL, NULL);
  PyExc_DocumentReadError = PyErr_NewException("process.DocumentReadError", NULL, NULL);
  Py_INCREF(PyExc_MaxsizeError);
  Py_INCREF(PyExc_ParseError);
  Py_INCREF(PyExc_DocumentCreationError);
  Py_INCREF(PyExc_DocumentReadError);
  PyModule_AddObject(module, "MaxsizeError", PyExc_MaxsizeError);
  PyModule_AddObject(module, "ParseError", PyExc_ParseError);
  PyModule_AddObject(module, "DocumentCreationError", PyExc_DocumentCreationError);
  PyModule_AddObject(module, "DocumentReadError", PyExc_DocumentReadError);

  return module;
}
