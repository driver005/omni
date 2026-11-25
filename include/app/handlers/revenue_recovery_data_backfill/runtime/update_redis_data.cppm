export module app_handlers_revenue_recovery_data_backfill:runtime_update_redis_data;

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
import app_metrics;
import app_store;
import app_crypto;
import app_locking;
import app_services;

import :input;
import :result;
import :runtime_common;

export namespace app::handlers::revenue_recovery_data_backfill {

class UpdateRedisDataRuntime
{
public:
    explicit UpdateRedisDataRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    /**
     * Upstream: routes/revenue_recovery_data_backfill.rs:106-128 ->
     * core/revenue_recovery_data_backfill.rs:157-261.
     */
    [[nodiscard]] std::expected<UpdateRedisDataResult, core::error::AppError>
    update_redis_data(const UpdateRedisDataInput& input) const
    {
        // Secret<String, PhoneNumberStrategy> Debug is masked; local SecretString::masked()
        // is the in-process substitute for that presentation.
        const auto secret = ::app::crypto::SecretString{input.parsed.get_payment_processor_token()};
        const auto& token = input.parsed.get_payment_processor_token();
        const auto& connector_customer_id = input.parsed.get_connector_customer_id();

        backfill_support::LocalRedis redis{m_context.cache()};
        auto tokens = redis.get_hash_fields(redis.tokens_key(connector_customer_id));
        const auto found = tokens.find(token);
        const auto not_found_message = "Token '" + secret.masked() +
                                       "' not found for connector customer id:- '" +
                                       connector_customer_id + "'";
        if (found == tokens.end()) {
            // core:173-180 -> GenericNotFoundError (IR_37, 404).
            return std::unexpected{m_support.make_error(
                core::error::AppErrorKind::NOT_FOUND,
                not_found_message,
                "IR_37"
            )};
        }

        auto status = m_support.decode_token_status(found->second);
        if (!status) {
            return std::unexpected{m_support.make_error(
                core::error::AppErrorKind::NOT_FOUND,
                not_found_message,
                "IR_37"
            )};
        }

        std::vector<std::string> updated_fields{};
        if (input.scheduled_update) {
            if (input.scheduled_update->kind == ScheduledKind::Null) {
                status->scheduled_at = std::nullopt; // core:196-204
                updated_fields.emplace_back("scheduled_at: set to null");
            } else {
                status->scheduled_at = input.scheduled_update->datetime; // core:185-195
                updated_fields.push_back("scheduled_at: " + input.scheduled_update->datetime);
            }
        }
        if (input.parsed.get_is_hard_decline()) {
            status->is_hard_decline = *input.parsed.get_is_hard_decline(); // core:211-215
            updated_fields.push_back(
                std::string{"is_hard_decline: "} +
                (*input.parsed.get_is_hard_decline() ? "true" : "false")
            );
        }
        if (input.parsed.get_error_code()) {
            status->error_code = *input.parsed.get_error_code(); // core:217-221
            updated_fields.push_back("error_code: " + *input.parsed.get_error_code());
        }
        status->modified_at = backfill_support::PrimitiveDateTime::now().storage_string();
        tokens[token] = m_support.encode_token_status(*status);

        // core:224-238: update_or_add... with the configured Redis TTL.
        redis.set_hash_fields(
            redis.tokens_key(connector_customer_id),
            tokens,
            m_support.redis_ttl_seconds()
        );

        // Local DB shadow keeps the repository row in sync with the Redis substitute.
        auto repository = m_context.repository();
        if (auto existing = repository.find_revenue_recovery_token_by_string(token)) {
            auto payload = m_support.update_payload_fields(
                existing->get_payload(),
                &input.parsed,
                input.scheduled_update.has_value()
                    ? std::optional<ScheduledUpdate>{*input.scheduled_update}
                    : std::nullopt
            );
            existing->set_payload(std::move(payload));
            (void)repository.update_revenue_recovery_token(*existing);
        }

        const std::string updated_fields_str = updated_fields.empty()
                                                   ? std::string{"no fields were updated"}
                                                   : m_support.join(updated_fields, ", ");
        m_support.record_flow(
            "RecoveryDataBackfill",
            "revenue_recovery_update_additional_redis_data",
            std::size_t{1},
            std::size_t{0}
        );

        using Response = ::app::dto::revenue_recovery_data_backfill::UpdateTokenStatusResponse;
        Response response;
        response.set_updated(true);
        response.set_message(
            "Successfully updated token '" + secret.masked() + "' for connector customer '" +
            connector_customer_id + "'. Updated fields: " + updated_fields_str
        );
        return UpdateRedisDataResult{std::move(response)};
    }

private:
    app::context::AppContext& m_context;
    RevenueRecoveryRuntimeSupport m_support;
};

} // namespace app::handlers::revenue_recovery_data_backfill
