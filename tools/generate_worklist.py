#!/usr/bin/env python3
"""
Generate WORKLIST.tsv — rollup per lib:obj with function counts
Also generates analysis/STATS.txt with summary stats for the plan.
"""

import os
import csv
from collections import defaultdict, Counter

MANIFEST_IN = os.path.join(os.path.dirname(__file__), '..', 'analysis', 'MANIFEST_CLASSIFIED.tsv')
WORKLIST_OUT = os.path.join(os.path.dirname(__file__), '..', 'analysis', 'WORKLIST.tsv')
STATS_OUT = os.path.join(os.path.dirname(__file__), '..', 'analysis', 'STATS.txt')


def main():
    rows = []
    with open(MANIFEST_IN, 'r', encoding='utf-8') as f:
        reader = csv.DictReader(f, delimiter='\t')
        for row in reader:
            rows.append(row)

    # Rollup by (lib, obj) for non-inline functions
    worklist = defaultdict(lambda: {'total': 0, 'inline': 0, 'non_inline': 0,
                                      'classes': set(), 'example_names': []})

    # Also roll up by target_cpp for game code
    by_obj = defaultdict(lambda: {'total': 0, 'inline': 0, 'non_inline': 0, 'name': ''})

    for row in rows:
        cls = row['class']
        is_inline = row['is_inline'] == '1'
        lib = row['lib']
        obj = row['obj']

        # Use obj as key for game code (no lib), lib:obj for engine
        if cls == 'game':
            key = obj
        else:
            key = f"{lib}:{obj}" if lib else obj

        wl = worklist[key]
        wl['total'] += 1
        if is_inline:
            wl['inline'] += 1
        else:
            wl['non_inline'] += 1
        wl['classes'].add(cls)
        if len(wl['example_names']) < 3:
            wl['example_names'].append(row['name'])

    # Write WORKLIST.tsv
    with open(WORKLIST_OUT, 'w', encoding='utf-8', newline='') as f:
        headers = ['lib', 'obj', 'class', 'total_funcs', 'non_inline', 'inline', 'examples']
        writer = csv.DictWriter(f, fieldnames=headers, delimiter='\t')
        writer.writeheader()
        for key, wl in sorted(worklist.items(), key=lambda x: -x[1]['non_inline']):
            # Parse key back to lib/obj
            if ':' in key:
                lib, obj = key.split(':', 1)
            else:
                lib, obj = '', key

            cls_str = ','.join(sorted(wl['classes']))
            writer.writerow({
                'lib': lib,
                'obj': obj,
                'class': cls_str,
                'total_funcs': wl['total'],
                'non_inline': wl['non_inline'],
                'inline': wl['inline'],
                'examples': ' | '.join(wl['example_names'][:3]),
            })

    print(f"Wrote {len(worklist)} entries to {WORKLIST_OUT}")

    # Generate stats
    lines = []
    lines.append("=== COD3 MANIFEST STATISTICS ===")
    lines.append("")

    # Summary by class
    cls_counts = Counter(r['class'] for r in rows)
    total = len(rows)
    lines.append("-- By classification --")
    for cls in ['game', 'engine', 'xdk', 'crt']:
        n = cls_counts[cls]
        lines.append(f"  {cls:10s}: {n:6d} ({n*100.0/total:5.1f}%)")
    lines.append(f"  {'TOTAL':10s}: {total:6d}")
    lines.append("")

    # Non-inline breakdown
    non_inline = [r for r in rows if r['is_inline'] != '1']
    lines.append("-- Non-inline functions --")
    lines.append(f"  Total: {len(non_inline)}")

    cls_non_inline = Counter(r['class'] for r in non_inline)
    for cls in ['game', 'engine', 'xdk', 'crt']:
        n = cls_non_inline[cls]
        lines.append(f"    {cls:10s}: {n:6d} ({n*100.0/len(non_inline):5.1f}%)")
    lines.append("")

    # Game objects breakdown
    lines.append("-- Game objects (non-inline) --")
    game_rows = [r for r in non_inline if r['class'] == 'game']
    game_by_obj = Counter(r['obj'] for r in game_rows)
    for obj, n in game_by_obj.most_common():
        lines.append(f"  {obj:25s}: {n:5d}")
    lines.append(f"  {'Total game':30s}: {len(game_rows):5d}")
    lines.append("")

    # Engine libs breakdown
    lines.append("-- Engine libraries (non-inline) --")
    eng_rows = [r for r in non_inline if r['class'] == 'engine']
    eng_by_lib = Counter(r['lib'] for r in eng_rows)
    for lib, n in eng_by_lib.most_common():
        eng_by_obj = Counter(r['obj'] for r in eng_rows if r['lib'] == lib)
        lines.append(f"  {lib}: {n:5d} funcs across {len(eng_by_obj)} files")
    lines.append(f"  {'Total engine':30s}: {len(eng_rows):5d}")
    lines.append("")

    # XDK breakdown
    lines.append("-- XDK libraries (non-inline) --")
    xdk_rows = [r for r in non_inline if r['class'] == 'xdk']
    xdk_by_lib = Counter(r['lib'] for r in xdk_rows)
    for lib, n in xdk_by_lib.most_common():
        lines.append(f"  {lib:20s}: {n:5d}")
    lines.append(f"  {'Total XDK (to shim)':30s}: {len(xdk_rows):5d}")
    lines.append("")

    # Summary
    to_reconstruct = cls_non_inline['game'] + cls_non_inline['engine']
    lines.append("-- RECONSTRUCTION TARGET --")
    lines.append(f"  Game code:     {cls_non_inline['game']:5d} functions")
    lines.append(f"  Engine code:   {cls_non_inline['engine']:5d} functions")
    lines.append(f"  To reconstruct: {to_reconstruct:5d} functions")
    lines.append(f"  To shim (XDK):  {cls_non_inline['xdk']:5d} functions")
    lines.append(f"  CRT (modern):   {cls_non_inline['crt']:5d} functions")
    lines.append(f"  Inline/COMDAT:  {total - len(non_inline):5d} functions (into headers)")

    with open(STATS_OUT, 'w', encoding='utf-8') as f:
        f.write('\n'.join(lines))
    print(f"Wrote stats to {STATS_OUT}")
    print('\n'.join(lines))


if __name__ == '__main__':
    main()
