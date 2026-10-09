# GPU / performance review of overlay generation

Date: 2026-10-09 · Branch reviewed: `dev/chart_extreme_markers` (`b221382`)
Host: RTX 4080 (Ada, 16 GB, driver 615.71.09), Ryzen 7 5800X (8C/16T), Arch Linux,
GStreamer 1.28.7, FFmpeg 9.0.2 (host), FFmpeg 8.0.1 (Docker image).

## TL;DR

| | today | proposed | how |
|---|---|---|---|
| **Transparent overlay** 4K30 ProRes 4444 | **11.1 fps**, 535 CPU-s / 900 frames | **38.5 fps** (3.5×), 107 CPU-s (5× less) | FFmpeg `prores_ks_vulkan` (GPU ProRes encoder) fed by the existing GL pipeline. No plugin code changes. |
| **Transparent overlay**, fully NVENC | n/a | ~57 fps incl. startup, ~0.02 CPU-s/frame | Two NVENC files: colour fill + alpha matte. NLE does the track-matte. |
| **Burn-in** (`video-overlay.sh --gpu`) 4K30 | **42 fps** | **79 fps** (1.9×) | NVDEC → GLMemory directly (no system-memory round trip). Needs `GST_GL_API=opengl3`. |
| **Correctness** | Semi-transparent / anti-aliased pixels are darkened (colour × α²) | Correct | One flag in `src/gsttelemetry.c:509`, plus an unpremultiply step for ProRes. |

Answers to the questions:

* **Can performance be improved?** Yes, by a lot. On the transparent path ~78% of
  wall time is CPU `videoconvert` (single-threaded) + CPU ProRes 4444. The
  telemetry rendering itself is a small fraction for typical layouts.
* **Can GPU encoding with alpha work?** Yes. Your build of FFmpeg (≥ 8.1) ships
  **`prores_ks_vulkan`**, a Vulkan-compute ProRes encoder that supports 4444 + alpha.
  I verified it on this host: output decodes as `yuva444p12le`, alpha matches the
  CPU encoder exactly, and colour is visually lossless. NVENC itself can't do what
  you were trying: `nvh26xenc`/`*_nvenc` drop alpha. NVENC's HEVC alpha layer
  exists only in the raw SDK, only 8-bit 4:2:0, and no FFmpeg or GStreamer element
  exposes it (§4).
* **Can "everything" move to the GPU?** Decode, scale/flip, composition and
  encode can (verified). Telemetry rasterisation (Cairo + Pango) could also, but a
  full rewrite is not worth it now: it's the cheapest stage in the measurements,
  and text shaping/rendering is the hardest part to port. Composition should
  **stay** as it is: one CPU-composited surface, one rectangle. Moving it to the
  GPU with per-widget overlay rectangles was tried on `origin/dev/perf` and made
  the run ~23% slower (§5.1). The single 33 MB upload per frame is not a
  bottleneck.

---

## 1. How generation works today

### 1.1 Plugin (per frame, `gst_telemetry_transform_frame_ip`)

```
Layout::draw (main thread)            evaluate params (exprtk), schedule a task per visible widget
   └─ worker pool (¾ of cores)        each widget redraws ITS OWN small cairo cache only if a param changed
Manager::draw (main thread)           cairo CLEAR of a full 3840×2160 ARGB32 surface (33 MB)      manager.cpp:199
                                      cairo_paint every widget cache onto it (pixman, CPU)        manager.cpp:210
gsttelemetry.c                        wrap the 33 MB buffer in ONE GstVideoOverlayRectangle       :509
   GL caps  → attach as GstVideoOverlayCompositionMeta (gloverlaycompositor blends it later)      :519
   CPU caps → gst_video_overlay_composition_blend() onto the frame (CPU)                         :524
```

Widget-level caching is good (only dirty widgets re-rasterise). The full-frame
work per frame is a 33 MB clear plus composite on the CPU, one new rectangle (so
`gloverlaycompositor` uploads one 33 MB texture), and on the CPU path a
full-frame blend. In the measurements these are not the bottleneck. 33 MB × 30
fps ≈ 1 GB/s, a small fraction of PCIe 4.0 x16, and it is a single large upload,
which is the cheap case for the driver (§5.1).

### 1.2 `transparent-overlay.sh` (non-test mode)

