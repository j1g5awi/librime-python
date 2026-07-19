from datetime import datetime
from dataclasses import dataclass
from typing import List, Optional


@dataclass
class TranslatorAnswer:
    text: Optional[str] = None
    length: Optional[int] = None
    candidate_type: Optional[str] = None
    comment: Optional[str] = None
    preedit: Optional[str] = None


def rime_main(s: str, segment, engine):
    if s == "date":
        now = datetime.now()
        date = now.strftime("%Y-%m-%d")
        return [
            TranslatorAnswer(
                text=date,
                length=len(s),
                candidate_type="date",
                comment=f"{now.year}-{now.month:02d}-{now.day:02d}",
                preedit="",
            )
        ]
