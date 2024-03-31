# 汉语拼音（普通话）云输入

from dataclasses import dataclass
from itertools import repeat
import requests
from typing import List, Optional
import unicodedata


@dataclass
class TranslatorAnswer:
    text: Optional[str] = None
    length: Optional[int] = None
    candidate_type: Optional[str] = None
    comment: Optional[str] = None
    preedit: Optional[str] = None


def format_pinyin(s: str) -> str:
    """
    >>> format_pinyin('hao3')
    'hǎo'
    >>> format_pinyin('nv3')
    'nǚ'
    """
    s = s.replace("v", "ü")
    if s.endswith("5"):
        s = s[:-1]
    elif s.endswith(tuple("1234")):
        tone = int(s[-1])
        tone_mark = "\u0304\u0301\u030c\u0300"[tone - 1]
        s = s[:-1]
        for component in ["iu", "ui", "a", "o", "e", "i", "u", "ü"]:
            if component in s:
                s = s.replace(component, component + tone_mark)
                break
        s = unicodedata.normalize("NFC", s)
    return s


def format_pinyin_sequence(s: str) -> str:
    """
    >>> format_pinyin_sequence('ni3 hao3')
    'nǐ hǎo'
    """
    return " ".join(map(format_pinyin, s.split(" ")))


def rime_main(input_str: str) -> Optional[List[TranslatorAnswer]]:
    num_of_items = 5

    payload = {
        "text": input_str,
        "itc": "zh-hant-t-i0-pinyin",
        "num": num_of_items,
        "cp": 0,
        "cs": 1,
        "ie": "utf-8",
        "oe": "utf-8",
        "app": "test",
    }
    r = requests.get("http://inputtools.google.com/request", params=payload)
    r.raise_for_status()
    response = r.json()

    if response[0] != "SUCCESS":
        return

    _, texts, _, d = response[1][0]
    annotations = d.get("annotation", repeat(None))
    matched_lengths = d.get("matched_length", repeat(None))
    return [
        TranslatorAnswer(
            text, length=matched_length, comment=format_pinyin_sequence(annotation)
        )
        for text, annotation, matched_length in zip(texts, annotations, matched_lengths)
    ]