```
videotestsrc black (CPU fill 33MB) → alpha alpha=0 (CPU pass) → videoconvert → glupload (33MB ↑)
 → telemetry (CPU render + 33MB ↑ texture) → gloverlaycompositor (GPU) → gldownload (33MB ↓)
 → videoconvert RGBA→A444_10LE (CPU, single thread) → avenc_prores_ks 4444 (CPU, all cores) → qtmux
```

The background frame is fully transparent, so the overlay buffer the plugin
draws *is* the output frame. Everything before `telemetry` and the
upload/composite/download round trip is redundant work.

### 1.3 `video-overlay.sh --gpu`

```
decodebin (picks nvh26xdec) → video/x-raw (forces SYSTEM memory: GPU→CPU copy) → videoconvert → glupload (CPU→GPU)
 → glvideoflip → gltransformation → telemetry → gloverlaycompositor → nvh264enc
audio: decode → avenc_aac re-encode
```

---

## 2. Measurements

Method: the repo's `builddir` plugin (built 2026-09-28) with `GST_PLUGIN_PATH`.
The layout is synthetic, 4K: 3 dynamic texts, 1 rectangle, 1 moving circle, 1
chart (`video_time`/`point_power`). The track is a synthetic 120 s GPX. The
example layouts in the repo depend on custom data and segments, so I couldn't
use them. All benchmarks are short, 150–900 frames. Wall time includes 0.5–1 s of
startup, so the short runs understate fps.

### 2.1 Transparent path, stage breakdown (150 frames, 4K)

| pipeline | wall | eff. fps |
|---|---|---|
| render only (`… ! telemetry ! fakesink`) | 2.00 s | 75 |
| + `gloverlaycompositor ! gldownload` | 3.06 s | 49 |
| **current full pipeline** (+ videoconvert + CPU ProRes) | **13.66 s** | **11** |
| quick fix: `videoconvert n-threads=0` | 9.61 s | 16 |
| GL pipeline → `fdsink` → `ffmpeg … prores_ks_vulkan` (no queues) | 7.19 s | 21 |
| same + `queue`s between GL stages | 4.89 s | 31 |
| CPU-only plugin path (no GL) → `ffmpeg … prores_ks_vulkan` | 4.56 s | 33 |
| **fill + matte on NVENC** (§4.2) | **2.63 s** | 57 |

Steady state, 900 frames:

| pipeline | wall | fps | CPU (user) |
|---|---|---|---|
| current `transparent-overlay.sh` | 80.8 s | 11.1 | 535 s |
| proposed (§7.1) | 23.4 s | **38.5** | 107 s |

### 2.2 Encoder alone (300 frames 4K, `yuva444p10le` input, synthetic source)

| encoder | wall | fps | CPU (user) |
|---|---|---|---|
| `prores_ks` 4444 (CPU, 16 threads) | 17.2 s | 17 | 266 s |
| `prores_ks_vulkan` 4444, `async_depth` 1 / 4 / 8 | 7.9 / 7.1 / 7.3 s | ~42 | 30 s (≈26 s of that is the lavfi source + swscale) |

`prores_ks_vulkan` is the current ceiling of the transparent path, at ~42 fps for
4K. The proposed pipeline (38.5 fps) is close to it.

Quality check, 120 frames vs source: alpha PSNR is identical for CPU and Vulkan
(the alpha is lossless in both). Colour Y PSNR is 82.1 dB (CPU) vs 78.7 dB
(Vulkan) on a 12-bit scale, i.e. visually lossless in both. Decoded pixel
samples match: bg α = 0 and fg α = 129 for the 50% overlay.
A Cinelerra mailing-list thread [3] reported block artefacts in an early
version of the Vulkan ProRes patch. I saw none here, but check a real render
before switching over.

### 2.3 Burn-in path (300 frames, 4K30 HEVC input, audio passthrough in both)

| pipeline | wall | fps |
|---|---|---|
| current (`video/x-raw ! videoconvert ! glupload`) | 7.11 s | 42 |
| `nvh265dec ! video/x-raw(memory:GLMemory) ! glcolorconvert` (GL memory end to end) | 3.78 s | **79** |
| `decodebin3 ! video/x-raw(memory:GLMemory)` (codec-agnostic; no flip/audio) | 4.48 s | 67 |

---

## 3. Findings

