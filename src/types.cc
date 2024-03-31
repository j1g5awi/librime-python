#include <pybind11/embed.h>
namespace py = pybind11;

class PythonCandidate{
public:
    PythonCandidate( const std::string& candidate_type, const std::string& text, const std::string& comment, const std::string& preedit )
        : candidate_type { candidate_type }, text { text }, comment { comment }, preedit { preedit } {}
    std::string candidate_type;
    std::string text;
    std::string comment;
    std::string preedit;
};

PYBIND11_EMBEDDED_MODULE(rimeext, m) {
    py::class_<PythonCandidate>(m, "PythonCandidate", py::dynamic_attr())
        .def(py::init<const std::string&, const std::string&, const std::string&, const std::string&>())
        .def_readwrite("candidate_type",&PythonCandidate::candidate_type)
        .def_readwrite("text", &PythonCandidate::text)
        .def_readwrite("comment", &PythonCandidate::comment)
        .def_readwrite("preedit", &PythonCandidate::preedit);
}
