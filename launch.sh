#!/bin/bash
# launch.sh — Start opons-voxd with rotating logs.
# Keeps opons_voxd.log (current) + opons_voxd.log.old (previous).
# Rotates when the current log exceeds 5 MB.

cd "$(dirname "$0")" || exit 1

LOG="opons_voxd.log"
MAX_BYTES=5242880  # 5 MB

if [ -f "$LOG" ] && [ "$(stat -c%s "$LOG" 2>/dev/null)" -gt "$MAX_BYTES" ]; then
    mv -f "$LOG" "${LOG}.old"
fi

echo "=== opons-voxd started at $(date) ===" >> "$LOG"
export OPONS_VOXD_COMMANDS=1
# Push-to-talk hotkey. Ctrl+Alt+W is reachable with the left hand
# alone on AZERTY (W is next to A; pinky=Ctrl, thumb=Alt,
# ring/middle=W) and isn't bound by default in most desktop
# environments. QWERTY users want ctrl+alt+z for the same physical
# position. The default ctrl+shift+space gets intercepted by some
# IMEs and terminals, so we override here.
export OPONS_VOXD_PTT_HOTKEY=ctrl+alt+w
exec ./opons-voxd 2>>"$LOG"
