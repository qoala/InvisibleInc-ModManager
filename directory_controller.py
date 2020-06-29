"""Tools for manipulating the mods/ directory of an II install."""

from typing import List, Optional

import os
import shutil
import sys

from config import Config
from modinfo import ModInfo


class DirectoryController:
    """Controller of the II mods/ directory."""
    _config: Config

    def __init__(self, config: Config):
        self._config = config

    def install_mod(self, mod: ModInfo) -> None:
        """Installs the specified mod from download to the II mods/ path."""
        mod_download_path = os.path.join(self._config.download_path, mod.mod_id)
        mod_install_path = os.path.join(self._config.install_path, mod.mod_id)

        if os.path.isdir(mod_install_path) and not os.path.islink(mod_install_path):
            shutil.rmtree(mod_install_path)
        elif os.path.lexists(mod_install_path):
            raise ValueError('Existing mod is not currently a directory: {0}'.format(mod_install_path))

        # Install via copy
        shutil.copytree(mod_download_path, mod_install_path)

    def install_mods(self, mods: List[ModInfo]) -> None:
        """Installs the specified mods from download to the II mods/ path."""
        for mod in mods:
            try:
                self.install_mod(mod)
            except (OSError, ValueError, shutil.Error) as e:
                print('Error installing {0}({1}): {2}'.format(
                    mod.name, mod.mod_id, e),
                        sys.stderr)

    def uninstall_mod(self, mod: ModInfo) -> None:
        """Uninstalls the specified mod from the II mods/ path."""
        mod_install_path = os.path.join(self._config.install_path, mod.mod_id)

        if os.path.isdir(mod_install_path) and not os.path.islink(mod_install_path):
            shutil.rmtree(mod_install_path)
        elif os.path.lexists(mod_install_path):
            raise ValueError('Existing mod is not currently a directory: {0}'.format(mod_install_path))

    def uninstall_mods(self, mods: List[ModInfo]) -> None:
        """Uninstalls the specified mods from the II mods/ path."""
        for mod in mods:
            try:
                self.uninstall_mod(mod)
            except (OSError, ValueError, shutil.Error) as e:
                print('Error uninstalling {0}({1}): {2}'.format(
                    mod.name, mod.mod_id, e),
                        sys.stderr)
