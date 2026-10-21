#!/usr/bin/env bash
# Launches a dedicated server and N clients that connect to it locally.
#
# Usage:
#   ./launch.sh                          # server + 2 packaged clients
#   ./launch.sh -c 4                     # server + 4 clients in a grid
#   ./launch.sh --mode editor            # current code via UnrealEditor.exe (no packaging)
#   ./launch.sh -c 8 --visible 0         # clients without rendering (bots / load tests)
#   ./launch.sh -c 10 --visible 2        # 2 clients with windows, 8 without rendering
#   ./launch.sh --stop                   # stop everything started by this script
#
# Options:
#   -c, --clients N        number of clients (default 2)
#   -p, --port N           server port (default 7780)
#   -m, --map PATH         server map (default /Game/Maps/Lobby/L_Lobby)
#       --mode MODE        packaged | editor (default packaged)
#       --server-build DIR packaged server folder (default: newest PUBLISH/ServerBuild_*)
#       --client-build DIR packaged client folder (default: newest PUBLISH/GameBuild_*)
#       --visible N        only the first N clients get a window, the rest are headless
#       --res WxH          client window size (default 800x450)
#       --max-fps N        client frame rate cap, 0 = unlimited (default 60)
#       --server-console   show the server log in its own console window
#       --timeout N        seconds to wait for the server (default 120)
#       --server-args STR  extra server arguments
#       --client-args STR  extra client arguments
#       --stop             stop processes started by this script

set -euo pipefail

CLIENTS=2
PORT=7780
MAP="/Game/Maps/Lobby/L_Lobby"
MODE="packaged"
SERVER_BUILD=""
CLIENT_BUILD=""
ENGINE_DIR="C:/UnrealEngine"
VISIBLE=-1
RES_X=800
RES_Y=450
MAX_FPS=60
SERVER_CONSOLE=0
SERVER_TIMEOUT=120
SERVER_ARGS=""
CLIENT_ARGS=""
STOP=0

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
PROJECT_FILE="$PROJECT_DIR/MultiplayerGame.uproject"
PUBLISH_DIR="$(dirname "$PROJECT_DIR")/PUBLISH"
LAUNCH_DIR="$PROJECT_DIR/Saved/Launch"
PID_FILE="$LAUNCH_DIR/pids.txt"

while [[ $# -gt 0 ]]; do
    case "$1" in
        -c|--clients)     CLIENTS="$2"; shift 2 ;;
        -p|--port)        PORT="$2"; shift 2 ;;
        -m|--map)         MAP="$2"; shift 2 ;;
        --mode)           MODE="${2,,}"; shift 2 ;;
        --server-build)   SERVER_BUILD="$2"; shift 2 ;;
        --client-build)   CLIENT_BUILD="$2"; shift 2 ;;
        --visible)        VISIBLE="$2"; shift 2 ;;
        --res)            RES_X="${2%x*}"; RES_Y="${2#*x}"; shift 2 ;;
        --max-fps)        MAX_FPS="$2"; shift 2 ;;
        --server-console) SERVER_CONSOLE=1; shift ;;
        --timeout)        SERVER_TIMEOUT="$2"; shift 2 ;;
        --server-args)    SERVER_ARGS="$2"; shift 2 ;;
        --client-args)    CLIENT_ARGS="$2"; shift 2 ;;
        --stop)           STOP=1; shift ;;
        -h|--help)        sed -n '2,27p' "$0" | sed 's/^# \{0,1\}//'; exit 0 ;;
        *)                echo "Unknown option: $1 (see --help)" >&2; exit 1 ;;
    esac
done

# Stops processes by Windows PID saved in PID_FILE (/T also stops their children)
stop_launched() {
    if [[ ! -f "$PID_FILE" ]]; then
        echo "Nothing to stop."
        return
    fi
    while read -r WIN_PID; do
        [[ -z "$WIN_PID" ]] && continue
        if taskkill //PID "$WIN_PID" //T //F >/dev/null 2>&1; then
            echo "Stopped PID $WIN_PID"
        fi
    done < "$PID_FILE"
    rm -f "$PID_FILE"
}