### F1. Transparent path: CPU ProRes and single-threaded `videoconvert` dominate (high impact)
`scripts/transparent-overlay.sh:156`, `scripts/transparent-overlay-docker.sh:186`.
78% of wall time. `videoconvert` defaults to one thread at 4K. `avenc_prores_ks`
saturates every core, so it also steals CPU from the telemetry worker pool
(`manager.cpp:113` spawns 12 workers on this host).
Fix: §7.1. Minimal fix: `videoconvert n-threads=0` (1.4×).

### F2. Premultiplied alpha declared as straight → colours darkened by α² (correctness bug)
`src/gsttelemetry.c:509` creates the rectangle with
`GST_VIDEO_OVERLAY_FORMAT_FLAG_NONE`, but Cairo `ARGB32` is *premultiplied*.
GStreamer's GL compositor (`gstgloverlaycompositor.c`) picks its blend mode from
that flag [4]:
* flag NONE → `glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA)`
  multiplies the already-premultiplied colour by α again.
* flag PREMULTIPLIED → `glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA)` (correct for Cairo data).

Measured on the current output: chart `background-below="rgba(1,1,1,0.3)"`
(white at 30%) is written as **RGB 23 / α 76** (76 × 76/255 = 23, dark grey).
Straight alpha would be 255 / 76 and premultiplied 76 / 76, so it matches
neither convention.
This also affects **burn-in videos**: anti-aliased text edges and translucent
fills over video come out darker than designed (dark fringes). The CPU blend path
(`gst_video_overlay_composition_blend`) handles the flag the same way.

Fix:
1. Pass `GST_VIDEO_OVERLAY_FORMAT_FLAG_PREMULTIPLIED_ALPHA` at `gsttelemetry.c:509`.
   This fixes burn-in. Effect on the GL blend follows from the quoted source; I
   didn't build and test the change.
2. For transparent output the composited frame is then premultiplied. ProRes 4444
   alpha is usually interpreted as straight. Apple's ProRes white paper only
   guarantees a lossless alpha up to 16 bits [5]; straight vs premultiplied is a
   per-clip setting in NLEs. So either unpremultiply on the GPU before
   `gldownload` (shader in §7.3), or tag the clip "premultiplied" in the NLE.
   FFmpeg's `unpremultiply` filter had no effect in my test on 9.0.2 (RGBA,
   gbrap and yuva444p inputs all came back unchanged), so don't rely on it.

### F3. Transparent path generates and uploads a blank 4K frame for nothing (medium)
`videotestsrc pattern=black ! alpha alpha=0.0 ! videoconvert ! glupload` costs
~10 ms of CPU per frame: 3.12 s for 300 frames with no telemetry at all.
Generating the transparent frame on the GPU (`gltestsrc ! glshader` that writes
`vec4(0)`) costs essentially nothing (0.16 s / 300 frames, output verified
α = 0).
Long-term fix: a `telemetrysrc` element (§5.3) that pushes the overlay
buffers directly.

### F4. Burn-in path round-trips every decoded frame through system memory (medium)
`scripts/video-overlay.sh:87`, `scripts/video-overlay-docker.sh:115`: the
`video/x-raw` caps force NVDEC to download to the CPU, then `glupload` sends the
frame back. Getting GL memory straight from the decoder required three things:

* **`GST_GL_API=opengl3`.** By default the surfaceless EGL context is GLES, and
  NVDEC logs `OpenGL context is not CUDA-compatible, fallback to system memory`.
  With desktop GL (4.5) NVDEC outputs GLMemory.
* **No plain `decodebin`.** `decodebin` followed by a GLMemory capsfilter
  **deadlocked** (0% CPU, no output). This is probably GL-context sharing while
  `decodebin` autoplugs (not root-caused). `decodebin3`, or explicit
  `qtdemux ! h265parse ! nvh265dec`, works.
* **`glcolorconvert` before `glvideoflip`.** NVDEC outputs NV12 GL memory and
  `glvideoflip` won't link to it directly.

Also: audio is decoded and re-encoded to AAC even when the source is AAC.
Passthrough (`demux.audio_0 ! aacparse ! mux.`) is lossless and free.

### F5. Plugin internals worth fixing (low–medium)
* **Per-frame full-frame CPU work.** Clear + composite of 33 MB, plus one 33 MB
  texture upload. This is acceptable: splitting it into per-widget uploads was
  measured to be slower (§5.1). If the clear ever shows up in traces, clear only
  the regions that were drawn into this pool buffer the last time it was used,
  not the whole surface. That needs per-buffer dirty-rect tracking, because the
  pool rotates buffers.
