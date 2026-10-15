#!/usr/bin/env bash
# Fast isolated tests for the real validator's retry decisions.
# Neither Steam nor Workbench is launched.
set -Eeuo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
VALIDATOR="$SCRIPT_DIR/validate-reforger-linux.sh"
TEMP="$(mktemp -d)"
trap 'rm -rf -- "$TEMP"' EXIT

mkdir -p "$TEMP/checkout" \
  "$TEMP/steam/steamapps/common/Arma Reforger/addons/data" \
  "$TEMP/prefix/pfx/dosdevices" "$TEMP/tools"
touch "$TEMP/checkout/addon.gproj"
touch "$TEMP/steam/steamapps/common/Arma Reforger/addons/data/ArmaReforger.gproj"
touch "$TEMP/tools/ArmaReforgerWorkbenchSteamDiag.exe"
ln -s / "$TEMP/prefix/pfx/dosdevices/z:"

MOCK_PROTON="$TEMP/mock-proton"
cat > "$MOCK_PROTON" <<'MOCK'
#!/usr/bin/env bash
set -Eeuo pipefail
n=0
if [[ -f "$MOCK_COUNTER" ]]; then n="$(cat "$MOCK_COUNTER")"; fi
n=$((n + 1))
printf '%s\n' "$n" > "$MOCK_COUNTER"
logroot="$REFORGER_PREFIX/pfx/drive_c/users/steamuser/Documents/My Games/ArmaReforgerWorkbench/logs"
mkdir -p "$logroot"
sleep 0.03
session="$logroot/session-$n"
case "$MOCK_SCENARIO" in
  crash_then_success|always_crash|zero_exit_crash)
    if [[ "$MOCK_SCENARIO" == always_crash || "$n" -eq 1 ]]; then
      mkdir -p "$session"
      echo "startup crash before ScriptEditor" > "$session/console.log"
      echo "double free or corruption (!prev)"
      if [[ "$MOCK_SCENARIO" == zero_exit_crash ]]; then exit 0; fi
      exit 134
    fi
    ;;
  compiler_error)
    mkdir -p "$session"
    echo "SCRIPT (E): scripts/Game/MyMod.c(9): error: Syntax error" > "$session/script.log"
    exit 0
    ;;
  crash_with_compiler_diagnostics)
    mkdir -p "$session"
    echo "SCRIPT (E): scripts/Game/MyMod.c(9): error: Syntax error" > "$session/console.log"
    echo "double free or corruption (!prev)"
    exit 134
    ;;
  no_logs)
    exit 0
    ;;
  nonzero_after_script_log)
    mkdir -p "$session"
    echo "Script validation succeeded" > "$session/script.log"
    exit 134
    ;;
  succeeds_first)
    ;;
  *)
    echo "unexpected scenario: $MOCK_SCENARIO" >&2
    exit 90
    ;;
esac
mkdir -p "$session"
echo "INFO: script validation completed" > "$session/script.log"
echo "INFO: all okay" > "$session/console.log"
exit 0
MOCK
chmod +x "$MOCK_PROTON"

export GITHUB_WORKSPACE="$TEMP/checkout"
export STEAM_ROOT="$TEMP/steam"
export REFORGER_PREFIX="$TEMP/prefix"
export REFORGER_PROTON="$MOCK_PROTON"
export REFORGER_WORKBENCH="$TEMP/tools/ArmaReforgerWorkbenchSteamDiag.exe"
export REFORGER_GAME_ADDONS="$TEMP/steam/steamapps/common/Arma Reforger/addons"
export REFORGER_WORKBENCH_MAX_ATTEMPTS=3
export REFORGER_WORKBENCH_RETRY_DELAY_SEC=0
export MOCK_COUNTER="$TEMP/counter"

test_case() {
  local scenario="$1" expected_code="$2" expected_calls="$3"
  local result=0
  export MOCK_SCENARIO="$scenario"
  export REFORGER_LOG_OUTPUT_DIR="$TEMP/results-$scenario"
  rm -rf -- "$TEMP/prefix/pfx/drive_c" "$REFORGER_LOG_OUTPUT_DIR"
  rm -f -- "$MOCK_COUNTER"

  if bash "$VALIDATOR" > "$TEMP/$scenario.stdout" 2>&1; then
    result=0
  else
    result=$?
  fi

  local calls=0
  if [[ -f "$MOCK_COUNTER" ]]; then calls="$(cat "$MOCK_COUNTER")"; fi

  if (( result != expected_code || calls != expected_calls )); then
    printf 'FAIL %s: result=%s (expected %s), attempts=%s (expected %s)\n' \
      "$scenario" "$result" "$expected_code" "$calls" "$expected_calls" >&2
    cat "$TEMP/$scenario.stdout" >&2
    exit 1
  fi
  printf '[retry-test] PASS %-35s attempts=%s result=%s\n' "$scenario" "$calls" "$result"
}

test_case succeeds_first 0 1
test_case crash_then_success 0 2
test_case zero_exit_crash 0 2
test_case always_crash 134 3
test_case compiler_error 1 1
test_case crash_with_compiler_diagnostics 134 1
test_case no_logs 1 1
test_case nonzero_after_script_log 134 1

echo '[retry-test] All Workbench startup retry mock cases passed.'
