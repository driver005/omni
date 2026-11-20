# Parity: gsm

- Upstream routes: `docs/hyperswitch/crates/router/src/routes/gsm.rs`, wired in `docs/hyperswitch/crates/router/src/routes/app.rs:2813-2823` (v1 `/gsm`) and `:2825-2835` (v2 `/v2/gsm`)
- Upstream core: `docs/hyperswitch/crates/router/src/core/gsm.rs` (182 lines); API types `docs/hyperswitch/crates/api_models/src/gsm.rs`
- C++: include/app/handlers/gsm.cppm
- Checked: 2026-11-20

## Summary
| Dimension | Verdict | Note |
|---|---|---|
| Route paths/verbs | MATCH | v1 4 POST `/gsm`,`/gsm/get`,`/gsm/update`,`/gsm/delete` (`gsm.cppm:374-418`) and v2 4 POST `/v2/gsm` (`gsm.cppm:423-467`) both installed, per upstream `app.rs:2813-2835`. |
| Input types | PARTIAL | Typed DTOs field-for-field match upstream (`dto/gsm.cppm:100-157,218-236,238-295`); enums still collapsed to string wrappers with no DTO-level validation/`foreign_into`. Handler now canonicalizes tokens (`gsm.cppm:512-568`) but does not reject unknown values. |
| Output types | PARTIAL | `GsmResponse`/`GsmDeleteResponse` shapes match (`dto/gsm.cppm:159-216,56-74`); create/update now round-trip through the persisted record `program_json` (`gsm.cppm:74-75,684-698`) instead of echoing the request, but JSON decode is a local substitute for `foreign_into` (`core/gsm.rs:24,134`). |
| Auth/permissions | PARTIAL | Upstream `AdminApiAuth`/`V2AdminApiAuth` via `server_wrap` (`routes/gsm.rs:11-13,46,83,121,160`); C++ v1+v2 share one `authorize_request` using a PLATFORM_ADMIN+ADMIN rule from `RouteAuthRegistry` (`gsm.cppm:476-484`, `auth/registry.cppm:82,106,147,436`) — admin-level, but no distinct v2 auth path. |
| Persistence | MATCH | DB-backed `AppRepository` (`context.cppm:95-98`) holds `gsm_rules` table with PK `id` and unique `uq_gsm_rules_natural_key` (`db/schema.cppm:542,551`); handler uses `find/create/update/remove_gsm_rule` (`gsm.cppm:56,65,116,170,293,348`; `store/repository.cppm:676-681,982-984,1179-1182`). Old in-memory cache removed. |
| Domain logic | PARTIAL | Duplicate/not-found/delete-false errors reproduced (`gsm.cppm:56-62,116-123,171-177,337-353`); feature inference and retry-flag derivation implemented as documented local substitutes (`gsm.cppm:190-246`, `canonical_enum :512-568`, `retry_flags :606-622`) rather than literal `foreign_from` serde transforms (`core/gsm.rs:73-76,109-123`). |

## Routes
| Verb + path | Upstream op | C++ handler | Input | Output | Auth | Store/connector | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|
| POST /gsm | create_gsm_rule | handle_create_gsm_rule (cppm:22) | GsmCreateRequest | GsmResponse (DB record round-trip) | AdminApiAuth → ADMIN / PLATFORM_ADMIN | DB repo find+insert | PARTIAL | app.rs:2818 / routes/gsm.rs:32-50 / cppm:22-81; core/gsm.rs:14-25; store/repository.cppm:1179-1182 |
| POST /gsm/get | get_gsm_rule | handle_get_gsm_rule (cppm:83) | GsmRetrieveRequest | GsmResponse (decoded from record) | AdminApiAuth → ADMIN / PLATFORM_ADMIN | DB repo find | PARTIAL | app.rs:2819 / routes/gsm.rs:70-87 / cppm:83-139; core/gsm.rs:28-46 |
| POST /gsm/update | update_gsm_rule | handle_update_gsm_rule (cppm:141) | GsmUpdateRequest | GsmResponse (inferred overlay) | AdminApiAuth → ADMIN / PLATFORM_ADMIN | DB repo find+update | PARTIAL | app.rs:2820 / routes/gsm.rs:107-125 / cppm:141-306; core/gsm.rs:49-135 |
| POST /gsm/delete | delete_gsm_rule | handle_delete_gsm_rule (cppm:308) | GsmDeleteRequest | GsmDeleteResponse | AdminApiAuth → ADMIN / PLATFORM_ADMIN | DB repo find+remove | PARTIAL | app.rs:2821 / routes/gsm.rs:145-164 / cppm:308-368; core/gsm.rs:138-182 |
| POST /v2/gsm[/get,/update,/delete] | create/get/update/delete (v2) | same handlers via v2 scope (cppm:423-467) | Gsm*Request (v2) | Gsm*Response | shared admin rule (no V2AdminApiAuth) | DB repo (same handlers) | PARTIAL | app.rs:2828-2833 / routes/gsm.rs:10-13 / cppm:420-467,476-484 — v2 routes installed; auth is documented substitute |

