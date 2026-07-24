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

ENV GST_GL_PLATFORM=egl
ENV GST_GL_WINDOW=none

COPY docker/entrypoint.sh /entrypoint.sh

ENTRYPOINT ["/entrypoint.sh"]
