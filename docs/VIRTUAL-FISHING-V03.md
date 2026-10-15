# LeafRP Virtual Fishing v0.3

Builds on v0.2, without any new assets, models, inventory changes or timers.

## New gameplay

- The original **Fish** action still takes 15 seconds and has a 70% successful bite chance.
- For a successful bite, 3% yield an **old boot** (junk, not a fish; does not increase catch count or total weight).
- Another 2% of successful bites yield a **golden perch** (1.0–3.0 kg), which counts as a fish and can set a new largest-fish record.
- The other 95% of successful bites yield ordinary perch/herring/pike in the previous 50%/35%/15% conditional proportions.
- A separate **View Fishing Stats** action shows fish caught, total fish weight, largest fish, boot count, and golden-perch count at any time.

Both actions share static state in the same local client script context. Stats are *not* persistent, server authoritative, or guaranteed to reset on respawn; there is no user-specific multiplayer record storage.

## Workbench setup

1. Save or commit local changes, fetch and check out `feature/virtual-fishing-v03` (includes v0.2).
2. Load the actual LeafRP project (not RP_TEST); compile the Game scripts in Workbench Edit mode.
3. Keep the existing fishing spot's `ActionsManagerComponent`, `LRP_FishingSpot` context, and 15-second `LRP_VirtualFishingUserAction`.
4. In **Additional Actions**, add `LRP_ViewFishingStatsUserAction` as a *second* action.
5. Give it the same `LRP_FishingSpot` Parent Context and its own `UIInfo` name `View Fishing Stats`. Set its duration to **0 seconds** for an instant stats check; keep the Fish action duration unchanged.
6. Save the scenario, enter Play mode manually, and verify the two interactions appear at the same spot.

## Acceptance checklist

- [ ] Both scripts compile (watch for line-specific compiler diagnostics).
- [ ] `Fish` still takes 15 seconds; early cancellation gives nothing.
- [ ] `View Fishing Stats` appears alongside `Fish` and immediately displays zeros before any catches.
- [ ] Stats reflect subsequent successful fish catches; boot finds increment only boots.
- [ ] Ordinary perch, herring and pike remain possible.
- [ ] Golden perch increases the fish count, weight and rare counter.
- [ ] Largest-fish record never decreases.
- [ ] Apple harvest and inventory Use still work.
- [ ] No claim of persistent or server-authoritative statistics is made.

Rare outcomes are intentionally infrequent. Lack of a golden perch over a handful of attempts is not evidence of failure.

This development branch is based on `feature/virtual-fishing-v02`; the PR targets that branch to avoid mixing the previous v0.1/v0.2 changes.