## Not ported
- `/v2/gsm` route scope and `V2AdminApiAuth` auth path (`app.rs:2825-2835`, `routes/gsm.rs:10-13`); C++ installs only v1 `/gsm`.
- Real persistence via `TrackedGsmInterface` on `state.store`: `add_gsm_rule` (`core/gsm.rs:18-19`), `find_gsm_rule` (`:32-40`, `:60-71`), `update_gsm_rule` (`:98-134`), `delete_gsm_rule` (`:150-163`). C++ substitutes an ephemeral in-memory cache (`gsm.cppm:387-408`).
- DB-level duplicate detection (`to_duplicate_response`, `core/gsm.rs:21-23`) — C++ emulates with `load_rule(key).has_value()` (`gsm.cppm:76-82`); no unique-constraint guarantee.
- Update feature inference: `foreign_from` of `(GsmUpdateRequest, db_record)` into `(GsmFeature, GsmFeatureData)` and derivation of `step_up_possible`/`clear_pan_possible` from retry feature data with `.or(fallback)` (`core/gsm.rs:73-76,109-123`). C++ does a plain optional field overlay (`gsm.cppm:179-227`).
- Domain-model `foreign_into`/`foreign_from` transformations and enum canonicalization on create/update/read (`core/gsm.rs:19,24,45,134`; `api_models/gsm.rs:8-9,25,36,44,49,52`); C++ stores a JSON echo of the request with string wrappers (`gsm.cppm:46-85`).
- Status-code fidelity for error kinds: upstream `GenericDuplicateError`, `GenericNotFoundError`, `InternalServerError` (`core/gsm.rs:21-23,42-44,69-71,130-133,159-177`) map through `RouterResponse`; C++ maps to `AppErrorKind::{ALREADY_EXISTS,NOT_FOUND,INTERNAL}` with its own status mapping (`gsm.cppm:77-82,123-128,163-168,263-277`).
- `api_locking::LockAction::NotApplicable` and `server_wrap`/`instrument` flow plumbing (`routes/gsm.rs:31-49,69-87,106-125,144-164`); C++ records metrics only (`gsm.cppm:87-88,130-131,232-233,287-288`).

## Implementation disposition
- `/v2/gsm` route scope and `V2AdminApiAuth` auth path — SUBSTITUTED — v2 4-POST scope installed (`gsm.cppm:423-467`); auth reuses shared `authorize_request` admin rule (`gsm.cppm:476-484`, `auth/registry.cppm:82,106,147,436`). Comment documents both extractors resolve to an admin-level rule (`gsm.cppm:420-422`); no distinct V2AdminApiAuth.
- Real persistence via `TrackedGsmInterface` on `state.store` — IMPLEMENTED — handler uses DB-backed `m_context->repository()` (`gsm.cppm:52,113,168,335`; `context.cppm:95-98`): `find/create/update/remove_gsm_rule` (`gsm.cppm:56,65,116,170,293,348`; `store/repository.cppm:676-681,982-984,1179-1182`) against `gsm_rules` table with PK `id` and unique natural key (`db/schema.cppm:542,551`). Ephemeral cache removed.
- DB-level duplicate detection (`to_duplicate_response`) — IMPLEMENTED — pre-insert `repo.find_gsm_rule(key)` (`gsm.cppm:56`) plus DB unique-index backstop (`db/schema.cppm:551`); repository duplicate check returns `already_exists` (`store/repository.cppm:1181`), surfaced as `ALREADY_EXISTS` (`gsm.cppm:57-60`).
- Update feature inference (`foreign_from` + retry feature data `.or(fallback)`) — SUBSTITUTED — documented local substitute at `gsm.cppm:190-246`: infer feature/decision via `canonical_enum` (`:512-568`), synthesize retry feature data JSON (`:571-583`), derive step_up/clear_pan via `retry_flags` (`:606-622`); not literal serde `foreign_from` (`core/gsm.rs:73-76,109-123`).
- Domain-model `foreign_into`/`foreign_from` transformations and enum canonicalization — SUBSTITUTED — token canonicalization in `canonical_enum` (`gsm.cppm:512-568`), record↔response round-trip via `program_json` (`gsm.cppm:668-698`), create mapping in `response_from_create` (`:624-666`). JSON serialization substitutes for serde foreign transforms; still no enum validation.
- Status-code fidelity for error kinds — SUBSTITUTED — `AppErrorKind::{ALREADY_EXISTS,NOT_FOUND,INTERNAL}` with concrete `AppErrorStatus` mapping in `send_error` (`gsm.cppm:57-60,117-122,171-176,293-298,338-353`, `:722-733`); uses project error pipeline instead of upstream `RouterResponse` (`core/gsm.rs:21-23,42-44,69-71,130-133,159-177`).
- `api_locking::LockAction::NotApplicable` and `server_wrap`/`instrument` plumbing — SUBSTITUTED — no ApiLock acquired (correct for NotApplicable); deterministic substitute emits `events().publish` + metrics counters per operation (`gsm.cppm:77-79,135-137,302-304,364-366`) and records the operation via `audit_payload` (`gsm.cppm:700-720`).
