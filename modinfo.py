"""Details from modinfo.txt files."""

from typing import Optional

import attr


@attr.s(frozen=True, auto_attribs=True, kw_only=True)
class ModInfo:
    """Details about a single mod.

    Attributes:
        mod_id: Identifier within mod manager and save files. Populated from
            the folder name within mods/, not modinfo.txt. Stable and unique
            within a local environment, otherwise saves would become corrupt.
        name: Author-specified mod name. Used as the identifier within Sim
            Constructor + Sequential Mod Loader. Stable across different
            users/installs.
        author: Mod author.
        version: Mod version.
    """
    mod_id: str
    name: str
    author: Optional[str] = None
    version: Optional[str] = None
