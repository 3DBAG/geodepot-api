#include <nanobind/nanobind.h>

#include <geodepot/geodepot.h>

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

namespace nb = nanobind;

NB_MODULE(_core, m) {
  m.doc() = R"pbdoc(
        geodepot API
        ------------

        .. currentmodule:: geodepot_api

        .. autosummary::
           :toctree: _generate

           get
    )pbdoc";

  nb::class_<geodepot::Repository>(m, "Repository")
    .def(nb::init_implicit<const std::string_view>(), "Initialize a geodepot repository from a local path or a URL")
    .def("get", &geodepot::Repository::get, "Get the full path to a data entry");

  #ifdef VERSION_INFO
    m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
  #else
    m.attr("__version__") = "dev";
  #endif
}
