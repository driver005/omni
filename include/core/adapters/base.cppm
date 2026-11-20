// Primary module interface for core_adapters.
//
// Contract: docs/parity/behavior/CONTRACTS.md section 5.
// All outbound calls go through a synchronous HttpTransport. Two transports ship:
//   * SocketHttpTransport  — real HTTP/1.1 over a POSIX socket, used against local mock URLs.
//   * CoreClientTransport  — uses core::client::Client builder + io_layer_http2 runtime when the
//     host owns the event loop (the transport cannot pump it itself).
// Tests inject MockHttpTransport, so no live credentials or network are needed.
export module core_adapters;

export import :config;
export import :json;
export import :outbox_hook;
export import :http;
export import :client_bridge;
export import :stripe;
export import :tinybird;
export import :smtp;
export import :webmention;
