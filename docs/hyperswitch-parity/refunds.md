# Parity: refunds

- Upstream routes: `docs/hyperswitch/crates/router/src/routes/refunds.rs`; registration `/refunds` + `/v2/refunds` in `docs/hyperswitch/crates/router/src/routes/app.rs:1640-1714`
- Upstream core: `docs/hyperswitch/crates/router/src/core/refunds.rs`
- C++: include/app/handlers/refunds.cppm
- Checked: 2026-11-19; re-verified 2026-11-20 against `refunds.cppm:1-1671`

## Summary
| Dimension | Verdict | Note |
|---|---|---|
| Route paths/verbs | MATCH | All 16 upstream ops (14 v1 + 5 v2, 3 shared handlers) have a C++ route; paths/verbs align at `refunds.cppm:767-990` vs `app.rs:1642-1710`. |
| Input types | PARTIAL | Typed DTOs but simplified; path `{id}` now extracted/required via `refund_id_from_path` (`refunds.cppm:999-1014`, used :62,:154,:316,:363,:425,:708); force_sync/all_keys/time-range read from body+query (:259-262,:1501-1508). |
| Output types | PARTIAL | Typed DTOs; platform list returns full items (:510-515), platform/filter returns `PlatformRefundListFilters` (:540), filter metadata via :613/:636/:659; fields simplified vs upstream. |
| Auth/permissions | PARTIAL | Every handler uses `auth_rules().rule_for` + `authorizer().authorize_route` (e.g. :42-44,:692-694); merchant/profile from header/`AuthContext` (:65,:79,:1482-1484); upstream scheme/permission granularity not reproduced. |
| Persistence | PARTIAL | `repository().save_refund`/`list_refunds`/`find_refunds_by_payment_id` + cache + config_store + connector_router; profile scoping implemented (:1475-1497); still no upstream DB/profile models. |
| Domain logic | PARTIAL (was STUB) | Payment-intent validation (:1099-1145), execute/reverse/sync (:1238-1292,:341-401,:235-292), status/error/webhook/offer-engine (:1177-1236), locking (:86-93), profile scoping (:1475-1497) present; connector/UCS, GSM and access-token paths are documented local substitutes, not MATCH. |

