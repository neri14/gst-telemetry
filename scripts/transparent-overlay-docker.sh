#!/bin/bash
#
# transparent-overlay-docker.sh
#
# Builds the gst-telemetry Docker image (neri14/gst-telemetry) and runs the
# transparent-overlay pipeline inside the container.  GPU mode is always on.
#
# Usage:
#   ./transparent-overlay-docker.sh --track FILE --layout FILE --output FILE
#         [--custom-data FILE] [--offset N] [--length N] [--fps N]
#         [--width N] [--height N]
#
# Limitations when running inside a container (vs. natively):
#   1. The script auto-resolves all file arguments to absolute paths and
#      bind-mounts their parent directories — you can pass relative paths
#      on the command line without worry.
#   2. Files referenced *inside* a track or layout XML by relative path
#      (e.g. <image src="icon.png"/>) will NOT resolve inside the container
#      unless that directory is also mounted.  Workaround: keep all assets
#      under one tree that is already mounted.
#   3. Requires the nvidia-container-toolkit on the host.
#   4. Output files are owned by your host user (container runs with --user).

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
DOCKERFILE_DIR="$SCRIPT_DIR/.."
IMAGE_NAME="neri14/gst-telemetry"

# ── argument parsing ─────────────────────────────────────────────────────────
OUTPUT_FILE=""
OUTPUT_LENGTH=""
OUTPUT_FPS="30"
OUTPUT_WIDTH="3840"
OUTPUT_HEIGHT="2160"
TRACK_FILE=""
LAYOUT_FILE=""
CUSTOM_DATA_FILE=""
OFFSET_VALUE="0"

while [[ $# -gt 0 ]]; do
    case $1 in
        --output)
            OUTPUT_FILE="$2"; shift 2 ;;
        --length)
            OUTPUT_LENGTH="$2"; shift 2 ;;
        --fps)
            OUTPUT_FPS="$2"; shift 2 ;;
        --width)
            OUTPUT_WIDTH="$2"; shift 2 ;;
        --height)
            OUTPUT_HEIGHT="$2"; shift 2 ;;
        --track)
            TRACK_FILE="$2"; shift 2 ;;
        --layout)
            LAYOUT_FILE="$2"; shift 2 ;;
        --custom-data)
            CUSTOM_DATA_FILE="$2"; shift 2 ;;
        --offset)
            OFFSET_VALUE="$2"; shift 2 ;;
        *)
            echo "Error: Unexpected argument '$1'" >&2
            exit 1 ;;
    esac
done

if [ -z "$OUTPUT_FILE" ] || [ -z "$TRACK_FILE" ] || [ -z "$LAYOUT_FILE" ]; then
    echo "Usage: $0 --track FILE --layout FILE --output FILE [--custom-data FILE] [--offset N] [--length N] [--fps N] [--width N] [--height N]" >&2
    exit 1
fi

if [[ "$OUTPUT_FILE" != *.mov ]]; then
    echo "Error: Output file must have .mov extension." >&2
    exit 1
fi

# ── resolve all paths to absolutes ───────────────────────────────────────────
mkdir -p "$(dirname "$OUTPUT_FILE")"
OUTPUT_FILE="$(realpath "$(dirname "$OUTPUT_FILE")")/$(basename "$OUTPUT_FILE")"

# Temporary directory for GStreamer — placed next to the output file so it
# lives on the same real drive (avoids tmpfs / RAM pressure for large videos).
TMPDIR_HOST="$(dirname "$OUTPUT_FILE")/.gst-tmp"
mkdir -p "$TMPDIR_HOST"

declare -A MOUNT_DIRS
MOUNT_DIRS["$(dirname "$OUTPUT_FILE")"]=1

