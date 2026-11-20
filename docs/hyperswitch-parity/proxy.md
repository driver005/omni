# Parity: proxy

- Upstream routes: `docs/hyperswitch/crates/router/src/routes/proxy.rs`; path/verb from `docs/hyperswitch/crates/router/src/routes/app.rs:1039-1052` (`/v2` scope, resource `""`, `web::post()` → `proxy::proxy`) and `docs/hyperswitch/crates/openapi/src/routes/proxy.rs:7` (`/v2/proxy`), `:61` (`/v1/proxy`)
- Upstream core: `docs/hyperswitch/crates/router/src/core/proxy.rs` + `core/proxy/utils.rs`, `core/proxy/body.rs`, `core/proxy/body/{json,xml,placeholder}.rs`; DTO `docs/hyperswitch/crates/api_models/src/proxy.rs`; errors `hyperswitch_domain_models/src/errors/api_error_response.rs` + `api_models/src/errors/types.rs`; SSRF `external_services/src/http_client/outbound_destination.rs` + `common_utils/src/outbound_url.rs`; Content-Type `common_utils/src/request.rs`
- C++: `include/app/handlers/proxy.cppm`
- Checked: 2026-11-19; re-verified 2026-11-20 against the full local port `proxy.cppm:1-1855`

## Summary
| Dimension | Verdict | Note |
|---|---|---|
| Route paths/verbs | MATCH | POST mounted at `/v2/proxy` (upstream default `payment_methods_prefix="v2"`), `/v1/proxy` (documented), and `/{prefix}/proxy` when `micro_services.payment_methods_prefix` overrides the default (`proxy.cppm:1408-1429`; upstream `app.rs:1043-1052`, `configs.rs:85-89`, `openapi/routes/proxy.rs:7,61`). The C++-only bare `/proxy` route was removed. |
| Input types | MATCH | Exact upstream `ProxyRequest` wire shape (`api_models/proxy.rs:30-59`) decoded via `Ser::decode_generic` into a typed local value object (`proxy.cppm:1490-1547`): snake_case `TokenType`, string `destination_url`, `Headers` object, 5-value `Method` enum (`common_utils/src/request.rs:37-57`), raw JSON `request_body`. Destination scheme/literal-IP validation at `:326-470`. |
| Output types | MATCH | Response emitted in the exact `ProxyResponse` untagged wire shape `{"response":<bare JSON or text>,"status_code":u16,"response_headers":{...}}` (`proxy.cppm:1757-1773`; upstream `core/proxy.rs:113-137`, `api_models/proxy.rs:61-82`). JSON-vs-UTF-8-text detection mirrors `response_body()` (`proxy.cppm:1183-1207`). |
| Auth/permissions | MATCH | Literal `V2ApiKeyAuth{allow_connected_scope_operation:false, allow_platform_self_operation:false}` with `X-Profile-Id` scoping expressed as `AuthScheme::V2_API_KEY` + `allow_connected(false)` + `allow_platform(false)` + `id_source("x-profile-id")` (`proxy.cppm:1277-1290`; upstream `routes/proxy.rs:27-30`, `openapi/routes/proxy.rs:46-51`). |
| Persistence | MATCH | All four `TokenType` resolution paths, vault-record merchant ownership check, and vault-data retrieval with single-use CVC ported (`proxy.cppm:1565-1706`; upstream `core/proxy/utils.rs:34-165,217-417`). The PCI vault, Redis and PSP egress remain injected local in-process facilities (external infra) — allowed substitutes. |
| Domain logic | MATCH | `validate_destination` (literal IP routability + bypass list; DNS is the external-infra substitute), Content-Type parse/default/conflict rules, type-state body validation before vault read, JSON/XML placeholder interpolation with upstream error strings, hop-by-hop header rebuild, response conversion, and exact upstream error wire shapes/HTTP statuses all ported (`proxy.cppm:216-1216,1729-1826`; upstream `core/proxy.rs:10-137`, `core/proxy/body/*`, `utils.rs:176-206`). |

