#!/bin/bash
# Start a virtual X display, VNC server, and noVNC web client
# Access the graphical output at http://localhost:6080

if [ ! -f /tmp/.X99-lock ]; then
    nohup Xvfb :99 -screen 0 1440x900x24 > /dev/null 2>&1 &
    sleep 1
fi

pgrep -x x11vnc > /dev/null || nohup x11vnc -display :99 -forever -nopw -quiet > /dev/null 2>&1 &
pgrep -x websockify > /dev/null || nohup websockify --web /usr/share/novnc 6080 localhost:5900 > /dev/null 2>&1 &

echo "noVNC running at http://localhost:6080"
echo "Run your fdf binary now: ./fdf maps/mars.fdf"