## Routes
| Verb + path | Upstream op | C++ handler | Input | Output | Auth | Store/connector | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|
| POST `/refunds` | `refunds_create` (v1 oltp) | `handle_refunds_create` | `RefundRequest` -> `RefundsCreateRequest` | `RefundResponse` | ApiKey / JWT `ProfileRefundWrite` vs `REFUND_WRITE` | upstream DB + connector; C++ intent validation + lock + `save_refund` + `execute_refund` | PARTIAL | `refunds.rs:90-123`; `refunds.cppm:36-142,1099-1145` |
| POST `/refunds/sync` | `refunds_retrieve_with_body` | `handle_refunds_retrieve_with_body` | `RefundsRetrieveRequest` -> `RefundsRetrieveBody` | `RefundResponse` | ApiKey vs `PAYMENT_READ` | upstream sync_refund_with_gateway; C++ force_sync/all_keys gating + `connector->sync` | PARTIAL | `refunds.rs:358-392`; `refunds.cppm:235-292,1147-1150,1285` |
| GET `/refunds/{id}` | `refunds_retrieve` (v1) | `handle_refunds_retrieve` | path `{id}` + `RefundsRetrieveBody` | `RefundResponse` | ApiKey/JWT `ProfileRefundRead` vs `PAYMENT_READ` | upstream refund_retrieve_core; C++ requires path id, raw response via query | PARTIAL | `refunds.rs:191-241`; `refunds.cppm:144-173,999-1014` |
| POST `/refunds/{id}` | `refunds_update` | `handle_refunds_update` | `RefundUpdateRequest` + path `{id}` -> body DTO | `RefundResponse` | ApiKey vs `REFUND_WRITE` | upstream DB update; C++ reason + metadata to config_store, cache, repository | PARTIAL | `refunds.rs:400-424`; `refunds.cppm:294-339` |
| POST `/refunds/{id}/reverse` | `refunds_reverse` | `handle_refunds_reverse` | `RefundReverseRequest` + path `{id}` -> body DTO | `RefundResponse` | ApiKey vs `REFUND_WRITE` | upstream void-post-refund state machine; C++ success guard + void metadata + `execute_refund("cancel")` | PARTIAL | `refunds.rs:431-456`; `core/refunds.rs:127-336`; `refunds.cppm:341-401,386-396` |
| POST `/refunds/list` | `refunds_list` (v1 olap) | `handle_refunds_list` | `RefundListRequest` | `RefundListResponse` | ApiKey/JWT `MerchantRefundRead` vs `PAYMENT_READ` | upstream OLAP filter; C++ filtered + merchant/profile-scoped list | PARTIAL | `refunds.rs:502-531`; `refunds.cppm:451-486,1475-1497` |
| GET `/refunds/platform/list` | `refunds_list_for_platform` | `handle_refunds_list_for_platform` | `PlatformRefundListRequest` query -> generic blob | `PlatformRefundListResponse` | upstream platform self-op vs `PAYMENT_READ` | C++ returns full platform items | PARTIAL | `refunds.rs:576-605`; `refunds.cppm:488-518,1614-1639` |
| GET `/refunds/platform/filter` | `refunds_filter_list_for_platform` | `handle_refunds_filter_list_for_platform` | `()` -> generic blob | `PlatformRefundListResponse` | upstream platform self-op vs `PAYMENT_READ` | upstream `get_platform_refund_filters`; C++ returns `PlatformRefundListFilters` | PARTIAL | `refunds.rs:612-640`; `refunds.cppm:520-543,1584-1612` |
| POST `/refunds/profile/list` | `refunds_list_profile` | `handle_refunds_list_profile` | `RefundListRequest` | `RefundListResponse` | JWT `ProfileRefundRead` vs `PAYMENT_READ` | upstream profile-scoped; C++ merchant/profile-scoped filtered list | PARTIAL | `refunds.rs:647-682`; `refunds.cppm:545-580,1475-1497` |
| POST `/refunds/filter` | `refunds_filter_list` | `handle_refunds_filter_list` | `TimeRange` -> `RefundListFilters` | `RefundListFilters` | ApiKey/JWT `MerchantRefundRead` vs `PAYMENT_READ` | C++ scoped `filter_metadata` (filter metadata, upstream expects refund list) | PARTIAL | `refunds.rs:689-718`; `refunds.cppm:582-616,1554-1582` |
| GET `/refunds/v2/filter` | `get_refunds_filters` | `handle_get_refunds_filters` | `()` -> generic blob | `RefundListMetaData` | ApiKey/JWT `MerchantRefundRead` vs `PAYMENT_READ` | upstream `get_filters_for_refunds`; C++ scoped, derives from records | PARTIAL | `refunds.rs:725-750`; `refunds.cppm:618-639,1523-1552,1475-1497` |
| GET `/refunds/aggregate` | `get_refunds_aggregates` | `handle_get_refunds_aggregates` | `TimeRange` query -> generic blob | `RefundAggregateResponse` | ApiKey/JWT `MerchantRefundRead` vs `PAYMENT_READ` | C++ honors start_time/end_time, counts by status | PARTIAL | `refunds.rs:794-824`; `refunds.cppm:664-684,1499-1521` |
| GET `/refunds/profile/aggregate` | `get_refunds_aggregate_profile` | `handle_get_refunds_aggregate_profile` | `TimeRange` query -> generic blob | `RefundAggregateResponse` | JWT `ProfileRefundRead` vs `PAYMENT_READ` | C++ profile-scoped + time-ranged counts | PARTIAL | `refunds.rs:851-887`; `refunds.cppm:740-760,1475-1497,1499-1521` |
| GET `/refunds/v2/profile/filter` | `get_refunds_filters_profile` | `handle_get_refunds_filters_profile` | `()` -> generic blob | `RefundListFilters` | JWT `ProfileRefundRead` vs `PAYMENT_READ` | upstream profile-scoped filters; C++ profile-scoped filter metadata | PARTIAL | `refunds.rs:757-790`; `refunds.cppm:641-662,1475-1497,1554-1582` |
| PUT `/refunds/{id}/manual-update` | `refunds_manual_update` | `handle_refunds_manual_update` | `RefundManualUpdateRequest` + path `{id}` | `RefundResponse` | upstream `AdminApiAuth` vs `REFUND_WRITE` | C++ caches connector_refund_id, sets status/error | PARTIAL | `refunds.rs:828-847`; `refunds.cppm:686-738` |
| POST `/v2/refunds` | `refunds_create` (v2) | `handle_refunds_create` | `RefundsCreateRequest` -> same DTO | `RefundResponse` | MerchantIdAuth / V2ApiKey + `ProfileRefundWrite` vs `REFUND_WRITE` | upstream generates GlobalRefundId + lock; C++ path-id-or-generated id + lock | PARTIAL | `refunds.rs:128-183`; `refunds.cppm:36-142,106,999-1014` |
| GET `/v2/refunds/{id}` | `refunds_retrieve` (v2) | `handle_refunds_retrieve` | path `GlobalRefundId` + query -> generic blob | `RefundResponse` | V2ApiKey/JWT `ProfileRefundRead` vs `PAYMENT_READ` | upstream `return_raw_connector_response`; C++ requires path id, raw via query | PARTIAL | `refunds.rs:245-292`; `refunds.cppm:144-173` |
| POST `/v2/refunds/{id}` | `refunds_retrieve_with_gateway_creds` | `handle_refunds_retrieve_with_gateway_creds` | `RefundsRetrievePayload` -> `RefundsRetrieveRequest` | `RefundResponse` | MerchantIdAuth / V2ApiKey vs `PAYMENT_READ` | upstream inserts MCD creds; C++ force_sync + config creds + `connector->sync` | PARTIAL | `refunds.rs:296-350`; `refunds.cppm:175-233,1238-1292` |
| PUT `/v2/refunds/{id}/update-metadata` | `refunds_metadata_update` | `handle_refunds_metadata_update` | `RefundMetadataUpdateRequest` + path | `RefundResponse` | upstream V2ApiKey vs `REFUND_WRITE` | upstream metadata state update; C++ id-scoped reason + metadata config | PARTIAL | `refunds.rs:460-495`; `refunds.cppm:403-449` |
| POST `/v2/refunds/list` | `refunds_list` (v2 olap) | `handle_refunds_list` | `RefundListRequest` | `RefundListResponse` | V2ApiKey/JWT `MerchantRefundRead` vs `PAYMENT_READ` | upstream account-scoped; C++ filtered + merchant-scoped list | PARTIAL | `refunds.rs:535-569`; `refunds.cppm:451-486,1475-1497` |