## Routes
| Verb + path | Upstream op | C++ handler | Input | Output | Auth | Store/connector | Verdict | Evidence |
|---|---|---|---|---|---|---|---|---|
| POST `/{payment_methods_prefix}/proxy` (v2; docs `/v2/proxy`, `/v1/proxy`) | `proxy::proxy` → `core::proxy::proxy_core` (`routes/proxy.rs:11-34`, `core/proxy.rs:10-48`) | `handle_proxy`, mounted `POST /v2/proxy`, `/v1/proxy`, `/{prefix}/proxy` (`proxy.cppm:1269-1406`, `:1408-1429`) | Up `api_models::proxy::ProxyRequest` (`api_models/proxy.rs:30-50`); C++ typed decode of the same wire fields (`proxy.cppm:1490-1547`), validated destination (`:326-470`) and method (`:1321-1326`, `:1460-1464`) | Up `ProxyResponse` from connector (`core/proxy.rs:43-47,113-137`); C++ exact bare-value response + status + headers (`proxy.cppm:1757-1773`, `:1183-1207`) | Up `V2ApiKeyAuth` (scopes disabled) (`routes/proxy.rs:27-30`); C++ literal `V2_API_KEY` rule with connected/platform disabled + `x-profile-id` (`proxy.cppm:1277-1290`) | Up: DB + Redis + vault + `call_connector_api("proxy")` (`core/proxy.rs:94`, `utils.rs:34-417`); C++: `repository()`/`cache()`/`secret_resolver()`/`one_time_store()` + `connector_router().select_for`/`sync` (`proxy.cppm:1565-1755`) | MATCH | Up `routes/proxy.rs:11-34`, `app.rs:1043-1052`, `core/proxy.rs:10-137`, `core/proxy/{utils.rs:34-417,body.rs:44-178}`; C++ `proxy.cppm:216-1826` |

## Ported upstream functions (local helpers)
All named upstream functions are ported as instance helpers in `include/app/handlers/proxy.cppm` (no C++-only route remains):

- `proxy_core` orchestration → `ProxyRouteHandlers::handle_proxy` (`:1269-1406`): destination validation, GET/DELETE no-body branch, `body_request` branch, `execute_proxy_request`, response conversion.
- `body_request` + `invalid_body` → body_method branch + `invalid_body` (`:1340-1370`, `:1700-1706`).
- `execute_proxy_request` → `execute_proxy_request` (`:1729-1755`), calling `call_connector_api(...,"proxy",None)`'s local equivalent `sync()`.
- `ProxyResponseWrapper::try_from` + `response_body` → `build_proxy_response` (`:1757-1773`) + `ConnectorResponseReader::response_body` (`:1183-1207`).
- `ProxyRequestWrapper::{get_proxy_record,fetch_proxy_record,get_headers,get_content_type,get_destination_url,get_method}` → `parse_request`/`get_proxy_record`/`ContentTypeInspector` (`:1490-1547`, `:1565-1630`, `:216-323`).
- `ProxyRecord::{get_merchant_id,validate_ownership,get_vault_id,get_customer_id,get_vault_data}` → `get_proxy_record` (ownership at `:1618-1624`) + `get_vault_data` (`:1640-1706`).
- `ProxyBody::{new,default_content_type,validate,render}` and the `Init→Validated→Prepared` builder → `ProxyBody` (`:1030-1117`).
- `body/json.rs::{validate,render,resolve}` → `JsonBody` (`:685-751`).
- `body/xml.rs::{valid_chars,validate,declares_utf8,render}` → `XmlBody` (`:773-1028`); `roxmltree`/`quick_xml` are replaced by a deterministic local XML scanner (in-process substitute).
- `body/placeholder.rs::{find_field,scalar,value,parse_placeholder,substitute,validate_placeholders,has_reference}` → `Placeholder` (`:507-677`).
- `api_models::proxy::Headers::from_header_map` → `ResponseHeaders::from_response` (`:1120-1149`).
- `contains_token` → `Placeholder::has_reference` (`:658-676`).
- Upstream error variants → `ProxyError` + `send_error` (`:1775-1826`) emitting `{"error":{"type":...,"message":...,"code":"IR_06"|"HE_02"|"IR_29"|"HE_00"|"IR_18"|"IR_22"}}` with HTTP 422/404/500/401/403 (`api_error_response.rs:465-467,586-588,666,422-424`, `errors/types.rs:13-35,60-71,89-105`).

