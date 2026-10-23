#!/usr/bin/env bash
# Shows the readable part of the logs from the latest launch.sh run:
# project messages (LogMGame, LogMLobby, ...), warnings and errors, with the time only.
#
# Usage:
#   ./logs.sh                 # server log
#   ./logs.sh client_1        # log of client 1
#   ./logs.sh all             # server and all clients, one after another
#   ./logs.sh server -f       # follow the server log live
#   ./logs.sh server --run 2026-10-23_20-36-37   # a specific run instead of the latest

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LAUNCH_DIR="$(dirname "$SCRIPT_DIR")/Saved/Launch"

TARGET="server"
FOLLOW=0
RUN=""

while [[ $# -gt 0 ]]; do
    case "$1" in
        -f|--follow) FOLLOW=1; shift ;;
        --run)       RUN="$2"; shift 2 ;;
        -h|--help)   sed -n '2,10p' "$0" | sed 's/^# \{0,1\}//'; exit 0 ;;
        *)           TARGET="$1"; shift ;;
    esac
done

if [[ -z "$RUN" ]]; then
    RUN="$(ls -d "$LAUNCH_DIR"/20*/ 2>/dev/null | sort -r | head -n 1)"
    [[ -z "$RUN" ]] && { echo "No runs found in $LAUNCH_DIR" >&2; exit 1; }
else
    RUN="$LAUNCH_DIR/$RUN"
fi
RUN="${RUN%/}"

# Known harmless engine warnings, the same ones launch.sh limits to errors
NOISE="(LogPlayerManagement|LogWorldPartition|LogStreaming|LogLinker|LogLiveCoding): Warning:"

# Keeps project lines, warnings and errors; turns "[2026.10.23-18.37.57:218][429]" into "18:37:57"
# and collapses repeated messages into one line with a counter.
filter() {
    grep --line-buffered -E "LogM[A-Z][A-Za-z]*:|Warning:|Error:" \
        | grep --line-buffered -v -E "$NOISE" \
        | sed -u -E 's/^\[[0-9.]+-([0-9]+)\.([0-9]+)\.([0-9]+):[0-9]+\]\[ *[0-9]+\]/\1:\2:\3 /' \
        | awk -v follow="$FOLLOW" '
            function flush() { if (count > 1) print last "  (x" count ")"; else if (count == 1) print last; fflush() }
            {
                message = $0; sub(/^[0-9:]+ /, "", message)
                if (message == last_message) { count++; next }
                flush(); last = $0; last_message = message; count = 1
                if (follow) { print last; fflush(); count = 0 }
            }
            END { flush() }'
}

show() {
    local FILE="$RUN/$1.log"
    [[ -f "$FILE" ]] || { echo "No $1.log in $RUN" >&2; return 1; }
    echo "=== $1  ($(basename "$RUN"))"
    if [[ $FOLLOW -eq 1 ]]; then
        tail -n +1 -f "$FILE" | filter
    else
        filter < "$FILE" || true
    fi
}

if [[ "$TARGET" == "all" ]]; then
    for LOG in "$RUN"/server.log "$RUN"/client_*.log; do
        show "$(basename "$LOG" .log)"
        echo
    done
else
    show "$TARGET"
fi