## Not ported
- Payment-intent validation: status `Succeeded|PartiallyCaptured`, amount vs intent metadata, post-capture-void guard, positive-amount check (`core/refunds.rs:426-465`).
- Last-successful payment-attempt lookup and profile-id auth validation (`core/refunds.rs:467-474,426,1112`).
- `merchant_connector_details` insertion into config and creds-identifier routing (`core/refunds.rs:476-487,1135-1147`).
- Refund execute state machine: UCS/direct/shadow execution path, access-token handling, gateway context, lineage ids, kill switch/rollout (`core/refunds.rs:504-699`).
- Refund status/error update logic: GSM unified-code mapping, 5xx no-status-change, timeout handling, integrity check -> ManualReview, webhook + offer-engine notification (`core/refunds.rs:701-863`).
- Reverse flow: refund success guard, payment/attempt lookup, void-post-refund metadata encoding, access token, UCS call, ExternalConnectorError mapping (`core/refunds.rs:127-336`).
- Refund sync: `should_call_refund` force_sync/all_keys gating, unified translation message, split-refund request derivation, RSync connector flow (`core/refunds.rs:1100-1212,1216-1354`).
- API locking: per-payment-id serialization on create (`refunds.rs:16-32,146`); C++ has no locking.
- Upstream auth model: `HeaderAuth`, `JWTAndEmbeddedAuth`, `V2ApiKeyAuth`, `MerchantIdAuth`, `AdminApiAuth`, platform self-operation flags (`refunds.rs:107-119,148-163,385,417,449,488,843`).
- Admin-only auth for manual update (`refunds.rs:843`).
- Path `{id}` extraction: create/update/reverse/retrieve handlers read id from body or ignore it (`refunds.cppm:130,267,330,402,483,960`).
- Profile-scoped list/filter/aggregate behavior (`refunds.rs:514,588,661-663,771,807,867`).
- Proper platform filter output (`get_platform_refund_filters`, `refunds.rs:623`) and filter metadata for `/filter` (`refund_filter_list`, `refunds.rs:701`).

