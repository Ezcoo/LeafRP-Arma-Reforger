# LeafRP Development Instructions

## Environment
- Ubuntu 26.04 with Arma Reforger Workbench running through Steam Proton.
- Enfusion MCP is connected to the running Workbench NET API.
- The active mod project is RP_TEST.
- Use Enfusion MCP for API research, Workbench inspection, script reloading, and log analysis.

## Development workflow
1. Inspect relevant Enforce Script files and APIs before modifying code.
2. Make minimal, focused changes; preserve unrelated edits.
3. Never launch additional Workbench instances.
4. Before reloading, verify Workbench is in Edit mode and the user has saved relevant work.
5. For script reloads, use wb_reload with target=scripts and allowValidationFallback=false.
6. Never automatically retry a reload or use validation as a substitute for compilation.
7. Record the native reload result, then inspect new Workbench logs for errors.
8. Distinguish successful API responses from independently verified gameplay behavior.
9. Do not automatically start or stop Play mode.
10. Do not commit or publish the EnfusionMCP development handler scripts.

## Quality and safety
- Use Git branches for significant changes.
- Keep source edits small and review diffs.
- Never overwrite unrelated modifications.
- Report compilation errors with file paths and line numbers.
- Ask before destructive editor operations, bulk changes, or publishing.
- Never claim successful gameplay testing unless it was actually performed.
