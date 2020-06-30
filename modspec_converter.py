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


def read_modspecs(blobs: List[List[str]], available_mods: List[ModInfo]) -> List[ModInfo]:
    """Read a modspec from the specified sets of lines."""
    mod_index = {m.mod_id: m for m in available_mods}
    
    mods = set()
    for blob_number, blob in enumerate(blobs, start=1):
        for line_number, line in enumerate(blob, start=1):
            if len(blobs) > 1:
                src = 'input #{0}, line {1}'.format(blob_number, line_number)
            else:
                src = 'line {0}'.format(line_number)

            try:
                if not line.rstrip('\n'):
                    continue
                mod_id, _, name = line.rstrip('\n').split(':', 2)
            except:
                print('ERROR on {0}: {1}'.format(src, line), file=sys.stderr)
                raise

            if mod_id in mod_index:
                mods.add(mod_index[mod_id])
            else:
                print('WARNING: No match for {0}:{1} ({2}) in available '
                        'mods.'.format(mod_id, name, src), file=sys.stderr)
    return sorted(mods, key=lambda m: m.mod_id)


def read_modspec(blob: List[str], available_mods: List[ModInfo]) -> List[ModInfo]:
    """Read a modspec from the specified set of lines."""
    return read_modspecs([reader], available_mods)


def from_modspec(modspec: str, available_mods: List[ModInfo]) -> List[ModInfo]:
    """Convert the given mods to a modspec string."""
    reader = io.StringIO(modspec)
    return read_modspec(reader.readlines(), available_mods)
