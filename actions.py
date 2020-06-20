"""Behavior of operations by the mod manager."""

import sys

from config import Config
from directory_controller import DirectoryController
import modinfo_parser
import modspec_converter


class ListDownloadedAction:
    """Lists all downloaded mods."""
    _config: Config

    def __init__(self, config: Config) -> None:
        self._config = config

    def run(self):
        mods = modinfo_parser.all_from_dir(self._config.download_path)
        modspec_converter.write_modspec(mods, sys.stdout)


class InstallAllAction:
    """Install all downloaded mods."""
    _config: Config
    _moddir: DirectoryController

    def __init__(self, config: Config):
        self._config = config
        self._moddir = DirectoryController(config)

    def run(self) -> None:
        mods = modinfo_parser.all_from_dir(self._config.download_path)
        self._moddir.install_mods(mods)


if __name__ == '__main__':
    import sys

    config = Config(install_path=sys.argv[1], download_path=sys.argv[2])
    # ListDownloadedAction(config).run()
    InstallAllAction(config).run()
