export module app_routing:offer_engine_runtime;

import std;
import app_dto;
import core_error;
import app_context;
import core_behavior;

import :offer_engine_result;
import :offer_engine_runtime_common;
import :offer_engine_runtime_connectivity_check;
import :offer_engine_runtime_browse_offers;

export namespace app::handlers::offer_engine {

class OfferEngineRuntime
{
public:
    explicit OfferEngineRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_connectivity_check{context},
        m_browse_offers{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<ConnectivityCheckResponse>
    connectivity_check(const ::core::behavior::CallContext& ctx) const
    {
        return m_connectivity_check.connectivity_check(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::dto::offer_engine::BrowseOffersResponse>
    browse_offers(
        const ::app::dto::offer_engine::BrowseOffersRequest& request,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_browse_offers.browse_offers(request, ctx);
    }

private:
    ::app::context::AppContext& m_context;
    ConnectivityCheckRuntime m_connectivity_check;
    BrowseOffersRuntime m_browse_offers;
};

} // namespace app::handlers::offer_engine
