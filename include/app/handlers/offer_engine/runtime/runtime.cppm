export module app_handlers_offer_engine:runtime;

import std;
import core_error;
import app_context;

import :input;
import :result;
import :runtime_common;
import :runtime_connectivity_check;
import :runtime_browse_offers;

export namespace app::handlers::offer_engine {

class OfferEngineRuntime
{
public:
    explicit OfferEngineRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_connectivity_check{context},
        m_browse_offers{context}
    {
    }

    [[nodiscard]] std::expected<ConnectivityCheckResult, core::error::AppError>
    connectivity_check(const ConnectivityCheckInput& input) const
    {
        return m_connectivity_check.connectivity_check(input);
    }

    [[nodiscard]] std::expected<BrowseOffersResult, core::error::AppError>
    browse_offers(const BrowseOffersInput& input) const
    {
        return m_browse_offers.browse_offers(input);
    }

private:
    app::context::AppContext& m_context;
    ConnectivityCheckRuntime m_connectivity_check;
    BrowseOffersRuntime m_browse_offers;
};

} // namespace app::handlers::offer_engine
