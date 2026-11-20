# Parity: payout_link

- Upstream routes: `docs/hyperswitch/crates/router/src/routes/payout_link.rs`; registration `web::scope("/payout_link")` with `GET /{merchant_id}/{payout_id}` in `docs/hyperswitch/crates/router/src/routes/app.rs:2647-2655`
- Upstream core: `docs/hyperswitch/crates/router/src/core/payout_link.rs` (v1 `initiate_payout_link` `:39-329`; `filter_payout_methods` `:331-451`; `check_currency_country_filters` `:453-489`; v2 is `todo!()` `:28-37`)
- C++: include/app/handlers/payout_link.cppm
- Checked: 2026-11-20

## Summary
| Dimension | Verdict | Note |
|---|---|---|
| Route paths/verbs | MATCH | Single op `GET /payout_link/:merchant_id/:payout_id` on both sides (`payout_link.cppm:306-322` vs `app.rs:2650-2652`, handler `payout_link.rs:13-39`). |
| Input types | PARTIAL | Upstream `web::Path<(MerchantId, PayoutId)>` → typed `PayoutLinkInitiateRequest` + headers (`payout_link.rs:17-28`); C++ still extracts a raw `(string,string)` path pair (`payout_link.cppm:32-40,629-655`) but now consumes headers for auth merchant-id, domain validation, and locale override (`payout_link.cppm:366-371,433-453,117-119`). |
| Output types | PARTIAL | Upstream `GenericLinkForm` wrapping `GenericLinks{allowed_domains, data:{ExpiredLink,PayoutLink,PayoutLinkStatus}, locale}`; C++ emits the 3 JSON shapes via typed DTOs and now populates `allowed_domains`, config/query `locale`, and the js/css/sdk/html-meta wrapper (`payout_link.cppm:607-627,585-605`); still local substitutes (deterministic link id, config-sourced domains/methods) → PARTIAL. |
| Auth/permissions | MATCH | Upstream `MerchantIdAuth(merchant_id)` from the path (`payout_link.rs:35`); `authorize_operation` now uses `auth_rules().rule_for`, rejects header/path merchant-id mismatch, binds the principal to the path id, then `authorizer().authorize_route` (`payout_link.cppm:354-374`). |
| Persistence | PARTIAL | Repo reads payout/merchant/attempt/customer/MCA (`payout_link.cppm:53-99,461-477,525`); link id, status and `Invalidated` now persisted in the repository configs table (`:65-76,169-176,218-222`), cache only for ephemeral TTL values. No dedicated link/address tables → PARTIAL. |
| Domain logic | PARTIAL | Now covers domain validation, attempt/error read, customer fetch, method filtering, required-fields, form layout and config defaults via documented local substitutes (`payout_link.cppm:109-119,144-199,509-583`); no upstream translation catalog or payout_link/address tables → PARTIAL. |

## Routes
| Verb + path | Upstream op | C++ handler | Input | Output | Auth | Store/connector | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|
| GET `/payout_link/{merchant_id}/{payout_id}` | `render_payout_link` (v1) → `initiate_payout_link` | `handle_render_payout_link` | typed `PayoutLinkInitiateRequest` vs raw path pair; headers now consumed (auth/domain/locale) | `GenericLinkForm`/`GenericLinks` vs direct JSON, now with domains/locale + wrapper | `MerchantIdAuth`-equivalent: principal bound to path id + header mismatch check | repo reads + configs-table link state + cache + `connector_router().select` for publishable key | PARTIAL | `payout_link.rs:13-39`; `core/payout_link.rs:39-329`; `app.rs:2647-2655`; `payout_link.cppm:24-304,354-374,509-627` |

