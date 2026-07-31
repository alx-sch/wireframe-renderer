FROM mcr.microsoft.com/devcontainers/base:ubuntu

RUN apt-get update && apt-get install -y \
    build-essential \
    gcc \
    make \
    git \
    bc \
    libx11-dev \
    libxext-dev \
    libbsd-dev \
    x11-apps \
    xvfb \
    x11vnc \
    novnc \
    websockify \
    && rm -rf /var/lib/apt/lists/*

COPY start-vnc.sh /usr/local/bin/start-vnc.sh
RUN chmod +x /usr/local/bin/start-vnc.sh

# Redirect root URL to the noVNC viewer
RUN echo '<html><head><meta http-equiv="refresh" content="0;url=vnc_lite.html?autoconnect=true"></head></html>' \
    > /usr/share/novnc/index.html

ENV DISPLAY=:99
