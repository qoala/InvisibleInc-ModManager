"""Parsing logic for extracting ModInfo objects from files."""

from typing import List, Optional, Tuple
import attr
import os
import sys


from modinfo import ModInfo

def _parse_line(line: str) -> Tuple[Optional[str], Optional[str]]:
    parts = line.split('=', 1)
    if len(parts) < 2:
        return None, None
    return parts[0].strip(), parts[1].strip()


def from_file(mod_id: str, filename: str) -> ModInfo:
    """Reads a ModInfo from the specified modinfo.txt."""
    data = {}
    with open(filename) as f:
        for line in f:
            key, value = _parse_line(line)
            if key in attr.fields_dict(ModInfo):
                data[key] = value
    return ModInfo(mod_id=mod_id, **data)


def _single_from_dir(root_path: str, mod_id: str) -> Optional[ModInfo]:
    mod_path = os.path.join(root_path, mod_id)
    info_filename = os.path.join(mod_path, 'modinfo.txt')
    if not os.path.isdir(mod_path) or not os.path.isfile(info_filename):
        return None
    try:
        return from_file(mod_id, info_filename)
    except (OSError, ValueError) as e:
        print('Error reading {0}: {1}'.format(info_filename, e), sys.stderr)
        return None


def all_from_dir(root_path: str) -> List[ModInfo]:
    """Reads ModInfo for all mods in a single directory."""
    mods = []
    for mod_id in os.listdir(root_path):
        mod = _single_from_dir(root_path, mod_id)
        if mod:
            mods.append(mod)
    return mods


if __name__ == '__main__':
    if len(sys.argv) == 2:
        mods = all_from_dir(sys.argv[1])
        for mod in sorted(mods, key=lambda m: m.name):
            print(mod)
    elif len(sys.argv) == 3:
        mod = from_file(sys.argv[1], sys.argv[2])
        print(mod)
