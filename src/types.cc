#include <pybind11/embed.h>
#include <pybind11/stl.h>
#include <rime/common.h>
#include <rime/composition.h>
#include <rime/config.h>
#include <rime/config/config_types.h>
#include <rime/context.h>
#include <rime/engine.h>
#include <rime/key_event.h>
#include <rime/schema.h>
#include <rime/segmentation.h>
#include <rime/candidate.h>
#include <rime/translation.h>
#include <rime/menu.h>
#include <rime/dict/vocabulary.h>
#include <rime/dict/reverse_lookup_dictionary.h>
#include <rime/algo/algebra.h>
#include <rime/ticket.h>
#include <rime/gear/translator_commons.h>
#include <rime/switcher.h>
#include <rime_api.h>

namespace py = pybind11;
using namespace rime;

// ---------------------------------------------------------------------------
// PythonCandidate (helper class for filter)
// ---------------------------------------------------------------------------
class PythonCandidate {
 public:
  PythonCandidate(const std::string& candidate_type,
                  const std::string& text,
                  const std::string& comment,
                  const std::string& preedit)
      : candidate_type(candidate_type),
        text(text),
        comment(comment),
        preedit(preedit) {}
  std::string candidate_type;
  std::string text;
  std::string comment;
  std::string preedit;
};

