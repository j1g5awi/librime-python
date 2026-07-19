# Generator-based translator — yields candidates lazily
# Signature: rime_main(input, segment, engine)
#   input:   str — current input text
#   segment: Segment — current segment info (start, end, tags)
#   engine:  Engine — access to context, schema, config

from dataclasses import dataclass
from typing import Optional


@dataclass
class Candidate:
    text: Optional[str] = None
    candidate_type: Optional[str] = None
    comment: Optional[str] = None
    preedit: Optional[str] = None
    start: Optional[int] = None
    end: Optional[int] = None
    length: Optional[int] = None


def rime_main(input, segment, engine):
    # Access schema config
    schema = engine.schema
    config = schema.config
    page_size = config.get_int("menu/page_size") or 5

    # Check segment info
    seg_start = segment.start
    seg_end = segment.end

    # Notifier example: monitor commits
    def on_commit(ctx):
        log.info(f"committed: {ctx.input}")

    engine.context.commit_notifier.connect(on_commit)

    # Yield candidates one by one (lazy)
    for i in range(min(3, page_size)):
        yield Candidate(
            text=f"result_{i}_for_{input}",
            length=seg_end - seg_start,
            candidate_type="pygen",
            comment=f"from generator (page={page_size})",
            preedit="",
            start=seg_start,
            end=seg_end,
        )

    # Access recent commit history
    history = engine.context.commit_history
    if not history.empty():
        last = history.back()
        log.info(f"last commit: {last.text}")
