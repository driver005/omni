// Verifies that every Hyperswitch route registered by AppRoutes is actually
// dispatchable by the compiled core_router trie. The parity docs mark routes as
// MATCH from static source comparison only; this test is the runtime half.
//
// Every (method, path) pair actually present in the compiled trie is enumerated,
// path params replaced with concrete values, and RouteHandler::match() called on
// each. A route that resolves to the trie but has no handler, or that falls out
// of the trie entirely, is reported with its full path and exact failure reason.

import std;
import interfaces;
import app_handlers;
import core_router;
import utils_openapi;
import io_layer_http2;

namespace {

enum class Outcome
{
    Ok,
    HandlerRan,
    Unrouted,
    NoHandler
};

Outcome classify(const std::exception& error)
{
    std::string_view message{error.what()};
    if (message.starts_with("Route not found")) {
        return Outcome::Unrouted;
    }
    if (message.starts_with("Wrong method for route")) {
        return Outcome::NoHandler;
    }
    return Outcome::HandlerRan; // handler itself threw — routing worked
}

} // namespace

int main()
{
    // Build the real Hyperswitch route table exactly like app_api_server does.
    // RouterContext holds a 4096-element array of heavyweight Route objects, so keep it
    // off the stack: the default 8 MiB test stack cannot hold one.
    auto handlers = std::make_unique<app::api::AppHandlers>();
    auto context = std::make_unique<core::router::RouterContext<>>();
    app::api::AppRoutes{}.install(*context, *handlers);

    const auto& registry = utils::openapi::Registry::get_routes();
    auto route_handler = core::router::RouteBuilder{}.build(std::move(*context));

    if (std::getenv("ROUTE_DUMP") != nullptr) {
        for (const auto& r: route_handler.enumerate_routes()) {
            std::println("{} {}", interfaces::io::types::method_str(r.method), r.path);
        }
        return 0;
    }

    std::size_t total{0};
    std::size_t routed{0};
    std::size_t unrouted{0};
    std::size_t no_handler{0};
    std::size_t handler_errors{0};

    // Paths the Control Center actually calls during signup/login/onboarding. The
    // compiled trie can be internally consistent yet still register doubled prefixes
    // (e.g. /user/user/signup), so check these explicitly as well.
    static constexpr std::array<std::pair<std::string_view, std::string_view>, 9> required{{
        {"GET", "/user/auth/list"},
        {"POST", "/user/signup"},
        {"POST", "/user/v2/signin"},
        {"POST", "/user/verify_email"},
        {"GET", "/user/data"},
        {"POST", "/user/create_merchant"},
        {"POST", "/user/switch/org"},
        {"GET", "/user/merchant_details"},
        {"GET", "/v2/user/list/merchant"},
    }};
    std::size_t missing_required{0};

    // Enumerate the compiled trie itself: the OpenAPI Registry has no entries for
    // intermediate segments of multi-segment Router paths, so it cannot reconstruct
    // every full path accurately.
    std::size_t registry_operations{0};
    for (const auto& route: registry) {
        registry_operations += route.get_operations().size();
    }

    const auto compiled = route_handler.enumerate_routes();
    for (const auto& [want_method, want_path]: required) {
        if (std::ranges::none_of(
                compiled,
                [&](const auto& entry)
                {
                    return interfaces::io::types::method_str(entry.method) == want_method &&
                           entry.path == want_path;
                }
            )) {
            ++missing_required;
            std::println("MISSING_REQUIRED {} {}", want_method, want_path);
        }
    }

    for (const auto& registered: compiled) {
        const auto method = registered.method;
        const std::string method_name{interfaces::io::types::method_str(method)};
        if (method_name.empty()) {
            continue;
        }

        std::string path = registered.path;
        for (;;) {
            const auto pos = path.find("{param}");
            if (pos == std::string::npos) {
                break;
            }
            path.replace(pos, 7, "1");
        }
        ++total;

        io::layer::http2::HttpRequest req{1};
        io::layer::http2::HttpResponse res{1};
        req.set_header(interfaces::io::types::Token::METHOD, method_name);
        req.set_header(interfaces::io::types::Token::PATH, path);
        req.set_header(interfaces::io::types::Token::CONTENT_TYPE, "application/json");
        req.set_header(interfaces::io::types::Token::ACCEPT, "application/json");

        bool sent = false;
        const auto send = [&sent]
        {
            sent = true;
        };
        try {
            route_handler.match(method, path, req, res, send);
            ++routed;
        } catch (const std::exception& error) {
            switch (classify(error)) {
                case Outcome::HandlerRan:
                    ++handler_errors;
                    break;
                case Outcome::Unrouted:
                    ++unrouted;
                    std::println("UNROUTED {} {} — {}", method_name, path, error.what());
                    break;
                case Outcome::NoHandler:
                    ++no_handler;
                    std::println("NO_HANDLER {} {} — {}", method_name, path, error.what());
                    break;
                default:
                    break;
            }
        }
    }

    std::println("registry operations: {} | trie dispatch entries: {}", registry_operations, total);
    std::println(
        "probes: {} | routed: {} | unrouted: {} | no handler: {} | handler threw: {}",
        total,
        routed,
        unrouted,
        no_handler,
        handler_errors
    );

    return (unrouted == 0 && no_handler == 0 && missing_required == 0) ? 0 : 1;
}
