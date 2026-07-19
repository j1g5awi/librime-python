#include <pybind11/embed.h>
#include <rime/candidate.h>
#include "python_gears.h"

namespace pythonext {

PythonGeneratorTranslation::PythonGeneratorTranslation(py::object generator)
    : generator_(generator) {
  iter_ = generator.attr("__iter__")();
  // prime the first candidate so Peek() works immediately
  cached_ = FetchNext();
  if (!cached_)
    set_exhausted(true);
}

rime::an<rime::Candidate> PythonGeneratorTranslation::Peek() {
  return cached_;
}

bool PythonGeneratorTranslation::Next() {
  if (exhausted())
    return false;
  cached_ = FetchNext();
  if (!cached_) {
    set_exhausted(true);
    return false;
  }
  return true;
}

rime::an<rime::Candidate> PythonGeneratorTranslation::FetchNext() {
  try {
    py::object item = iter_.attr("__next__")();
    return MakeCandidate(item);
  } catch (const py::error_already_set& e) {
    if (e.matches(PyExc_StopIteration))
      return nullptr;
    LOG(ERROR) << "generator error: " << e.what();
    return nullptr;
  }
}

rime::an<rime::Candidate> PythonGeneratorTranslation::MakeCandidate(
    py::object item) {
  std::string text = item.attr("text").cast<std::string>();
  std::string candidate_type = item.attr("candidate_type").cast<std::string>();
  std::string comment = item.attr("comment").cast<std::string>();
  std::string preedit = item.attr("preedit").cast<std::string>();
  size_t start = 0, end = 0;
  if (py::hasattr(item, "start"))
    start = item.attr("start").cast<size_t>();
  if (py::hasattr(item, "end"))
    end = item.attr("end").cast<size_t>();
  if (py::hasattr(item, "length"))
    end = start + item.attr("length").cast<size_t>();
  return rime::New<rime::SimpleCandidate>(candidate_type, start, end, text,
                                           comment, preedit);
}

}  // namespace pythonext
