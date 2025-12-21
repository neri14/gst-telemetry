#!/usr/bin/env python3

import argparse
import os
import sys

os.environ["GST_PLUGIN_PATH"] = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "builddir"))

import gi
gi.require_version("Gst", "1.0")
from gi.repository import Gst, GLib

Gst.init(None)

parser = argparse.ArgumentParser(description="Transcode a file with GStreamer")
parser.add_argument("input", help="input file path")
parser.add_argument("output", help="output file path")
parser.add_argument("--track", help="GPS track file path", default=None)
parser.add_argument("--layout", help="Layout XML file path", default=None)
args = parser.parse_args()

INPUT = args.input
OUTPUT = args.output
TRACK = args.track
LAYOUT = args.layout

pipeline = Gst.Pipeline.new("test-pipeline")

# source / demuxer
src = Gst.ElementFactory.make("filesrc", "src")
src.set_property("location", INPUT)
dec = Gst.ElementFactory.make("decodebin", "dec")

# muxer / sink
mux = Gst.ElementFactory.make("mp4mux", "mux")
mux.set_property("faststart", True)
sink = Gst.ElementFactory.make("filesink", "sink")
sink.set_property("location", OUTPUT)

for e in (src, dec, mux, sink):
    if not e:
        print("Missing element:", e)
        sys.exit(1)

pipeline.add(src)
pipeline.add(dec)
pipeline.add(mux)
pipeline.add(sink)

if not Gst.Element.link(src, dec):
    print("Failed to link src -> dec")
    sys.exit(1)

if not Gst.Element.link(mux, sink):
    print("Failed to link mux -> sink")
    sys.exit(1)

branches = {"video": None, "audio": None}

def create_video_branch():
    q = Gst.ElementFactory.make("queue", "q_video")
    convert = Gst.ElementFactory.make("videoconvert", "vc")

    telemetry = Gst.ElementFactory.make("telemetry", "tele")
    telemetry.set_property("offset", 1.0)
    if TRACK:
        telemetry.set_property("track", TRACK)
    if LAYOUT:
        telemetry.set_property("layout", LAYOUT)

    enc = Gst.ElementFactory.make("x264enc", "enc_v")
    enc.set_property("tune", "zerolatency")
    enc.set_property("bitrate", 120000)  # kbps
    enc.set_property("speed-preset", "ultrafast")
    enc.set_property("threads", 0) # auto
    q_out = Gst.ElementFactory.make("queue", "q_video_out")

    for e in (q, convert, telemetry, enc, q_out):
        if not e:
            print("Missing video element; check installed plugins")
            sys.exit(1)

    pipeline.add(q)
    pipeline.add(convert)
    pipeline.add(telemetry)
    pipeline.add(enc)
    pipeline.add(q_out)
    q.sync_state_with_parent()
    convert.sync_state_with_parent()
    telemetry.sync_state_with_parent()
    enc.sync_state_with_parent()
    q_out.sync_state_with_parent()
    if not q.link(convert) or not convert.link(telemetry) or not telemetry.link(enc) or not enc.link(q_out):
        print("Failed to link video branch")
        sys.exit(1)

    # link encoder src pad to mux request pad
    q_out_src = q_out.get_static_pad("src")
    req = mux.get_request_pad("video_%u")
    if q_out_src.link(req) != Gst.PadLinkReturn.OK:
        print("Failed to link video encoder -> mux")
        sys.exit(1)

    branches["video"] = q

def create_audio_branch():
    q = Gst.ElementFactory.make("queue", "q_audio")
    aconv = Gst.ElementFactory.make("audioconvert", "ac")
    ares = Gst.ElementFactory.make("audioresample", "ar")
    aenc = Gst.ElementFactory.make("avenc_aac", "enc_a")
    if not aenc:
        # fallback to faac/voaacenc if present
        aenc = Gst.ElementFactory.make("faac", "enc_a")

    try:
        aenc.set_property("bitrate", 128000)
    except Exception:
        pass

    for e in (q, aconv, ares, aenc):
        if not e:
            print("Missing audio element; check installed plugins")
            sys.exit(1)

    pipeline.add(q)
    pipeline.add(aconv)
    pipeline.add(ares)
    pipeline.add(aenc)
    q.sync_state_with_parent()
    aconv.sync_state_with_parent()
    ares.sync_state_with_parent()
    aenc.sync_state_with_parent()

    if not q.link(aconv) or not aconv.link(ares) or not ares.link(aenc):
        print("Failed to link audio branch")
        sys.exit(1)

    aenc_src = aenc.get_static_pad("src")
    req = mux.get_request_pad("audio_%u")
    if aenc_src.link(req) != Gst.PadLinkReturn.OK:
        print("Failed to link audio encoder -> mux")
        sys.exit(1)

    branches["audio"] = q

def on_pad_added(db, pad):
    caps = pad.get_current_caps() or pad.query_caps(None)
    s = caps.to_string()
    if "video" in s:
        if branches["video"] is None:
            create_video_branch()
        sinkpad = branches["video"].get_static_pad("sink")
    elif "audio" in s:
        if branches["audio"] is None:
            create_audio_branch()
        sinkpad = branches["audio"].get_static_pad("sink")
    else:
        return

    if sinkpad.is_linked():
        return

    res = pad.link(sinkpad)
    if res != Gst.PadLinkReturn.OK:
        print("Failed to link decodepad -> branch:", res)

dec.connect("pad-added", on_pad_added)

# bus watch
bus = pipeline.get_bus()
loop = GLib.MainLoop()

def on_message(bus, msg):
    t = msg.type
    if t == Gst.MessageType.EOS:
        print("EOS")
        loop.quit()
    elif t == Gst.MessageType.ERROR:
        err, dbg = msg.parse_error()
        print("Error:", err, dbg)
        loop.quit()
    return True

bus.add_signal_watch()
bus.connect("message", on_message)

pipeline.set_state(Gst.State.PLAYING)

# Progress reporting: periodic query of position/duration and file size
def _format_time(ns):
    return float(ns) / Gst.SECOND

def progress_cb():
    ok_pos, pos = pipeline.query_position(Gst.Format.TIME)
    ok_dur, dur = pipeline.query_duration(Gst.Format.TIME)

    if ok_pos:
        pos_s = _format_time(pos)
    else:
        pos_s = None

    if ok_dur and dur > 0:
        dur_s = _format_time(dur)
    else:
        dur_s = None

    if pos_s is not None and dur_s is not None:
        pct = (pos / dur) * 100 if dur > 0 else 0
        print(f"Progress: {pos_s:.2f}s / {dur_s:.2f}s ({pct:.1f}%)")
    elif pos_s is not None:
        print(f"Progress: {pos_s:.2f}s")
    else:
        print("Progress: unknown")

    return True  # keep the timeout active

# call every 2000 ms
GLib.timeout_add(2000, progress_cb)

try:
    loop.run()
except KeyboardInterrupt:
    pass
pipeline.set_state(Gst.State.NULL)
print("Done")