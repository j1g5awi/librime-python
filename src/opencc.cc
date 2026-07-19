#include <pybind11/embed.h>
#include <rime/common.h>
#include <rime_api.h>

#if __has_include(<opencc/opencc.h>)
#include <opencc/opencc.h>
#define RIME_PYTHON_HAS_OPENCC 1
#endif

namespace py = pybind11;

#ifdef RIME_PYTHON_HAS_OPENCC

namespace pythonext {

class OpenccWrapper {
 public:
  OpenccWrapper(const std::string& config) {
    std::string user_dir = rime_get_api()->get_user_data_dir();
    std::string shared_dir = rime_get_api()->get_shared_data_dir();
    std::string path;

    std::string user_path = user_dir + "/opencc/" + config + ".json";
    std::string shared_path = shared_dir + "/opencc/" + config + ".json";
    if (std::filesystem::exists(user_path))
      path = user_path;
    else if (std::filesystem::exists(shared_path))
      path = shared_path;
    else
      path = config;

    converter_ = std::make_unique<opencc::SimpleConverter>(path);
  }

  std::string Convert(const std::string& text) const {
    return converter_->Convert(text);
  }

 private:
  std::unique_ptr<opencc::SimpleConverter> converter_;
};

}  // namespace pythonext

PYBIND11_EMBEDDED_MODULE(rimeext_opencc, m) {
  m.doc() = "OpenCC bindings for librime-python";

  py::class_<pythonext::OpenccWrapper>(m, "Opencc")
      .def(py::init<const std::string&>(), py::arg("config"))
      .def("convert", &pythonext::OpenccWrapper::Convert)
      .def("convert_text", &pythonext::OpenccWrapper::Convert);
}

#else

PYBIND11_EMBEDDED_MODULE(rimeext_opencc, m) {
  m.doc() = "OpenCC bindings (not available)";
}

#endif
