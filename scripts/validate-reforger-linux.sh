#!/usr/bin/env bash
# Validate a Git checkout with Proton + Enfusion Workbench on Linux.
set -Eeuo pipefail

info() { printf '[reforger-ci] %s\n' "$*"; }
fail() { printf '[reforger-ci] ERROR: %s\n' "$*" >&2; exit 2; }

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
WORKSPACE="$(realpath "${GITHUB_WORKSPACE:-"$SCRIPT_DIR/.."}")"
STEAM_ROOT="$(realpath -m "${STEAM_ROOT:-"$HOME/.local/share/Steam"}")"
STEAMAPPS="$STEAM_ROOT/steamapps"
PROTON="${REFORGER_PROTON:-"$STEAMAPPS/common/Proton - Experimental/proton"}"
WORKBENCH="${REFORGER_WORKBENCH:-"$STEAMAPPS/common/Arma Reforger Tools/Workbench/ArmaReforgerWorkbenchSteamDiag.exe"}"
GAME_ADDONS="${REFORGER_GAME_ADDONS:-"$STEAMAPPS/common/Arma Reforger/addons"}"
GPROJ_REL="${REFORGER_GPROJ_REL:-addon.gproj}"
GPROJ="$WORKSPACE/$GPROJ_REL"
PREFIX="${REFORGER_PREFIX:-"$STEAMAPPS/compatdata/1874910"}"
LOG_ROOT="$PREFIX/pfx/drive_c/users/steamuser/Documents/My Games/ArmaReforgerWorkbench/logs"
LOG_OUTPUT="${REFORGER_LOG_OUTPUT_DIR:-"${RUNNER_TEMP:-"$WORKSPACE/.ci-logs"}/reforger-workbench-logs"}"

[[ -f "$PROTON" ]] || fail "Proton not found: $PROTON"
[[ -f "$WORKBENCH" ]] || fail "Workbench executable not found: $WORKBENCH"
[[ -f "$GAME_ADDONS/data/ArmaReforger.gproj" ]] || fail "Base game project not found below: $GAME_ADDONS"
[[ -f "$GPROJ" ]] || fail "Mod project not found: $GPROJ. Commit addon.gproj and its source files."
[[ -d "$PREFIX/pfx" ]] || fail "Proton prefix not found: $PREFIX. Start Tools through Steam once."
[[ -e "$PREFIX/pfx/dosdevices/z:" ]] || fail "Proton Z: mapping not found in $PREFIX/pfx/dosdevices"

to_proton_z_path() {
  local path
  path="$(realpath "$1")"
  printf 'Z:%s' "$path"
}

# Make the base-game addons discoverable by Workbench.
GPROJ_WIN="$(to_proton_z_path "$GPROJ")"
GAME_ADDONS_WIN="$(to_proton_z_path "$GAME_ADDONS")"

export STEAM_COMPAT_DATA_PATH="$PREFIX"
export STEAM_COMPAT_CLIENT_INSTALL_PATH="$STEAM_ROOT"
export STEAM_COMPAT_APP_ID=1874910
export SteamAppId=1874910
export SteamGameId=1874910

# Allow two retries (three launches total) for known Proton startup crashes.
# No retry for compiler errors, missing logs without a crash signature, or
# failures after Workbench has produced a script.log.
MAX_ATTEMPTS="${REFORGER_WORKBENCH_MAX_ATTEMPTS:-3}"
RETRY_DELAY="${REFORGER_WORKBENCH_RETRY_DELAY_SEC:-2}"
[[ "$MAX_ATTEMPTS" =~ ^[123]$ ]] || fail "REFORGER_WORKBENCH_MAX_ATTEMPTS must be 1, 2 or 3."
[[ "$RETRY_DELAY" =~ ^([0-9]|1[0-9]|2[0-9]|30)$ ]] || fail "REFORGER_WORKBENCH_RETRY_DELAY_SEC must be 0-30."

mkdir -p "$LOG_OUTPUT"
info "Validating Git checkout: $GPROJ"
info "Base game addons: $GAME_ADDONS"
info "Compiler diagnostics destination: $LOG_OUTPUT"
info "Workbench startup attempts allowed: $MAX_ATTEMPTS"

