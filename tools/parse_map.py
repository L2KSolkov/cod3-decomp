#!/usr/bin/env python3
"""
Parse codmp_xboxr.map into analysis/MANIFEST.tsv
Only includes function entries (flag 'f').

Columns: ida_ea, va, name, flags, lib, obj, segment, offset, is_inline
"""

import re
import os
import csv

MAP_PATH = os.path.join(os.path.dirname(__file__), '..', 'codmp_xboxr.map')
OUT_PATH = os.path.join(os.path.dirname(__file__), '..', 'analysis', 'MANIFEST.tsv')

# IDA segment base addresses (from binary survey)
SEGMENT_BASES = {
    2: 0x40c000,   # .text
    9: 0xc8f720,   # MP_LELCS
    # Other segments are XDK — ea not critical
}

def parse_map(map_path):
    functions = []
    in_publics = False

    with open(map_path, 'r', encoding='utf-8', errors='replace') as f:
        for line in f:
            line = line.rstrip('\n\r')

            if not in_publics:
                if 'Publics by Value' in line:
                    in_publics = True
                continue

            if not line.strip():
                continue

            parts = line.split()
            if len(parts) < 3:
                continue

            # Parse segment:offset
            seg_match = re.match(r'^([0-9A-Fa-f]+):([0-9A-Fa-f]+)$', parts[0])
            if not seg_match:
                continue
            segment = int(seg_match.group(1), 16)
            offset = int(seg_match.group(2), 16)

            name = parts[1]
            va_str = parts[2]
            if not re.match(r'^[0-9A-Fa-f]{8}$', va_str):
                continue
            va = int(va_str, 16)

            # Compute IDA ea from segment base + offset
            ida_ea = SEGMENT_BASES.get(segment, 0) + offset

            # Determine flags and lib:object
            data_remaining = parts[3:]
            flags = ''
            if data_remaining and data_remaining[0] == 'f':
                flags = 'f'
                if len(data_remaining) > 1 and data_remaining[1] == 'i':
                    flags = 'f i'
                    data_remaining = data_remaining[2:]
                else:
                    data_remaining = data_remaining[1:]

                lib_obj_raw = ' '.join(data_remaining) if data_remaining else ''
                colon_idx = lib_obj_raw.find(':')
                if colon_idx >= 0:
                    lib = lib_obj_raw[:colon_idx].strip()
                    obj = lib_obj_raw[colon_idx+1:].strip()
                else:
                    lib = ''
                    obj = lib_obj_raw.strip()

                if lib in ('<linker-defined>', '<absolute>'):
                    continue

                functions.append({
                    'ida_ea': ida_ea,
                    'va': va,
                    'name': name.strip(),
                    'flags': flags,
                    'lib': lib,
                    'obj': obj,
                    'segment': segment,
                    'offset': offset,
                    'is_inline': 'i' in flags,
                })

    return functions


def write_tsv(functions, out_path):
    seen = set()
    unique = []
    duplicates = 0
    for func in functions:
        key = (func['name'], func['va'])
        if key in seen:
            duplicates += 1
            continue
        seen.add(key)
        unique.append(func)

    inline_count = sum(1 for f in unique if f['is_inline'])
    print(f"Total function entries: {len(functions)}")
    print(f"Duplicate entries: {duplicates}")
    print(f"Unique functions: {len(unique)}")
    print(f"  Inline/COMDAT (f i): {inline_count}")
    print(f"  Non-inline (f): {len(unique) - inline_count}")

    with open(out_path, 'w', encoding='utf-8', newline='') as f:
        headers = ['ida_ea', 'va', 'name', 'flags', 'lib', 'obj',
                   'segment', 'offset', 'is_inline']
        writer = csv.DictWriter(f, fieldnames=headers, delimiter='\t')
        writer.writeheader()
        rows = []
        for func in unique:
            rows.append({
                'ida_ea': f"0x{func['ida_ea']:08X}",
                'va': f"0x{func['va']:08X}",
                'name': func['name'],
                'flags': func['flags'],
                'lib': func['lib'],
                'obj': func['obj'],
                'segment': str(func['segment']),
                'offset': f"0x{func['offset']:08X}",
                'is_inline': '1' if func['is_inline'] else '0',
            })
        writer.writerows(rows)

    print(f"Wrote {len(unique)} entries to {out_path}")


if __name__ == '__main__':
    funcs = parse_map(MAP_PATH)
    write_tsv(funcs, OUT_PATH)