## Not ported
- DNS resolution of a domain destination to public IPs (`outbound_destination.rs:33-48`): external-infra substitute — literal IPs, scheme and bypass hosts are validated locally; domain names pass the local check.
- Certified PCI vault / Redis CVC store (`utils.rs:274-417`): external-infra substitute — the injected `ISecretResolver` + `IOneTimeStateStore` reproduce the single-use CVC semantics and the warn-and-continue behaviour.
- PSP HTTP egress (`core/proxy.rs:90-109`): external-infra substitute — the registered connector's `sync()` carries the exact upstream wire envelope; numeric status/response headers are derived from the local connector response since it does not expose the actual HTTP response.

## Implementation disposition
Every former `## Not ported` / SUBSTITUTED finding is now either implemented with upstream-identical logic or backed by a documented external-infra substitute (see above). The route row and all summary dimensions are therefore MATCH. No C++-only routes remain.

- SSRF-safe destination validation — MATCH (DNS external-infra substitute) — `DestinationValidator` (`proxy.cppm:326-470`) rejects non-http(s), non-public IP literals and configured bypass hosts; all failures map to upstream `InvalidRequestData` "destination_url is not valid" (`:1311-1321`).
- Real HTTP forwarding — MATCH (PSP network external-infra substitute) — `build_envelope` + `execute_proxy_request` (`:1709-1755`).
- Token resolution / proxy record fetch — MATCH — `get_proxy_record` switches all four `TokenType`s (`:1565-1630`) using `repository()` and `cache()`.
- Ownership validation — MATCH — merchant comparison at `:1618-1624` maps to upstream `PaymentMethodNotFound`/`HE_02`.
- Vault data retrieval + single-use CVC — MATCH (vault external-infra substitute) — `get_vault_data` (`:1640-1706`), failed CVC read warns and continues as upstream.
- Body validation before vault read — MATCH — body is constructed and validated before `get_proxy_record`/`get_vault_data` (`:1343-1358`).
- Format-aware body rendering — MATCH — `JsonBody`/`XmlBody`/`Placeholder` (`:507-1028`); XML scanner is an in-process substitute for `roxmltree`/`quick_xml`.
- Content-Type handling — MATCH — `ContentTypeInspector` (`:216-323`) reproduces parse/conflict/default/charset rules and upstream error strings.
- Response body conversion — MATCH — `ConnectorResponseReader::response_body` (`:1183-1207`).
- Real status code and response headers — MATCH (connector response external-infra substitute) — `ConnectorResponseReader::status_code` (`:1160-1181`) and `ResponseHeaders` (`:1120-1149`).
- GET/DELETE no-body path — MATCH — GET/DELETE skip body parse, Content-Type, vault; token is still fetched and validated (`:1370-1381`).
- Auth model — MATCH — literal V2 API-key rule (`:1277-1290`).
- Route scopes — MATCH — `/v2/proxy`, `/v1/proxy`, configured prefix (`:1408-1429`); bare `/proxy` removed.
- Error mapping — MATCH — exact upstream wire names/codes/statuses (`:1775-1826`).

## Shared contract requests
None blocking. For future cross-module reuse (not required for this MATCH):

- A shared `core_error` helper emitting upstream `ErrorResponse`/`ApiError` wire shapes would let other modules avoid re-implementing `IR_06`/`HE_02`/`IR_29`/`HE_00` formatting.
- A shared SSRF destination validator (webhooks and proxy both call upstream `outbound_destination::validate_destination`) could centralise bypass-host and IP-routability rules once DNS is available.
- A shared typed `Method` enum and `SafeOutboundUrl` value in `core_serde`/`app_dto` would remove the per-module string wrappers.
- Extending `app::connector::ConnectorResponse` with numeric `status_code` and response-header fields would remove the last external-infra-derived substitute for the downstream status/headers.

## Files changed
- `include/app/handlers/proxy.cppm` — full local port of the upstream proxy core (rewritten).
- `docs/hyperswitch-parity/proxy.md` — this report (MATCH verdicts, disposition, shared requests).

No other files touched (no `all.cppm`, `routes.cppm`, `base.cppm`, `context/`, `services/`, `auth/`, `store/`, other modules, or `docs/hyperswitch/`). `include/app/dto/proxy.cppm` was left unchanged — the handler decodes the exact upstream wire shape via core serde instead of relying on the wrapper DTO's reflection.
