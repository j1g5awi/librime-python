#include <pybind11/embed.h>
#include <filesystem>
#include <mutex>
#include <type_traits>
#include <rime/common.h>
#include <rime/registry.h>
#include <rime_api.h>
#include "python_translator.h"
#include "python_filter.h"
#include "python_processor.h"
#include "python_segmentor.h"

namespace py = pybind11;

static std::once_flag g_python_init_flag;

static void InitializePythonOnce();
static void InitPythonAndComponents();

static void InitializePythonOnce() {
    try {
        py::initialize_interpreter();
    } catch ( const std::runtime_error& e ) {
        LOG( WARNING ) << "Python interpreter already initialized: " << e.what();
    }
    try {
        py::module_::import( "rimeext" );
    } catch ( const py::error_already_set& e ) {
        LOG( ERROR ) << "failed to import rimeext module: " << e.what();
    }
    try {
        py::module_::import( "rimeext_ext" );
    } catch ( const py::error_already_set& ) {
    }
}

/**
 * Determine the path to the Python script file according to the name string of a rime component.
 *
 * @param tagName The name string of the rime component.
 * @return Path to the Python script.
 * @exception Throws `std::runtime_error` if the file does not exist.
 */
static std::string getPythonScriptRealPath( const std::string& tagName ) {
    const std::string fileName { "python/"+tagName + ".py" };

    auto resolveDir = [](const char* (*getter)()) -> std::filesystem::path {
        if (!getter) return {};
        const char* dir = getter();
        if (!dir || !*dir) return {};
        try {
            return std::filesystem::canonical(dir);
        } catch (const std::exception& e) {
            LOG(WARNING) << "failed to resolve path '" << dir << "': " << e.what();
            return std::filesystem::path(dir);
        }
    };

    const auto userDir = resolveDir(rime_get_api()->get_user_data_dir);
    const auto sharedDir = resolveDir(rime_get_api()->get_shared_data_dir);
    const auto userFile = userDir / fileName;
    const auto sharedFile = sharedDir / fileName;
    const bool userFileExists = !userDir.empty() && std::filesystem::exists(userFile);
    const bool sharedFileExists = !sharedDir.empty() && std::filesystem::exists(sharedFile);

    if (!userFileExists && !sharedFileExists) {
        LOG(ERROR) << "Expected file " << fileName << " not found.";
        return "";
    }

    return (userFileExists ? userFile : sharedFile).string();
}

template <typename T>
class PythonComponent : public T::Component {
   public:
    PythonComponent() {};

    T* Create( const rime::Ticket& ticket ) {
        LOG(INFO) << "PythonComponent::Create klass=" << ticket.klass
                  << " ns=" << ticket.name_space;

        InitPythonAndComponents();

        const rime::Ticket ticket_ { ticket.engine, ticket.name_space, ticket.name_space };
        const std::string& tagName { ticket_.name_space };

        const std::string pythonScriptPath { getPythonScriptRealPath( tagName ) };
        if ( pythonScriptPath.empty() ) {
            LOG(ERROR) << "script not found for tag '" << tagName << "'";
            return nullptr;
        }

        LOG( INFO ) << "reading Python script file " << pythonScriptPath << ".";

        try {
            py::dict module_ns;
            py::eval_file( pythonScriptPath, module_ns, module_ns );
            if ( !module_ns.contains( "rime_main" ) ) {
                LOG(ERROR) << "rime_main not defined in " << pythonScriptPath;
                return nullptr;
            }
            py::function py_entry { module_ns[ "rime_main" ].cast<py::function>() };
            py::object code { py_entry.attr( "__code__" ) };
            int argc = code.attr( "co_argcount" ).cast<int>();
            int flags = code.attr( "co_flags" ).cast<int>();
            if constexpr ( std::is_same_v<T, pythonext::PythonTranslator> ) {
                py::object inspect = py::module_::import( "inspect" );
                bool is_gen = inspect.attr( "isgeneratorfunction" )( py_entry ).cast<bool>();
                LOG(INFO) << "creating translator, is_gen=" << is_gen;
                return new T { ticket_, py_entry, ticket.engine, argc, is_gen };
            } else {
                bool pass_engine = ( argc >= 2 ) || ( flags & 0x04 );
                LOG(INFO) << "creating processor/filter/segmentor, pass_engine=" << pass_engine;
                return new T { ticket_, py_entry, ticket.engine, pass_engine };
            }
        } catch ( const py::error_already_set& e ) {
            LOG( ERROR ) << "python error: " << e.what();
            return nullptr;
        } catch ( const std::exception& e ) {
            LOG( ERROR ) << "cpp error: " << e.what();
            return nullptr;
        }
    }
};

static void InitPythonAndComponents() {
    std::call_once(g_python_init_flag, []() {
        InitializePythonOnce();
        rime::Registry::instance().Register(
            "python_translator", new PythonComponent<pythonext::PythonTranslator>());
        rime::Registry::instance().Register(
            "python_filter", new PythonComponent<pythonext::PythonFilter>());
        rime::Registry::instance().Register(
            "python_processor", new PythonComponent<pythonext::PythonProcessor>());
        rime::Registry::instance().Register(
            "python_segmentor", new PythonComponent<pythonext::PythonSegmentor>());
    });
}

// Register via RIME_REGISTER_MODULE (called by fcitx5-rime on Linux).
#if defined(_WIN32)
// Windows: register via static init (module init may not be called).
template <typename T>
struct ComponentRegistrar {
    ComponentRegistrar(const char* name) {
        rime::Registry::instance().Register(name, new PythonComponent<T>());
    }
};
static ComponentRegistrar<pythonext::PythonTranslator> reg_translator("python_translator");
static ComponentRegistrar<pythonext::PythonFilter> reg_filter("python_filter");
static ComponentRegistrar<pythonext::PythonProcessor> reg_processor("python_processor");
static ComponentRegistrar<pythonext::PythonSegmentor> reg_segmentor("python_segmentor");

static void rime_pythonext_initialize() {}
static void rime_pythonext_finalize() {}
#else
// Linux: register in module init (avoids cross-library dynamic_cast issues).
static void rime_pythonext_initialize() {
    InitPythonAndComponents();
}
static void rime_pythonext_finalize() {}
#endif

RIME_REGISTER_MODULE(pythonext)
