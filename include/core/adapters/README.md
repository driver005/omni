# core_adapters

Target `//include/core/adapters:core_adapters`, module `core_adapters`.
Contract: `docs/parity/behavior/CONTRACTS.md` §5.

## API

- `Config { base_url, api_key, workspace_id, from_address }`
- `Config from_env(std::string_view prefix)` (+ `stripe_from_env()`, `tinybird_from_env()`, `smtp_from_env()`, `webmention_from_env()`)
- `StripeAdapter`: `create_session`, `configure`, `branding`, `preview` → Ghost-shaped JSON `std::string` (each also has a `*_detailed()` returning `Result{status, body}`)
- `TinybirdAdapter`: `token`, `stats(pipe, params)` → JSON string (`*_detailed()` variants)
- `SmtpAdapter`: `bool send(message)` → bool; `send_detailed()` / `last_error_json()` carry the Ghost error envelope
- `WebmentionAdapter`: `fetch`, `source`, `target` → JSON string (`*_detailed()` variants)

Every failure returns a documented Ghost envelope (`{"errors":[{message,type,code?,context?}]}`):
`AdapterNotConfigured` (503), `AdapterTransportError` (502), `UpstreamError` (upstream status),
`AdapterEmptyResponse` / `AdapterInvalidJson` (502). No exceptions escape into callers.

## Env vars

`from_env("PREFIX")` reads first non-empty of:

| Field | Vars |
|---|---|
| `base_url` | `PREFIX_BASE_URL`, `PREFIX_URL`, `PREFIX_ENDPOINT` |
| `api_key` | `PREFIX_API_KEY`, `PREFIX_TOKEN`, `PREFIX_ADMIN_TOKEN` |
| `workspace_id` | `PREFIX_WORKSPACE_ID`, `PREFIX_SITE_UUID` |
| `from_address` | `PREFIX_FROM_ADDRESS`, `PREFIX_FROM` |

Contract vars: `STRIPE_BASE_URL`, `TINYBIRD_BASE_URL`, `SMTP_URL`, `WEBMENTION_ENDPOINT`,
plus matching `*_API_KEY` / `*_WORKSPACE_ID` / `*_FROM_ADDRESS`. No live credentials needed —
point the base URLs at local mock servers.

## Transports

- `SocketHttpTransport` (default): real HTTP/1.1 over POSIX socket to the configured mock URL.
- `CoreClientTransport`: uses `core::client::Client` builder + `core_client::Register` against an
  injected `io_layer_http2` `IClient` runtime; responses are correlated through the host's
  receive callback with a bounded timeout. Inject when the host owns the event loop.
- `MockHttpTransport`: in-process routes, used by `test.cc`.

## Outbox integration need

`core_outbox` (`//include/core/outbox:core_outbox`) does not exist yet. Until it lands,
`outbox_hook.cppm` records every outbound call in a thread-safe in-process log
(`recorded_calls()` / `clear_recorded_calls()`) and forwards it to an optional hook
(`set_outbound_hook()`). Once `core_outbox` lands, wire the hook to:

```cpp
core::adapters::set_outbound_hook([&outbox](const core::adapters::OutboundCall& call) {
    outbox.emit(core::outbox::Event{
        .type = "adapter.outbound",
        .payload = call.to_json(),
        .aggregate_id = call.operation,
    });
});
```

No shared files, central routes, or other agents' directories were modified.
