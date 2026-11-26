export module app_payment:payments_runtime_payments_post_session_tokens;

import std;
import interfaces;
import core_serde;
import core_error;
import app_context;
import app_model;
import app_dto;
import app_store;
import app_connector;
import app_crypto;
import core_cache;
import app_http;

import :payments_input;
import :payments_result;
import :payments_runtime_common;

export namespace app::handlers::payments {

class PaymentsPostSessionTokensRuntime
{
public:
    explicit PaymentsPostSessionTokensRuntime(
        ::app::context::AppContext& context,
        PaymentsRuntimeSupport& support
    ) noexcept :
        m_context{context},
        m_support{support}
    {
    }

    [[nodiscard]] std::expected<PaymentsPostSessionTokensResult, core::error::AppError>
    payments_post_session_tokens(const PaymentsPostSessionTokensInput& input) const
    {
        m_support.issue_session_token(input.data);
        return PaymentsPostSessionTokensResult{m_support.payment_response_view(input.data)};
    }

private:
    ::app::context::AppContext& m_context;
    PaymentsRuntimeSupport& m_support;
};

} // namespace app::handlers::payments
