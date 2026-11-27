export module app_payment:pm_auth_runtime;
import core_behavior;

import std;
import core_serde;
import core_error;
import app_context;

import :pm_auth_input;
import :pm_auth_result;
import :pm_auth_runtime_common;
import :pm_auth_runtime_link_token_create;
import :pm_auth_runtime_exchange_token;

export namespace app::handlers::pm_auth {

class PmAuthRuntime
{
public:
    explicit PmAuthRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_link_token_create{context},
        m_exchange_token{context}
    {
    }

    template<class Dto>
    [[nodiscard]] core::behavior::Outcome<::app::dto::pm_auth::LinkTokenCreateResponse>
    link_token_create(const Dto& dto, const core::behavior::CallContext& ctx) const
    {
        return m_link_token_create.link_token_create(dto, ctx);
    }

    template<class Dto>
    [[nodiscard]] core::behavior::Outcome<serde::Value>
    exchange_token(const Dto& dto, const core::behavior::CallContext& ctx) const
    {
        return m_exchange_token.exchange_token(dto, ctx);
    }

private:
    ::app::context::AppContext& m_context;
    PmAuthLinkTokenCreateRuntime m_link_token_create;
    PmAuthExchangeTokenRuntime m_exchange_token;
};

} // namespace app::handlers::pm_auth
