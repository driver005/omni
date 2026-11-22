# Hyperswitch Control Center — backed by the C++ port

Upstream UI cloned from `https://github.com/juspay/hyperswitch-control-center` (HEAD
`cc72991b13d749c3154952b1724f2e08fe5c5b1c`) into `docs/hyperswitch-control-center`.

## What is running

- **C++ server** (`//include/app/testing:app_api_server`) on `localhost:8080`, built from the
  real port (`AppHandlers` + `AppRoutes`) with an in-memory `AppContext`. No mock API.
- **UI dev server** (`npm start`, webpack-dev-server) on `localhost:9000`, serving the built
  dashboard. Its `/api` proxy targets `http://localhost:8080` — the C++ server.
- **Public tunnel** (cloudflared) exposing `:9000`. URL is in `/tmp/cf-tunnel3.log`; re-read
  with `grep -oE 'https://[a-z0-9-]+\.trycloudflare\.com' /tmp/cf-tunnel3.log | head -1`
  (a fresh URL is issued each time cloudflared restarts).

## Verified

- `GET /api/health` through the public tunnel returns `health is good` — served by the
  C++ `HealthRouteHandlers`.
- `GET /api/payments/list` returns the C++ port's `{"error":"UNAUTHORIZED",...}` auth
  response (real handler, not a mock).

## Router fixes needed to make the port runnable

`install()` had never been executed before, so latent core-router bugs surfaced:

- `include/core/router/builder.cppm`: accept empty paths (`web::resource("")`), widen
  `RouterContext` capacities (4096), and fix the child-offset calculation
  (`next_row_start + cumulative_children`, advancing `flat_index` per row).
- `include/core/router/router.cppm`: widen `RouteHandler` capacities (4096), widen
  `current_index`/`m_table_index`/`children_length` to `size_t` (the 717-node table
  overflowed the old `uint8_t`), and add a fallback that matches the empty-path
  scope-root child when a request path ends at a scope.

## Restart after a container restart

```sh
cd /workspaces/omni
(ulimit -s unlimited; nohup ./bazel-bin/include/app/testing/app_api_server 8080 > /tmp/appsrv-final.log 2>&1 &)
cd docs/hyperswitch-control-center
nohup npm start > /tmp/hcc-ui-final.log 2>&1 &
curl -fsSL -o /tmp/cloudflared https://github.com/cloudflare/cloudflared/releases/latest/download/cloudflared-linux-amd64 && chmod +x /tmp/cloudflared
nohup /tmp/cloudflared tunnel --no-autoupdate --url http://localhost:9000 > /tmp/cf-tunnel3.log 2>&1 &
```

Note: `ulimit -s unlimited` is required for the C++ server (large router template
arrays on the stack).
