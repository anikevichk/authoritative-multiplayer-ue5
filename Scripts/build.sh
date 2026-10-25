#!/usr/bin/env bash
# Packages the client and/or the dedicated server into the next numbered PUBLISH folder.
#
# Usage:
#   ./build.sh                           # client + server, Development
#   ./build.sh -t server                 # server only
#   ./build.sh -t client --config Shipping
#
# Options:
#   -t, --target TARGET    client | server | all (default all)
#       --config CONFIG    Development | Shipping (default Development; Shipping has no console)
#       --no-cook          only compile and stage, reuse previously cooked content

set -euo pipefail

TARGET="all"
CONFIG="Development"
COOK="-cook"
ENGINE_DIR="C:/UnrealEngine"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
PROJECT_FILE="$PROJECT_DIR/MultiplayerGame.uproject"
PUBLISH_DIR="$(dirname "$PROJECT_DIR")/PUBLISH"
UAT="$ENGINE_DIR/Engine/Build/BatchFiles/RunUAT.bat"

while [[ $# -gt 0 ]]; do
    case "$1" in
        -t|--target) TARGET="${2,,}"; shift 2 ;;
        --config)    CONFIG="$2"; shift 2 ;;
        --no-cook)   COOK="-skipcook"; shift ;;
        -h|--help)   sed -n '2,12p' "$0" | sed 's/^# \{0,1\}//'; exit 0 ;;
        *)           echo "Unknown option: $1 (see --help)" >&2; exit 1 ;;
    esac
done

# Returns the next free folder name for a prefix, e.g. GameBuild_03 after GameBuild_02
next_build_dir() {
    local PREFIX="$1" LAST NUM
    LAST="$(ls -d "$PUBLISH_DIR"/"$PREFIX"_[0-9]* 2>/dev/null | sort -r | head -n 1)"
    NUM=0
    [[ -n "$LAST" ]] && NUM=$((10#${LAST##*_}))
    printf "%s/%s_%02d" "$PUBLISH_DIR" "$PREFIX" $((NUM + 1))
}

# Runs BuildCookRun and archives the result.
#   $1 - archive folder, the rest are target-specific UAT arguments
run_uat() {
    local ARCHIVE_DIR="$1"
    shift
    echo "==> Building into $ARCHIVE_DIR"
    MSYS_NO_PATHCONV=1 "$UAT" BuildCookRun \
        -project="$(cygpath -w "$PROJECT_FILE")" \
        -platform=Win64 -build $COOK -stage -pak -archive -utf8output \
        -archivedirectory="$(cygpath -w "$ARCHIVE_DIR")" \
        "$@"
}

case "$TARGET" in
    client|server|all) ;;
    *) echo "Unknown target: $TARGET (client | server | all)" >&2; exit 1 ;;
esac

mkdir -p "$PUBLISH_DIR"

if [[ "$TARGET" == "client" || "$TARGET" == "all" ]]; then
    run_uat "$(next_build_dir GameBuild)" -clientconfig="$CONFIG"
fi

if [[ "$TARGET" == "server" || "$TARGET" == "all" ]]; then
    run_uat "$(next_build_dir ServerBuild)" -server -serverplatform=Win64 -serverconfig="$CONFIG" -noclient
fi

echo "==> Done. Start with: ./launch.sh"