resolve_optional() {
    local var_name="$1"
    local val="${!var_name}"
    if [ -n "$val" ]; then
        val="$(realpath "$val")"
        printf -v "$var_name" '%s' "$val"
        MOUNT_DIRS["$(dirname "$val")"]=1
    fi
}
resolve_optional TRACK_FILE
resolve_optional LAYOUT_FILE
resolve_optional CUSTOM_DATA_FILE

# ── calculate output length if not provided ──────────────────────────────────
if [ -z "$OUTPUT_LENGTH" ]; then
    OUTPUT_LENGTH=$(TZ=UTC awk -F'[<>]' '/<time>/{ts=$3; gsub(/[-:TZ]/," ",ts); t=mktime(ts); if(!start)start=t; end=t} END{print end-start}' "$TRACK_FILE")
    echo "Calculated output length: $OUTPUT_LENGTH seconds"
fi

TOTAL_FRAMES=$(echo -e "$OUTPUT_LENGTH\t$OUTPUT_FPS" | awk '{print $1 * $2}')
if [ "$TOTAL_FRAMES" -le 0 ]; then
    echo "Error: Output length must be greater than 0." >&2
    exit 1
fi

# ── build Docker image ───────────────────────────────────────────────────────
echo "==> Building Docker image: $IMAGE_NAME"
docker build -t "$IMAGE_NAME" "$DOCKERFILE_DIR"

# ── volume arguments (bind-mount each involved directory) ────────────────────
VOLUME_ARGS=""
for d in "${!MOUNT_DIRS[@]}"; do
    VOLUME_ARGS="$VOLUME_ARGS -v $d:$d"
done

# ── UUT property string ──────────────────────────────────────────────────────
PROPERTIES="offset=$OFFSET_VALUE"
[ -n "$TRACK_FILE" ]       && PROPERTIES="$PROPERTIES track=$TRACK_FILE"
[ -n "$LAYOUT_FILE" ]      && PROPERTIES="$PROPERTIES layout=$LAYOUT_FILE"
[ -n "$CUSTOM_DATA_FILE" ] && PROPERTIES="$PROPERTIES custom-data=$CUSTOM_DATA_FILE"

# ── GPU pipeline (always on) ─────────────────────────────────────────────────
PIPELINE="gst-launch-1.0 -e videotestsrc pattern=black num-buffers=$TOTAL_FRAMES \
! video/x-raw,format=RGBA,width=$OUTPUT_WIDTH,height=$OUTPUT_HEIGHT,framerate=$OUTPUT_FPS/1 \
! alpha alpha=0.0 ! videoconvert ! glupload ! \"video/x-raw(memory:GLMemory),width=$OUTPUT_WIDTH,height=$OUTPUT_HEIGHT,format=RGBA\" \
! telemetry $PROPERTIES ! \"video/x-raw(memory:GLMemory,meta:GstVideoOverlayComposition)\" ! gloverlaycompositor ! gldownload \
! videoconvert ! pngenc ! qtmux ! filesink location=$OUTPUT_FILE"

# ── run ──────────────────────────────────────────────────────────────────────
echo "==> Running pipeline in container..."
echo "    Output:      $OUTPUT_FILE"
echo "    Track:       $TRACK_FILE"
echo "    Layout:      $LAYOUT_FILE"
echo "    Length:      $OUTPUT_LENGTH s"
echo "    FPS:         $OUTPUT_FPS"
echo "    Resolution:  ${OUTPUT_WIDTH}x${OUTPUT_HEIGHT}"
echo "    Tmp dir:     $TMPDIR_HOST"
echo "    Mounts:      ${!MOUNT_DIRS[*]}"

# shellcheck disable=SC2086
docker run --rm \
    --gpus all \
    --user "$(id -u):$(id -g)" \
    -t \
    -e GST_GL_WINDOW=surfaceless \
    -e "TMPDIR=$TMPDIR_HOST" \
    -e "HOME=$TMPDIR_HOST" \
    $VOLUME_ARGS \
    --entrypoint /bin/bash \
    "$IMAGE_NAME" \
    -c "$PIPELINE"
