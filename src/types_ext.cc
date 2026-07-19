#include <pybind11/embed.h>
#include <pybind11/stl.h>
#include <rime/common.h>
#include <rime/dict/reverse_lookup_dictionary.h>

namespace py = pybind11;
using namespace rime;

PYBIND11_EMBEDDED_MODULE(rimeext_ext, m) {
  m.doc() = "Extended Rime API bindings";

  py::class_<ReverseLookupDictionary, an<ReverseLookupDictionary>>(
      m, "ReverseLookupDictionary")
      .def(py::init([](const string& dict_name) {
             auto component =
                 dynamic_cast<ReverseLookupDictionaryComponent*>(
                     ReverseLookupDictionary::Require("reverse_lookup_dictionary"));
             if (!component)
               return an<ReverseLookupDictionary>();
             return an<ReverseLookupDictionary>(component->Create(dict_name));
           }),
           py::arg("dict_name"))
      .def("lookup",
           [](ReverseLookupDictionary& d, const string& text) -> py::object {
             string result;
             if (d.ReverseLookup(text, &result))
               return py::str(result);
             return py::none();
           })
      .def("lookup_stems",
           [](ReverseLookupDictionary& d, const string& text) -> py::object {
             string result;
             if (d.LookupStems(text, &result))
               return py::str(result);
             return py::none();
           });
}
