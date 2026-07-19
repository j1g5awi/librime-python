#pragma once

#include <pybind11/embed.h>
#include <rime/common.h>
#include <rime/engine.h>
#include <rime/processor.h>

namespace py = pybind11;

namespace pythonext {

class PythonProcessor : public rime::Processor {
   public:
    PythonProcessor( const rime::Ticket& ticket, py::function py_entry, rime::Engine* engine, bool pass_engine );
    virtual rime::ProcessResult ProcessKeyEvent( const rime::KeyEvent& key_event ) override;

   private:
    const py::function py_entry;
    bool pass_engine_;
};

}  // namespace pythonext
