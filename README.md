# Invisible Inc. Mod Manager

Utility for downloading mods from Steam and configuring which mods are actively installed.
Some mods cause problems when disabled within the game, so this manager allows you to keep inactive mods outside of the
game folder.

Currently only supports a command line interface.

## Building from source

Requires qt5-dev/qt5base-dev, zlib1g-dev, cmake(>=3.15).

```
cmake -S iimodmanager -B out
cmake --build out

out/iimodman-cli/iimodman-cli --help
```

(TODO: add install configuration to cmake files, instead of needing to refer to the binary in the output directory)

## Usage

[Usage](https://github.com/qoala/InvisibleInc-ModManager/wiki/Usage) on the wiki.
