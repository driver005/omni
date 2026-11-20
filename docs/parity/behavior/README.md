# Route behavior reports

One report per Ghost controller module under
`docs/Ghost/ghost/core/server/api/endpoints/`. Method: [`CHECKLIST.md`](CHECKLIST.md).

Each report documents, per route, the full Ghost behavior (input, auth,
status/headers, complete service/model call chain, cache, side effects, output)
and the current C++ implementation with `file:line` evidence on both sides,
followed by a comparison verdict and residual gaps.

## Aggregate

| Verdict | Count |
|---|---|
| MATCH | 0 |
| PARTIAL | 267 |
| MISMATCH | 32 |
| MISSING | 9 |
| EXTRA | 6 |
| **Total routes** | **314** |

Reports: 89. Route detail sections: 314. Every report contains
`## Summary`, `## Route behavior table`, `## Route details`,
`## Module-level notes`, and `file:line` evidence on both sides.
