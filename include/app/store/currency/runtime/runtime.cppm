export module app_store:currency_runtime;

import std;
import core_error;
import core_serde;
import app_dto;
import app_context;

import :currency_input;
import :currency_result;
import :currency_runtime_common;
import :currency_runtime_retrieve_forex;
import :currency_runtime_convert_forex;

export namespace app::handlers::currency {

class CurrencyRuntime
{
public:
    explicit CurrencyRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_retrieve_forex{context},
        m_convert_forex{context}
    {
    }

    [[nodiscard]] std::expected<serde::Value, core::error::AppError>
    retrieve_forex(const RetrieveForexInput& input) const
    {
        return m_retrieve_forex.retrieve_forex(input);
    }

    [[nodiscard]] std::
        expected<::app::dto::currency::CurrencyConversionResponse, core::error::AppError>
        convert_forex(const ConvertForexInput& input) const
    {
        return m_convert_forex.convert_forex(input);
    }

private:
    ::app::context::AppContext& m_context;
    RetrieveForexRuntime m_retrieve_forex;
    ConvertForexRuntime m_convert_forex;
};

} // namespace app::handlers::currency
