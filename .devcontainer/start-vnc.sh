#!/bin/bash
# Start a virtual X display, VNC server, and noVNC web client
# Access the graphical output at http://localhost:6080

if [ ! -f /tmp/.X99-lock ]; then
    Xvfb :99 -screen 0 1440x900x24 &
    sleep 1
fi

pgrep -x x11vnc > /dev/null || x11vnc -display :99 -forever -nopw -quiet &
pgrep -x websockify > /dev/null || websockify --web /usr/share/novnc 6080 localhost:5900 &

echo "noVNC running at http://localhost:6080"
echo "Run your fdf binary now: ./fdf maps/mars.fdf"
