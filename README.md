# LeafRP-Arma-Reforger

Vanilla-only Arma Reforger roleplay mod, developed with Enfusion Workbench on Linux using Proton.

## Development and testing

- [Linux Workbench / GitHub Actions compilation](docs/LINUX-CI.md)
- [Apple picking and automated tests](docs/APPLE-AUTOTESTS.md)
- [Physical Apple inventory item prototype](docs/APPLE-INVENTORY-ITEM.md)
- [Apple tree interaction setup](docs/APPLE-PICKING-PROTOTYPE.md)

The GitHub Actions pipeline runs Workbench script validation, then Reforger's built-in Autotest framework in the diagnostic game and checks its JUnit report. No third-party mod dependencies.