## Implementation disposition
- Payment-intent validation — IMPLEMENTED — `validate_refund_against_payment` (`refunds.cppm:1099-1145`): status gate :1110-1116, amount vs intent metadata `blocked_amount` :1135-1140, post-capture-void guard :1128-1133, positive-amount check :1137-1139; called on create at :81.
- Last-successful attempt + profile-id auth — IMPLEMENTED — `find_last_successful_attempt` (`refunds.cppm:1084-1097`, used :127,:385) selects latest success attempt; profile auth at :1117-1123.
- merchant_connector_details + creds routing — IMPLEMENTED — config `put` :118-125, `extract_creds_identifier` :1048-1062, `connector_router().select_for` :1259, gateway fallback :223-226.
- Refund execute state machine — SUBSTITUTED — `execute_refund` :1238-1292 (router/local connector :1259-1288, kill/shadow-kill :1270-1275, process trackers :1277-1290); real UCS/direct/shadow paths, access token, gateway context and lineage ids need external connector service (`core/refunds.rs:504-699`).
- Refund status/error update — SUBSTITUTED — `apply_connector_response` :1177-1236 (empty-status no-change :1183-1188, integrity->Review :1191-1201, timeout :1216-1223, webhook via `webhook_delivery` :1173, offer-engine via `events` :1233); GSM table replaced by hardcoded AE/IE codes and 5xx generalized to empty status (`core/refunds.rs:701-863`).
- Reverse flow — SUBSTITUTED — success guard :374-378, payment/attempt lookup :384-385, void metadata :386-392 implemented; access token / UCS call / ExternalConnectorError mapping replaced by local `execute_refund("cancel")` :396,:1238-1292 (`core/refunds.rs:127-336`).
- Refund sync — SUBSTITUTED — `should_call_refund` :1147-1150 (used :281), split derivation via `events` :1152-1160, `connector->sync` :1285; upstream unified translation message and RSync-specific flow absent (`core/refunds.rs:1100-1354`).
- API locking — IMPLEMENTED — `locks().acquire("refund:"+payment_id, 30s, "refund_create")` :86-93, release :98/:138, contention metric :89.
- Upstream auth model — SUBSTITUTED — all handlers use `auth_rules().rule_for` + `authorizer().authorize_route` (e.g. :42-44) with merchant/profile from header/`AuthContext` (:65,:79,:1482-1484); upstream scheme names mapped to local rule registry, not reproduced literally.
- Admin-only auth for manual update — SUBSTITUTED — `rule_for("refunds_manual_update")` :692-694; admin-only policy lives in external rule config, not literal `AdminApiAuth` (`refunds.rs:843`).
- Path `{id}` extraction — IMPLEMENTED — `refund_id_from_path` :999-1014 used in create/retrieve/update/reverse/metadata/manual handlers (:62,:154,:316,:363,:425,:708); required on retrieve/gateway/metadata (:155-159,:198-202,:426-430).
- Profile-scoped list/filter/aggregate — IMPLEMENTED — `scoped_records(profile_scoped)` :1475-1497 used at :481,:575,:610,:656,:679,:755; profile scope from config_store :1428-1432.
- Platform filter + `/filter` metadata — IMPLEMENTED — `platform_filter_metadata` :1584-1612 (used :540), `filter_metadata` :1554-1582 (used :613), `filter_list_metadata` :1523-1552 (used :636); platform list full items :510-515.
