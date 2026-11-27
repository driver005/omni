export module app_payment:revenue_recovery_data_backfill_runtime_unlock_status;

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

import :revenue_recovery_data_backfill_input;
import :revenue_recovery_data_backfill_result;
import :revenue_recovery_data_backfill_runtime_common;

export namespace app::handlers::revenue_recovery_data_backfill {

class UnlockStatusRuntime
{
public:
    explicit UnlockStatusRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    /**
     * Upstream: routes/revenue_recovery_data_backfill.rs:130-160 ->
     * core/revenue_recovery_data_backfill.rs:66-117 ->
     * RedisTokenManager::unlock_connector_customer_status
     * (types/storage/revenue_recovery_redis_operation.rs:232-286).
     */
    [[nodiscard]] std::expected<
        ::app::dto::revenue_recovery_data_backfill::UnlockStatusResponse,
        core::error::AppError>
    unlock_status(const UnlockStatusInput& input) const
    {
        const auto& customer_id = input.parsed.get_connector_customer_id();
        const auto& expected_owner = input.parsed.get_payment_intent_id().get_value();

        backfill_support::LocalRedis redis{m_context.cache()};
        const auto native_key = redis.status_key(customer_id);
        const auto legacy_lock_key = m_support.status_lock_key(customer_id);
        auto stored = redis.get_key(native_key);
        bool native_exists = stored.has_value();
        if (!native_exists) {
            // Keep the LockRegistry contract used by the sibling redis handler:
            // lock:revenue_recovery.status:{id} holds the payment id as a plain string.
            const auto legacy = m_context.cache().get(m_support.lock_scoped_key(legacy_lock_key));
            if (legacy.hit && !core::cache::IsMiss{}(legacy.value)) {
                stored = core::cache::value_as<std::string>(legacy.value, std::string{});
                native_exists = true;
            }
        }

        if (!native_exists) {
            // Redis GET on a missing key is an error (RedisConnectionError) ->
            // core:89-93 InternalServerError (HE_00, 500).
            return std::unexpected{m_support.make_error(
                core::error::AppErrorKind::INTERNAL,
                "Something went wrong",
                "HE_00"
            )};
        }
        if (!stored || *stored != expected_owner) {
            // RedisError::DeleteFailed -> InvalidPaymentIdProvided (core:81-88, IR_48, 404).
            const auto resource = "The Status token for connector costumer id:- " + customer_id +
                                  " is locked by different PaymentIntent ID";
            return std::unexpected{m_support.make_error(
                core::error::AppErrorKind::NOT_FOUND,
                "The Status token for connector costumer id " + resource +
                    " is locked by different PaymentIntent ID",
                "IR_48"
            )};
        }
        redis.delete_key(native_key);
        (void)m_context.locks().release(legacy_lock_key);

        m_support.record_flow(
            "RecoveryDataBackfillStatus",
            "revenue_recovery_data_backfill_status",
            std::size_t{1},
            std::size_t{0}
        );

        using Response = ::app::dto::revenue_recovery_data_backfill::UnlockStatusResponse;
        Response response;
        response.set_unlocked(true);
        return ::app::dto::revenue_recovery_data_backfill::UnlockStatusResponse{
            std::move(response)
        };
    }

private:
    ::app::context::AppContext& m_context;
    RevenueRecoveryRuntimeSupport m_support;
};

} // namespace app::handlers::revenue_recovery_data_backfill
