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
TEST_MODE=false
TEST_BG_COLOR="0x808080ff"

normalize_test_bg_color() {
    local input="$1"
    case "${input,,}" in
        black) echo "0x000000ff" ;;
        gray|grey) echo "0x808080ff" ;;
        white) echo "0xffffffff" ;;
        red) echo "0xff0000ff" ;;
        green) echo "0x00ff00ff" ;;
        lime) echo "0x00ff00ff" ;;
        blue) echo "0x0000ffff" ;;
        pink) echo "0xffc0cbff" ;;
        transparent) echo "0x00000000" ;;
        0x[0-9a-fA-F][0-9a-fA-F][0-9a-fA-F][0-9a-fA-F][0-9a-fA-F][0-9a-fA-F][0-9a-fA-F][0-9a-fA-F]) echo "$input" ;;
        *)
            echo "Error: Unsupported test background color '$input'. Use black/gray/pink/lime/white/red/green/blue or 0xRRGGBBAA." >&2
            exit 1
            ;;
    esac
}

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
        --test|test)
            TEST_MODE=true
            if [[ $# -gt 1 && "$2" != --* ]]; then
                TEST_BG_COLOR="$(normalize_test_bg_color "$2")"
                shift 2
            else
                shift
            fi
            ;;
        --test-bg-color)
            TEST_BG_COLOR="$(normalize_test_bg_color "$2")"
            shift 2
            ;;
        *)
            echo "Error: Unexpected argument '$1'"
            exit 1
            shift
            ;;
    esac
done

if [ -z "$OUTPUT_FILE" ] || [ -z "$TRACK_FILE" ] || [ -z "$LAYOUT_FILE" ]; then
    echo "Usage: $0 --track <track_file> --layout <layout_file> [--custom-data <file>] [--offset <offset_value>] [--length <output_length_in_seconds>] [--width <output_width>] [--height <output_height>] [--fps <output_fps>] [--dev] [test|--test [color]] [--test-bg-color <color>] --output <output_file.mov|output_file.mp4>"
    exit 1
fi

if $TEST_MODE; then
    if [[ "$OUTPUT_FILE" != *.mp4 ]]; then
        echo "Error: Test mode output file must have .mp4 extension."
        exit 1
    fi
else
    if [[ "$OUTPUT_FILE" != *.mov ]]; then
        echo "Error: Output file must have .mov extension."
        exit 1
    fi
fi

if [ -z "$OUTPUT_LENGTH" ]; then
    OUTPUT_LENGTH=$(TZ=UTC awk -F'[<>]' '/<time>/{ts=$3; gsub(/[-:TZ]/," ",ts); t=mktime(ts); if(!start)start=t; end=t} END{print end-start}' "$TRACK_FILE")
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

if $TEST_MODE; then
    gst-launch-1.0 -e videotestsrc pattern=solid-color foreground-color=$TEST_BG_COLOR num-buffers=$TOTAL_FRAMES \
        ! video/x-raw,format=RGBA,width=$OUTPUT_WIDTH,height=$OUTPUT_HEIGHT,framerate=$OUTPUT_FPS/1 \
        ! videoconvert ! glupload ! "video/x-raw(memory:GLMemory),width=$OUTPUT_WIDTH,height=$OUTPUT_HEIGHT,format=RGBA" \
        ! telemetry $PROPERTIES ! "video/x-raw(memory:GLMemory,meta:GstVideoOverlayComposition)" ! gloverlaycompositor \
        ! glcolorscale ! "video/x-raw(memory:GLMemory),width=1920,height=1080" \
        ! glcolorconvert ! "video/x-raw(memory:GLMemory),format=NV12" \
        ! nvh264enc bitrate=60000 ! h264parse ! mp4mux faststart=true ! filesink location=$OUTPUT_FILE
else
    gst-launch-1.0 -e videotestsrc pattern=black num-buffers=$TOTAL_FRAMES \
        ! video/x-raw,format=RGBA,width=$OUTPUT_WIDTH,height=$OUTPUT_HEIGHT,framerate=$OUTPUT_FPS/1 \
        ! alpha alpha=0.0 ! videoconvert ! glupload ! "video/x-raw(memory:GLMemory),width=$OUTPUT_WIDTH,height=$OUTPUT_HEIGHT,format=RGBA" \
        ! telemetry $PROPERTIES ! "video/x-raw(memory:GLMemory,meta:GstVideoOverlayComposition)" ! gloverlaycompositor ! gldownload \
        ! videoconvert ! video/x-raw,format=A444_10LE ! avenc_prores_ks profile=4 threads=0 ! qtmux ! filesink location=$OUTPUT_FILE
fi

rm -rf "$TMPDIR"