latest_build() {
    local PATTERN="$1" SUB_DIR="$2" BUILD
    BUILD="$(ls -d "$PUBLISH_DIR"/$PATTERN 2>/dev/null | sort -r | head -n 1)"
    if [[ -z "$BUILD" ]]; then
        echo "No $PATTERN folder found in $PUBLISH_DIR" >&2
        exit 1
    fi
    echo "$BUILD/$SUB_DIR"
}

# Starts a Windows process detached from this shell and records its Windows PID.
#   $1 - window style (Normal / Hidden)
#   $2 - priority class (Normal / AboveNormal / High)
#   $3 - executable, the rest are its arguments
start_process() {
    local WINDOW_STYLE="$1" PRIORITY="$2" EXE_WIN ARG ARG_STR=""
    EXE_WIN="$(cygpath -w "$3")"
    shift 3
    for ARG in "$@"; do
        if [[ "$ARG" =~ ^(-[A-Za-z]+=)(.*\ .*)$ ]]; then
            ARG="${BASH_REMATCH[1]}\"${BASH_REMATCH[2]}\""
        elif [[ "$ARG" == *" "* ]]; then
            ARG="\"$ARG\""
        fi
        ARG_STR+="$ARG "
    done
    ARG_STR="${ARG_STR//\'/\'\'}"

    local PS_COMMAND
    PS_COMMAND="\$p = Start-Process -FilePath '$EXE_WIN' -ArgumentList '$ARG_STR' -WindowStyle $WINDOW_STYLE -PassThru;"
    PS_COMMAND+=" \$p.PriorityClass = '$PRIORITY'; \$p.Id"

    local WIN_PID
    WIN_PID="$(MSYS_NO_PATHCONV=1 powershell -NoProfile -Command "$PS_COMMAND" | tr -d '\r')"
    if [[ ! "$WIN_PID" =~ ^[0-9]+$ ]]; then
        echo "Failed to start $EXE_WIN" >&2
        exit 1
    fi
    echo "$WIN_PID" >> "$PID_FILE"
    echo "$WIN_PID"
}

if [[ $STOP -eq 1 ]]; then
    stop_launched
    exit 0
fi

mkdir -p "$LAUNCH_DIR"

# Processes from a previous run would keep the port busy
if [[ -f "$PID_FILE" ]]; then
    echo "Stopping processes from the previous run..."
    stop_launched
fi

# Executables and base arguments.
# Packaged builds are started from MultiplayerGame/Binaries/Win64 directly: the exe in the
# build root is only a small launcher that starts this one as a child process.
case "$MODE" in
    packaged)
        [[ -z "$SERVER_BUILD" ]] && SERVER_BUILD="$(latest_build "ServerBuild_*" "WindowsServer")"
        [[ -z "$CLIENT_BUILD" ]] && CLIENT_BUILD="$(latest_build "GameBuild_*" "Windows")"
        SERVER_EXE="$SERVER_BUILD/MultiplayerGame/Binaries/Win64/MultiplayerGameServer.exe"
        CLIENT_EXE="$CLIENT_BUILD/MultiplayerGame/Binaries/Win64/MultiplayerGame.exe"
        SERVER_BASE_ARGS=()
        CLIENT_BASE_ARGS=()
        ;;
    editor)
        SERVER_EXE="$ENGINE_DIR/Engine/Binaries/Win64/UnrealEditor.exe"
        CLIENT_EXE="$SERVER_EXE"
        SERVER_BASE_ARGS=("$(cygpath -w "$PROJECT_FILE")" -server)
        CLIENT_BASE_ARGS=("$(cygpath -w "$PROJECT_FILE")" -game)
        ;;
    *)
        echo "Unknown mode: $MODE (packaged | editor)" >&2
        exit 1
        ;;
esac

for EXE in "$SERVER_EXE" "$CLIENT_EXE"; do
    if [[ ! -f "$EXE" ]]; then
        echo "Executable not found: $EXE" >&2
        exit 1
    fi
done

# Separate log folder for every run
RUN_DIR="$LAUNCH_DIR/$(date +%Y-%m-%d_%H-%M-%S)"
mkdir -p "$RUN_DIR"
SERVER_LOG="$RUN_DIR/server.log"

