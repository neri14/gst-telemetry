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
    && rm -rf /var/lib/apt/lists/*

ENV GST_GL_PLATFORM=egl
ENV GST_GL_WINDOW=none

COPY docker/entrypoint.sh /entrypoint.sh

ENTRYPOINT ["/entrypoint.sh"]
