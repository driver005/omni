export module app_payment:card_issuer_runtime_add;

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

class CardIssuerAddRuntime
{
public:
    explicit CardIssuerAddRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // core/card_issuer.rs:16-39 `add_card_issuer`.
    [[nodiscard]] std::expected<::app::dto::card_issuer::CardIssuerResponse, core::error::AppError>
    add(const CardIssuerAddInput& input) const
    {
        const std::string issuer_name = input.request.get_issuer_name().get_value();
        // CardIssuerName nutype validation (common_utils/new_type.rs:21-24):
        // serde rejects an empty or >255-char name with a 422 on the request path.
        if (const auto invalid = m_support.validate_issuer_name(issuer_name)) {
            return std::unexpected{*invalid};
        }

        // `CardIssuerId::generate()` (common_utils/id_type/card_issuer.rs:37-41)
        // -> 5-char id (consts.rs:283); failure maps to InternalServerError.
        const std::string id = m_support.new_id();
        if (id.empty()) {
            return std::unexpected{m_support.make_error(
                core::error::AppErrorKind::INTERNAL_ERROR,
                "card issuer id generation failed"
            )};
        }

        // created_at == last_modified_at == now. The in-process record stores
        // created_at in its column and last_modified_at in metadata (no dedicated
        // column); neither is part of the wire response.
        auto record = m_support.to_store_record(id, issuer_name);
        const auto stamp = m_support.now_stamp();
        record.set_created_at(stamp);
        record.set_metadata("{\"last_modified_at\":\"" + stamp + "\"}");

        auto repository = m_context.repository();
        auto created = repository.create_card_issuer(std::move(record));
        if (!created) {
            // core/card_issuer.rs:34-36 to_duplicate_response(GenericDuplicateError)
            if (created.error().get_kind() == core::error::AppErrorKind::ALREADY_EXISTS) {
                return std::unexpected{m_support.make_error(
                    core::error::AppErrorKind::ALREADY_EXISTS,
                    "A card issuer with this name already exists"
                )};
            }
            return std::unexpected{created.error()};
        }

        // ForeignTryInto<CardIssuer> (types/transformers.rs:3034-3047); name was
        // already validated on input, so conversion cannot fail here.
        auto response = m_support.response_from(*created);

        // `#[instrument(skip_all, fields(flow = Flow::AddCardIssuer))]` substitute.
        m_context.metrics().record_flow("AddCardIssuer");
        m_context.metrics().counter("CARD_ISSUER_CREATED").increment();

        return ::app::dto::card_issuer::CardIssuerResponse{std::move(response)};
    }

private:
    ::app::context::AppContext& m_context;
    CardIssuerRuntimeSupport m_support;
};

} // namespace app::handlers::card_issuer
