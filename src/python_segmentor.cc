#include <pybind11/embed.h>
#include <rime/segmentation.h>
#include "python_segmentor.h"

namespace pythonext {

PythonSegmentor::PythonSegmentor(const rime::Ticket& ticket,
                                 py::function py_entry,
                                 rime::Engine* engine, bool pass_engine)
    : Segmentor(ticket), py_entry(py_entry), pass_engine_(pass_engine) {}

bool PythonSegmentor::Proceed(rime::Segmentation* segmentation) {
  if (!segmentation) return false;
  try {
    py::object seg_obj = py::cast(segmentation,
                                   py::return_value_policy::reference);
    if (pass_engine_) {
      py::object engine_obj = py::cast(engine_,
                                       py::return_value_policy::reference);
      return py_entry(seg_obj, engine_obj).cast<bool>();
    }
    return py_entry(seg_obj).cast<bool>();
  } catch (const py::error_already_set& e) {
    LOG(ERROR) << "python_segmentor py error: " << e.what();
    return false;
  } catch (const std::exception& e) {
    LOG(ERROR) << "python_segmentor cpp error: " << e.what();
    return false;
  }
}

}  // namespace pythonext
