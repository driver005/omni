export module app_handlers_currency:runtime_convert_forex;

import std;
import interfaces;
import core_serde;
import app_dto;
import app_context;
import core_cache;
import core_error;
import app_metrics;
import app_locking;

import :input;
import :result;
import :runtime_common;

export namespace app::handlers::currency {

class ConvertForexRuntime
{
public:
    explicit ConvertForexRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // ── routes/currency.rs:36-72 / core/currency.rs:28-54 ───────────────────
    [[nodiscard]] std::expected<ConvertForexResult, core::error::AppError>
    convert_forex(const ConvertForexInput& input) const
    {
        m_support.record_forex_flow();

        // core::convert_forex (core/currency.rs:28-54); already applies the error taxonomy.
        auto response = convert_forex_core(
            input.params.get_amount().get_amount_as_i64(),
            input.params.get_to_currency(),
            input.params.get_from_currency()
        );
        if (!response) {
            return std::unexpected{response.error()};
        }

        // ApplicationResponse::Json(CurrencyConversionResponse) (:34-36).
        m_context.metrics().counter("FOREX_CONVERTED").increment();
        return ConvertForexResult{*response};
    }

private:
    // ── core/currency.rs:28-54 — convert_forex ───────────────────────────────
    [[nodiscard]] std::
        expected<app::dto::currency::CurrencyConversionResponse, core::error::AppError>
        convert_forex_core(std::int64_t amount, std::string_view to, std::string_view from) const
    {
        auto result = convert_currency(amount, to, from);
        if (!result) {
            // match err.current_context() (core/currency.rs:45-52).
            if (result.error() == ForexError::CurrencyNotAcceptable) {
                return std::unexpected{
                    m_support.invalid_request_data("The provided currency is not acceptable")
                };
            }
            return std::unexpected{m_support.internal_server_error()};
        }
        return *result;
    }

    // ── utils/currency.rs:515-543 — convert_currency ─────────────────────────
    [[nodiscard]] std::expected<app::dto::currency::CurrencyConversionResponse, ForexError>
    convert_currency(std::int64_t amount, std::string_view to, std::string_view from) const
    {
        const auto ttl = m_support.data_expiration_delay_seconds();
        // get_forex_rates(...).change_context(ForexError::ApiError) (:521-524).
        auto rates = m_support.get_forex_rates(ttl);
        if (!rates) {
            return std::unexpected{ForexError::ApiError};
        }

        // enums::Currency::from_str(...).change_context(CurrencyNotAcceptable) (:526-532).
        if (!m_support.is_supported_currency(from) || !m_support.is_supported_currency(to)) {
            return std::unexpected{ForexError::CurrencyNotAcceptable};
        }

        // currency_conversion::conversion::convert(&rates.data, from, to, amount) (:534-537).
        auto converted = convert_minor(rates->data, from, to, amount);
        if (!converted) {
            return std::unexpected{ForexError::ConversionError};
        }

        app::dto::currency::CurrencyConversionResponse response;
        response.set_converted_amount(*converted); // converted_amount.to_string() (:540)
        response.set_currency(std::string{to});    // to_currency.to_string() (:541)
        return response;
    }

    // ── currency_conversion/src/conversion.rs:10-27 — convert ────────────────
    [[nodiscard]] std::optional<std::string> convert_minor(
        const ExchangeRates& rates,
        std::string_view from,
        std::string_view to,
        std::int64_t amount
    ) const
    {
        // Money::from_minor(amount, currency) -> major Decimal with the currency exponent.
        const int exponent = m_support.minor_unit_exponent(from);
        const Decimal major{static_cast<double>(amount) / std::pow(10.0, exponent), exponent};

        const auto from_factors = m_support.factors_for(rates, from);
        const auto to_factors = m_support.factors_for(rates, to);
        if (!from_factors || !to_factors) {
            return std::nullopt; // ConversionNotSupported -> ConversionError
        }

        // to == base: forward_conversion(major, from) (conversion.rs:18-19).
        if (to == rates.base_currency) {
            return m_support.decimal_to_string(
                m_support.multiply(major, m_support.decimal_from_string(from_factors->from_factor))
            );
        }
        // from == base: backward_conversion(major, to) (conversion.rs:20-21).
        if (from == rates.base_currency) {
            return m_support.decimal_to_string(
                m_support.multiply(major, m_support.decimal_from_string(to_factors->to_factor))
            );
        }
        // cross: forward(from) then backward(to) (conversion.rs:22-26).
        const auto base =
            m_support.multiply(major, m_support.decimal_from_string(from_factors->from_factor));
        return m_support.decimal_to_string(
            m_support.multiply(base, m_support.decimal_from_string(to_factors->to_factor))
        );
    }

    app::context::AppContext& m_context;
    CurrencyRuntimeSupport m_support;
};

} // namespace app::handlers::currency
