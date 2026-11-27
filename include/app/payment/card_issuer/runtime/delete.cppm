export module app_payment:card_issuer_runtime_delete;
import core_behavior;

import std;
import core_error;
import app_context;
import app_dto;
import app_metrics;
import app_store;

import :card_issuer_input;
import :card_issuer_result;
import :card_issuer_runtime_common;

export namespace app::handlers::card_issuer {

class CardIssuerDeleteRuntime
{
public:
    explicit CardIssuerDeleteRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // core/card_issuer.rs:63-82 `delete_card_issuer`.
    [[nodiscard]] core::behavior::Outcome<::app::dto::card_issuer::CardIssuerDeleteResponse>
    remove(const core::behavior::CallContext& ctx) const
    {
        const auto input = CardIssuerDeleteData{ctx};
        const std::string& id = input.id;

        // core/card_issuer.rs:72-74 to_not_found_response message.
        const auto not_found = [this, &id]()
        {
            return m_support.make_error(
                core::error::AppErrorKind::NOT_FOUND,
                "Card issuer with id " + std::string{id} + " not found"
            );
        };
        if (id.empty()) {
            return std::unexpected{not_found()};
        }

        auto repository = m_context.repository();
        auto stored = repository.find_card_issuer_by_string(id);
        if (!stored) {
            return std::unexpected{not_found()};
        }

        // The store's `is_deleted` result drives the wire `deleted` flag; a
        // failed delete surfaces as InternalServerError (core/card_issuer.rs:68-81).
        const bool is_deleted = repository.remove_card_issuer(stored->get_id());
        if (!is_deleted) {
            return std::unexpected{m_support.make_error(
                core::error::AppErrorKind::INTERNAL_ERROR,
                "failed to delete card issuer"
            )};
        }

        ::app::dto::card_issuer::CardIssuerIdValue response_id;
        response_id.set_value(std::string{id});
        ::app::dto::card_issuer::CardIssuerDeleteResponse response;
        response.set_id(std::move(response_id));
        response.set_deleted(is_deleted);

        // `#[instrument(skip_all, fields(flow = Flow::DeleteCardIssuer))]` substitute.
        m_context.logger().record_flow("DeleteCardIssuer");
        m_context.logger().counter("CARD_ISSUER_DELETED").increment();

        return ::app::dto::card_issuer::CardIssuerDeleteResponse{std::move(response)};
    }

private:
    ::app::context::AppContext& m_context;
    CardIssuerRuntimeSupport m_support;
};

} // namespace app::handlers::card_issuer
