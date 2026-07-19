#include <pybind11/embed.h>
#include <rime/candidate.h>
#include <rime/segmentation.h>
#include "python_translator.h"
#include "python_gears.h"

namespace pythonext {

PythonTranslator::PythonTranslator( const rime::Ticket& ticket,
                                    py::function py_entry,
                                    rime::Engine* engine,
                                    int argc, bool is_generator )
    : Translator { ticket }, py_entry { py_entry },
      engine_ { engine }, argc_ { argc }, is_generator_ { is_generator } {}

py::object PythonTranslator::BuildArgs(const std::string& input,
                                       const rime::Segment& segment) {
  py::object engine_obj = py::cast(engine_,
                                   py::return_value_policy::reference);
  py::object seg_obj = py::cast(segment);
  switch (argc_) {
    case 1:
      return py_entry(input);
    case 2:
      return py_entry(input, seg_obj);
    default:
      return py_entry(input, seg_obj, engine_obj);
  }
}

rime::an<rime::Translation> PythonTranslator::QueryGenerator(
    const std::string& input, const rime::Segment& segment) {
  py::object gen = BuildArgs(input, segment);
  if (gen.is_none())
    return nullptr;
  return rime::New<PythonGeneratorTranslation>(gen);
}

rime::an<rime::Translation> PythonTranslator::QueryList(
    const std::string& input, const rime::Segment& segment) {
  const size_t segment_start { segment.start };
  const size_t segment_end { segment.end };
  const py::object output { BuildArgs(input, segment) };
  if ( output.is_none() )
    return nullptr;

  const py::list output_list { output };
  if ( output_list.empty() )
    return nullptr;

  auto translation { rime::New<rime::FifoTranslation>() };
  for ( const py::handle& candidate : output_list ) {
    const std::string text { candidate.attr( "text" ).cast<std::string>() };
    const std::string candidate_type {
        candidate.attr( "candidate_type" ).cast<std::string>() };
    const std::string comment { candidate.attr( "comment" ).cast<std::string>() };
    const std::string preedit { candidate.attr( "preedit" ).cast<std::string>() };
    size_t start = segment_start, end = segment_end;
    if ( py::hasattr( candidate, "start" ) )
      start = candidate.attr( "start" ).cast<size_t>();
    if ( py::hasattr( candidate, "end" ) )
      end = candidate.attr( "end" ).cast<size_t>();
    translation->Append(
        rime::New<rime::SimpleCandidate>( candidate_type, start, end,
                                          text, comment, preedit ) );
  }
  return translation;
}

rime::an<rime::Translation> PythonTranslator::Query(
    const std::string& input, const rime::Segment& segment ) noexcept {
  try {
    if ( is_generator_ )
      return QueryGenerator( input, segment );
    else
      return QueryList( input, segment );
  } catch ( const py::error_already_set& e ) {
    LOG( ERROR ) << e.what();
    return nullptr;
  }
}

}  // namespace pythonext