echo "Mode:    $MODE"
echo "Server:  $SERVER_EXE"
echo "Client:  $CLIENT_EXE"
echo "Logs:    $RUN_DIR"
echo

# Server runs above normal priority so that clients on the same machine do not starve it.
# Its console is hidden by default: clicking into a console (QuickEdit) pauses log output
# and freezes the whole server.
ARGS=("${SERVER_BASE_ARGS[@]}" "$MAP" "-port=$PORT" "-abslog=$(cygpath -w "$SERVER_LOG")")
SERVER_WINDOW="Hidden"
if [[ $SERVER_CONSOLE -eq 1 ]]; then
    ARGS+=(-log)
    SERVER_WINDOW="Normal"
fi
[[ -n "$SERVER_ARGS" ]] && read -r -a EXTRA <<< "$SERVER_ARGS" && ARGS+=("${EXTRA[@]}")
SERVER_PID="$(start_process "$SERVER_WINDOW" AboveNormal "$SERVER_EXE" "${ARGS[@]}")"
echo "Server started (PID $SERVER_PID), waiting for it to listen on port $PORT..."

READY=0
for (( i = 0; i < SERVER_TIMEOUT * 2; i++ )); do
    if ! tasklist //FI "PID eq $SERVER_PID" 2>/dev/null | grep -q "$SERVER_PID"; then
        echo "Server exited. See $SERVER_LOG" >&2
        exit 1
    fi
    if [[ -f "$SERVER_LOG" ]] && grep -q "listening on port" "$SERVER_LOG"; then
        READY=1
        break
    fi
    sleep 0.5
done
if [[ $READY -eq 1 ]]; then
    echo "Server is ready."
else
    echo "Warning: server did not report listening within ${SERVER_TIMEOUT}s, starting clients anyway." >&2
fi

# Client windows are laid out in a grid that fits the primary screen
SCREEN_WIDTH="$(powershell -NoProfile -Command "Add-Type -AssemblyName System.Windows.Forms; [System.Windows.Forms.Screen]::PrimaryScreen.WorkingArea.Width" 2>/dev/null | tr -d '\r' || true)"
[[ "$SCREEN_WIDTH" =~ ^[0-9]+$ ]] || SCREEN_WIDTH=1920
COLUMNS=$(( SCREEN_WIDTH / (RES_X + 20) ))
(( COLUMNS < 1 )) && COLUMNS=1

for (( i = 1; i <= CLIENTS; i++ )); do
    CLIENT_LOG="$RUN_DIR/client_$i.log"
    ARGS=("${CLIENT_BASE_ARGS[@]}" "127.0.0.1:$PORT?Name=Player$i" "-abslog=$(cygpath -w "$CLIENT_LOG")")

    # Uncapped clients take several CPU cores each and slow down the server
    (( MAX_FPS > 0 )) && ARGS+=("-ExecCmds=t.MaxFPS $MAX_FPS")

    CLIENT_WINDOW="Normal"
    if (( VISIBLE >= 0 && i > VISIBLE )); then
        ARGS+=(-nullrhi -nosound)
        CLIENT_WINDOW="Hidden"
    else
        INDEX=$(( i - 1 ))
        WIN_X=$(( (INDEX % COLUMNS) * (RES_X + 20) ))
        WIN_Y=$(( (INDEX / COLUMNS) * (RES_Y + 40) ))
        ARGS+=(-windowed "-ResX=$RES_X" "-ResY=$RES_Y" "-WinX=$WIN_X" "-WinY=$WIN_Y")
    fi
    [[ -n "$CLIENT_ARGS" ]] && read -r -a EXTRA <<< "$CLIENT_ARGS" && ARGS+=("${EXTRA[@]}")

    CLIENT_PID="$(start_process "$CLIENT_WINDOW" Normal "$CLIENT_EXE" "${ARGS[@]}")"
    echo "Client $i started (PID $CLIENT_PID)"
    sleep 0.5
done

echo
echo "Done. Server log:  tail -f \"$SERVER_LOG\""
echo "Stop everything:   ./launch.sh --stop"
