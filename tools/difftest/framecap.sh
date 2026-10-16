#!/bin/bash
# usage: framecap.sh GAMEDIR OUTPREFIX   - deterministic-as-possible capture set
# for differential testing (run under tools/reap.py). Captures the game window
# itself, so the two binaries' frames line up regardless of desktop placement.
G=$1; O=$2
cd "$G" || exit 1
export WINEDEBUG=-all
xvfb-run -a -s "-screen 0 1024x768x24" bash -c '
wine ./hover.exe > "$1.log" 2>&1 & WP=$!
sleep 12; xdotool key Return; sleep 50
W=$(xdotool search --name "^Hover!" | tail -1); echo win=$W
xdotool key --window $W Return; sleep 3; import -window $W "$1_menu.png"
xdotool key --window $W F2; sleep 3;  import -window $W "$1_start3.png"
sleep 7;                              import -window $W "$1_start10.png"
for i in 1 2 3 4 5 6 7 8; do xdotool key --window $W Right; sleep 0.25; done
sleep 2;                              import -window $W "$1_turn.png"
xdotool keydown --window $W Up; sleep 3; xdotool keyup --window $W Up; sleep 2
import -window $W "$1_drive.png"
sleep 8;                              import -window $W "$1_drive8.png"
kill $WP; wineserver -k; sleep 2' _ "$O"
