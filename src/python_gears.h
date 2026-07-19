#pragma once

#include <pybind11/embed.h>
#include <rime/common.h>
#include <rime/candidate.h>
#include <rime/translation.h>

namespace py = pybind11;

namespace pythonext {

class PythonGeneratorTranslation : public rime::Translation {
 public:
  PythonGeneratorTranslation(py::object generator);

  rime::an<rime::Candidate> Peek() override;
  bool Next() override;

  static rime::an<rime::Candidate> CandidateFromPyObject(py::object item);

 private:
  rime::an<rime::Candidate> FetchNext();
  rime::an<rime::Candidate> MakeCandidate(py::object item);

  py::object generator_;
  py::object iter_;
  rime::an<rime::Candidate> cached_;
};

}  // namespace pythonext
