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


    def _validate_existing(self, mod_install_path: str) -> Optional[str]:
        """Raises an error if the intended install path shouldn't be modified.

        Prevents this program from destroying or modifying files in the II mods/
        directory that it did not originally put there.
        * If the intended path is a real file or directory, not a symlink.
        * If the intended path is a symlink to a target outside the managed mods
            download path.

        Returns:
            The existing link target as an absolute path, if any.
        """
        if (os.path.lexists(mod_install_path) and
                not os.path.islink(mod_install_path)):
            raise ValueError('Existing real dir/file (try "import" '
                    'first?): {0}'.format(mod_install_path))
        if os.path.islink(mod_install_path):
            existing_target = os.readlink(mod_install_path)
            if not os.path.isabs(existing_target):
                existing_target = os.path.join(
                        os.path.dirname(os.path.abspath(mod_install_path)),
                        existing_target)
            existing_target_parent = os.path.dirname(existing_target)

            if existing_target_parent != os.path.abspath(
                    self._config.download_path):
                raise ValueError(
                        'Existing link to non-managed folder: {0}'.format(
                            mod_install_path))
            return existing_target
        return None


    def link_mod(self, mod: ModInfo) -> None:
        """Installs the specified mod from download to the II mods/ path."""
        mod_download_path = os.path.join(self._config.download_path, mod.mod_id)
        mod_install_path = os.path.join(self._config.install_path, mod.mod_id)
        abs_mod_download_path = os.path.abspath(mod_download_path)

        existing_target = self._validate_existing(mod_install_path)

        if existing_target == abs_mod_download_path:
            # Already installed.
            return
        elif existing_target:
            # Clear previous link
            os.remove(mod_install_path)

        # Install via symlink
        os.symlink(
                abs_mod_download_path,
                mod_install_path,
                target_is_directory=True)

    def link_mods(self, mods: List[ModInfo]) -> None:
        """Installs the specified mods from download to the II mods/ path."""
        for mod in mods:
            try:
                self.install_mod(mod)
            except (OSError, ValueError) as e:
                print('Error installing {0}({1}): {2}'.format(
                    mod.name, mod.mod_id, e),
                        sys.stderr)


    def unlink_mod(self, mod: ModInfo) -> None:
        """Uninstalls the specified mod from the II mods/ path."""
        mod_install_path = os.path.join(self._config.install_path, mod.mod_id)

        existing_target = self._validate_existing(mod_install_path)

        if existing_target:
            os.remove(mod_install_path)

    def unlink_mods(self, mods: List[ModInfo]) -> None:
        """Uninstalls the specified mods from the II mods/ path."""
        for mod in mods:
            try:
                self.uninstall_mod(mod)
            except (OSError, ValueError) as e:
                print('Error uninstalling {0}({1}): {2}'.format(
                    mod.name, mod.mod_id, e),
                        sys.stderr)

    def install_mod(self, mod: ModInfo) -> None:
        """Installs the specified mod from download to the II mods/ path."""
        mod_download_path = os.path.join(self._config.download_path, mod.mod_id)
        mod_install_path = os.path.join(self._config.install_path, mod.mod_id)

        if os.path.isdir(mod_install_path):
            # Already installed.
            return

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
