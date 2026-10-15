#!/usr/bin/env bash
# EXPERIMENTAL: run vanilla Reforger's built-in -autotest against a mod checkout.
# Fail closed if the game does not produce a fresh JUnit XML result.
set -Eeuo pipefail

WORKSPACE="$(realpath "${GITHUB_WORKSPACE:-"$(dirname "$0")/.."}")"
STEAM="${STEAM_ROOT:-"$HOME/.local/share/Steam"}"
APPS="$STEAM/steamapps"
GAME="$APPS/common/Arma Reforger/ArmaReforgerSteamDiag.exe"
PROTON="${REFORGER_PROTON:-"$APPS/common/Proton - Experimental/proton"}"
PREFIX="${REFORGER_PREFIX:-"$APPS/compatdata/1874910"}"
LOG_ROOT="$PREFIX/pfx/drive_c/users/steamuser/Documents/My Games/ArmaReforger/logs"
OUTPUT="${RUNNER_TEMP:-"$WORKSPACE/.ci-logs"}/reforger-autotests"
SUITE="${REFORGER_TEST_SUITE:-LRP_TEST_AppleHarvestSuite}"

if [[ ! -f "$GAME" || ! -f "$PROTON" || ! -f "$WORKSPACE/addon.gproj" ]]; then
  echo "[autotest-ci] Missing game, Proton, or addon.gproj" >&2
  exit 2
fi

mkdir -p "$OUTPUT"
STARTED="$OUTPUT/run-started.marker"
touch "$STARTED"

to_win() { printf 'Z:%s' "$(realpath "$1")"; }
export STEAM_COMPAT_DATA_PATH="$PREFIX"
export STEAM_COMPAT_CLIENT_INSTALL_PATH="$STEAM"
export STEAM_COMPAT_APP_ID=1874880
export SteamAppId=1874880
export SteamGameId=1874880

printf '[autotest-ci] Running %s in ReforgerSteamDiag under Proton\n' "$SUITE"
set +e
(
  cd "$(dirname "$GAME")" || exit 2
  timeout --signal=TERM --kill-after=5s 150s "$PROTON" waitforexitandrun "$GAME" \
    -addonsDir "$(to_win "$(dirname "$WORKSPACE")")" \
    -addons RPTEST \
    -gproj "$(to_win "$WORKSPACE/addon.gproj")" \
    -autotest "$SUITE" \
    -window -screenWidth 1000 -screenHeight 700 -noFocus -forceUpdate
)
RESULT=$?
set -e

echo "[autotest-ci] Game exit code: $RESULT"
MY_GAMES="$PREFIX/pfx/drive_c/users/steamuser/Documents/My Games"
if [[ -d "$MY_GAMES" ]]; then
  echo "[autotest-ci] Fresh game/Workbench diagnostic files:"
  while IFS= read -r -d '' file; do
    relative="${file#"$MY_GAMES"/}"
    echo "[autotest-ci]   $relative"
    mkdir -p "$OUTPUT/$(dirname "$relative")"
    cp -- "$file" "$OUTPUT/$relative" || true
  done < <(find "$MY_GAMES" -type f \( -name '*.log' -o -name 'junit.xml' \) -newer "$STARTED" -print0)
fi
echo "[autotest-ci] Relevant game diagnostics:"
grep -RinE 'autotest|junit|script.{0,20}error|invalid.{0,40}test|failed to compile|addon not|RPTEST|LRP_TEST_' "$OUTPUT" --include='*.log' | tail -n 55 || true

# Verify actual test execution, never trust a zero exit status by itself.
REPORT="$(find "$OUTPUT" -type f -name 'junit.xml' -print -quit)"
if [[ -z "$REPORT" ]]; then
  echo "[autotest-ci] ERROR: No fresh JUnit XML report. Tests were not verified." >&2
  exit 1
fi
if (( RESULT != 0 )); then
  echo "[autotest-ci] ERROR: Reforger did not exit normally." >&2
  exit 1
fi

python3 - "$REPORT" <<'PY'
import sys
import xml.etree.ElementTree as ET

root = ET.parse(sys.argv[1]).getroot()
cases = root.findall(".//testcase")
if root.tag == "testcase":
    cases = [root]
if not cases:
    sys.exit("[autotest-ci] ERROR: Report has zero testcases")
errors = [c for c in cases if c.find("failure") is not None or c.find("error") is not None]
skipped = [c for c in cases if c.find("skipped") is not None]
names = {c.get("name", "") for c in cases}
expected = {
    "LRP_TEST_AppleHarvest_StartsFull",
    "LRP_TEST_AppleHarvest_PickReducesStock",
    "LRP_TEST_AppleHarvest_EmptyRejectsHarvest",
    "LRP_TEST_AppleHarvest_RegrowsAfterDelay",
    "LRP_TEST_AppleHarvest_DisabledRegrowth",
    "LRP_TEST_AppleHarvest_TreePrefabRegistersContext",
    "LRP_TEST_AppleHarvest_TreeActionVisibleToCharacter",
    "LRP_TEST_AppleHarvest_InventoryApplePrefab",
    "LRP_TEST_AppleHarvest_AppleIsInsertedIntoCharacterInventory",
    "LRP_TEST_AppleHarvest_AppleIsEdible",
    "LRP_TEST_AppleHarvest_CanEatAtFullHealth",
    "LRP_TEST_AppleHarvest_EatUsingInventoryUse",
}
missing = [name for name in sorted(expected) if not any(name in actual for actual in names)]
print(f"[autotest-ci] JUnit: {len(cases)} testcases, {len(errors)} failures/errors, {len(skipped)} skipped")
if errors or skipped or missing:
    print(f"[autotest-ci] Missing expected tests: {missing}", file=sys.stderr)
    sys.exit(1)
print("[autotest-ci] All expected apple harvest and prefab interaction tests passed")
PY
