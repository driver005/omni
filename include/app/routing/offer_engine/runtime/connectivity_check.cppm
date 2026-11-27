export module app_routing:offer_engine_runtime_connectivity_check;

import std;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import core_error;
import app_auth;
import app_http;
import app_crypto;
import app_metrics;
import app_connector;
import app_services;

import :offer_engine_input;
import :offer_engine_result;
import :offer_engine_runtime_common;

export namespace app::handlers::offer_engine {

class ConnectivityCheckRuntime
{
public:
    explicit ConnectivityCheckRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    /** @brief Port of `check_offer_engine_connectivity(state)` (connectivity.rs:26-72). */
    [[nodiscard]] std::expected<ConnectivityCheckResponse, core::error::AppError>
    connectivity_check(const ConnectivityCheckInput& input) const
    {
        (void)input;
        m_context.metrics().record_flow("OfferEngineConnectivityCheck");
        m_context.metrics().counter("OFFER_ENGINE_CONNECTIVITY_CHECK").increment();

        // Global dimensions, so no merchant/profile context is folded in.
        const OfferEngineDimensions global{};
        const auto source = m_support.resolve_offer_engine_credential_source(global);

        std::optional<std::expected<ResolvedOfferEngineConfig, OfferEngineError>> resolved{};
        switch (source) {
            case ::app::dto::offer_engine::OfferEngineCredentialSource::None:
                break;
            case ::app::dto::offer_engine::OfferEngineCredentialSource::Application:
                resolved = m_support.resolve_application_offer_config();
                break;
            case ::app::dto::offer_engine::OfferEngineCredentialSource::Merchant:
                resolved = std::unexpected{OfferEngineError{
                    OfferEngineError::Kind::MissingMerchantConfig,
                    "credential source is 'merchant' but no merchant context is available in a "
                    "global connectivity check"
                }};
                break;
        }

        ConnectivityCheckResponse response{};
        response.set_reachable(std::nullopt);
        response.set_status_code(std::nullopt);

        if (resolved.has_value() && !resolved->has_value()) {
            // Some(Err(err)) -> detail "Offer Engine config could not be resolved: {err:?}"
            response.set_enabled(false);
            response.set_detail(
                std::format(
                    "Offer Engine config could not be resolved: {}",
                    resolved->error().debug_string()
                )
            );
        } else if (!resolved.has_value()) {
            // None -> exact disabled detail from connectivity.rs:54-56.
            response.set_enabled(false);
            response.set_detail(
                "Offer Engine is not enabled in global config "
                "(offer_engine.enabled is false or credential source is none)"
            );
        } else {
            // Some(Ok(config)) -> OfferEngineClient::new(...).check_connectivity()
            OfferEngineClient client{m_context, **resolved, m_support.trace_header_name()};
            const auto result = client.check_connectivity();
            response.set_enabled(true);
            response.set_reachable(result.reachable);
            response.set_status_code(result.status_code);
            response.set_detail(result.detail);
        }

        return response;
    }

private:
    ::app::context::AppContext& m_context;
    OfferEngineRuntimeSupport m_support;
};

} // namespace app::handlers::offer_engine
