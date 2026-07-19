# Custom segmentor — controls how input is split into segments
# Signature: rime_main(segmentation, engine) -> bool
#   Return True  = segmentor accepts this segmentation
#   Return False = segmentor rejects it

def rime_main(segmentation, engine):
    # Access current input
    input_text = segmentation.input
    if not input_text:
        return False

    # Only process if Chinese mode
    if not engine.context.get_option("ascii_mode"):
        # Add a full-length segment (entire input as one)
        from rimeext import Segment
        seg = Segment(0, len(input_text))
        seg.prompt = "custom"
        seg.tags = {"abc"}
        return segmentation.add_segment(seg)

    return False