* **Process-global hack.** `static GstVideoOverlayComposition *old_comp`
  (`gsttelemetry.c:532`) is shared by *all* element instances in the process,
  and it keeps one composition alive after `stop`. A plausible root cause for
  the stale-overlay bug it works around: `gloverlaycompositor` wraps the
  rectangle's pixel memory (`gst_gl_video_allocation_params_new_wrapped_data`)
  and uploads lazily. Once the composition is released, `buffer_pool_acquire`
  sees the buffer as writable and reuses it, possibly before the lazy upload has
  happened. This is not verified. If it holds, the fix belongs in the pool: don't
  hand out a buffer again until the compositor has released it, rather than
  holding the previous composition in a process-global.
* **Unused-in-GL buffers.** Overlay buffers are plain system memory
  (`buffer_pool.c:99`). That's fine for the CPU path; in GL mode it guarantees an
  upload.

### F6. Docker portability (informational)
* **FFmpeg too old.** The image's FFmpeg is **8.0.1**, which has no
  `prores_ks_vulkan` ("ProRes Vulkan encoder" landed in 8.1 [1]). To use §7.1 in
  Docker, the image needs FFmpeg ≥ 8.1 (build from source or a newer base), plus
  the Vulkan ICD.
* **Driver capabilities.** The image inherits
  `NVIDIA_DRIVER_CAPABILITIES=compute,utility` from `nvidia/cuda`. Per NVIDIA's
  docs [6], GL/EGL/Vulkan need `graphics` and the Video Codec SDK needs `video`.
  On *this* host NVENC and NVIDIA GL still worked in the container (checked), so
  the host runtime config covers it. Add
  `-e NVIDIA_DRIVER_CAPABILITIES=compute,utility,video,graphics` for
  portability.

---

## 4. GPU encoding with alpha: options

| option | GPU? | alpha quality | NLE support | verified here |
|---|---|---|---|---|
| **A. `prores_ks_vulkan` 4444** | Vulkan compute | lossless alpha, 4:4:4 10/12-bit | universal (ProRes 4444) | ✅ works, 2.4× CPU speed, ~0 CPU |
| **B. NVENC fill + matte (two files)** | NVENC + GL shaders | 8-bit 4:2:0 lossy matte | any NLE with track/luma matte (Resolve, Premiere, FCP) | ✅ works, ~57 fps |
| C. NVENC HEVC alpha layer | NVENC | 8-bit 4:2:0 only | mostly Apple ecosystem; check your NLE | ❌ no FFmpeg/GStreamer support |
| D. `ffv1_vulkan` (lossless) | Vulkan | lossless | poor in NLEs | not tested |

### 4.1 A: Vulkan ProRes (recommended)
FFmpeg Changelog, version 8.1: "ProRes Vulkan encoder" [1]. Exposed as
`prores_ks_vulkan`, with profiles `4444` and `4444xq` and option `alpha_bits`
(default 16). Upstream source runs the alpha plane through a Vulkan compute
shader [2]. Input must be Vulkan frames (`hwupload`); the sw_format used was
`yuva444p10le`. RGB→YUV stays on the CPU (swscale, multithreaded; ~100+ fps at
4K here). I tried GPU conversion but couldn't use it:
* `scale_vulkan=format=yuva444p10le` "works" but writes **wrong colours**: the
  output is tagged `gbr`, Y/U/V PSNR is 5–15 dB, and it doesn't apply the
  RGB→YUV matrix.
* `libplacebo=format=yuva444p10le` fails to initialise on this build.

### 4.2 B: fill + matte on NVENC (fully GPU, smallest files)
The GL pipeline `tee`s into two branches, each `glshader → glcolorconvert NV12 → nvh265enc`:
* fill: `rgb / a` (unpremultiplied colour, opaque)
* matte: `vec4(a, a, a, 1)`

This is verified to run (150 frames 2.6 s, 3.5 CPU-s) and both outputs decode. In
the NLE, put the fill on a track and use the matte file as luma/track matte.
Trade-offs: two files, alpha is lossy 4:2:0 (fine for soft edges, slight halo on
1-px lines), and editor setup per clip. Chroma subsampling on thin coloured
lines is visible only on close inspection. Bump `nvh265enc` bitrate, or use
`Y444` (supported by `nvh265enc` sink caps) for the fill if needed.

