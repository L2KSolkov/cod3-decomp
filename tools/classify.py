#!/usr/bin/env python3
"""
Classify each MANIFEST.tsv entry as game/engine/xdk/crt/other
and add a 'class' column. Writes analysis/MANIFEST_CLASSIFIED.tsv
"""

import os
import csv

MANIFEST_IN = os.path.join(os.path.dirname(__file__), '..', 'analysis', 'MANIFEST.tsv')

# Classification rules by library
XDK_LIBS = {
    'dsoundd', 'd3d8d', 'xonlinesd', 'xgraphicsd', 'uixd',
    'xvoiced', 'd3dx8d', 'xapilibd', 'dmusicd', 'xboxkrnl',
    'xbdm', 'uuid',
}

CRT_LIBS = {
    'LIBCMTD', 'LIBCMT', 'libcpmtd',
}

ENGINE_LIBS = {
    'MPBrocCore_xboxd', 'mp_level.xboxd',
    'phys_xboxr', 'render_xboxr', 'ngl_xboxr',
    'nal_xboxr', 'nsl_xboxr', 'nfl_xboxr', 'nvl_xboxr',
    'aeps_xboxr', 'mem_mp_xboxr', 'core_xboxr',
    'tl_xboxr', 'cdl_xboxr', 'controller_xboxr',
    'jobqueue_xboxr', 'peripherals_xboxr', 'apk_xboxr',
    'inplace_xboxr',
    'bdCore', 'bdConnection', 'bdNet', 'bdPeer',
    'bdSocket', 'bdPlatform',
    'zlib_xboxr',
}

# Game objects (no lib prefix) — these are amalgamated .o files
GAME_OBJECTS = {
    'g.o', 'game.o', 'game2.o', 'shell.o', 'core.o',
    'scr.o', 'mp.o', 'mp_shell.o', 'mp_actors.o',
    'render.o', 'anim.o', 'streamer.o', 'physics.o',
    'game_xbox.o', 'mp_xbox.o', 'cg.o', 'cl.o', 'sv.o',
    'nextgen.o', 'CallFunctor.o',
}

def classify(lib, obj, is_inline, flags):
    """Return (class, is_game_code)"""
    if lib in XDK_LIBS:
        return 'xdk'
    if lib in CRT_LIBS:
        return 'crt'
    if lib in ENGINE_LIBS:
        return 'engine'

    # No lib prefix — check if it's a game object
    if lib == '':
        if obj in GAME_OBJECTS:
            return 'game'
        if obj == '' and is_inline:
            # Empty obj with inline flag - could be from any lib
            # Check name for hints
            return 'unknown_inline'
        return 'unknown'

    # Unrecognized lib — flag for review
    return 'unclassified'


def main():
    rows = []
    with open(MANIFEST_IN, 'r', encoding='utf-8') as f:
        reader = csv.DictReader(f, delimiter='\t')
        fieldnames = reader.fieldnames + ['class']
        for row in reader:
            lib = row.get('lib', '')
            obj = row.get('obj', '')
            is_inline = row.get('is_inline', '0') == '1'
            flags = row.get('flags', '')
            cls = classify(lib, obj, is_inline, flags)
            row['class'] = cls
            rows.append(row)

    out_path = os.path.join(os.path.dirname(__file__), '..', 'analysis', 'MANIFEST_CLASSIFIED.tsv')
    with open(out_path, 'w', encoding='utf-8', newline='') as f:
        writer = csv.DictWriter(f, fieldnames=fieldnames, delimiter='\t')
        writer.writeheader()
        writer.writerows(rows)

    # Print stats
    from collections import Counter
    cls_counts = Counter(r['class'] for r in rows)
    total = len(rows)
    print(f"Total functions: {total}")
    for cls in sorted(cls_counts.keys()):
        count = cls_counts[cls]
        pct = count * 100.0 / total
        print(f"  {cls:20s}: {count:6d} ({pct:5.1f}%)")

    # Game + engine to reconstruct
    to_port = cls_counts.get('game', 0) + cls_counts.get('engine', 0)
    print(f"\n  To reconstruct: {to_port} ({to_port*100.0/total:.1f}%)")
    print(f"  To shim (xdk):  {cls_counts.get('xdk', 0)} ({cls_counts['xdk']*100.0/total:.1f}%)")
    
    # Show unknowns
    unknown = [r for r in rows if r['class'] in ('unknown', 'unknown_inline', 'unclassified')]
    if unknown:
        print(f"\n  Unknown/unclassified: {len(unknown)}")
        unk_libs = Counter(f"{r['lib']}:{r['obj']}" for r in unknown)
        for lib_obj, count in unk_libs.most_common(20):
            print(f"    {lib_obj}: {count}")

    print(f"\nWrote {out_path}")


if __name__ == '__main__':
    main()
