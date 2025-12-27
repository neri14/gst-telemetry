#!/bin/bash

INPUT_FILE=""
OUTPUT_FILE=""
TRACK_FILE=""
LAYOUT_FILE=""

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
    echo "Usage: $0 <input_file> <output_file> [--track <track_file>] [--layout <layout_file>]"
    exit 1
fi

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
GST_PLUGIN_PATH="$SCRIPT_DIR/../builddir"
export GST_PLUGIN_PATH

PROPERTIES="offset=1.0"
if [ -n "$TRACK_FILE" ]; then
    PROPERTIES="$PROPERTIES track=$TRACK_FILE"
fi
if [ -n "$LAYOUT_FILE" ]; then
    PROPERTIES="$PROPERTIES layout=$LAYOUT_FILE"
fi

UUT="telemetry $PROPERTIES"

gst-launch-1.0 -v filesrc location=$INPUT_FILE ! decodebin name=dec \
dec. ! queue ! video/x-raw ! videoconvert ! videoflip method=automatic ! $UUT ! x264enc bitrate=120000 speed-preset=ultrafast tune=zerolatency ! queue ! mux. \
dec. ! queue ! audio/x-raw ! audioconvert ! audioresample ! avenc_aac bitrate=128000 ! queue ! mux. \
mp4mux name=mux faststart=true ! filesink location=$OUTPUT_FILE
