export module app_payment:tokenization_runtime_delete_tokenized_data_api;

import std;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import core_cache;
import core_error;
import app_auth;
import app_http;
import app_crypto;
import app_metrics;
import app_store;
import app_connector;
import app_events;
import app_workflow;
import app_keystore;

import :tokenization_input;
import :tokenization_result;
import :tokenization_runtime_common;

export namespace app::handlers::tokenization {

class DeleteTokenizedDataRuntime
{
public:
    explicit DeleteTokenizedDataRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    /** Port of core::tokenization::delete_tokenized_data_core (core/tokenization.rs:100-163). */
    [[nodiscard]] std::
        expected<::app::dto::tokenization::DeleteTokenDataResponse, core::error::AppError>
        delete_tokenized_data(const DeleteTokenizedDataInput& input) const
    {
        // core:109-115 — get_entity_id_vault_id_by_token_id -> TokenizationRecordNotFound.
        auto record = m_context.repository().find_tokenization_by_string(input.token_id);
        if (!record) {
            return std::unexpected{m_support.upstream_error(
                core::error::AppErrorKind::NOT_FOUND,
                "TokenizationRecordNotFound",
                std::format(
                    "Tokenization record not found for the given token_id '{}' ",
                    input.token_id
                )
            )};
        }

        // core:117-124 — when(customer mismatch) -> UnprocessableEntity.
        if (record->get_customer_id() != input.customer_id) {
            return std::unexpected{m_support.upstream_error(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "UnprocessableEntity",
                "Tokenization record does not belong to the customer"
            )};
        }

        // core:126-130 — when(is_disabled) -> GenericNotFoundError.
        if (record->get_status() == m_support.disabled_flag()) {
            return std::unexpected{m_support.upstream_error(
                core::error::AppErrorKind::NOT_FOUND,
                "GenericNotFoundError",
                "Tokenization is already disabled for the id"
            )};
        }

        // core:133 — VaultId::generate(tokenization_record.locker_id).
        const std::string vault_id = record->get_vault_id();

        // core:135-143 — delete_payment_method_data_from_vault_internal.
        m_support.delete_payment_method_data_from_vault_internal(
            vault_id,
            record->get_customer_id()
        );

        // core:146-156 — update_tokenization_record(DeleteTokenizationRecordUpdate{Disabled}).
        record->set_status(m_support.disabled_flag());
        if (!m_context.repository().update_tokenization(*record)) {
            return std::unexpected{m_support.upstream_error(
                core::error::AppErrorKind::INTERNAL_ERROR,
                "InternalServerError",
                "Failed to update tokenization record"
            )};
        }

        // core:158-161 — DeleteTokenDataResponse { id: token_id }.
        // Real typed DTO with a transparent plain-string id field.
        ::app::dto::tokenization::DeleteTokenDataResponse response{};
        response.set_id(std::string{input.token_id});
        return ::app::dto::tokenization::DeleteTokenDataResponse{response};
    }

private:
    ::app::context::AppContext& m_context;
    TokenizationRuntimeSupport m_support;
};

} // namespace app::handlers::tokenization