// ---------------------------------------------------------------------------
// Main binding module
// ---------------------------------------------------------------------------
PYBIND11_EMBEDDED_MODULE(rimeext, m) {
  m.doc() = "Rime internal API bindings for Python plugins";

  // ========================================================================
  // PythonCandidate
  // ========================================================================
  py::class_<PythonCandidate>(m, "PythonCandidate", py::dynamic_attr())
      .def(py::init<const std::string&, const std::string&, const std::string&,
                    const std::string&>())
      .def_readwrite("candidate_type", &PythonCandidate::candidate_type)
      .def_readwrite("text", &PythonCandidate::text)
      .def_readwrite("comment", &PythonCandidate::comment)
      .def_readwrite("preedit", &PythonCandidate::preedit);

  // ========================================================================
  // Candidate
  // ========================================================================
  py::class_<Candidate, an<Candidate>>(m, "Candidate")
      .def("get_genuine", &Candidate::GetGenuineCandidate)
      .def("get_genuines", &Candidate::GetGenuineCandidates)
      .def_property("type",
                    [](const Candidate& c) { return c.type(); },
                    [](Candidate& c, const string& v) { c.set_type(v); })
      .def_property("start",
                    [](const Candidate& c) { return c.start(); },
                    [](Candidate& c, size_t v) { c.set_start(v); })
      .def_property("end",
                    [](const Candidate& c) { return c.end(); },
                    [](Candidate& c, size_t v) { c.set_end(v); })
      .def_property("quality",
                    [](const Candidate& c) { return c.quality(); },
                    [](Candidate& c, double v) { c.set_quality(v); })
      .def_property_readonly("text", &Candidate::text)
      .def_property_readonly("comment", &Candidate::comment)
      .def_property_readonly("preedit", &Candidate::preedit);

  py::class_<SimpleCandidate, Candidate, an<SimpleCandidate>>(m, "SimpleCandidate")
      .def(py::init<const string&, size_t, size_t,
                    const string&, const string&, const string&>(),
           py::arg("type"), py::arg("start"), py::arg("end"),
           py::arg("text"), py::arg("comment") = "",
           py::arg("preedit") = "")
      .def_property("text",
                    &SimpleCandidate::text,
                    &SimpleCandidate::set_text)
      .def_property("comment",
                    &SimpleCandidate::comment,
                    &SimpleCandidate::set_comment)
      .def_property("preedit",
                    &SimpleCandidate::preedit,
                    &SimpleCandidate::set_preedit);

  py::class_<ShadowCandidate, Candidate, an<ShadowCandidate>>(m, "ShadowCandidate")
      .def(py::init<const an<Candidate>&, const string&,
                    const string&, const string&, bool>(),
           py::arg("candidate"), py::arg("type"),
           py::arg("text") = "", py::arg("comment") = "",
           py::arg("inherit_comment") = true)
      .def_property_readonly("item", &ShadowCandidate::item);

  py::class_<UniquifiedCandidate, Candidate, an<UniquifiedCandidate>>(m, "UniquifiedCandidate")
      .def(py::init<const an<Candidate>&, const string&,
                    const string&, const string&>(),
           py::arg("candidate"), py::arg("type"),
           py::arg("text") = "", py::arg("comment") = "")
      .def("append", &UniquifiedCandidate::Append)
      .def_property_readonly("items", &UniquifiedCandidate::items);

  // ========================================================================
  // Menu
  // ========================================================================
  py::class_<Menu, an<Menu>>(m, "Menu")
      .def(py::init<>())
      .def("add_translation", &Menu::AddTranslation)
      .def("prepare", &Menu::Prepare)
      .def("get_candidate_at", &Menu::GetCandidateAt)
      .def_property_readonly("candidate_count", &Menu::candidate_count)
      .def("empty", &Menu::empty);

  // ========================================================================
  // Translation
  // ========================================================================
  py::class_<Translation, an<Translation>>(m, "Translation")
      .def_property_readonly("exhausted", &Translation::exhausted);

  py::class_<FifoTranslation, Translation, an<FifoTranslation>>(m, "FifoTranslation")
      .def(py::init<>())
      .def("append", &FifoTranslation::Append);

  // ========================================================================
  // Segment
  // ========================================================================
  py::class_<Segment>(m, "Segment")
      .def(py::init<int, int>(), py::arg("start_pos"), py::arg("end_pos"))
      .def_readwrite("start", &Segment::start)
      .def_readwrite("end", &Segment::end)
      .def_readwrite("length", &Segment::length)
      .def_property("status",
                     [](const Segment& s) -> py::object {
                       switch (s.status) {
                         case Segment::kVoid:      return py::str("void");
                         case Segment::kGuess:     return py::str("guess");
                         case Segment::kSelected:  return py::str("selected");
                         case Segment::kConfirmed: return py::str("confirmed");
                         default:                  return py::str("unknown");
                       }
                     },
                     [](Segment& s, const string& v) {
                       if (v == "void")      s.status = Segment::kVoid;
                       else if (v == "guess")     s.status = Segment::kGuess;
                       else if (v == "selected")  s.status = Segment::kSelected;
                       else if (v == "confirmed") s.status = Segment::kConfirmed;
                     })
      .def_readwrite("tags", &Segment::tags)
      .def_readwrite("menu", &Segment::menu)
      .def_readwrite("selected_index", &Segment::selected_index)
      .def_readwrite("prompt", &Segment::prompt)
      .def("clear", &Segment::Clear)
      .def("close", &Segment::Close)
      .def("reopen", &Segment::Reopen)
      .def("has_tag", &Segment::HasTag)
      .def("has_any_tag_in", &Segment::HasAnyTagIn)
      .def("get_candidate_at", &Segment::GetCandidateAt)
      .def("get_selected_candidate", &Segment::GetSelectedCandidate);

  // ========================================================================
  // Segmentation
  // ========================================================================
  py::class_<Segmentation>(m, "Segmentation")
      .def(py::init<>())
      .def("add_segment", &Segmentation::AddSegment)
      .def("forward", &Segmentation::Forward)
      .def("trim", &Segmentation::Trim)
      .def("has_finished_segmentation", &Segmentation::HasFinishedSegmentation)
      .def("get_current_start_position", &Segmentation::GetCurrentStartPosition)
      .def("get_current_end_position", &Segmentation::GetCurrentEndPosition)
      .def("get_current_segment_length", &Segmentation::GetCurrentSegmentLength)
      .def("get_confirmed_position", &Segmentation::GetConfirmedPosition)
      .def_property_readonly("input", &Segmentation::input)
      .def("empty",
           [](const Segmentation& s) { return s.empty(); })
      .def("size",
           [](const Segmentation& s) { return s.size(); })
      .def("back",
           [](Segmentation& s) -> Segment& { return s.back(); },
           py::return_value_policy::reference)
      .def("pop_back",
           [](Segmentation& s) { s.pop_back(); })
      .def("__getitem__",
           [](Segmentation& s, size_t i) -> Segment& { return s[i]; },
           py::return_value_policy::reference)
      .def("__iter__",
           [](Segmentation& s) {
             return py::make_iterator(s.begin(), s.end());
           },
           py::keep_alive<0, 1>());

  // ========================================================================
  // Composition (inherits Segmentation)
  // ========================================================================
  py::class_<Composition, Segmentation>(m, "Composition")
      .def(py::init<>())
      .def("has_finished_composition", &Composition::HasFinishedComposition)
      .def("get_prompt", &Composition::GetPrompt)
      .def("get_commit_text", &Composition::GetCommitText)
      .def("get_script_text", &Composition::GetScriptText,
           py::arg("keep_selection") = true);

  // ========================================================================
  // Preedit
  // ========================================================================
  py::class_<Preedit>(m, "Preedit")
      .def_readwrite("text", &Preedit::text)
      .def_readwrite("caret_pos", &Preedit::caret_pos)
      .def_readwrite("sel_start", &Preedit::sel_start)
      .def_readwrite("sel_end", &Preedit::sel_end);

  // ========================================================================
  // CommitRecord + CommitHistory
  // ========================================================================
  py::class_<CommitRecord>(m, "CommitRecord")
      .def(py::init<const string&, const string&>(),
           py::arg("type"), py::arg("text"))
      .def_readwrite("type", &CommitRecord::type)
      .def_readwrite("text", &CommitRecord::text);

  py::class_<CommitHistory>(m, "CommitHistory")
      .def(py::init<>())
      .def("push",
           [](CommitHistory& h, const string& type, const string& text) {
             h.push_back(CommitRecord(type, text));
           })
      .def("push_key_event",
           [](CommitHistory& h, const KeyEvent& ke) { h.push_back(CommitRecord(ke.keycode())); })
      .def("back",
           [](CommitHistory& h) -> py::object {
             if (h.empty()) return py::none();
             return py::cast(&h.back(), py::return_value_policy::reference);
           })
      .def("pop_back", [](CommitHistory& h) { h.pop_back(); })
      .def("clear", [](CommitHistory& h) { h.clear(); })
      .def("empty", [](CommitHistory& h) { return h.empty(); })
      .def("repr", &CommitHistory::repr)
      .def("latest_text", &CommitHistory::latest_text)
      .def("__len__", [](const CommitHistory& h) { return h.size(); })
      .def("__getitem__",
           [](CommitHistory& h, size_t i) -> CommitRecord& {
             auto it = h.begin(); std::advance(it, i); return *it;
           },
           py::return_value_policy::reference)
      .def("__iter__",
           [](CommitHistory& h) {
             return py::make_iterator(h.begin(), h.end());
           },
           py::keep_alive<0, 1>());

  // ========================================================================
  // Context
  // ========================================================================
  py::class_<Context>(m, "Context")
      .def("commit", &Context::Commit)
      .def("get_commit_text", &Context::GetCommitText)
      .def("get_script_text", &Context::GetScriptText)
      .def("get_preedit", &Context::GetPreedit)
      .def("is_composing", &Context::IsComposing)
      .def("has_menu", &Context::HasMenu)
      .def("get_selected_candidate", &Context::GetSelectedCandidate)
      .def("push_input",
           py::overload_cast<char>(&Context::PushInput))
      .def("push_input",
           py::overload_cast<const string&>(&Context::PushInput))
      .def("pop_input", &Context::PopInput, py::arg("len") = 1)
      .def("delete_input", &Context::DeleteInput, py::arg("len") = 1)
      .def("clear", &Context::Clear)
      .def("select", &Context::Select)
      .def("highlight", &Context::Highlight)
      .def("confirm_current_selection", &Context::ConfirmCurrentSelection)
      .def("delete_current_selection", &Context::DeleteCurrentSelection)
      .def("confirm_previous_selection", &Context::ConfirmPreviousSelection)
      .def("reopen_previous_selection", &Context::ReopenPreviousSelection)
      .def("clear_previous_segment", &Context::ClearPreviousSegment)
      .def("reopen_previous_segment", &Context::ReopenPreviousSegment)
      .def("clear_non_confirmed_composition", &Context::ClearNonConfirmedComposition)
      .def("refresh_non_confirmed_composition", &Context::RefreshNonConfirmedComposition)
      .def_property("input",
                    [](const Context& c) -> const string& { return c.input(); },
                    [](Context& c, const string& v) { c.set_input(v); })
      .def_property("caret_pos",
                    &Context::caret_pos,
                    &Context::set_caret_pos)
      .def_property("composition",
                    [](Context& c) -> Composition& { return c.composition(); },
                     [](Context& c, Composition comp) { c.set_composition(std::move(comp)); },
                    py::return_value_policy::reference)
      .def_property_readonly("commit_history",
                             [](Context& c) -> CommitHistory& { return c.commit_history(); },
                             py::return_value_policy::reference)
      .def("get_option", &Context::get_option)
      .def("set_option", &Context::set_option)
      .def("get_property", &Context::get_property)
      .def("set_property", &Context::set_property)
      .def("clear_transient_options", &Context::ClearTransientOptions);

  // ========================================================================
  // KeyEvent
  // ========================================================================
  py::class_<KeyEvent>(m, "KeyEvent")
      .def(py::init<>())
      .def(py::init<int, int>(), py::arg("keycode"), py::arg("modifier"))
      .def(py::init<const string&>(), py::arg("repr"))
      .def("keycode",
           [](const KeyEvent& k) { return k.keycode(); })
      .def("modifier",
           [](const KeyEvent& k) { return k.modifier(); })
      .def("repr", &KeyEvent::repr)
      .def("shift", &KeyEvent::shift)
      .def("ctrl", &KeyEvent::ctrl)
      .def("alt", &KeyEvent::alt)
      .def("caps", &KeyEvent::caps)
      .def("super", &KeyEvent::super)
      .def("release", &KeyEvent::release)
      .def("__eq__", &KeyEvent::operator==)
      .def("__lt__", &KeyEvent::operator<);

  // ========================================================================
  // KeySequence
  // ========================================================================
  py::class_<KeySequence>(m, "KeySequence")
      .def(py::init<>())
      .def(py::init<const string&>(), py::arg("repr"))
      .def("parse", &KeySequence::Parse)
      .def("repr", &KeySequence::repr)
      .def("to_key_events",
           [](const KeySequence& ks) {
             py::list result;
             for (const auto& ke : ks) result.append(ke);
             return result;
           })
      .def("__len__", [](const KeySequence& ks) { return ks.size(); })
      .def("__getitem__",
           [](const KeySequence& ks, size_t i) { return ks[i]; });

  // ========================================================================
  // Engine
  // ========================================================================
  py::class_<Engine>(m, "Engine")
      .def("process_key",
           [](Engine& e, const KeyEvent& ke) -> int {
             return e.ProcessKey(ke) ? 1 : 2;
           })
      .def("compose", &Engine::Compose)
      .def("commit_text", &Engine::CommitText)
      .def("apply_schema",
           [](Engine& e, Schema* schema) { e.ApplySchema(schema); })
      .def_property_readonly("schema", &Engine::schema,
                             py::return_value_policy::reference)
      .def_property_readonly("context", &Engine::context,
                             py::return_value_policy::reference)
      .def_property("active_engine",
                    [](Engine& e) -> Engine* { return e.active_engine(); },
                    [](Engine& e, Engine* v) { e.set_active_engine(v); },
                    py::return_value_policy::reference);

  // ========================================================================
  // Schema
  // ========================================================================
  py::class_<Schema>(m, "Schema")
      .def(py::init<const string&>(), py::arg("schema_id"))
      .def_property_readonly("schema_id", &Schema::schema_id)
      .def_property_readonly("schema_name", &Schema::schema_name)
      .def_property("config",
                    [](Schema& s) { return s.config(); },
                    [](Schema& s, Config* c) { s.set_config(c); },
                    py::return_value_policy::reference)
      .def_property_readonly("page_size", &Schema::page_size)
      .def_property("select_keys",
                    &Schema::select_keys,
                    &Schema::set_select_keys);

  // ========================================================================
  // Config tree types
  // ========================================================================
  py::class_<ConfigItem, an<ConfigItem>>(m, "ConfigItem")
      .def_property_readonly("type",
                             [](const ConfigItem& i) -> string {
                               switch (i.type()) {
                                 case ConfigItem::kNull:   return "null";
                                 case ConfigItem::kScalar: return "scalar";
                                 case ConfigItem::kList:   return "list";
                                 case ConfigItem::kMap:    return "map";
                                 default: return "null";
                               }
                             })
      .def("empty", &ConfigItem::empty)
      .def("get_value",
           [](const an<ConfigItem>& i) -> an<ConfigValue> { return As<ConfigValue>(i); })
      .def("get_list",
           [](const an<ConfigItem>& i) -> an<ConfigList> { return As<ConfigList>(i); })
      .def("get_map",
           [](const an<ConfigItem>& i) -> an<ConfigMap> { return As<ConfigMap>(i); });

  py::class_<ConfigValue, ConfigItem, an<ConfigValue>>(m, "ConfigValue")
      .def(py::init<>())
      .def(py::init<const string&>())
      .def(py::init<bool>())
      .def(py::init<int>())
      .def(py::init<double>())
      .def("get_bool",
           [](const ConfigValue& v) -> py::object {
             bool val; return v.GetBool(&val) ? py::object(py::bool_(val)) : py::object(py::none());
           })
      .def("get_int",
           [](const ConfigValue& v) -> py::object {
             int val; return v.GetInt(&val) ? py::object(py::int_(val)) : py::object(py::none());
           })
      .def("get_double",
           [](const ConfigValue& v) -> py::object {
             double val; return v.GetDouble(&val) ? py::object(py::float_(val)) : py::object(py::none());
           })
      .def("get_string",
           [](const ConfigValue& v) -> py::object {
             string val; return v.GetString(&val) ? py::object(py::str(val)) : py::object(py::none());
           })
      .def("set_bool", &ConfigValue::SetBool)
      .def("set_int", &ConfigValue::SetInt)
      .def("set_double", &ConfigValue::SetDouble)
      .def("set_string",
           [](ConfigValue& v, const string& s) { v.SetString(s); })
      .def_property_readonly("str", &ConfigValue::str);

  py::class_<ConfigList, ConfigItem, an<ConfigList>>(m, "ConfigList")
      .def(py::init<>())
      .def("get_at", &ConfigList::GetAt)
      .def("get_value_at", &ConfigList::GetValueAt)
      .def("set_at", &ConfigList::SetAt)
      .def("append", &ConfigList::Append)
      .def("insert", &ConfigList::Insert)
      .def("clear", &ConfigList::Clear)
      .def("resize", &ConfigList::Resize)
      .def("empty", &ConfigList::empty)
      .def_property_readonly("size", &ConfigList::size)
      .def("__len__", &ConfigList::size)
      .def("__getitem__",
           [](const an<ConfigList>& l, size_t i) { return l->GetAt(i); })
      .def("__setitem__",
           [](ConfigList& l, size_t i, an<ConfigItem> item) { l.SetAt(i, item); });

  py::class_<ConfigMap, ConfigItem, an<ConfigMap>>(m, "ConfigMap")
      .def(py::init<>())
      .def("has_key", &ConfigMap::HasKey)
      .def("get", &ConfigMap::Get)
      .def("get_value", &ConfigMap::GetValue)
      .def("set", &ConfigMap::Set)
      .def("clear", &ConfigMap::Clear)
      .def("empty", &ConfigMap::empty)
      .def("keys",
            [](ConfigMap& map) {
              vector<string> keys;
              for (const auto& p : map) keys.push_back(p.first);
              return keys;
            })
      .def("__len__",
            [](ConfigMap& map) {
              size_t n = 0;
              for (auto it = map.begin(); it != map.end(); ++it) ++n;
              return n;
            })
      .def("__getitem__",
           [](const an<ConfigMap>& map, const string& key) { return map->Get(key); })
      .def("__setitem__",
           [](ConfigMap& map, const string& key, an<ConfigItem> item) { map.Set(key, item); })
      .def("__contains__",
           [](const ConfigMap& map, const string& key) { return map.HasKey(key); });

  // ========================================================================
  // Config
  // ========================================================================
  py::class_<Config>(m, "Config")
      .def(py::init<>())
      .def("load_from_file",
            [](Config& c, const string& p) { return c.LoadFromFile(rime::path(p)); })
      .def("save_to_file",
            [](Config& c, const string& p) { return c.SaveToFile(rime::path(p)); })
      .def("save", &Config::Save)
      .def("is_null", &Config::IsNull)
      .def("is_value", &Config::IsValue)
      .def("is_list", &Config::IsList)
      .def("is_map", &Config::IsMap)
      .def("get_bool",
           [](Config& c, const string& path) -> py::object {
             bool val; return c.GetBool(path, &val) ? py::object(py::bool_(val)) : py::object(py::none());
           })
      .def("get_int",
           [](Config& c, const string& path) -> py::object {
             int val; return c.GetInt(path, &val) ? py::object(py::int_(val)) : py::object(py::none());
           })
      .def("get_double",
           [](Config& c, const string& path) -> py::object {
             double val; return c.GetDouble(path, &val) ? py::object(py::float_(val)) : py::object(py::none());
           })
      .def("get_string",
           [](Config& c, const string& path) -> py::object {
             string val; return c.GetString(path, &val) ? py::object(py::str(val)) : py::object(py::none());
           })
      .def("get_list",
           [](Config& c, const string& path) -> an<ConfigList> {
             return c.GetList(path);
           })
      .def("get_map",
           [](Config& c, const string& path) -> an<ConfigMap> {
             return c.GetMap(path);
           })
      .def("get_list_size", &Config::GetListSize)
      .def("set_bool", &Config::SetBool)
      .def("set_int", &Config::SetInt)
      .def("set_double", &Config::SetDouble)
      .def("set_string",
           py::overload_cast<const string&, const string&>(&Config::SetString))
      .def("set_string",
           [](Config& c, const string& path, const char* v) { return c.SetString(path, v); })
      .def("set_item",
           [](Config& c, const string& path, an<ConfigItem> item) { return c.SetItem(path, item); })
      .def("get_item",
           [](Config& c, const string& path) { return c.GetItem(path); })
      .def("get_value",
           [](Config& c, const string& path) { return c.GetValue(path); });

  // ========================================================================
  // Projection
  // ========================================================================
  py::class_<Projection, an<Projection>>(m, "Projection")
      .def(py::init<>())
      .def("load",
           [](Projection& p, py::object rules) {
             if (py::isinstance<py::list>(rules)) {
               auto cl = New<ConfigList>();
               for (auto item : rules) {
                 if (py::isinstance<py::str>(item))
                   cl->Append(New<ConfigValue>(item.cast<string>()));
               }
               return p.Load(cl);
             }
             if (py::isinstance<ConfigList>(rules))
               return p.Load(rules.cast<an<ConfigList>>());
             return false;
           })
      .def("apply",
           [](Projection& p, const string& s) -> string {
             string result = s;
             p.Apply(&result);
             return result;
           },
           py::arg("text"));

  // ========================================================================
  // Spans
  // ========================================================================
  py::class_<Spans>(m, "Spans")
      .def(py::init<>())
      .def("add_vertex", &Spans::AddVertex)
      .def("add_span", &Spans::AddSpan)
      .def("add_spans", &Spans::AddSpans)
      .def("clear", &Spans::Clear)
      .def("previous_stop", &Spans::PreviousStop)
      .def("next_stop", &Spans::NextStop)
      .def("has_vertex", &Spans::HasVertex)
      .def("count_between",
           [](const Spans& s, size_t start, size_t end) { return s.Count(start, end); })
      .def_property_readonly("count",
                             static_cast<size_t(Spans::*)()const>(&Spans::Count))
      .def_property_readonly("start", &Spans::start)
      .def_property_readonly("end", &Spans::end);

  // ========================================================================
  // DictEntry + Code
  // ========================================================================
  py::class_<DictEntry, an<DictEntry>>(m, "DictEntry")
      .def(py::init<>())
      .def_readwrite("text", &DictEntry::text)
      .def_readwrite("comment", &DictEntry::comment)
      .def_readwrite("preedit", &DictEntry::preedit)
      .def_readwrite("weight", &DictEntry::weight)
      .def_readwrite("commit_count", &DictEntry::commit_count)
      .def_readwrite("custom_code", &DictEntry::custom_code)
      .def_readwrite("remaining_code_length", &DictEntry::remaining_code_length);

  py::class_<Code>(m, "Code")
      .def(py::init<>())
      .def("push",
           [](Code& c, int syllable_id) { c.push_back(syllable_id); })
      .def("to_list",
           [](const Code& c) {
             py::list result;
             for (auto v : c) result.append(v);
             return result;
           })
      .def("__len__", [](const Code& c) { return c.size(); });

  // ========================================================================
  // Phrase
  // ========================================================================
  py::class_<Phrase, Candidate, an<Phrase>>(m, "Phrase")
      .def_property_readonly("weight", &Phrase::weight)
      .def_property_readonly("code",
                             [](const Phrase& p) -> const Code& { return p.code(); })
      .def_property_readonly("entry",
                             [](const Phrase& p) -> an<DictEntry> {
                               return New<DictEntry>(p.entry());
                             })
      .def("spans", &Phrase::spans);

  // ========================================================================
  // ReverseDb
  // ========================================================================
  py::class_<ReverseDb, an<ReverseDb>>(m, "ReverseDb")
      .def(py::init([](const string& db_name) {
             string user_dir = rime_get_api()->get_user_data_dir();
             string shared_dir = rime_get_api()->get_shared_data_dir();
             string path = user_dir + "/" + db_name + ".reverse.bin";
             if (!std::filesystem::exists(path))
               path = shared_dir + "/" + db_name + ".reverse.bin";
              return New<ReverseDb>(rime::path(path));
           }),
           py::arg("db_name"))
      .def("lookup",
           [](ReverseDb& db, const string& key) {
             string result;
             if (db.Lookup(key, &result))
               return result;
             return string();
           });

  // ========================================================================
  // Switcher
  // ========================================================================
  py::class_<Switcher, an<Switcher>>(m, "Switcher")
      .def(py::init([](Engine* engine) {
             Ticket ticket(engine, "switcher", "");
             return New<Switcher>(ticket);
           }),
           py::arg("engine"))
      .def("process_key",
           [](Switcher& s, const KeyEvent& ke) -> int {
             return s.ProcessKey(ke) ? 1 : 2;
           })
      .def("select_next_schema", &Switcher::SelectNextSchema)
      .def("refresh_menu", &Switcher::RefreshMenu)
      .def("activate", &Switcher::Activate)
      .def("deactivate", &Switcher::Deactivate)
      .def_property_readonly("active", &Switcher::active);

  // ========================================================================
  // rime_api utility table
  // ========================================================================
  py::dict rime_api;
  rime_api["get_rime_version"] = py::cpp_function([]() {
    return string(rime_get_api()->get_version());
  });
  rime_api["get_shared_data_dir"] = py::cpp_function([]() {
    return string(rime_get_api()->get_shared_data_dir());
  });
  rime_api["get_user_data_dir"] = py::cpp_function([]() {
    return string(rime_get_api()->get_user_data_dir());
  });
  rime_api["get_sync_dir"] = py::cpp_function([]() {
    return string(rime_get_api()->get_sync_dir());
  });
  rime_api["get_user_id"] = py::cpp_function([]() {
    return string(rime_get_api()->get_user_id());
  });
  m.attr("rime_api") = rime_api;

  // ========================================================================
  // log table
  // ========================================================================
  py::dict log;
  log["info"] = py::cpp_function([](const string& msg) { LOG(INFO) << msg; });
  log["warning"] = py::cpp_function([](const string& msg) { LOG(WARNING) << msg; });
  log["error"] = py::cpp_function([](const string& msg) { LOG(ERROR) << msg; });
  m.attr("log") = log;
}