### 4.3 C: NVENC HEVC alpha (why your attempts failed)
The NVENC Programming Guide (SDK 13.0) §8.11 [7] says:
* Alpha layer encoding is HEVC-only, enabled with
  `NV_ENC_CONFIG_HEVC::enableAlphaLayerEncoding` and `alphaLayerBitrateRatio`.
* Input must be NV12 + separate alpha, or ARGB/ABGR.
* It is **not supported** with 10-bit input, 4:2:2/4:4:4 input, output in video
  memory, or weighted prediction.

Neither `hevc_nvenc` (FFmpeg 9.0.2) nor `nvh265enc` (GStreamer 1.28.7) has an
alpha option on this host. They accept RGBA/BGRA and silently drop alpha, and
`glcolorconvert … NV12` drops it too. Using it would mean custom NVENC SDK code
plus correct MOV signalling. An FFmpeg ticket [8] shows interop problems with
NVENC alpha streams. Not recommended unless your NLE specifically wants HEVC
with alpha.

---

## 5. Moving the telemetry plugin itself to the GPU

### 5.1 Level 1: GPU composition with per-widget overlay rectangles (tried, rejected)
The idea: instead of compositing all widget caches into one 4K surface
(`Manager::draw`), attach one `GstVideoOverlayRectangle` per widget cache and
let `gloverlaycompositor` blend them.

This was already implemented and measured on `origin/dev/perf` (`cf1edc1`,
`RESULT.md`), about 2970 frames with `example/layout.xml`:

| | widget drawing | wall | CPU (user) |
|---|---|---|---|
| single surface, one rectangle | 30.5 ms/frame | 2m38s | 2m41s |
| one rectangle per widget, composited on GPU | 26.9 ms/frame | **3m14s** | **3m17s** |

Drawing got cheaper, because the clear and composite were gone, but the run was
~23% slower overall. User CPU grew by the same ~37 s as wall time, ~12 ms per
frame. That points at per-rectangle CPU and driver overhead, not PCIe bandwidth.
For every rectangle it hasn't seen before, `gloverlaycompositor` [4]:
* creates a `GstGLCompositionOverlay` with its own VAO plus position, texcoord
  and index buffers, set up on the GL thread;
* allocates a new `GstGLMemory` texture wrapping the rectangle's pixels and maps
  it, which is a texture allocation plus a synchronous upload on the GL thread;
* frees the overlays (textures, VAOs, buffers) that are no longer in the
  composition;
* draws each overlay as its own draw call.

The cost scales with the **number of changed rectangles**, not with bytes. One
33 MB upload into one texture is the cheap case. Dozens of small
create/upload/destroy cycles per frame, each crossing to the GL thread, are the
expensive case.

The original version of this section claimed that reusing rectangle objects for
unchanged widgets would avoid the problem. That only helps static widgets: every
widget whose value changes each frame (speed, power, time, the chart, the
position marker) still needs a new rectangle, and therefore a new texture and a
new VAO, every frame. Real layouts have many such widgets, so the per-rectangle
overhead stays. `cf1edc1` also created a new rectangle for every widget on every
frame. With reuse, the result would be somewhere between the two rows above, and
there's no reason to expect it to beat the single surface.

**Conclusion: keep the single surface and single rectangle.** If upload ever
needs to shrink, the GPU-side way is §5.2: one persistent full-frame texture,
updated with `glTexSubImage2D` (ideally through a PBO) for only the union of
dirty widget rectangles. That means few calls, no per-frame allocations and no
per-widget VAOs. It only makes sense if a trace shows the upload as a
bottleneck, and the measurements here don't.

### 5.2 Level 2: telemetry as a `GstGLFilter`
Own the GL step. Keep one persistent overlay texture, update only its dirty
regions with `glTexSubImage2D`, blend it with premultiplied blending, and
optionally output the unpremultiplied or matte variant directly. This gives more
control (fused unpremultiply/matte, no meta negotiation, no per-frame texture
allocation), but you'd maintain your own GL code. Don't use per-widget textures
here either, for the reasons in §5.1.

