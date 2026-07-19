# QWERTY → Colemak key remapper
# Ascii mode only: remaps keys before Rime processes them
#
# Return value:
#   1 = kAccepted (key handled, consume it)
#   2 = kNoop    (key not handled, pass through)

keymap = {}

qwerty = "qwertyuioasdfghjklzxcvbnm"
colemak = "qwfpgjluyarstdhneizxcvbkm"
for q, c in zip(qwerty, colemak):
    keymap[q] = c
    keymap["Shift+" + q.upper()] = c.upper()

keymap["p"] = ":"
keymap["Shift+P"] = ";"
keymap["semicolon"] = "o"
keymap["Shift+colon"] = "O"
keymap["apostrophe"] = '"'
keymap["Shift+quotedbl"] = "'"


def rime_main(key, engine):
    context = engine.context

    if not context.get_option("ascii_mode"):
        return 2  # kNoop

    if key.release() or key.alt() or key.super() or key.ctrl():
        return 2  # kNoop

    value = keymap.get(key.repr())
    if value is not None:
        engine.commit_text(value)
        return 1  # kAccepted

    return 2  # kNoop
