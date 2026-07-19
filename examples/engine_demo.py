# Example showing how to access Rime internal objects
# via the `engine` parameter passed to rime_main
#
# Translator entry point:
#   def rime_main(input: str, engine) -> Optional[List[TranslatorAnswer]]
#
# Filter entry point:
#   def rime_main(candidate, engine) -> FilterAnswer (modified candidate)
#
# `engine` gives access to:
#   engine.schema          -> Schema (schema_id, schema_name, config, page_size)
#   engine.context         -> Context (input, composition, options, properties)
#   engine.context.composition -> Composition (iterable of Segment)
#   engine.commit_text()   -> commit text directly

from dataclasses import dataclass
from typing import List, Optional


@dataclass
class Candidate:
    text: Optional[str] = None
    candidate_type: Optional[str] = None
    comment: Optional[str] = None
    preedit: Optional[str] = None


class FilterAnswer:
    def __init__(self, should_skip=False, should_remove=False,
                 candidate_type=None, text=None, comment=None, direct_call=True):
        assert not direct_call, "Use Skip(), Remove() or Rewrite()"
        self.should_skip = should_skip
        self.should_remove = should_remove
        self.candidate_type = candidate_type
        self.text = text
        self.comment = comment

    @classmethod
    def Skip(cls):
        return cls(should_skip=True, direct_call=False)

    @classmethod
    def Remove(cls):
        return cls(should_remove=True, direct_call=False)

    @classmethod
    def Rewrite(cls, **kwargs):
        return cls(**kwargs, direct_call=False)


# ---------- Translator example: read config ----------
def rime_main(s: str, engine) -> Optional[List[Candidate]]:
    schema = engine.schema
    config = schema.config

    # Read scheme-specific settings
    page_size = config.get_int("menu/page_size") or 5
    select_keys = schema.select_keys

    # Access context
    ctx = engine.context
    composing = ctx.is_composing()
    current_input = ctx.input

    # Read the full composition (all segments)
    segments = []
    for seg in ctx.composition:
        segments.append(f"{seg.start}-{seg.end} tags={list(seg.tags)}")

    # Build a candidate showing internal state
    info = (
        f"[{schema.schema_id}] page={page_size} keys={select_keys} "
        f"composing={composing} input={current_input} "
        f"segments={segments}"
    )
    return [Candidate(text=info, length=len(s), candidate_type="pyext",
                      comment="engine demo", preedit="")]


# ---------- Filter example: skip based on schema config ----------
# def rime_main(candidate, engine) -> FilterAnswer:
#     # Only remove candidates if scheme has a specific option
#     ctx = engine.context
#     if ctx.get_option("remove_duplicates"):
#         if len(candidate.text) <= 1:
#             return FilterAnswer.Remove()
#     return FilterAnswer.Skip()
