#!/bin/bash

INPUT_FILE=""
OUTPUT_FILE=""
TRACK_FILE=""
LAYOUT_FILE=""
OFFSET_VALUE="3.5"
GPU_MODE=false

while [[ $# -gt 0 ]]; do
    case $1 in
        --track)
            TRACK_FILE="$2"
            shift 2
            ;;
        --layout)
            LAYOUT_FILE="$2"
            shift 2
            ;;
        --offset)
            OFFSET_VALUE="$2"
            shift 2
            ;;
        --gpu)
            GPU_MODE=true
            shift
            ;;
        *)
            if [ -z "$INPUT_FILE" ]; then
                INPUT_FILE="$1"
            elif [ -z "$OUTPUT_FILE" ]; then
                OUTPUT_FILE="$1"
            else
                echo "Error: Unexpected argument '$1'"
                exit 1
            fi
            shift
            ;;
    esac
done

if [ -z "$INPUT_FILE" ] || [ -z "$OUTPUT_FILE" ]; then
    echo "Usage: $0 <input_file> <output_file> [--track <track_file>] [--layout <layout_file>] [--offset <offset_value>] [--gpu]"
    exit 1
fi

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
GST_PLUGIN_PATH="$SCRIPT_DIR/../builddir"
export GST_PLUGIN_PATH

PROPERTIES="offset=$OFFSET_VALUE"
if [ -n "$TRACK_FILE" ]; then
    PROPERTIES="$PROPERTIES track=$TRACK_FILE"
fi
if [ -n "$LAYOUT_FILE" ]; then
    PROPERTIES="$PROPERTIES layout=$LAYOUT_FILE"
fi

UUT="telemetry $PROPERTIES"

export TMPDIR=".tmp"
mkdir -p $TMPDIR

if [ "$GPU_MODE" = true ]; then
    export GST_GL_WINDOW=surfaceless
    gst-launch-1.0 filesrc location=$INPUT_FILE ! decodebin name=dec \
    dec. ! queue ! video/x-raw ! videoconvert ! glupload ! \
           glvideoflip video-direction=auto ! taginject tags="image-orientation=rotate-0" ! gltransformation ! 'video/x-raw(memory:GLMemory),width=3840,height=2160' ! \
           $UUT ! 'video/x-raw(memory:GLMemory, meta:GstVideoOverlayComposition)' ! gloverlaycompositor ! nvh264enc bitrate=120000 ! h264parse ! queue ! mux. \
    dec. ! queue ! audio/x-raw ! audioconvert ! audioresample ! avenc_aac bitrate=128000 ! queue ! mux. \
    mp4mux name=mux faststart=true ! filesink location=$OUTPUT_FILE
else
    gst-launch-1.0 filesrc location=$INPUT_FILE ! decodebin name=dec \
    dec. ! queue ! videoconvert ! videoflip video-direction=auto ! $UUT ! \
           x264enc bitrate=120000 speed-preset=ultrafast tune=zerolatency ! queue ! mux. \
    dec. ! queue ! audio/x-raw ! audioconvert ! audioresample ! avenc_aac bitrate=128000 ! queue ! mux. \
    mp4mux name=mux faststart=true ! filesink location=$OUTPUT_FILE
fi
