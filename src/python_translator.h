#pragma once

#include <pybind11/embed.h>
#include <rime/common.h>
#include <rime/engine.h>
#include <rime/translation.h>
#include <rime/translator.h>

namespace py = pybind11;

namespace pythonext {

class PythonTranslator : public rime::Translator {
   public:
    PythonTranslator( const rime::Ticket& ticket, py::function py_entry,
                      rime::Engine* engine, int argc, bool is_generator );
    virtual rime::an<rime::Translation> Query( const std::string& input,
                                               const rime::Segment& segment ) noexcept override;

   private:
    rime::an<rime::Translation> QueryList(const std::string& input,
                                           const rime::Segment& segment);
    rime::an<rime::Translation> QueryGenerator(const std::string& input,
                                                const rime::Segment& segment);
    py::object BuildArgs(const std::string& input, const rime::Segment& segment);

    const py::function py_entry;
    rime::Engine* engine_;
    int argc_;
    bool is_generator_;
};

}  // namespace pythonext