### 5.3 Level 2b: `telemetrysrc` for the transparent mode
For transparent output there is no input video. A `GstPushSrc` that pushes the
rendered overlay (straight-alpha BGRA, timestamps from fps) removes the
blank-frame source, alpha element, upload, composite and download. It's the
simplest way to feed `prores_ks_vulkan`: the CPU-only path already measured
33 fps, limited by the encoder.

### 5.4 Level 3: full GPU rasterisation (not recommended now)
Candidates: Skia (Ganesh/Graphite with GL/Vulkan; has text shaping), NanoVG
(GL; weak text and AA), ThorVG, Vello. Costs:
* rewrite all widget drawing (~2.7k lines) and replace Pango text, the hardest
  part to match visually
* a heavy build dependency (Skia)
* another GPU context to share with GStreamer's GL/CUDA/Vulkan

Benefit: rendering of the measured layout costs ~2–4 ms/frame, which is already
below every other stage, and widget caching avoids most re-rasterisation. Before
considering this, build with `-Denable_tracing=true` and trace a **real** heavy
layout (e.g. `example/layout.xml`, ~355 nodes). If one widget type dominates
(e.g. charts re-stroking thousands of points), optimise that widget, e.g. with
incremental chart drawing.

---

## 6. Roadmap (impact / effort)

1. **Scripts only, ~1 h:** switch the transparent path to §7.1 (3.5×, 5× less
   CPU), and the burn-in path to §7.2 (1.9×). Add
   `GST_GL_API=opengl3`. Pass AAC audio through.
2. **Small code fix:** `FLAG_PREMULTIPLIED_ALPHA` (F2) + unpremultiply shader
   for ProRes. This is a correctness fix for both outputs.
3. ~~Per-widget rectangles (§5.1).~~ Tried on `origin/dev/perf`; slower. Keep
   the single surface.
4. **Medium:** `telemetrysrc` (§5.3) for the transparent mode.
5. **Optional:** fill + matte NVENC mode (§4.2) as a fast "preview/draft"
   transparent output.
6. **Docker:** FFmpeg ≥ 8.1 in the image (F6).
7. **Only after tracing real layouts:** targeted widget optimisation. Full GPU
   rasterisation only if tracing proves rasterisation is the bottleneck.

---

## 7. Ready-to-use pipelines (tested on this host)

### 7.1 Transparent overlay → ProRes 4444 on the GPU (no plugin change)

```bash
export GST_GL_WINDOW=surfaceless GST_GL_API=opengl3
CLEAR='#version 100
#ifdef GL_ES
precision mediump float;
#endif
varying vec2 v_texcoord;
uniform sampler2D tex;
void main () { gl_FragColor = vec4 (0.0); }'

gst-launch-1.0 -q -e \
  gltestsrc pattern=black num-buffers=$TOTAL_FRAMES \
  ! "video/x-raw(memory:GLMemory),format=RGBA,width=$W,height=$H,framerate=$FPS/1" \
  ! glshader fragment="\"$CLEAR\"" \
  ! telemetry $PROPERTIES \
  ! "video/x-raw(memory:GLMemory,meta:GstVideoOverlayComposition)" ! queue \
  ! gloverlaycompositor ! gldownload ! video/x-raw,format=RGBA ! queue ! fdsink fd=1 \
| ffmpeg -hide_banner -y -f rawvideo -pix_fmt rgba -s ${W}x${H} -framerate $FPS -i - \
    -init_hw_device vulkan=vk -filter_hw_device vk \
    -vf format=yuva444p10le,hwupload \
    -c:v prores_ks_vulkan -profile:v 4444 -async_depth 4 "$OUTPUT_FILE"
```
Notes:
* Use `-q` on `gst-launch-1.0`, because its stdout is the video stream.
* After the F2 fix, insert the unpremultiply shader (§7.3) after
  `gloverlaycompositor`.
* ProRes is intra-only, so if you ever need more than ~42 fps you can render
  time-ranges in parallel and join them losslessly with FFmpeg's concat demuxer
  (not tested).

### 7.2 Burn-in, GL memory end to end

