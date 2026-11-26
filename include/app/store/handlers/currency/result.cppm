export module app_store:currency_result;

import std;
import core_serde;
import app_dto;

export namespace app::handlers::currency {

// ── retrieve_forex ──────────────────────────────────────────────────────────
// FxExchangeRatesCacheEntry nests a dynamic-key map of per-currency factor objects,
// which the flat DTO enums cannot express. The typed carrier is a serde::Value
// document built from field assignments; the handler serializes it directly.
class RetrieveForexResult
{
public:
    RetrieveForexResult() = default;

    explicit RetrieveForexResult(serde::Value document) noexcept :
        m_document{std::move(document)}
    {
    }

    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

private:
    serde::Value m_document{};
};

// ── convert_forex ───────────────────────────────────────────────────────────
// CurrencyConversionResponse fields mirror the DTO the old core encoded, so
// field names, order and values stay byte-identical.
class ConvertForexResult : public ::app::dto::currency::CurrencyConversionResponse
{
public:
    ConvertForexResult() = default;

    explicit ConvertForexResult(::app::dto::currency::CurrencyConversionResponse value) noexcept :
        CurrencyConversionResponse{std::move(value)}
    {
    }
};

} // namespace app::handlers::currency

template<>
struct serde::Serializable<::app::handlers::currency::ConvertForexResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<::app::dto::currency::CurrencyConversionResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "currency_convert_result";
    }
};
