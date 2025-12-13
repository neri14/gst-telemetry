#!/bin/bash

# Read args: input output here
if [ $# -ne 2 ]; then
    echo "Usage: $0 <input_file> <output_file>"
    exit 1
fi

INPUT_FILE="$1"
OUTPUT_FILE="$2"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
GST_PLUGIN_PATH="$SCRIPT_DIR/../builddir"
export GST_PLUGIN_PATH

PROPERTIES="offset=1.0"
UUT="telemetry $PROPERTIES"

gst-launch-1.0 -v filesrc location=$INPUT_FILE ! decodebin name=dec \
dec. ! queue ! video/x-raw ! videoconvert ! $UUT ! x264enc bitrate=120000 speed-preset=ultrafast tune=zerolatency ! queue ! mux. \
dec. ! queue ! audio/x-raw ! audioconvert ! audioresample ! avenc_aac bitrate=128000 ! queue ! mux. \
mp4mux name=mux faststart=true ! filesink location=$OUTPUT_FILE
