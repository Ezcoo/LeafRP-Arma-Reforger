# Workbench startup retries in Linux CI

The self-hosted Linux GitHub Actions runner validates Enfusion scripts with **Arma Reforger Tools Workbench via Proton Experimental**. On some launches Proton exits before Workbench writes `script.log`, reporting errors such as:

```
double free or corruption (!prev)
```

This is a Workbench/Proton startup failure, not evidence of a mod compilation error. The CI validator now retries these failures **up to two times** (three launches total).

## Strict retry conditions

`scripts/validate-reforger-linux.sh` retries **only if all conditions hold**:

1. The latest attempt did **not** produce a fresh `script.log`.
2. Proton stdout/stderr contains an allocator-crash signature (such as `double free or corruption`, `free(): invalid pointer`, or a recognized corrupted `malloc()` message).
3. None of the fresh diagnostic logs contain an Enforce Script compilation error.
4. There is another retry available.

All other failures, including actual compiler errors, unknown startup failures, or a nonzero exit code **after a compilation log exists**, fail immediately. A zero process exit alone never counts as passing; CI still requires a fresh, nonempty `script.log` with no script errors.

## Settings

The default maximum is **3 attempts** (initial launch + two retries) with a **2-second** interval. Both are optional environment variables:

- `REFORGER_WORKBENCH_MAX_ATTEMPTS`: `1`, `2`, or `3` (default `3`)
- `REFORGER_WORKBENCH_RETRY_DELAY_SEC`: `0` to `30` seconds (default `2`)

The GitHub Actions job still has a **30-minute** maximum; repeated long-running crashes can consume most of it. Only the Workbench compilation step is retried. The Reforger game-side test suite runs once, after successful validation.

## Artifacts

Under the GitHub Actions `workbench-logs` artifact, each startup attempt has its own directory:

```
attempt-1/proton-output.log
attempt-1/console.log
attempt-1/script.log (only if produced)
attempt-2/proton-output.log
attempt-2/script.log
...
```

Each attempt uses a new marker timestamp, so stale log files from previous attempts cannot satisfy the current attempt's compilation check.

## Regression checks

The `Test Workbench startup crash retry decisions` CI step executes `scripts/test-reforger-validation-retry.sh`. It invokes the actual validation script with a mocked Proton process to cover:

- Success on the first launch
- Crash then successful retry
- Crash with a deceptive zero process exit
- Repeated crashes until retry budget exhausted
- Genuine Enforce compiler errors (no retry)
- Compiler errors accompanying a crash (no retry)
- Missing log files without a recognized crash (no retry)
- Nonzero exit after a valid compilation log (no retry)

This mock does not change Steam installations, game files, or the real Proton prefix.
