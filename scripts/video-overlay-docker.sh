#!/bin/bash
#
# video-overlay-docker.sh
#
# Builds the gst-telemetry Docker image (neri14/gst-telemetry) and runs the
# GPU video-overlay pipeline inside the container.  GPU mode is always on.
#
# Usage:
#   ./video-overlay-docker.sh <input> <output.mp4> [--track FILE] [--layout FILE]
#                             [--custom-data FILE] [--offset N]
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
INPUT_FILE=""
OUTPUT_FILE=""
TRACK_FILE=""
LAYOUT_FILE=""
CUSTOM_DATA_FILE=""
OFFSET_VALUE="0"

while [[ $# -gt 0 ]]; do
    case $1 in
        --track)
            TRACK_FILE="$2"; shift 2 ;;
        --layout)
            LAYOUT_FILE="$2"; shift 2 ;;
        --custom-data)
            CUSTOM_DATA_FILE="$2"; shift 2 ;;
        --offset)
            OFFSET_VALUE="$2"; shift 2 ;;
        *)
            if [ -z "$INPUT_FILE" ]; then
                INPUT_FILE="$1"
            elif [ -z "$OUTPUT_FILE" ]; then
                OUTPUT_FILE="$1"
            else
                echo "Error: Unexpected argument '$1'" >&2
                exit 1
            fi
            shift ;;
    esac
done

if [ -z "$INPUT_FILE" ] || [ -z "$OUTPUT_FILE" ]; then
    echo "Usage: $0 <input_file> <output_file.mp4> [--track FILE] [--layout FILE] [--custom-data FILE] [--offset N]" >&2
    exit 1
fi

if [[ "$OUTPUT_FILE" != *.mp4 ]]; then
    echo "Error: Output file must have .mp4 extension." >&2
    exit 1
fi

# ── resolve all paths to absolutes ───────────────────────────────────────────
INPUT_FILE="$(realpath "$INPUT_FILE")"
mkdir -p "$(dirname "$OUTPUT_FILE")"
OUTPUT_FILE="$(realpath "$(dirname "$OUTPUT_FILE")")/$(basename "$OUTPUT_FILE")"

# Temporary directory for GStreamer — placed next to the output file so it
# lives on the same real drive (avoids tmpfs / RAM pressure for large videos).
TMPDIR_HOST="$(dirname "$OUTPUT_FILE")/.gst-tmp"
mkdir -p "$TMPDIR_HOST"

declare -A MOUNT_DIRS
MOUNT_DIRS["$(dirname "$INPUT_FILE")"]=1
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
PIPELINE="gst-launch-1.0 filesrc location=$INPUT_FILE ! decodebin name=dec \
dec. ! queue ! video/x-raw ! videoconvert ! glupload ! \
glvideoflip video-direction=auto ! taginject tags=\"image-orientation=rotate-0\" ! gltransformation ! 'video/x-raw(memory:GLMemory),width=3840,height=2160' ! \
telemetry $PROPERTIES ! 'video/x-raw(memory:GLMemory,meta:GstVideoOverlayComposition)' ! gloverlaycompositor ! nvh264enc bitrate=120000 ! h264parse ! queue ! mux. \
dec. ! queue ! audio/x-raw ! audioconvert ! audioresample ! avenc_aac bitrate=128000 ! queue ! mux. \
mp4mux name=mux faststart=true ! filesink location=$OUTPUT_FILE"

# ── run ──────────────────────────────────────────────────────────────────────
# Clear stale GStreamer plugin registry so nvenc elements are freshly discovered
rm -rf "$TMPDIR_HOST/.cache"
echo "==> Running pipeline in container..."
echo "    Input:       $INPUT_FILE"
echo "    Output:      $OUTPUT_FILE"
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
