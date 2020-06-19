"""Converters to/from a modspec describing the set of installed mods.

A modspec is a series of lines in the following format:
    '{mod_id}:{version}:{name}'

Blank lines are ignored.
"""

from typing import List

import io
import sys

from modinfo import ModInfo


def write_modspec(mods: List[ModInfo], writer: io.TextIOBase) -> None:
    """Write the given mods as a modspec to the specified IO stream."""
    for mod in sorted(mods, key=lambda m: m.mod_id):
        line = '{mod_id}:{version}:{name}\n'.format(
                mod_id=mod.mod_id,
                version=mod.version if mod.version else '',
                name=mod.name)
        writer.write(line)


def to_modspec(mods: List[ModInfo]) -> str:
    """Convert the given mods to a modspec string."""
    writer = io.StringIO()
    write_modspec(mods, writer)
    return writer.getvalue()


def read_modspec(reader: io.TextIOBase, available_mods: List[ModInfo]) -> None:
    """Read a modspec from the specified IO stream."""
    mod_index = {m.mod_id: m for m in available_mods}
    
    mods = set()
    for line_number, line in enumerate(reader, 1):
        if not line:
            next
        mod_id, _, name = line.rstrip('\n').split(':', 2)

        if mod_id in mod_index:
            mods.add(mod_index[mod_id])
        else:
            print('WARNING: No match for {0}:{1} (line {2}) in available '
                    'mods.'.format(mod_id, name, line_number), file=sys.stderr)
    return sorted(mods, key=lambda m: m.mod_id)


def from_modspec(modspec: str, available_mods: List[ModInfo]) -> None:
    """Convert the given mods to a modspec string."""
    reader = io.StringIO(modspec)
    return read_modspec(reader, available_mods)
