"""Behavior of operations by the mod manager."""

import io
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


class ListInstalledAction:
    """Lists all installed mods."""
    _config: Config

    def __init__(self, config: Config) -> None:
        self._config = config

    def run(self):
        mods = modinfo_parser.all_from_dir(self._config.install_path)
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


class InstallFromSpecAction:
    """Install all downloaded mods."""
    _config: Config
    _moddir: DirectoryController

    def __init__(self, config: Config):
        self._config = config
        self._moddir = DirectoryController(config)

    def run(self, reader: io.TextIOBase) -> None:
        downloaded_mods = modinfo_parser.all_from_dir(
                self._config.download_path)
        current_mods = set(modinfo_parser.all_from_dir(
                self._config.install_path))
        requested_mods = modspec_converter.read_modspec(
                reader, downloaded_mods)
        self._moddir.uninstall_mods(current_mods.difference(requested_mods))
        self._moddir.install_mods(requested_mods)


if __name__ == '__main__':
    import config_parser

    config = config_parser.read_config('iimodman.conf')
    # ListDownloadedAction(config).run()
    # ListInstalledAction(config).run()
    # InstallAllAction(config).run()
    # InstallFromSpecAction(config).run(sys.stdin)
