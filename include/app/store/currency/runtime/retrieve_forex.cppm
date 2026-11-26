export module app_store:currency_runtime_retrieve_forex;

import std;
import interfaces;
import core_serde;
import app_dto;
import app_context;
import core_cache;
import core_error;
import app_metrics;
import app_locking;

import :currency_input;
import :currency_result;
import :currency_runtime_common;

export namespace app::handlers::currency {

class RetrieveForexRuntime
{
public:
    explicit RetrieveForexRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // ── routes/currency.rs:11-33 / core/currency.rs:15-26 ───────────────────
    [[nodiscard]] std::expected<RetrieveForexResult, core::error::AppError>
    retrieve_forex(const RetrieveForexInput& input) const
    {
        (void)input;
        // Flow::RetrieveForexFlow + api_locking::LockAction::NotApplicable (server_wrap).
        m_support.record_forex_flow();

        auto entry = retrieve_forex_core();
        if (!entry) {
            // change_context(GenericNotFoundError{"Unable to fetch forex rates"}) (:22-24).
            return std::unexpected{m_support.generic_not_found("Unable to fetch forex rates")};
        }

        // ApplicationResponse::Json(FxExchangeRatesCacheEntry): typed document.
        auto result = RetrieveForexResult{m_support.forex_entry_document(*entry)};
        m_context.metrics().counter("FOREX_RATES_RETRIEVED").increment();
        return result;
    }

private:
    // ── core/currency.rs:15-26 — retrieve_forex ─────────────────────────────
    [[nodiscard]] std::expected<FxExchangeRatesCacheEntry, core::error::AppError>
    retrieve_forex_core() const
    {
        // conf.forex_api.get_inner().data_expiration_delay_in_seconds (:18).
        const auto ttl = m_support.data_expiration_delay_seconds();
        auto rates = m_support.get_forex_rates(ttl); // :20
        if (!rates) {
            return std::unexpected{m_support.generic_not_found("Unable to fetch forex rates")};
        }
        // ApplicationResponse::Json(...) — value returned to the route.
        return *rates;
    }

    ::app::context::AppContext& m_context;
    CurrencyRuntimeSupport m_support;
};

} // namespace app::handlers::currency
