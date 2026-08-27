# Verification evidence ledger

`tools/verify_ledger.py` derives the function denominator, V1 source location,
and V2 debug-symbol presence from the release map, source tree, and build
artifacts. V3--V5 are granted only by rows in this directory. The generated
`analysis/VERIFY.tsv`, `analysis/VERIFY_SUMMARY.tsv`, and
`analysis/VERIFY_ANOMALIES.tsv` are reports and may be regenerated.

Each evidence file is tab-separated with this exact header:

```text
ida_ea	name	gate	result	session	date	source_ref	detail
```

Required fields are `ida_ea`, `name`, `gate`, `result`, `session`, `date`, and
`source_ref`; `detail` may be empty. `gate` is one of `V1` through `V5`, and
`result` is `PASS`, `FAIL`, or `ADJUDICATE`. `source_ref` must identify the
actual disassembly, test, or review artifact supporting that row. Do not use
object-level or file-level rows: evidence is keyed by `(ida_ea, name, gate)`
and never inherited by another function.

Rows are append-only. The latest row for a key is authoritative, so a later
`FAIL` remains failed until a later, separately sourced `PASS` is appended.