RESULT=1
for (( ATTEMPT=1; ATTEMPT<=MAX_ATTEMPTS; ATTEMPT++ )); do
  ATTEMPT_DIR="$LOG_OUTPUT/attempt-$ATTEMPT"
  mkdir -p "$ATTEMPT_DIR"
  START_MARKER="$ATTEMPT_DIR/started.marker"
  touch "$START_MARKER"

  info "Starting Workbench attempt $ATTEMPT/$MAX_ATTEMPTS"

  # Tee captures allocator crashes even if Workbench fails before script.log
  # exists. PIPESTATUS preserves Proton's code, rather than tee's exit code.
  set +e
  (
    cd -- "$(dirname "$WORKBENCH")" || exit 2
    "$PROTON" waitforexitandrun "$WORKBENCH" \
      -addonsDir "$GAME_ADDONS_WIN" \
      -gproj "$GPROJ_WIN" \
      -wbModule=ScriptEditor \
      -run \
      -validate \
      -exitAfterInit
  ) 2>&1 | tee "$ATTEMPT_DIR/proton-output.log"
  PROCESS_RESULT="${PIPESTATUS[0]}"
  set -e

  # Only inspect logs generated after THIS attempt began. Retaining one
  # artifact directory per launch prevents stale successes from passing CI.
  LATEST_LOG=""
  if [[ -d "$LOG_ROOT" ]]; then
    while IFS= read -r -d '' file; do
      if [[ -z "$LATEST_LOG" || "$file" -nt "$LATEST_LOG" ]]; then
        LATEST_LOG="$file"
      fi
    done < <(find "$LOG_ROOT" -type f -name '*.log' -newer "$START_MARKER" -print0 2>/dev/null)
  fi

  SCRIPT_LOG_PRESENT=0
  SCRIPT_ERRORS=1
  if [[ -n "$LATEST_LOG" ]]; then
    LATEST_DIR="$(dirname "$LATEST_LOG")"
    info "Workbench log session (attempt $ATTEMPT): $LATEST_DIR"
    cp -a -- "$LATEST_DIR/." "$ATTEMPT_DIR/"

    SCRIPT_LOG="$LATEST_DIR/script.log"
    if [[ -f "$SCRIPT_LOG" && "$SCRIPT_LOG" -nt "$START_MARKER" ]]; then
      SCRIPT_LOG_PRESENT=1
      if [[ ! -s "$SCRIPT_LOG" ]]; then
        info "ERROR: Workbench script.log is empty; cannot validate scripts."
      elif grep -Eiq 'SCRIPT[[:space:]]*\(E\)|[Ss]cript validation failed|[Cc]an.t compile .*script module|\.c\([0-9]+\):[[:space:]]*error:' "$SCRIPT_LOG"; then
        info "ERROR: Workbench reported Enforce Script compilation errors."
      else
        SCRIPT_ERRORS=0
        info "No compiler errors found in the current Workbench script.log."
      fi
    else
      info "ERROR: The latest Workbench session has no fresh script.log."
    fi
  else
    info "ERROR: No new Workbench .log files found in $LOG_ROOT."
  fi

  info "Potential compiler diagnostics (attempt $ATTEMPT):"
  mapfile -d '' -t LOG_FILES < <(find "$ATTEMPT_DIR" -type f -name '*.log' -print0)
  if (( ${#LOG_FILES[@]} > 0 )); then
    grep -nEi '\(E\)|SCRIPT.*[Ee]rror|error|failed|failure' "${LOG_FILES[@]}" | tail -n 100 || true
  fi

  if (( PROCESS_RESULT == 0 && SCRIPT_ERRORS == 0 )); then
    RESULT=0
    info "Workbench validation succeeded on attempt $ATTEMPT/$MAX_ATTEMPTS."
    break
  fi

  RESULT=$PROCESS_RESULT
  if (( RESULT == 0 )); then
    RESULT=1
    info "Process returned 0, but Workbench logs did not verify compilation."
  fi

  # Fail closed. Only retry when startup crashes with a recognizable allocator
  # signature AND no script.log exists. Never mask real Enforce compile errors.
  if (( ATTEMPT >= MAX_ATTEMPTS || SCRIPT_LOG_PRESENT != 0 )); then
    break
  fi
  if grep -REiq --include='*.log' 'SCRIPT[[:space:]]*\(E\).*error:|[Ss]cript validation failed|[Cc]an.t compile .*script module|\.c\([0-9]+\):[[:space:]]*error:' "$ATTEMPT_DIR"; then
    info "Compiler errors found in diagnostic output; will not retry."
    break
  fi
  if ! grep -Eiq 'double free or corruption|free\(\): invalid pointer|malloc\(\): corrupted|corrupted size vs\. prev_size' "$ATTEMPT_DIR/proton-output.log"; then
    info "No recognized Proton startup crash signature; will not retry."
    break
  fi

  info "Recognized Workbench/Proton startup allocator crash before compilation; retrying ($ATTEMPT/$MAX_ATTEMPTS)."
  sleep "$RETRY_DELAY"
done

info "Effective validation exit code: $RESULT"
if [[ -n "${GITHUB_STEP_SUMMARY:-}" ]]; then
  if (( RESULT == 0 )); then
    printf '### Arma Reforger validation: passed\n\nProject: %s\n' "$GPROJ_REL" >> "$GITHUB_STEP_SUMMARY"
  else
    printf '### Arma Reforger validation: failed (exit %s)\n\nInspect the Workbench logs artifact.\n' "$RESULT" >> "$GITHUB_STEP_SUMMARY"
  fi
fi
exit "$RESULT"
