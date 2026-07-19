#pragma once

#include <pybind11/embed.h>
#include <rime/common.h>
#include <rime/engine.h>
#include <rime/segmentor.h>

namespace py = pybind11;

namespace pythonext {

class PythonSegmentor : public rime::Segmentor {
 public:
  PythonSegmentor(const rime::Ticket& ticket, py::function py_entry,
                  rime::Engine* engine, bool pass_engine);
  virtual bool Proceed(rime::Segmentation* segmentation) override;

 private:
  const py::function py_entry;
  bool pass_engine_;
};

}  // namespace pythonext
