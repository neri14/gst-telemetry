# ---- Build Stage ----
FROM nvidia/cuda:13.3.0-runtime-ubuntu26.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    # Build tools
    meson \
    ninja-build \
    pkg-config \
    gcc \
    g++ \
    # GStreamer dev headers
    libgstreamer1.0-dev \
    libgstreamer-plugins-base1.0-dev \
    # Plugin dependencies
    libcairo2-dev \
    libpango1.0-dev \
    libpugixml-dev \
    && rm -rf /var/lib/apt/lists/*

# Copy source code and subprojects (exprtk wrap)
COPY meson.build meson.options /build/
COPY src/ /build/src/
COPY subprojects/ /build/subprojects/

WORKDIR /build
RUN meson setup builddir --werror --prefix /usr/ --libdir=lib/x86_64-linux-gnu \
    && ninja -C builddir \
    && DESTDIR=/staging ninja -C builddir install

# ---- FFmpeg Build Stage ----
# Ubuntu's FFmpeg (8.0) predates the prores_ks_vulkan encoder (8.1) used by
# transparent-overlay-docker.sh. Vulkan shaders are compiled at build time (glslang).
FROM nvidia/cuda:13.3.0-runtime-ubuntu26.04 AS ffmpeg-builder

ENV DEBIAN_FRONTEND=noninteractive

ARG FFMPEG_VERSION=9.0.2
ARG FFMPEG_SHA256=8c3850283eb25fa026482078a04051e0be17347b09ef81a0849bec15a96e002e

RUN apt-get update && apt-get install -y \
    build-essential \
    nasm \
    pkg-config \
    curl \
    xz-utils \
    libvulkan-dev \
    glslang-tools \
    spirv-headers \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /ffmpeg
RUN curl -fsSL -o ffmpeg.tar.xz https://ffmpeg.org/releases/ffmpeg-${FFMPEG_VERSION}.tar.xz \
    && echo "${FFMPEG_SHA256}  ffmpeg.tar.xz" | sha256sum -c - \
    && tar -xJf ffmpeg.tar.xz --strip-components=1 \
    && ./configure --prefix=/usr/local --enable-vulkan --disable-doc --disable-ffplay --disable-debug \
    && make -j"$(nproc)" \
    && make install

# ---- Runtime Stage ----
FROM nvidia/cuda:13.3.0-runtime-ubuntu26.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    gstreamer1.0-tools \
    gstreamer1.0-plugins-base \
    gstreamer1.0-plugins-good \
    gstreamer1.0-plugins-bad \
    gstreamer1.0-plugins-ugly \
    gstreamer1.0-gl \
    gstreamer1.0-libav \
    libvulkan1 \
    mesa-utils \
    libegl1 \
    libgl1 \
    libgles2 \
    # Runtime deps for the telemetry plugin
    libcairo2 \
    libpango-1.0-0 \
    libpangocairo-1.0-0 \
    libpugixml1v5 \
    && rm -rf /var/lib/apt/lists/*

# Copy the built plugin from the build stage
COPY --from=builder /staging/usr/ /usr/

COPY --from=ffmpeg-builder /usr/local/bin/ffmpeg /usr/local/bin/ffprobe /usr/local/bin/

ENV GST_GL_PLATFORM=egl
ENV GST_GL_WINDOW=none
# graphics: GL/EGL/Vulkan (ICD), video: NVDEC/NVENC
ENV NVIDIA_DRIVER_CAPABILITIES=compute,utility,video,graphics

COPY docker/entrypoint.sh /entrypoint.sh

ENTRYPOINT ["/entrypoint.sh"]
