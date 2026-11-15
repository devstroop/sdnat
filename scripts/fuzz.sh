#!/usr/bin/env bash
# Parser fuzz campaign. Runs `zig build fuzz` repeatedly until the requested
# duration elapses, accumulating coverage in the Zig cache. A non-zero exit
# from the fuzzer (crash/assertion) stops the campaign and leaves the log.
set -u

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

show_usage() {
    cat <<'EOF'
usage: scripts/fuzz.sh [options]

  --duration TIME        total campaign time, e.g. 72h, 10m, 30s (default 10m)
  --iterations N         iterations per target per pass (default 100000)
  --zig PATH             zig binary (default: zig or $ZIG)
  --no-build             reuse the existing fuzz binary
  --keep                 keep the log on success
  -h, --help             show this help
EOF
}

DURATION="10m"
ITERATIONS="100000"
ZIG="${ZIG:-zig}"
NO_BUILD="0"
KEEP="0"

while [ "$#" -gt 0 ]; do
    case "$1" in
        --duration) DURATION="$2"; shift 2 ;;
        --iterations) ITERATIONS="$2"; shift 2 ;;
        --zig) ZIG="$2"; shift 2 ;;
        --no-build) NO_BUILD="1"; shift ;;
        --keep) KEEP="1"; shift ;;
        -h|--help) show_usage; exit 0 ;;
        *) echo "FAIL: unknown option $1" >&2; show_usage; exit 2 ;;
    esac
done

command -v "$ZIG" >/dev/null 2>&1 || { echo "FAIL: zig not found" >&2; exit 1; }

parse_duration() {
    local s="$1"
    case "$s" in
        *h) echo $(( ${s%h} * 3600 )) ;;
        *m) echo $(( ${s%m} * 60 )) ;;
        *s) echo $(( ${s%s} )) ;;
        *) echo "$s" ;;
    esac
}
TOTAL="$(parse_duration "$DURATION")"
[ "$TOTAL" -gt 0 ] 2>/dev/null || { echo "FAIL: bad duration $DURATION" >&2; exit 2; }

WORK="$(mktemp -d)"
LOG="$WORK/fuzz.log"

START="$(date +%s)"
PASS=0
echo "SDNAT fuzz campaign: duration=${DURATION} iterations=${ITERATIONS} zig=$ZIG" | tee "$LOG"
while :; do
    NOW="$(date +%s)"
    ELAPSED=$(( NOW - START ))
    [ "$ELAPSED" -ge "$TOTAL" ] && break
    PASS=$(( PASS + 1 ))
    echo "=== pass $PASS (elapsed ${ELAPSED}s) ===" | tee -a "$LOG"
    if [ "$NO_BUILD" != "1" ]; then
        if ! zig build fuzz --fuzz="$ITERATIONS" >>"$LOG" 2>&1; then
            echo "FAIL: fuzz pass $PASS exited non-zero (see $LOG)" >&2
            exit 1
        fi
    else
        if ! "$ZIG" build fuzz --fuzz="$ITERATIONS" >>"$LOG" 2>&1; then
            echo "FAIL: fuzz pass $PASS exited non-zero (see $LOG)" >&2
            exit 1
        fi
    fi
done

END="$(date +%s)"
echo "campaign complete: passes=$PASS elapsed=$(( END - START ))s log=$LOG" | tee -a "$LOG"
