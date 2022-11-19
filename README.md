# Invisible Inc. Mod Manager

Utility for downloading mods from Steam and configuring which mods are actively installed.
Some mods cause problems when disabled within the game, so this manager allows you to keep inactive mods outside of the
game folder.

## Building from source

Requires Qt5 or Qt6. Update the Qt version in the build command to match.
Requires ZLIB, cmake(>=3.15).

```
cmake -S iimodmanager -B out -D IIMODMAN_QT_MAJOR_VERSION=6
cmake --build out

out/iimodman-cli/iimodman-cli --help
out/iimodman-gui/iimodman-gui
```

(TODO: add install configuration to cmake files, instead of needing to refer to the binary in the output directory)

## Usage

[Usage](https://github.com/qoala/InvisibleInc-ModManager/wiki/Usage) on the wiki.
