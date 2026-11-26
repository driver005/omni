export module app_payment:card_issuer_runtime_update;

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

class CardIssuerUpdateRuntime
{
public:
    explicit CardIssuerUpdateRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // core/card_issuer.rs:41-61 `update_card_issuer`.
    [[nodiscard]] std::expected<CardIssuerUpdateResult, core::error::AppError>
    update(const CardIssuerUpdateInput& input) const
    {
        const std::string& id = input.id;
        const std::string issuer_name = input.request.get_issuer_name().get_value();
        if (const auto invalid = m_support.validate_issuer_name(issuer_name)) {
            return std::unexpected{*invalid};
        }

        // core/card_issuer.rs:56-58 to_not_found_response message.
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

        auto record = *stored;
        record.set_name(issuer_name);
        // UpdateCardIssuer.last_modified_at (core/card_issuer.rs:47-50) is kept
        // in metadata in the in-process record (no dedicated column locally).
        record.set_metadata("{\"last_modified_at\":\"" + m_support.now_stamp() + "\"}");
        if (!repository.update_card_issuer(record)) {
            return std::unexpected{not_found()};
        }

        auto response = m_support.response_from(record);

        // `#[instrument(skip_all, fields(flow = Flow::UpdateCardIssuer))]` substitute.
        m_context.metrics().record_flow("UpdateCardIssuer");
        m_context.metrics().counter("CARD_ISSUER_UPDATED").increment();

        return CardIssuerUpdateResult{std::move(response)};
    }

private:
    ::app::context::AppContext& m_context;
    CardIssuerRuntimeSupport m_support;
};

} // namespace app::handlers::card_issuer
