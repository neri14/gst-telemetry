#!/bin/bash

# Read args: input output here
if [ $# -ne 2 ]; then
    echo "Usage: $0 <input_file> <output_file>"
    exit 1
fi

INPUT_FILE="$1"
OUTPUT_FILE="$2"

#tested plugin would go between videoconvert and x264enc
gst-launch-1.0 -v filesrc location=$INPUT_FILE ! decodebin name=dec \
dec. ! queue ! video/x-raw ! videoconvert ! x264enc bitrate=120000 speed-preset=ultrafast tune=zerolatency ! queue ! mux. \
dec. ! queue ! audio/x-raw ! audioconvert ! audioresample ! avenc_aac bitrate=128000 ! queue ! mux. \
mp4mux name=mux faststart=true ! filesink location=$OUTPUT_FILE
