"""Configuration of II Mod Manager."""

from typing import List, Optional, Tuple

import attr

@attr.s(frozen=True, auto_attribs=True, kw_only=True)
class Config:
    """Configuration set for II Mod Manager.

    Attributes:
        install_path: The mods/ directory within the II installation.
        download_path: The directory containing downloaded mods. Mods are
            installed from here to the II install directory.
    """
    install_path: str
    download_path: str
