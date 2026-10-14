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

mkdir -p "$LOG_OUTPUT"
START_MARKER="$LOG_OUTPUT/validation-started.marker"
touch "$START_MARKER"

info "Validating Git checkout: $GPROJ"
info "Base game addons: $GAME_ADDONS"
info "Compiler diagnostics destination: $LOG_OUTPUT"

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
)
RESULT=$?
set -e

# Proton may return 0 even when Enfusion reports compilation errors.
# Inspect the current session's script.log rather than trusting $? alone.
SCRIPT_ERRORS=0
LATEST_LOG=""
if [[ -d "$LOG_ROOT" ]]; then
  while IFS= read -r -d '' file; do
    if [[ -z "$LATEST_LOG" || "$file" -nt "$LATEST_LOG" ]]; then
      LATEST_LOG="$file"
    fi
  done < <(find "$LOG_ROOT" -type f -name '*.log' -newer "$START_MARKER" -print0 2>/dev/null)
fi

if [[ -n "$LATEST_LOG" ]]; then
  LATEST_DIR="$(dirname "$LATEST_LOG")"
  info "Workbench log session: $LATEST_DIR"
  cp -a -- "$LATEST_DIR/." "$LOG_OUTPUT/"

  SCRIPT_LOG="$LATEST_DIR/script.log"
  if [[ ! -f "$SCRIPT_LOG" ]]; then
    info "ERROR: The latest Workbench session has no script.log; cannot verify compilation."
    SCRIPT_ERRORS=1
  elif grep -Eiq "SCRIPT[[:space:]]*\\(E\\)|[Ss]cript validation failed|[Cc]an.t compile .*script module|\\.c\\([0-9]+\\):[[:space:]]*error:" "$SCRIPT_LOG"; then
    info "ERROR: Workbench reported Enforce Script compilation errors."
    SCRIPT_ERRORS=1
  else
    info "No compiler errors found in the current Workbench script.log."
  fi

  info 'Potential compiler diagnostics:'
  mapfile -d '' -t LOG_FILES < <(find "$LOG_OUTPUT" -type f -name '*.log' -print0)
  if (( ${#LOG_FILES[@]} > 0 )); then
    grep -nEi '\(E\)|SCRIPT.*[Ee]rror|error|failed|failure' "${LOG_FILES[@]}" | tail -n 100 || true
  fi
else
  info "ERROR: No new Workbench .log files found in $LOG_ROOT; cannot verify compilation."
  SCRIPT_ERRORS=1
fi

if (( RESULT == 0 && SCRIPT_ERRORS != 0 )); then
  info "Process returned 0, but validation has failed according to Workbench logs."
  RESULT=1
fi

info "Effective validation exit code: $RESULT"
if [[ -n "${GITHUB_STEP_SUMMARY:-}" ]]; then
  if (( RESULT == 0 )); then
    printf '### Arma Reforger validation: passed\n\nProject: %s\n' "$GPROJ_REL" >> "$GITHUB_STEP_SUMMARY"
  else
    printf '### Arma Reforger validation: failed (exit %s)\n\nInspect the Workbench logs artifact.\n' "$RESULT" >> "$GITHUB_STEP_SUMMARY"
  fi
fi
exit "$RESULT"
