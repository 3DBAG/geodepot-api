#include <pybind11/pybind11.h>

#include <geodepot/geodepot.h>

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

namespace py = pybind11;

PYBIND11_MODULE(_core, m) {
  m.doc() = R"pbdoc(
        geodepot API
        ------------

        .. currentmodule:: geodepot

        .. autosummary::
           :toctree: _generate

           init
           get
    )pbdoc";

  m.def("init", &geodepot::init, "Initialize a geodepot repository");
  m.def("get", &geodepot::get, "Get the full path to a data entry");

  #ifdef VERSION_INFO
    m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
  #else
    m.attr("__version__") = "dev";
  #endif
}
