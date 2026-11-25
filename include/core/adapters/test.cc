#include <arpa/inet.h>
#include <cstdlib>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

import std;
import core_adapters;

using namespace core::adapters;

namespace {

class Check
{
public:
    template<typename A, typename B>
    void equal(const A& actual, const B& expected, std::string_view label)
    {
        if (actual != expected) {
            std::println(
                std::cerr,
                "FAIL {}: expected '{}', got '{}'",
                label,
                std::format("{}", expected),
                std::format("{}", actual)
            );
            ++m_failures;
        }
    }

    void yes(bool condition, std::string_view label)
    {
        if (!condition) {
            std::println(std::cerr, "FAIL {}", label);
            ++m_failures;
        }
    }

    int failures() const noexcept
    {
        return m_failures;
    }

private:
    int m_failures{0};
};

// Tiny local HTTP mock server — exercises SocketHttpTransport with a real socket.
class LocalServer
{
public:
    LocalServer()
    {
        m_fd = ::socket(AF_INET, SOCK_STREAM, 0);
        int reuse = 1;
        ::setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        address.sin_port = 0;
        ::bind(m_fd, reinterpret_cast<sockaddr*>(&address), sizeof(address));
        ::listen(m_fd, 4);
        sockaddr_in bound{};
        socklen_t length = sizeof(bound);
        ::getsockname(m_fd, reinterpret_cast<sockaddr*>(&bound), &length);
        m_port = ntohs(bound.sin_port);
        m_thread = std::thread{
            [this]
            {
                while (!m_stop.load(std::memory_order_relaxed)) {
                    const int client = ::accept(m_fd, nullptr, nullptr);
                    if (client < 0) {
                        break;
                    }
                    char buffer[4'096];
                    ::recv(client, buffer, sizeof(buffer), 0);
                    static constexpr std::string_view body =
                        R"({"checkout_config":[{"design":{"customize":false}}]})";
                    const auto response = std::format(
                        "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: "
                        "{}\r\nConnection: close\r\n\r\n{}",
                        body.size(),
                        body
                    );
                    ::send(client, response.data(), response.size(), 0);
                    ::close(client);
                }
            }
        };
    }

    ~LocalServer()
    {
        m_stop.store(true, std::memory_order_relaxed);
        // Unblock accept() with a self-connect.
        const int client = ::socket(AF_INET, SOCK_STREAM, 0);
        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        address.sin_port = htons(m_port);
        ::connect(client, reinterpret_cast<sockaddr*>(&address), sizeof(address));
        ::close(client);
        m_thread.join();
        ::close(m_fd);
    }

    [[nodiscard]] std::string url() const noexcept
    {
        return std::format("http://127.0.0.1:{}", m_port);
    }

private:
    int m_fd{-1};
    std::uint16_t m_port{0};
    std::atomic<bool> m_stop{false};
    std::thread m_thread;
};

std::shared_ptr<MockHttpTransport> make_mock()
{
    auto transport = std::make_shared<MockHttpTransport>();
    transport->route(
        "POST",
        "/v1/checkout/sessions",
        [](const HttpRequest&)
        {
            return HttpResponse{.status = 200, .body = R"({"id":"cs_test_1"})"};
        }
    );
    transport->route(
        "PUT",
        "/v1/checkout/config",
        [](const HttpRequest&)
        {
            return HttpResponse{
                .status = 200,
                .body = R"({"checkout_config":[{"design":{"customize":true}}]})"
            };
        }
    );
    transport->route(
        "GET",
        "/v1/checkout/branding",
        [](const HttpRequest&)
        {
            return HttpResponse{.status = 200, .body = R"({"displayName":"Ghost","design":null})"};
        }
    );
    transport->route(
        "POST",
        "/v1/checkout/preview",
        [](const HttpRequest&)
        {
            return HttpResponse{
                .status = 200,
                .body = R"({"checkout_preview":[{"url":"https://checkout.stripe.com/c/pay/x"}]})"
            };
        }
    );
    transport->route(
        "GET",
        "/v0/token",
        [](const HttpRequest&)
        {
            return HttpResponse{.status = 200, .body = R"({"tinybird":{"token":"tb_1"}})"};
        }
    );
    transport->route(
        "GET",
        "/v0/pipes/",
        [](const HttpRequest&)
        {
            return HttpResponse{.status = 200, .body = R"({"data":[{"visits":42}]})"};
        }
    );
    transport->route(
        "POST",
        "/send",
        [](const HttpRequest&)
        {
            return HttpResponse{.status = 202};
        }
    );
    transport->route(
        "POST",
        "webmention",
        [](const HttpRequest&)
        {
            return HttpResponse{.status = 202};
        }
    );
    transport->route(
        "GET",
        "?target=",
        [](const HttpRequest&)
        {
            return HttpResponse{.status = 200, .body = R"({"webmentions":[]})"};
        }
    );
    transport->route(
        "GET",
        "?url=",
        [](const HttpRequest&)
        {
            return HttpResponse{
                .status = 200,
                .body = R"({"webmention":{"endpoint":"https://webmention.io/webmention"}})"
            };
        }
    );
    return transport;
}

} // namespace

int main()
{
    Check check;

    // 1. Config from env (mock base URLs, no live credentials).
    ::setenv("STRIPE_BASE_URL", "http://127.0.0.1:1", 1);
    ::setenv("STRIPE_API_KEY", "sk_test", 1);
    ::setenv("TINYBIRD_BASE_URL", "http://127.0.0.1:2", 1);
    ::setenv("TINYBIRD_WORKSPACE_ID", "ws_1", 1);
    ::setenv("SMTP_URL", "http://127.0.0.1:3", 1);
    ::setenv("SMTP_FROM_ADDRESS", "ghost@example.test", 1);
    ::setenv("WEBMENTION_ENDPOINT", "http://127.0.0.1:4", 1);
    const auto stripe_cfg = stripe_from_env();
    check.equal(stripe_cfg.base_url, "http://127.0.0.1:1", "STRIPE_BASE_URL");
    check.equal(stripe_cfg.api_key, "sk_test", "STRIPE_API_KEY");
    const auto tinybird_cfg = tinybird_from_env();
    check.equal(tinybird_cfg.base_url, "http://127.0.0.1:2", "TINYBIRD_BASE_URL");
    check.equal(tinybird_cfg.workspace_id, "ws_1", "TINYBIRD_WORKSPACE_ID");
    check.equal(smtp_from_env().base_url, "http://127.0.0.1:3", "SMTP_URL");
    check.equal(smtp_from_env().from_address, "ghost@example.test", "SMTP_FROM_ADDRESS");
    check.equal(webmention_from_env().base_url, "http://127.0.0.1:4", "WEBMENTION_ENDPOINT");

    // 2. JSON helpers.
    check.equal(
        ghost_error("NoPermissionError", "denied"),
        R"({"errors":[{"message":"denied","type":"NoPermissionError"}]})",
        "ghost_error"
    );
    check.yes(valid_json(R"({"a":"x"} )"), "valid_json object");
    check.yes(!valid_json(R"({"a":)"), "invalid_json rejected");

    // 3. Adapter happy paths against the in-process mock.
    clear_recorded_calls();
    std::size_t hook_calls = 0;
    set_outbound_hook(
        [&hook_calls](const OutboundCall&)
        {
            ++hook_calls;
        }
    );
    auto transport = make_mock();
    const Config stripe{"http://127.0.0.1:1", "sk_test", "", ""};
    StripeAdapter stripe_adapter{stripe, transport};
    check.equal(
        stripe_adapter.create_session(R"({"tier_id":"t1"})"),
        R"({"id":"cs_test_1"})",
        "stripe create_session"
    );
    check.equal(
        stripe_adapter.configure(R"({"design":{}})"),
        R"({"checkout_config":[{"design":{"customize":true}}]})",
        "stripe configure"
    );
    check.yes(stripe_adapter.branding().contains("Ghost"), "stripe branding");
    check.yes(
        stripe_adapter.preview(R"({"tier_id":"t1"})").contains("checkout_preview"),
        "stripe preview"
    );

    const Config tinybird_cfg2{"http://127.0.0.1:2", "tb_key", "ws_1", ""};
    TinybirdAdapter tinybird_adapter{tinybird_cfg2, transport};
    check.equal(tinybird_adapter.token(), R"({"tinybird":{"token":"tb_1"}})", "tinybird token");
    check.yes(tinybird_adapter.stats("visits", "range=7d").contains("visits"), "tinybird stats");

    const Config smtp_cfg{"http://127.0.0.1:3", "", "", "ghost@example.test"};
    SmtpAdapter smtp_adapter{smtp_cfg, transport};
    check.yes(smtp_adapter.send("hello"), "smtp send 202 accepted");
    check.equal(smtp_adapter.last_error_json(), "", "smtp no error on success");

    const Config webmention_cfg{"http://127.0.0.1:4", "", "", ""};
    WebmentionAdapter webmention_adapter{webmention_cfg, transport};
    check.yes(
        webmention_adapter.source("https://a.test", "https://b.test").contains("webmention"),
        "webmention source"
    );
    check.yes(
        webmention_adapter.target("https://b.test").contains("webmentions"),
        "webmention target"
    );
    check.yes(
        webmention_adapter.fetch("https://b.test").contains("webmention"),
        "webmention fetch"
    );

    const auto calls = recorded_calls();
    check.equal(calls.size(), 10U, "outbox recorded every outbound call");
    check.equal(hook_calls, 10U, "external outbox hook fired for every call");
    for (const auto& call: calls) {
        if (!call.ok || call.status < 200 || call.status >= 300) {
            std::println(
                std::cerr,
                "  diag: {}/{} status={} ok={} error='{}'",
                call.adapter,
                call.operation,
                call.status,
                call.ok,
                call.error
            );
        }
    }
    check.yes(
        std::ranges::all_of(
            calls,
            [](const OutboundCall& call)
            {
                return call.ok && call.status >= 200 && call.status < 300;
            }
        ),
        "all recorded calls succeeded"
    );
    check.yes(
        std::ranges::any_of(
            calls,
            [](const OutboundCall& call)
            {
                return call.adapter == "smtp" && call.operation == "send" &&
                       call.request_body.contains("ghost@example.test");
            }
        ),
        "smtp From address recorded"
    );

    // 4. Failure paths return Ghost-shaped errors, never crash.
    auto failing = std::make_shared<MockHttpTransport>();
    failing->set_default(
        [](const HttpRequest&)
        {
            HttpResponse response;
            response.error = "connection refused";
            return response;
        }
    );
    StripeAdapter failing_stripe{stripe, failing};
    const auto failure_body = failing_stripe.branding();
    check.yes(
        failure_body.contains("\"errors\"") && failure_body.contains("AdapterTransportError"),
        "transport failure is Ghost-shaped"
    );
    check.equal(failing_stripe.branding_detailed().status, 502, "transport failure status");

    auto http_error = std::make_shared<MockHttpTransport>();
    http_error->set_default(
        [](const HttpRequest&)
        {
            return HttpResponse{.status = 500, .body = "boom"};
        }
    );
    const auto upstream = TinybirdAdapter{tinybird_cfg2, http_error}.token();
    check.yes(
        upstream.contains("UpstreamError") && upstream.contains("http_500"),
        "HTTP 500 Ghost error"
    );
    check.yes(upstream.contains("boom"), "error context carries upstream body");

    // 5. Unconfigured adapters return a Ghost error without any outbound call.
    clear_recorded_calls();
    StripeAdapter unconfigured;
    check.yes(unconfigured.branding().contains("AdapterNotConfigured"), "unconfigured Ghost error");
    check.equal(recorded_calls().size(), 0U, "no outbound call when unconfigured");
    set_outbound_hook({});

    // 6. Real HTTP against a local socket mock server.
    LocalServer server;
    StripeAdapter real_adapter{Config{server.url(), "", "", ""}};
    const auto real_body = real_adapter.branding();
    check.yes(real_body.contains("checkout_config"), "real socket HTTP round-trip");
    check.yes(
        std::ranges::any_of(
            recorded_calls(),
            [](const OutboundCall& call)
            {
                return call.adapter == "stripe" && call.operation == "branding" && call.ok;
            }
        ),
        "real socket call recorded in outbox"
    );

    if (check.failures() == 0) {
        std::println("core_adapters: all checks passed");
    }
    return check.failures() == 0 ? 0 : 1;
}
