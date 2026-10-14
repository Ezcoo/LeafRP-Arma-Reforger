# Linux CI setup for LeafRP (Arma Reforger)

This setup validates committed Enforce Script files using the **actual** Enfusion Workbench executable via Proton. It has not yet been tested end-to-end in GitHub Actions. The underlying CLI validation was confirmed working locally on Ubuntu with Proton Experimental.

## Project layout

Commit your mod folder contents to the repository root, including `addon.gproj` and `Scripts/Game/*.c`. Adjust `REFORGER_GPROJ_REL` in the workflow if the .gproj lives in a subfolder. Workbench must validate the Git checkout, not an unrelated local addon copy.

## Runner setup

1. In GitHub, visit **Settings → Actions → Runners → New self-hosted runner** and select Linux x64. Follow GitHub's generated setup commands on your Ubuntu development machine, using a dedicated unprivileged account where practical.
2. Configure the runner with a custom label `reforger-workbench` (in addition to standard `self-hosted`, `linux`, `x64` labels).
3. Keep the runner in your active GUI desktop session initially. Workbench under Proton may need a display session and access to Steam libraries. Don't enable it as a system service until you've verified headless execution.
4. Install and launch Arma Reforger Tools and the base game at least once through Steam using Proton Experimental.
5. Ensure the runner account can use the same Proton prefix, game files, Steam library and X11/Wayland graphical session. Never run the job as root.

Default paths:
- Steam: `$HOME/.local/share/Steam`
- Proton: `steamapps/common/Proton - Experimental/proton`
- Workbench: `steamapps/common/Arma Reforger Tools/Workbench/ArmaReforgerWorkbenchSteamDiag.exe`
- Base addons: `steamapps/common/Arma Reforger/addons`
- Tools prefix: `steamapps/compatdata/1874910`

If these differ, export `STEAM_ROOT`, `REFORGER_PROTON`, `REFORGER_WORKBENCH`, `REFORGER_GAME_ADDONS`, or `REFORGER_PREFIX` in the runner account environment.

## Run manually first

From a clone of this repository:

```bash
REFORGER_GPROJ_REL=addon.gproj bash scripts/validate-reforger-linux.sh
echo "Exit: $?"
```

The validator converts absolute Linux paths into Proton's `Z:` mapping, passes `-addonsDir`, `-gproj`, `-wbModule=ScriptEditor -run -validate`, and attempts to collect the current Workbench session's logs.

Confirm both cases:
- Valid source returns exit code **0**.
- A deliberately broken disposable script produces a **nonzero** code and its diagnostics appear in the captured logs. Revert the script afterward.

If the script reports no mod project, it means that `addon.gproj` has not been committed yet. That is expected with an empty initial repository.

## GitHub Actions

After committing mod files and runner registration, use the **Actions** tab → **Validate Reforger scripts (Linux)** → **Run workflow**. Trusted pushes to main also trigger checks. It intentionally does **not** execute code from pull requests on your personal machine.

The workflow uploads a `workbench-logs` artifact, even on compilation failure. Check the runner job logs and the artifact if a run fails. The Workbench log directory is typically in `compatdata/1874910/pfx/drive_c/users/steamuser/Documents/My Games/ArmaReforgerWorkbench/logs`.

## Security and operational notes

A self-hosted runner executes repository-controlled commands on your computer, including any workflows merged into main. Keep the repository private, require review before merging third-party code, avoid untrusted PR workflows, and do not place privileged credentials in the runner environment.

The runner must be online to execute builds. Steam/Proton is not guaranteed to work unattended outside a desktop session.

Useful documentation:
- https://docs.github.com/en/actions/hosting-your-own-runners/managing-self-hosted-runners/adding-self-hosted-runners
- https://community.bohemia.net/wiki/Arma_Reforger:Startup_Parameters
