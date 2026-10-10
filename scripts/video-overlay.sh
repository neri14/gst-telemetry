#!/bin/bash

INPUT_FILE=""
OUTPUT_FILE=""
TRACK_FILE=""
LAYOUT_FILE=""
CUSTOM_DATA_FILE=""
OFFSET_VALUE="0"
GPU_MODE=false
DEV_MODE=false

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
        --custom-data)
            CUSTOM_DATA_FILE="$2"
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
        --dev)
            DEV_MODE=true
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
    echo "Usage: $0 <input_file> <output_file.mp4> [--track <track_file>] [--layout <layout_file>] [--custom-data <file>] [--offset <offset_value>] [--gpu] [--dev]"
    exit 1
fi

if [[ "$OUTPUT_FILE" != *.mp4 ]]; then
    echo "Error: Output file must have .mp4 extension." >&2
    exit 1
fi

if $DEV_MODE; then
    SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    GST_PLUGIN_PATH="$SCRIPT_DIR/../builddir"
    export GST_PLUGIN_PATH
fi

PROPERTIES="offset=$OFFSET_VALUE"
if [ -n "$TRACK_FILE" ]; then
    PROPERTIES="$PROPERTIES track=$TRACK_FILE"
fi
if [ -n "$LAYOUT_FILE" ]; then
    PROPERTIES="$PROPERTIES layout=$LAYOUT_FILE"
fi
if [ -n "$CUSTOM_DATA_FILE" ]; then
    PROPERTIES="$PROPERTIES custom-data=$CUSTOM_DATA_FILE"
fi

UUT="telemetry $PROPERTIES"

export TMPDIR=".tmp"
mkdir -p $TMPDIR

if [ "$GPU_MODE" = true ]; then
    export GST_GL_WINDOW=surfaceless
    # NVDEC outputs GL memory only into a desktop GL context (not GLES)
    export GST_GL_API=opengl3

    # AAC is passed through, other audio is re-encoded, no audio stream means no audio branch
    AUDIO_CODEC="reencode"
    if command -v ffprobe >/dev/null; then
        AUDIO_CODEC=$(ffprobe -v error -select_streams a:0 -show_entries stream=codec_name -of csv=p=0 "$INPUT_FILE")
    fi
    case "$AUDIO_CODEC" in
        aac)
            DECODE_CAPS="video/x-raw(ANY);audio/mpeg,mpegversion=4"
            AUDIO_BRANCH="dec. ! queue ! aacparse ! queue ! mux."
            ;;
        "")
            DECODE_CAPS="video/x-raw(ANY)"
            AUDIO_BRANCH=""
            ;;
        *)
            DECODE_CAPS="video/x-raw(ANY);audio/x-raw(ANY)"
            AUDIO_BRANCH="dec. ! queue ! audio/x-raw ! audioconvert ! audioresample ! avenc_aac bitrate=128000 ! queue ! mux."
            ;;
    esac

    # decodebin3, not decodebin: decodebin deadlocks when the decoder negotiates GL memory.
    # NVDEC GL memory passes through videoconvert; software-decoded frames are converted to NV12
    # (glupload accepts e.g. I422_10LE from ProRes but renders it wrong).
    gst-launch-1.0 filesrc location=$INPUT_FILE ! decodebin3 name=dec caps="\"$DECODE_CAPS\"" \
    dec. ! queue ! videoconvert ! 'video/x-raw(memory:GLMemory);video/x-raw,format=NV12' ! glupload ! glcolorconvert ! 'video/x-raw(memory:GLMemory),format=RGBA' ! \
           glvideoflip video-direction=auto ! taginject tags="image-orientation=rotate-0" ! gltransformation ! 'video/x-raw(memory:GLMemory),width=3840,height=2160' ! \
           $UUT ! 'video/x-raw(memory:GLMemory, meta:GstVideoOverlayComposition)' ! gloverlaycompositor ! nvh264enc bitrate=120000 ! h264parse ! queue ! mux. \
    $AUDIO_BRANCH \
    mp4mux name=mux faststart=true ! filesink location=$OUTPUT_FILE
else
    gst-launch-1.0 filesrc location=$INPUT_FILE ! decodebin name=dec \
    dec. ! queue ! videoconvert ! videoflip video-direction=auto ! $UUT ! \
           x264enc bitrate=120000 speed-preset=ultrafast tune=zerolatency ! queue ! mux. \
    dec. ! queue ! audio/x-raw ! audioconvert ! audioresample ! avenc_aac bitrate=128000 ! queue ! mux. \
    mp4mux name=mux faststart=true ! filesink location=$OUTPUT_FILE
fi