## Not ported
- Allowed-domain validation from request headers (`validate_payout_link_render_request_and_get_allowed_domains`); C++ emits `allowed_domains:[]` unconditionally (`core/payout_link.rs:82-85,124-130`; `payout_link.cppm:160`).
- `payout_attempt` fetch by id and unified code/message translation on the status page (`find_payout_attempt_by_merchant_id_payout_id_payout_attempt_id`, `get_translated_unified_code_and_message`); C++ reads error code/message only from cache (`core/payout_link.rs:58-66,279-286`; `payout_link.cppm:133-134,187-196`).
- `payout_link` entity lookup by `link_id` + required-value guard; C++ synthesizes a deterministic `"pl_<hash>"` link id (`core/payout_link.rs:67-80`; `payout_link.cppm:90`).
- Customer and address fetch in the initiated flow (`find_customer_by_customer_id_merchant_id`, `find_address_by_address_id`); C++ falls back to payout id for customer and never loads an address (`core/payout_link.rs:135-175`; `payout_link.cppm:110-111,213-215`).
- Payout-method filtering: `filter_payout_methods` + `check_currency_country_filters` over enabled MCAs, currency/country filters, connector-per-PMT map, default method fallback, cards-first sort, required-fields override, and `form_layout`; none present in C++ (`core/payout_link.rs:177-220,248,331-489`).
- Upstream UI config defaults from `state.conf.generic_link.payout_link` (merchant_name/logo/theme) and `sdk_url`/`html_meta_tags`/`css_data`/`js_data` wrapper; C++ hardcodes theme `"light"`, empty logo, and a bare JSON body (`core/payout_link.rs:91-104,252-267`; `payout_link.cppm:144-150,236`).
- Persistence of `Invalidated` link status via `update_payout_link`; C++ writes only to cache (`core/payout_link.rs:114-122`; `payout_link.cppm:157`).
- `locale` from server state and `allowed_domains` field inside `GenericLinks`; C++ hardcodes `"en"` and empty domains (`core/payout_link.rs:124-130,247,273,324`; `payout_link.cppm:164,231`).
- Upstream auth model `MerchantIdAuth` tying the caller to the path `merchant_id`; C++ does not bind auth principal to the path id (`payout_link.rs:35`; `payout_link.cppm:30-48`).
- Upstream v2 `initiate_payout_link` is itself `todo!()` — no parity target there (`core/payout_link.rs:28-37`).

## Implementation disposition
Re-verified against `include/app/handlers/payout_link.cppm`. One line per original `## Not ported` bullet.

- Allowed-domain validation — SUBSTITUTED — domains from config key `generic_link.payout_link.allowed_domains` via `parse_csv` (`:109`), header checks (`sec-fetch-dest`, `origin`/`referer`, host match) in `validate_allowed_domains` (`:422-454`); `allowed_domains` now emitted in `generic_links` (`:607-627`). Domain list is config-driven, not the upstream request-validation function; test-mode/empty-list skip at `:430`.
- `payout_attempt` fetch + unified code/message — SUBSTITUTED — `find_payout_attempt` over `repository().list_payout_attempts()` (`:190,456-468`); attempt `get_error_code`/`get_error_message` used directly with cache fallback (`:191-198`). No local unified-code translation catalog (documented substitute).
- `payout_link` lookup by `link_id` — SUBSTITUTED — still deterministic `"pl_<hash>"` (`:67`) but now persisted and validated through `repository().get_config`/`set_config` (`:65-76`); configs table substitutes for the absent link table.
- Customer and address fetch — SUBSTITUTED — `find_customer` via `repository().find_customer_by_string` (`:149,470-478`); no address entity/table, so required-field override derived from customer metadata `"address"` (`:154`) and payout-id fallback retained (`:156-158`).
- Payout-method filtering — SUBSTITUTED — `apply_payout_methods` (`:298,509-583`): enabled MCA enumeration (`:525`), config currency filter (`:517-524`), config/default method fallback (`:537-542`), cards-first stable sort + dedupe (`:544-547`), required-fields override from `has_address` (`:559`), `form_layout` from config (`:576-581`). Country filter is config-driven because `PayoutRecord` has no country field (`:522-523`).
- UI config defaults + wrapper — IMPLEMENTED — `config_value` reads `generic_link.payout_link` keys (`:340-345`); theme/logo/display_name/sdk_url from config (`:101-106`); `link_form_data`/`status_form_data` emit `js_data`/`css_data`/`sdk_url`/`html_meta_tags` (`:585-605`). Local config store is the config service substitute for `state.conf`; no hardcoding remains.
- `Invalidated` persistence — SUBSTITUTED — status now persisted via `repository().set_config(status_key_db, "invalidated")` (`:218-222`) and read back via `get_config` (`:169-176`); configs table substitutes for `update_payout_link`.
- `locale` + `allowed_domains` in `GenericLinks` — IMPLEMENTED — locale from query override → `global_config_value("locale")` → `"en"` (`:116-119`); domains from config (`:109`); both emitted by `generic_links` (`:607-627`).
- Auth model `MerchantIdAuth` — IMPLEMENTED — `authorize_operation` uses `auth_rules().rule_for("render_payout_link")`, rejects header/path merchant-id mismatch, binds principal to the path id, then `authorize_route` (`:42-48,354-374`); equivalent to upstream path-bound auth.
- Upstream v2 `initiate_payout_link` `todo!()` — DEFERRED — upstream has no implementation (`core/payout_link.rs:28-37`); no parity target.
