#!/bin/bash

# Script to generate an overlay video with transparent background.

OUTPUT_FILE=""
OUTPUT_LENGTH=""
OUTPUT_FPS="30"
OUTPUT_WIDTH="3840"
OUTPUT_HEIGHT="2160"
TRACK_FILE=""
LAYOUT_FILE=""
CUSTOM_DATA_FILE=""
OFFSET_VALUE="0"
DEV_MODE=false

while [[ $# -gt 0 ]]; do
    case $1 in
        --output)
            OUTPUT_FILE="$2"
            shift 2
            ;;
        --length)
            OUTPUT_LENGTH="$2"
            shift 2
            ;;
        --fps)
            OUTPUT_FPS="$2"
            shift 2
            ;;
        --width)
            OUTPUT_WIDTH="$2"
            shift 2
            ;;
        --height)
            OUTPUT_HEIGHT="$2"
            shift 2
            ;;
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
        --dev)
            DEV_MODE=true
            shift
            ;;
        *)
            echo "Error: Unexpected argument '$1'"
            exit 1
            shift
            ;;
    esac
done

if [ -z "$OUTPUT_FILE" ] || [ -z "$TRACK_FILE" ] || [ -z "$LAYOUT_FILE" ]; then
    echo "Usage: $0 --track <track_file> --layout <layout_file> [--custom-data <file>] [--offset <offset_value>] [--length <output_length_in_seconds>] [--width <output_width>] [--height <output_height>] [--fps <output_fps>] [--dev] --output <output_file>"
    exit 1
fi

if [[ "$OUTPUT_FILE" != *.mov ]]; then
    echo "Error: Output file must have .mov extension."
    exit 1
fi

if [ -z "$OUTPUT_LENGTH" ]; then
    OUTPUT_LENGTH=$(TZ=UTC awk -F'[<>]' '/<time>/{ts=$3; gsub(/[-:TZ]/," ",ts); t=mktime(ts); if(!start)start=t; end=t} END{print end-start}' $TRACK_FILE)
    echo "Calculated output length: $OUTPUT_LENGTH seconds"
fi

TOTAL_FRAMES=$(echo -e "$OUTPUT_LENGTH\t$OUTPUT_FPS" |  awk '{print $1 * $2}')
if [ "$TOTAL_FRAMES" -le 0 ]; then
    echo "Error: Output length must be greater than 0."
    exit 1
fi

if $DEV_MODE; then
    SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    GST_PLUGIN_PATH="$SCRIPT_DIR/../builddir"
    export GST_PLUGIN_PATH
fi

PROPERTIES="offset=$OFFSET_VALUE track=$TRACK_FILE layout=$LAYOUT_FILE"
if [ -n "$CUSTOM_DATA_FILE" ]; then
    PROPERTIES="$PROPERTIES custom-data=$CUSTOM_DATA_FILE"
fi

export TMPDIR=".tmp"
export GST_GL_WINDOW="surfaceless"

gst-launch-1.0 -e videotestsrc pattern=black num-buffers=$TOTAL_FRAMES \
    ! video/x-raw,format=RGBA,width=$OUTPUT_WIDTH,height=$OUTPUT_HEIGHT,framerate=$OUTPUT_FPS/1 \
    ! alpha alpha=0.0 ! videoconvert ! glupload ! "video/x-raw(memory:GLMemory),width=$OUTPUT_WIDTH,height=$OUTPUT_HEIGHT,format=RGBA" \
    ! telemetry $PROPERTIES ! "video/x-raw(memory:GLMemory,meta:GstVideoOverlayComposition)" ! gloverlaycompositor ! gldownload \
    ! videoconvert ! pngenc ! qtmux ! filesink location=$OUTPUT_FILE

rm -rf $TMPDIR
