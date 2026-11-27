export module app_routing:offer_engine_runtime_browse_offers;

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
import core_behavior;

import :offer_engine_result;
import :offer_engine_runtime_common;

export namespace app::handlers::offer_engine {

class BrowseOffersRuntime
{
public:
    explicit BrowseOffersRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    /** @brief Port of `browse_offers(state, auth.platform, profile_id, payload)` (browse.rs:22-85).
     */
    [[nodiscard]] core::behavior::Outcome<::app::dto::offer_engine::BrowseOffersResponse>
    browse_offers(
        const ::app::dto::offer_engine::BrowseOffersRequest& request,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        m_context.logger().record_flow("OfferEngineBrowseOffers");
        m_context.logger().counter("OFFER_ENGINE_BROWSE_OFFERS").increment();

        // Upstream `auth.platform` supplies the dimensions (browse.rs:22-28).
        OfferEngineDimensions dimensions{};
        dimensions.processor_merchant_id = ctx.merchant_id;
        dimensions.organization_id = ctx.org_id;
        if (!ctx.profile_id.empty()) {
            dimensions.profile_id = ctx.profile_id;
        }

        // resolve_offer_engine_credential_source + config resolution with the
        // AccessForbidden change_context (browse.rs:36-64).
        ResolvedOfferEngineConfig config{};
        const auto source = m_support.resolve_offer_engine_credential_source(dimensions);
        switch (source) {
            case ::app::dto::offer_engine::OfferEngineCredentialSource::None:
                return std::unexpected{forbidden()};
            case ::app::dto::offer_engine::OfferEngineCredentialSource::Application:
                {
                    auto resolved = m_support.resolve_application_offer_config();
                    if (!resolved) {
                        return std::unexpected{forbidden()};
                    }
                    config = std::move(*resolved);
                    break;
                }
            case ::app::dto::offer_engine::OfferEngineCredentialSource::Merchant:
                {
                    auto resolved =
                        m_support.resolve_merchant_offer_config(dimensions.processor_merchant_id);
                    if (!resolved) {
                        return std::unexpected{forbidden()};
                    }
                    config = std::move(*resolved);
                    break;
                }
        }

        // BrowseOfferListRequest { currency: request.offer_payment_info.map(|i| i.currency) }
        // (browse.rs:66-68).
        BrowseOfferListRequest list_request{};
        if (request.get_offer_payment_info().has_value()) {
            list_request.currency = request.get_offer_payment_info()->get_currency();
        }

        OfferEngineClient client{m_context, std::move(config), m_support.trace_header_name()};
        auto client_response = client.browse_offers(list_request);
        if (!client_response) {
            // .change_context(InternalServerError).attach_printable("Offer Engine /list failed")
            // (browse.rs:71-75).
            return std::unexpected{internal_server_error()};
        }

        // Upstream response: filter status == OfferStatus::Eligible, map BrowseOffer::from
        // (browse.rs:77-85, types.rs:373-392).
        ::app::dto::offer_engine::BrowseOffersResponse typed_response{};
        std::vector<::app::dto::offer_engine::BrowseOffer> offers{};
        for (const auto& entry: client_response->get_offers()) {
            if (entry.get_status() != "ELIGIBLE") {
                continue;
            }
            offers.push_back(m_support.browse_offer_from(entry));
        }
        typed_response.set_offers(std::move(offers));

        m_context.logger().record_flow("OfferEngineEligibilityFilter");
        m_context.logger().counter("OFFER_ENGINE_ELIGIBILITY_FILTER").increment();

        return std::move(typed_response);
    }

private:
    /**
     * @brief `ApiErrorResponse::AccessForbidden` (api_error_response.rs:643-646).
     * Status 403 comes from AppErrorKind::FORBIDDEN; upstream code IR_22 rides in
     * `kind_name` so the serialized `error` field preserves it.
     */
    [[nodiscard]] core::error::AppError forbidden() const
    {
        auto error = core::error::AppErrorBuilder{}(
            core::error::AppErrorKind::FORBIDDEN,
            "Access forbidden. Not authorized to access this resource offer_engine"
        );
        error.set_kind_name("IR_22");
        return error;
    }

    /**
     * @brief `ApiErrorResponse::InternalServerError` (api_error_response.rs:422-424).
     * Status 500 comes from AppErrorKind::INTERNAL; HE_00 goes in `kind_name`.
     */
    [[nodiscard]] core::error::AppError internal_server_error() const
    {
        auto error = core::error::AppErrorBuilder{}(
            core::error::AppErrorKind::INTERNAL,
            "Something went wrong"
        );
        error.set_kind_name("HE_00");
        return error;
    }

    ::app::context::AppContext& m_context;
    OfferEngineRuntimeSupport m_support;
};

} // namespace app::handlers::offer_engine