```bash
export GST_GL_WINDOW=surfaceless GST_GL_API=opengl3
gst-launch-1.0 -e filesrc location=$IN ! qtdemux name=demux \
  demux.video_0 ! queue ! decodebin3 ! 'video/x-raw(memory:GLMemory)' \
    ! glcolorconvert ! 'video/x-raw(memory:GLMemory),format=RGBA' \
    ! glvideoflip video-direction=auto ! taginject tags="image-orientation=rotate-0" \
    ! gltransformation ! 'video/x-raw(memory:GLMemory),width=3840,height=2160' \
    ! telemetry $PROPERTIES ! 'video/x-raw(memory:GLMemory,meta:GstVideoOverlayComposition)' \
    ! gloverlaycompositor ! nvh264enc bitrate=120000 ! h264parse ! queue ! mux. \
  demux.audio_0 ! queue ! aacparse ! mux. \
  mp4mux name=mux faststart=true ! filesink location=$OUT
```
Notes:
* I benchmarked this with `h265parse ! nvh265dec` in place of `decodebin3`
  (3.78 s / 300 frames), and `decodebin3` separately without flip/audio. I
  haven't run this exact combination.
* Rotated (portrait) input not tested.
* Keep `avenc_aac` re-encoding if the source audio isn't AAC.

### 7.3 Shaders
```glsl
// unpremultiply (for ProRes after the F2 fix): straight alpha out
vec4 c = texture2D (tex, v_texcoord);
gl_FragColor = c.a > 0.0 ? vec4 (c.rgb / c.a, c.a) : vec4 (0.0);

// fill for §4.2 (opaque, straight colour)
gl_FragColor = c.a > 0.0 ? vec4 (c.rgb / c.a, 1.0) : vec4 (0.0, 0.0, 0.0, 1.0);

// matte for §4.2
gl_FragColor = vec4 (c.a, c.a, c.a, 1.0);
```
Each needs the same `#version 100` / precision / `varying vec2 v_texcoord;
uniform sampler2D tex;` header as `CLEAR` above. Fill + matte branch:
`tee name=t t. ! queue ! glshader fragment=FILL ! glcolorconvert ! "video/x-raw(memory:GLMemory),format=NV12" ! nvh265enc ! h265parse ! mp4mux ! filesink …`
and the same with `MATTE`.

---

## 8. Not verified / caveats
* Benchmarks use a light synthetic layout. Heavy real layouts make rendering a
  larger share. Per-widget caching already limits re-rasterisation; per-widget
  GPU composition does not help (§5.1).
* The per-rectangle overhead explanation in §5.1 is read from the
  `gloverlaycompositor` source and agrees with the `origin/dev/perf`
  measurement. It wasn't profiled separately.
* F2's fix was derived from GStreamer source and measured output, but not built
  and tested.
* The `decodebin` + GLMemory deadlock wasn't root-caused (decodebin3 and explicit
  decoders work).
* NLE import of `prores_ks_vulkan` output (Resolve/Premiere/FCP) not tested; it's
  standard ProRes 4444 per ffprobe.
* The `old_comp` root cause in F5 is a hypothesis.

## Sources
1. FFmpeg Changelog (8.1: "ProRes Vulkan encoder"): https://raw.githubusercontent.com/FFmpeg/FFmpeg/master/Changelog
2. `libavcodec/proresenc_kostya_vulkan.c`: https://github.com/FFmpeg/FFmpeg/blob/master/libavcodec/proresenc_kostya_vulkan.c
3. Cinelerra-GG list, Vulkan ProRes encoder MR testing: https://lists.cinelerra-gg.org/archives/list/cin@lists.cinelerra-gg.org/message/VUUXCVXKV3SZH4LOGAAFZUNF6IPS5NVC
4. `gst-libs/gst/gl/gstgloverlaycompositor.c` (blend funcs, per-rectangle caching): https://github.com/GStreamer/gstreamer/blob/main/subprojects/gst-plugins-base/gst-libs/gst/gl/gstgloverlaycompositor.c
5. Apple ProRes White Paper, "Apple ProRes 4444 Alpha Channel Support": https://apple.com.cn/final-cut-pro/docs/Apple_ProRes_White_Paper_December_2013.pdf
6. NVIDIA Container Toolkit, driver capabilities: https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/latest/docker-specialized.html
7. NVENC Video Encoder API Programming Guide 13.0, §8.11 Alpha layer encoding: https://docs.nvidia.com/video-technologies/video-codec-sdk/13.0/nvenc-video-encoder-api-prog-guide/index.html
8. FFmpeg ticket #9088 (HEVC alpha interop): https://trac.ffmpeg.org/ticket/9088
9. NVIDIA forum, NVENC HEVC alpha usage: https://forums.developer.nvidia.com/t/pynvenc-hevc-alpha/271976
