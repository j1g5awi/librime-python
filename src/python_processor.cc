#include <pybind11/embed.h>
#include <rime/key_event.h>
#include "python_processor.h"

namespace pythonext {

PythonProcessor::PythonProcessor( const rime::Ticket& ticket, py::function py_entry, rime::Engine* engine, bool pass_engine )
    : Processor { ticket }, py_entry { py_entry }, pass_engine_ { pass_engine } {}

rime::ProcessResult PythonProcessor::ProcessKeyEvent( const rime::KeyEvent& key_event ) {
    try {
        py::object key_obj { py::cast( &key_event, py::return_value_policy::reference ) };
        py::object engine_obj { py::cast( engine_, py::return_value_policy::reference ) };

        py::object result {
            pass_engine_
                ? py_entry( key_obj, engine_obj )
                : py_entry( key_obj )
        };

        int code = result.cast<int>();
        return code == 1 ? rime::kAccepted : rime::kNoop;
    } catch ( const py::error_already_set& e ) {
        LOG( ERROR ) << e.what();
        return rime::kNoop;
    }
}

}  // namespace pythonext
