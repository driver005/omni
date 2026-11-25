export module app_handlers_currency:runtime;

import std;
import core_error;
import app_context;

import :input;
import :result;
import :runtime_common;
import :runtime_retrieve_forex;
import :runtime_convert_forex;

export namespace app::handlers::currency {

class CurrencyRuntime
{
public:
    explicit CurrencyRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_retrieve_forex{context},
        m_convert_forex{context}
    {
    }

    [[nodiscard]] std::expected<RetrieveForexResult, core::error::AppError>
    retrieve_forex(const RetrieveForexInput& input) const
    {
        return m_retrieve_forex.retrieve_forex(input);
    }

    [[nodiscard]] std::expected<ConvertForexResult, core::error::AppError>
    convert_forex(const ConvertForexInput& input) const
    {
        return m_convert_forex.convert_forex(input);
    }

private:
    app::context::AppContext& m_context;
    RetrieveForexRuntime m_retrieve_forex;
    ConvertForexRuntime m_convert_forex;
};

} // namespace app::handlers::currency
