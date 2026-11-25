export module app_handlers_proxy:runtime_proxy;

import std;
import core_serde;
import core_error;
import app_context;
import app_metrics;

import :input;
import :result;
import :runtime_common;

export namespace app::handlers::proxy {

/**
 * @brief Single route sub-runtime: port of `proxy_core` (`core/proxy.rs:24-137`).
 *
 * All request data arrives in ProxyInput; all outcomes are a typed ProxyResult
 * document or a typed AppError. No transport types are referenced here.
 */
class ProxyForwardRuntime
{
public:
    explicit ProxyForwardRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<ProxyResult, core::error::AppError>
    proxy(const ProxyInput& input) const
    {
        // proxy_core step 1: validate_destination() -> InvalidRequestData "destination_url is not
        // valid".
        const auto bypass_hosts = m_context.config_store()
                                      .get_value("proxy", "bypass_proxy_hosts")
                                      .value_or(std::string{});
        if (DestinationValidator{}(input.request.destination_url, bypass_hosts)) {
            return std::unexpected{m_support.make_error(
                m_errors(ProxyError::Kind::InvalidRequestData, "destination_url is not valid")
            )};
        }

        // Upstream Method is a five-value enum (common_utils/src/request.rs:37-57).
        const auto method = TextTools{}.upper(input.request.method);
        if (!m_support.is_supported_method(method)) {
            return std::unexpected{m_support.make_error(
                m_errors(ProxyError::Kind::InvalidRequestData, "method is not valid")
            )};
        }

        std::string rendered_bytes{};
        std::unordered_map<std::string, std::string> outbound_headers{};
        const bool body_method = method != "GET" && method != "DELETE";

        std::optional<ProxyRecordValue> record{};
        if (body_method) {
            // Upstream body_request(): parse Content-Type, then construct and validate
            // the body before fetching the proxy record or reading the vault.
            auto content = ContentTypeInspector{}(input.request.headers);
            if (!content) {
                return std::unexpected{m_support.make_error(std::move(content.error()))};
            }
            auto made = ProxyBody{}.make(input.request.request_body, *content);
            if (!made) {
                return std::unexpected{m_support.make_error(m_support.invalid_body(made.error()))};
            }
            auto proxy_body = std::move(*made);
            if (auto result = proxy_body.validate(); !result) {
                return std::unexpected{
                    m_support.make_error(m_support.invalid_body(result.error()))
                };
            }
            auto resolved = m_support.get_proxy_record(
                input.request.token_type,
                input.request.token,
                input.merchant_id
            );
            if (!resolved) {
                return std::unexpected{m_support.make_error(std::move(resolved.error()))};
            }
            record = *std::move(resolved);
            auto vault = m_support.get_vault_data(*record);
            if (!vault) {
                return std::unexpected{m_support.make_error(std::move(vault.error()))};
            }
            auto built = proxy_body.build(*vault, input.request.headers);
            if (!built) {
                return std::unexpected{m_support.make_error(m_support.invalid_body(built.error()))};
            }
            rendered_bytes = std::move(built->bytes);
            outbound_headers = std::move(built->headers);
        } else {
            // Upstream GET/DELETE: fetch and validate the token but send no body and
            // never read the vault; attach default headers plus the caller's headers.
            auto resolved = m_support.get_proxy_record(
                input.request.token_type,
                input.request.token,
                input.merchant_id
            );
            if (!resolved) {
                return std::unexpected{m_support.make_error(std::move(resolved.error()))};
            }
            record = *std::move(resolved);
            for (const auto& [name, value]: input.request.headers) {
                outbound_headers[TextTools{}.lower(name)] = value;
            }
        }

        // proxy_core step 3: execute_proxy_request() via call_connector_api("proxy").
        const auto envelope = m_support.build_envelope(
            input.request.destination_url,
            method,
            outbound_headers,
            rendered_bytes,
            body_method
        );
        auto response = m_support.execute_proxy_request(
            input.merchant_id,
            record->vault_id.empty() ? input.request.token : record->vault_id,
            envelope
        );
        if (!response) {
            return std::unexpected{m_support.make_error(std::move(response.error()))};
        }

        // proxy_core step 4: ProxyResponse::try_from(ProxyResponseWrapper(res)).
        auto document = m_support.build_proxy_response(*response);
        if (!document) {
            return std::unexpected{m_support.make_error(std::move(document.error()))};
        }

        m_context.metrics().record_flow("proxy");
        m_context.metrics().counter("proxy.proxy").increment();
        return ProxyResult{std::move(*document)};
    }

private:
    app::context::AppContext& m_context;
    ProxyRuntimeSupport m_support;
    ProxyErrorBuilder m_errors{};
};

} // namespace app::handlers::proxy
