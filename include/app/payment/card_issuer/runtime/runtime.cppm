export module app_payment:card_issuer_runtime;

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

    [[nodiscard]] std::expected<::app::dto::card_issuer::CardIssuerResponse, core::error::AppError>
    add(const CardIssuerAddInput& input) const
    {
        return m_add.add(input);
    }

    [[nodiscard]] std::expected<::app::dto::card_issuer::CardIssuerResponse, core::error::AppError>
    update(const CardIssuerUpdateInput& input) const
    {
        return m_update.update(input);
    }

    [[nodiscard]] std::
        expected<::app::dto::card_issuer::CardIssuerDeleteResponse, core::error::AppError>
        remove(const CardIssuerDeleteInput& input) const
    {
        return m_delete.remove(input);
    }

    [[nodiscard]] std::
        expected<::app::dto::card_issuer::CardIssuerListResponse, core::error::AppError>
        list(const CardIssuerListInput& input) const
    {
        return m_list.list(input);
    }

private:
    ::app::context::AppContext& m_context;
    CardIssuerAddRuntime m_add;
    CardIssuerUpdateRuntime m_update;
    CardIssuerDeleteRuntime m_delete;
    CardIssuerListRuntime m_list;
};

} // namespace app::handlers::card_issuer
