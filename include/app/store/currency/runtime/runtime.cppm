export module app_store:currency_runtime;

import std;
import core_behavior;
import core_error;
import core_serde;
import app_dto;
import app_context;

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

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    retrieve_forex(const core::behavior::CallContext& ctx) const
    {
        return m_retrieve_forex.retrieve_forex(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::dto::currency::CurrencyConversionResponse>
    convert_forex(
        const ::app::dto::currency::CurrencyConversionParams& params,
        const core::behavior::CallContext& ctx
    ) const
    {
        return m_convert_forex.convert_forex(params, ctx);
    }

private:
    ::app::context::AppContext& m_context;
    RetrieveForexRuntime m_retrieve_forex;
    ConvertForexRuntime m_convert_forex;
};

} // namespace app::handlers::currency
