export module core_adapters:client_bridge;

import std;
import interfaces;
import core_client;
import :http;

export namespace core::adapters {

/**
 * @brief HttpTransport backed by the core_client builder + an io_layer_http2 IClient runtime.
 *
 * The adapters do not own an event loop, so responses are correlated through
 * core::client::Register::dispatch() driven by the host's receive callback, and send()
 * blocks on a promise with a bounded timeout. Inject this transport when a live HTTP/2
 * runtime exists (e.g. the worker host); otherwise the default SocketHttpTransport is used.
 */
class CoreClientTransport final : public HttpTransport
{
public:
    explicit CoreClientTransport(
        interfaces::IClient& runtime,
        std::chrono::milliseconds timeout = std::chrono::milliseconds{5'000}
    ) :
        m_runtime{runtime},
        m_timeout{timeout}
    {
    }

    [[nodiscard]] HttpResponse send(const HttpRequest& request) override
    {
        HttpResponse failure;
        const auto parsed = parse_url(request.url);
        if (!parsed) {
            failure.error = std::format("invalid URL for core_client transport: {}", request.url);
            return failure;
        }

        std::string target = parsed->path;
        if (!parsed->query.empty()) {
            target += '?';
            target += parsed->query;
        }

        // core_client builder stamps method/path/headers/body onto the runtime's concrete
        // request type — no protocol type is named here.
        auto builder = core::client::Client::custom(request.method, target);
        for (const auto& [name, value]: request.headers) {
            builder.add_header(name, value);
        }
        builder.add_header("Host", parsed->authority);
        builder.add_body(request.body);
        auto wire_request = builder.build(m_runtime);

        auto promise = std::make_shared<std::promise<HttpResponse>>();
        auto future = promise->get_future();
        core::client::Register registry;
        registry.set_runtime(m_runtime);
        registry.send(
            std::move(wire_request),
            [promise](interfaces::io::IResponse& response)
            {
                HttpResponse result;
                result.status =
                    static_cast<int>(interfaces::io::types::status_code(response.get_status()));
                for (const std::byte byte: response.get_body()) {
                    result.body.push_back(static_cast<char>(byte));
                }
                promise->set_value(std::move(result));
            }
        );

        if (future.wait_for(m_timeout) != std::future_status::ready) {
            failure.error =
                std::format("core_client response timeout after {}ms", m_timeout.count());
            return failure;
        }
        return future.get();
    }

private:
    interfaces::IClient& m_runtime;
    std::chrono::milliseconds m_timeout;
};

} // namespace core::adapters
