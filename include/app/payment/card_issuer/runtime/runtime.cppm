export module app_payment:card_issuer_runtime;
import core_behavior;

import std;
import core_error;
import app_context;

import :card_issuer_input;
import :card_issuer_result;
import :card_issuer_runtime_common;
import :card_issuer_runtime_add;
import :card_issuer_runtime_update;
import :card_issuer_runtime_delete;
import :card_issuer_runtime_list;

export namespace app::handlers::card_issuer {

class CardIssuerRuntime
{
public:
    explicit CardIssuerRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_add{context},
        m_update{context},
        m_delete{context},
        m_list{context}
    {
    }

    template<class Dto>
    [[nodiscard]] core::behavior::Outcome<::app::dto::card_issuer::CardIssuerResponse>
    add(const Dto& dto, const core::behavior::CallContext& ctx) const
    {
        return m_add.add(dto, ctx);
    }

    template<class Dto>
    [[nodiscard]] core::behavior::Outcome<::app::dto::card_issuer::CardIssuerResponse>
    update(const Dto& dto, const core::behavior::CallContext& ctx) const
    {
        return m_update.update(dto, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::dto::card_issuer::CardIssuerDeleteResponse>
    remove(const core::behavior::CallContext& ctx) const
    {
        return m_delete.remove(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<::app::dto::card_issuer::CardIssuerListResponse>
    list(const core::behavior::CallContext& ctx) const
    {
        return m_list.list(ctx);
    }

private:
    ::app::context::AppContext& m_context;
    CardIssuerAddRuntime m_add;
    CardIssuerUpdateRuntime m_update;
    CardIssuerDeleteRuntime m_delete;
    CardIssuerListRuntime m_list;
};

} // namespace app::handlers::card_issuer
