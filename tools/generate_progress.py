#!/usr/bin/env python3
"""
Seed PROGRESS.tsv from WORKLIST.tsv.
All entries start as TODO. Non-inline functions only.
"""

import os
import csv

WORKLIST_IN = os.path.join(os.path.dirname(__file__), '..', 'analysis', 'WORKLIST.tsv')
PROGRESS_OUT = os.path.join(os.path.dirname(__file__), '..', 'PROGRESS.tsv')


def main():
    rows = []
    with open(WORKLIST_IN, 'r', encoding='utf-8') as f:
        reader = csv.DictReader(f, delimiter='\t')
        for row in reader:
            # Only include game + engine non-inline entries
            if 'game' not in row['class'] and 'engine' not in row['class']:
                continue
            if int(row['non_inline']) == 0:
                continue
            rows.append(row)

    with open(PROGRESS_OUT, 'w', encoding='utf-8', newline='') as f:
        headers = ['source_cpp', 'lib', 'obj', 'class', 'funcs', 'status', 'notes']
        writer = csv.DictWriter(f, fieldnames=headers, delimiter='\t')
        writer.writeheader()
        for row in rows:
            writer.writerow({
                'source_cpp': f"{row['lib']}:{row['obj']}" if row['lib'] else row['obj'],
                'lib': row['lib'],
                'obj': row['obj'],
                'class': row['class'],
                'funcs': row['non_inline'],
                'status': 'TODO',
                'notes': '',
            })

    todo_count = len(rows)
    total_funcs = sum(int(r['non_inline']) for r in rows)
    print(f"PROGRESS.tsv seeded: {todo_count} entries, {total_funcs} total functions to port")
    print(f"All status: TODO")


if __name__ == '__main__':
    main()
