export module app_payment:mandates_runtime_revoke_mandate;
import core_behavior;

import std;
import interfaces;
import core_error;
import app_context;
import app_connector;
import app_metrics;
import app_store;

import :mandates_input;
import :mandates_result;
import :mandates_runtime_common;

export namespace app::handlers::mandates {

class RevokeMandateRuntime
{
public:
    explicit RevokeMandateRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // ───────────────────────── Core: revoke_mandate ─────────────────────────
    // core/mandate.rs:60-165.
    [[nodiscard]] core::behavior::Outcome<::app::model::mandates::MandateRevokedResponse>
    revoke_mandate(const core::behavior::CallContext& ctx) const
    {
        const auto input = RevokeMandateData{ctx};
        m_support.record_flow(
            "MandatesRevoke"
        ); // LockAction::NotApplicable (routes/mandates.rs:65)

        const auto found = m_support.find_mandate_for_merchant(input.merchant_id, input.mandate_id);
        if (!found) {
            return std::unexpected(m_support.make_error(
                core::error::AppErrorKind::NOT_FOUND,
                "Mandate does not exist in our records", // MandateNotFound
                "HE_02"
            ));
        }
        auto record = *found;

        const auto revocable = m_support.classify_revoke_status(record.get_status());
        if (revocable != RevokeStatus::Revocable) {
            return std::unexpected(m_support.make_error(
                core::error::AppErrorKind::INVALID_REQUEST, // MandateValidationFailed -> 400
                revocable == RevokeStatus::AlreadyRevoked
                    ? "Mandate Validation Failed: Mandate has already been revoked"
                    : "Mandate Validation Failed",
                "HE_03"
            ));
        }

        // helpers::get_profile_id_for_mandate (core/mandate/helpers.rs:13-44).
        const auto profile = m_support.get_profile_id_for_mandate(record);
        if (!profile) {
            return std::unexpected(profile.error());
        }

        // payment_helper::get_merchant_connector_account (core/payments/helpers.rs:5222).
        const auto mca = m_support.get_merchant_connector_account(
            *profile,
            record.get_connector(),
            input.merchant_id
        );
        if (!mca) {
            return std::unexpected(m_support.make_error(
                core::error::AppErrorKind::INTERNAL,
                "merchant connector account not found for mandate revoke"
            ));
        }

        // ConnectorData::get_connector_by_name + get_connector_integration +
        // utils::construct_mandate_revoke_router_data (core/mandate/utils.rs:19-104).
        auto router_data = m_support.construct_mandate_revoke_router_data(
            record,
            input.merchant_id,
            *profile,
            mca->get_connector_account_details()
        );

        auto connector = m_context.connector_router().select_for(router_data);
        if (!connector) {
            connector = m_context.connector_router().select(input.merchant_id);
        }
        if (!connector) {
            m_context.logger().record_flow("mandate.revoke.connector_unavailable");
            return std::unexpected(
                m_context.connector_router().last_error().value_or(m_support.make_error(
                    core::error::AppErrorKind::CONNECTOR_ERROR,
                    "no connector available for mandate revoke"
                ))
            );
        }

        // execute_connector_processing_step(CallConnectorAction::Trigger)
        // (core/mandate.rs:113-122).
        const auto connector_response = connector->mandate_revoke(router_data);
        if (connector_response.get_status().empty() || connector_response.get_retryable()) {
            // Err(err) -> ExternalConnectorError (core/mandate.rs:148-155). The local error
            // transport carries code/message/connector/status_code/reason in its message.
            const std::string code{"CE"};
            const std::string message = connector_response.get_message().empty()
                                            ? "connector rejected mandate revoke"
                                            : connector_response.get_message();
            return std::unexpected(m_support.make_error(
                core::error::AppErrorKind::CONNECTOR_ERROR,
                std::format(
                    "{}: {} (connector={}, status_code={}, reason={})",
                    code,
                    message,
                    record.get_connector(),
                    502,
                    connector_response.get_raw_json()
                ),
                "CE_00"
            ));
        }
        m_support.record_flow("mandate.revoke.connector_succeeded");

        // Ok(_) -> MandateUpdate::StatusUpdate { Revoked } (core/mandate.rs:125-145).
        record.set_status("revoked");
        m_context.repository().update_mandate(record);

        // MandateRevokedResponse (api_models/mandates.rs:15-33), encoded by the
        // handler via serde::Ser::serialize into the exact wire shape.
        return ::app::model::mandates::MandateRevokedResponse{
            m_support.revoked_response(record.get_mandate_id(), record.get_status())
        };
    }

private:
    ::app::context::AppContext& m_context;
    MandatesRuntimeSupport m_support;
};

} // namespace app::handlers::mandates
