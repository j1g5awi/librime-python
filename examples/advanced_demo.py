# Advanced: config tree, notifiers, reverse lookup, commit history
# Demonstrates all new binding capabilities

from dataclasses import dataclass
from typing import List, Optional
import rimeext  # noqa — ensure bindings are loaded


@dataclass
class TranslatorAnswer:
    text: Optional[str] = None
    length: Optional[int] = None
    candidate_type: Optional[str] = None
    comment: Optional[str] = None
    preedit: Optional[str] = None


def rime_main(input, segment, engine):
    ctx = engine.context
    schema = engine.schema
    config = schema.config

    # ---------------------------------------------------------------
    # 1. Config tree manipulation
    # ---------------------------------------------------------------
    # Read values
    page_size = config.get_int("menu/page_size") or 5

    # Navigate nested config
    dict_list = config.get_list("translator/dictionary")
    if dict_list is not None:
        for i in range(dict_list.size):
            item = dict_list.get_value_at(i)
            if item is not None:
                log.info(f"dict {i}: {item.str}")

    # ---------------------------------------------------------------
    # 2. Commit history
    # ---------------------------------------------------------------
    history = ctx.commit_history
    recent_text = history.latest_text() if not history.empty() else ""
    log.info(f"latest commit: {recent_text}")

    # ---------------------------------------------------------------
    # 3. Option/property manipulation
    # ---------------------------------------------------------------
    old_val = ctx.get_option("full_shape")
    ctx.set_option("simplification", True)

    # ---------------------------------------------------------------
    # 4. Notifiers (hook into context events)
    # ---------------------------------------------------------------
    conns = []

    def on_select(ctx):
        cand = ctx.get_selected_candidate()
        log.info(f"selected: {cand.text}")

    conns.append(ctx.select_notifier.connect(on_select))

    # ---------------------------------------------------------------
    # 5. Preedit
    # ---------------------------------------------------------------
    preedit = ctx.get_preedit()
    log.info(f"preedit: '{preedit.text}' at pos {preedit.caret_pos}")

    # ---------------------------------------------------------------
    # 6. Output candidates
    # ---------------------------------------------------------------
    return [
        TranslatorAnswer(
            text=f"[demo] opts={dict(old_val=old_val)}",
            length=len(input),
            candidate_type="pyadv",
            comment=recent_text or "no history",
            preedit="",
        )
    ]
