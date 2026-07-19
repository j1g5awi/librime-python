#include <pybind11/embed.h>
#include <filesystem>
#include <type_traits>
#include <rime/common.h>
#include <rime/registry.h>
#include <rime_api.h>
#include "python_translator.h"
#include "python_filter.h"
#include "python_processor.h"
#include "python_segmentor.h"

namespace py = pybind11;

/**
 * Determine the path to the Python script file according to the name string of a rime component.
 *
 * @param tagName The name string of the rime component.
 * @return Path to the Python script.
 * @exception Throws `std::runtime_error` if the file does not exist.
 */
static std::string getPythonScriptRealPath( const std::string& tagName ) {
    // determine the file name
    const std::string fileName { "python/"+tagName + ".py" };

    // the file should either exist in the user directory or the shared directory
    const std::filesystem::path userDir { std::filesystem::canonical( rime_get_api()->get_user_data_dir() ) };
    const std::filesystem::path sharedDir { std::filesystem::canonical( rime_get_api()->get_shared_data_dir() ) };
    const std::filesystem::path userFile { userDir / fileName };
    const std::filesystem::path sharedFile { sharedDir / fileName };
    const bool userFileExists { std::filesystem::exists( userFile ) };
    const bool sharedFileExists { std::filesystem::exists( sharedFile ) };

    if ( !userFileExists && !sharedFileExists ) {
        LOG( ERROR ) << "Expected file " << fileName << " in the user directory ("
                     << userDir.string() << ") or in the shared directory ("
                     << sharedDir.string() << ").";
        return "";
    }

    // prefer the file in the user directory
    return ( userFileExists ? userFile : sharedFile ).string();
}

template <typename T>
class PythonComponent : public T::Component {
   public:
    PythonComponent() {};

    T* Create( const rime::Ticket& ticket ) {
        const rime::Ticket ticket_ { ticket.engine, ticket.name_space, ticket.name_space };
        const std::string& tagName { ticket_.name_space };

        // load Python script file
        const std::string pythonScriptPath { getPythonScriptRealPath( tagName ) };
        if ( pythonScriptPath.empty() )
            return nullptr;

        LOG( INFO ) << "reading Python script file " << pythonScriptPath << ".";

        try {
            py::dict module_ns;
            py::eval_file( pythonScriptPath, module_ns, module_ns );
            py::function py_entry { module_ns[ "rime_main" ].cast<py::function>() };
            py::object code { py_entry.attr( "__code__" ) };
            int argc = code.attr( "co_argcount" ).cast<int>();
            int flags = code.attr( "co_flags" ).cast<int>();
            if constexpr ( std::is_same_v<T, pythonext::PythonTranslator> ) {
                py::object inspect = py::module_::import( "inspect" );
                bool is_gen = inspect.attr( "isgeneratorfunction" )( py_entry ).cast<bool>();
                return new T { ticket_, py_entry, ticket.engine, argc, is_gen };
            } else {
                bool pass_engine = ( argc >= 2 ) || ( flags & 0x04 );
                return new T { ticket_, py_entry, ticket.engine, pass_engine };
            }
        } catch ( const py::error_already_set& e ) {
            LOG( ERROR ) << e.what();
            return nullptr;
        }
    }
};

static void rime_pythonext_initialize() {
    LOG( INFO ) << "registering components from module 'pythonext'.";

    try {
        py::initialize_interpreter();
    } catch ( const std::runtime_error& e ) {
        LOG( WARNING ) << "Python interpreter already initialized: " << e.what();
    }

    // ensure embedded rimeext module is loaded into the running interpreter
    try {
        py::module_::import( "rimeext" );
    } catch ( const py::error_already_set& e ) {
        LOG( ERROR ) << "failed to import rimeext module: " << e.what();
    }
    try {
        py::module_::import( "rimeext_ext" );
    } catch ( const py::error_already_set& ) {
        // optional module, ignore
    }

    rime::Registry& r { rime::Registry::instance() };
    r.Register( "python_translator", new PythonComponent<pythonext::PythonTranslator>() );
    r.Register( "python_filter", new PythonComponent<pythonext::PythonFilter>() );
    r.Register( "python_processor", new PythonComponent<pythonext::PythonProcessor>() );
    r.Register( "python_segmentor", new PythonComponent<pythonext::PythonSegmentor>() );
}

static void rime_pythonext_finalize() {
    if ( Py_IsInitialized() ) {
        py::finalize_interpreter();
    }
}

RIME_REGISTER_MODULE( pythonext )
