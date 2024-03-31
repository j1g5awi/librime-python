# 删除长度为 2 的候选，并将长度为 3 的候选转为简体
from typing import Optional
from dataclasses import dataclass
from opencc import OpenCC

t2s = OpenCC("t2s").convert


@dataclass
class Candidate:
    text: Optional[str] = None
    candidate_type: Optional[str] = None
    comment: Optional[str] = None
    preedit: Optional[str] = None


class FilterAnswer:
    """
    The answer to a filter query.
    """

    def __init__(
        self,
        should_skip: bool = False,
        should_remove: bool = False,
        candidate_type: Optional[str] = None,
        text: Optional[str] = None,
        comment: Optional[str] = None,
        direct_call: bool = True,
    ):
        assert not direct_call, "Please use `Skip`, `Remove` or `Rewrite` instead of calling this constructor directly"

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


def rime_main(filter_query: Candidate) -> FilterAnswer:
    text = filter_query.text
    comment = filter_query.comment
    if len(text) == 2:
        return FilterAnswer.Remove()
    if len(text) == 3:
        return FilterAnswer.Rewrite(text=t2s(text), comment=comment + " [简]")
    return FilterAnswer.Skip()
