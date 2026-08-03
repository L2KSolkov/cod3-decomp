#!/usr/bin/env python3
"""
Cross-reference codmp_xboxr.map (release) with codmp_xboxd.map (debug).
Produces analysis/CROSSREF.tsv with both addresses per symbol.
"""

import re
import csv
import os

RELEASE_MAP = os.path.join(os.path.dirname(__file__), '..', 'codmp_xboxr.map')
DEBUG_MAP   = os.path.join(os.path.dirname(__file__), '..', 'codmp_xboxd.map')
OUT_PATH    = os.path.join(os.path.dirname(__file__), '..', 'analysis', 'CROSSREF.tsv')

# IDA segment bases
REL_TEXT_BASE = 0x40c000
DBG_TEXT_BASE = 0x508e20

def parse_map(path, text_base):
    funcs = {}
    in_publics = False
    with open(path, encoding='utf-8', errors='replace') as f:
        for line in f:
            line = line.rstrip()
            if not in_publics:
                if 'Publics by Value' in line:
                    in_publics = True
                continue
            if not line.strip(): continue
            parts = line.split()
            if len(parts) < 5: continue
            seg_match = re.match(r'^([0-9A-Fa-f]+):([0-9A-Fa-f]+)$', parts[0])
            if not seg_match: continue
            seg = int(seg_match.group(1), 16)
            off = int(seg_match.group(2), 16)
            if not re.match(r'^[0-9A-Fa-f]{8}$', parts[2]): continue
            if parts[3] != 'f': continue
            flags_idx = 4
            is_inline = False
            if len(parts) > 4 and parts[4] == 'i':
                is_inline = True
                flags_idx = 5
            lib_obj_raw = ' '.join(parts[flags_idx:])
            colon = lib_obj_raw.find(':')
            lib = lib_obj_raw[:colon].strip() if colon >= 0 else ''
            obj = lib_obj_raw[colon+1:].strip() if colon >= 0 else lib_obj_raw.strip()
            if lib in ('<linker-defined>', '<absolute>'): continue
            ea = text_base + off if seg == 2 else 0
            funcs[parts[1]] = (ea, is_inline, lib, obj)
    return funcs

print("Parsing release map...")
rel = parse_map(RELEASE_MAP, REL_TEXT_BASE)
print(f"  Release: {len(rel)} functions")

print("Parsing debug map...")
dbg = parse_map(DEBUG_MAP, DBG_TEXT_BASE)
print(f"  Debug:   {len(dbg)} functions")

# Cross-reference
common = set(rel.keys()) & set(dbg.keys())
only_rel = set(rel.keys()) - set(dbg.keys())
only_dbg = set(dbg.keys()) - set(rel.keys())

print(f"\nCommon (in both):  {len(common)}")
print(f"Only in release:   {len(only_rel)}")
print(f"Only in debug:     {len(only_dbg)}")

# Write cross-reference
with open(OUT_PATH, 'w', newline='', encoding='utf-8') as f:
    w = csv.writer(f, delimiter='\t')
    w.writerow(['name', 'rel_ea', 'dbg_ea', 'is_inline', 'lib', 'obj'])
    for name in sorted(common):
        r_ea, r_inl, r_lib, r_obj = rel[name]
        d_ea, d_inl, d_lib, d_obj = dbg[name]
        w.writerow([
            name,
            f'0x{r_ea:08X}', f'0x{d_ea:08X}',
            '1' if r_inl else '0',
            r_lib, r_obj
        ])

print(f"\nWrote {len(common)} cross-referenced entries to {OUT_PATH}")
