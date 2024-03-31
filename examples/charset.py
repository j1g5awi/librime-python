# 过滤拓展区汉字

import re

# https://ayaka.shn.hk/hanregex/
# CJK Unified Ideographs Extension A to G
extblk = re.compile(
    r"[\u3400-\u4dbf\U00020000-\U0002a6df\U0002a700-\U0002ebef\U00030000-\U0003134f]"
)


def rime_main(candidate):
    candidate.should_skip = False
    candidate.should_remove = False
    if extblk.match(candidate.text):
        candidate.comment = "拓展区"
        return candidate
    candidate.should_skip = True
    return candidate
