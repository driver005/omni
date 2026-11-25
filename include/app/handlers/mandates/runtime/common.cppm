export module app_handlers_mandates:runtime_common;

import std;
import core_serde;
import core_error;
import app_context;
import app_model;
import app_store;
import app_connector;
import app_metrics;
import app_keystore;

export namespace app::handlers::mandates {

using Record = ::app::store::mandates::MandateRecord;

// common_enums::MandateStatus has exactly four variants (enums.rs:3633-3639).
// Active | Inactive | Pending are revocable; Revoked is a validation failure
// (core/mandate.rs:76-79,158-163).
enum class RevokeStatus
{
    Revocable,
    AlreadyRevoked,
    NotRevocable,
};

/**
 * @brief Shared business helpers for the mandate sub-runtimes.
 *
 * Mirrors the upstream core/mandate.rs lookups, core/mandate/helpers.rs,
 * core/mandate/utils.rs response mapping and the payments MCA resolution used by
 * revoke. External infrastructure (processor key store, locker, connector-data
 * chain, DB storage_scheme) is replaced by the in-process AppContext facilities
 * with the same call order and observable shapes.
 */
class MandatesRuntimeSupport
{
public:
    explicit MandatesRuntimeSupport(app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    // Upstream codes (HE_02, HE_03, CE_00, ...) live in AppError::kind_name,
    // which the wire serializes as the `error` field; `message` stays the real
    // human-readable text. Status comes from AppErrorKind via get_status().
    [[nodiscard]] core::error::AppError make_error(
        core::error::AppErrorKind kind,
        std::string message,
        std::string_view upstream_code = {}
    ) const
    {
        auto error = core::error::AppErrorBuilder{}.operator()(kind, std::move(message));
        if (!upstream_code.empty()) {
            error.set_kind_name(std::string{upstream_code});
        }
        return error;
    }

    void record_flow(std::string_view flow) const
    {
        // router_env::instrument(Flow::Mandates*) + metrics.
        m_context.metrics().record_flow(flow);
        m_context.metrics().counter(flow).increment();
    }

    /**
     * @brief find_mandate_by_merchant_id_mandate_id (db/mandate.rs:12-17).
     * The merchant id is applied as a DB predicate so a foreign mandate is invisible
     * (upstream returns MandateNotFound instead of a 403 ownership error).
     */
    [[nodiscard]] std::optional<Record>
    find_mandate_for_merchant(std::string_view merchant_id, std::string_view mandate_id) const
    {
        auto query = app::store::MandateQuery{};
        if (!merchant_id.empty()) {
            query.merchant_id(std::string{merchant_id});
        }
        for (const auto& row: m_context.repository().find_mandates_by_customer_id(query)) {
            if (row.get_mandate_id() == mandate_id) {
                return row;
            }
        }
        return std::nullopt;
    }

    /**
     * @brief get_profile_id_for_mandate (core/mandate/helpers.rs:13-44).
     * Upstream loads the original payment intent and reads its profile id. The local
     * payment record has no profile column, so the profile persisted on the mandate is
     * used after verifying the original payment exists.
     */
    [[nodiscard]] std::expected<std::string, core::error::AppError>
    get_profile_id_for_mandate(const Record& record) const
    {
        if (record.get_payment_id().empty()) {
            return std::unexpected(
                make_error(core::error::AppErrorKind::NOT_FOUND, "Payment not found", "HE_02")
            );
        }
        if (!m_context.repository().find_payment_by_string(record.get_payment_id())) {
            return std::unexpected(
                make_error(core::error::AppErrorKind::NOT_FOUND, "Payment not found", "HE_02")
            );
        }
        if (record.get_profile_id().empty()) {
            return std::unexpected(
                make_error(core::error::AppErrorKind::NOT_FOUND, "Profile id is Null", "HE_02")
            );
        }
        return record.get_profile_id();
    }

    /** @brief get_mandate_type (core/mandate/helpers.rs:46-85), retained as a local validator. */
    [[maybe_unused]]
    [[nodiscard]] std::expected<std::optional<std::string_view>, core::error::AppError>
    get_mandate_type(
        bool has_mandate_data,
        bool has_off_session,
        bool has_off_session_future_usage,
        bool has_customer_acceptance,
        bool has_token,
        bool has_wallet_payment_method
    ) const
    {
        if (has_mandate_data && has_off_session && has_off_session_future_usage &&
            has_customer_acceptance && has_token) {
            return std::unexpected(make_error(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "Expected one out of recurring_details and mandate_data but got both"
            ));
        }
        if ((has_off_session_future_usage && has_customer_acceptance && has_token) ||
            (has_off_session_future_usage && has_customer_acceptance) ||
            (has_mandate_data && has_off_session_future_usage)) {
            return std::optional<std::string_view>{"NewMandateTransaction"};
        }
        if ((has_off_session_future_usage && has_token) || has_off_session ||
            (has_off_session_future_usage && has_wallet_payment_method)) {
            return std::optional<std::string_view>{"RecurringMandateTransaction"};
        }
        return std::optional<std::string_view>{};
    }

    [[nodiscard]] std::optional<app::store::merchant_connectors::MerchantConnectorRecord>
    get_merchant_connector_account(
        std::string_view profile_id,
        std::string_view connector,
        std::string_view merchant_id
    ) const
    {
        for (const auto& mca: m_context.repository().list_merchant_connectors()) {
            if (!merchant_id.empty() && mca.get_merchant_id() != merchant_id) {
                continue;
            }
            if (!profile_id.empty() && mca.get_profile_id() != profile_id) {
                continue;
            }
            if (!connector.empty() && mca.get_connector_name() != connector) {
                continue;
            }
            return mca;
        }
        return std::nullopt;
    }

    /** @brief construct_mandate_revoke_router_data (core/mandate/utils.rs:19-104). */
    [[nodiscard]] app::connector::ConnectorRequest construct_mandate_revoke_router_data(
        const Record& record,
        std::string_view merchant_id,
        std::string_view profile_id,
        std::string_view connector_auth_details
    ) const
    {
        constexpr std::string_view kAttemptId{"irrelevant_attempt_id_in_mandate_revoke_flow"};
        constexpr std::string_view kReferenceId{
            "irrelevant_connector_request_reference_id_in_mandate_revoke_flow"
        };
        const std::string payment_id =
            record.get_payment_id().empty() ? "mandate_revoke" : record.get_payment_id();
        const std::string auth_json =
            connector_auth_details.empty() ? "null" : std::string{connector_auth_details};

        return app::connector::ConnectorRequestBuilder{}
            .operation("mandate_revoke")
            .merchant_id(std::string{merchant_id})
            .resource_id(record.get_mandate_id())
            .raw_payload(std::format(
                "{{\"mandate_id\":{},\"connector_mandate_id\":null,\"connector\":{},"
                "\"merchant_id\":{},\"customer_id\":{},\"payment_id\":{},"
                "\"attempt_id\":{},\"connector_request_reference_id\":{},"
                "\"connector_auth_type\":{},\"profile_id\":{}}}",
                json_quote(record.get_mandate_id()),
                json_quote(record.get_connector()),
                json_quote(merchant_id),
                json_quote(record.get_customer_id()),
                json_quote(payment_id),
                json_quote(kAttemptId),
                json_quote(kReferenceId),
                auth_json,
                json_quote(profile_id)))();
    }

    /**
     * @brief MandateResponse::from_db_mandate (types/api/mandates.rs:36-135).
     *
     *  v1 mapping sets amount/currency/profile_id/mandate_metadata to None (skipped),
     *  always emits customer_acceptance, and emits card only for the Card payment method.
     *  Card/acceptance details are read from the in-process KeyStore stand-in for the
     *  processor key store / locker using (purpose, mandate_id) as the natural key.
     *
     *  Returns the real typed `MandateResponse` DTO (include/app/model/mandates.cppm).
     *  Its serde wire accessors reproduce the exact upstream v1 field shape: bare-scalar
     *  newtypes, serde-omitted empty Options (upstream skip_serializing_if), and verbatim
     *  card/acceptance objects. `serde::Ser::serialize` is the only wire encoder.
     */
    [[nodiscard]] app::model::mandates::MandateResponse from_db_mandate(const Record& record) const
    {
        using DTO = app::model::mandates::MandateResponse;

        DTO dto{};
        dto.set_mandate_id(record.get_mandate_id());
        // original_payment_id, Option -> omitted by serde when absent.
        if (!record.get_payment_id().empty()) {
            app::model::mandates::PaymentIdValue payment_id{};
            payment_id.set_value(record.get_payment_id());
            dto.set_payment_id(std::move(payment_id));
        }
        // amount / currency / profile_id / mandate_metadata: always None in v1, left
        // empty so serde omits them (upstream `skip_serializing_if = Option::is_none`).
        app::model::mandates::MandateStatusValue status{};
        status.set_value(record.get_status());
        dto.set_status(std::move(status));
        dto.set_payment_method_id(record.get_payment_method_id());
        dto.set_payment_method(record.get_payment_method());
        if (!record.get_payment_method_type().empty()) {
            dto.set_payment_method_type(record.get_payment_method_type());
        }

        // "card": null unless the method is Card and the locker stand-in holds an object.
        std::string card{"null"};
        if (is_card(record.get_payment_method())) {
            if (const auto secret =
                    m_context.key_store().get("mandate_card", record.get_mandate_id());
                secret.has_value() && !secret->value().empty() && secret->value().front() == '{') {
                card = secret->value();
            }
        }
        dto.set_card_wire(std::move(card));

        // "customer_acceptance" is always emitted (types/api/mandates.rs:117-128).
        // acceptance_type from ip address (unavailable locally -> offline), accepted_at
        // from the mandate (unavailable -> null), online object always present.
        std::string acceptance{"{\"acceptance_type\":\"offline\",\"accepted_at\":null,"
                               "\"online\":{\"ip_address\":null,\"user_agent\":\"\"}}"};
        if (const auto secret =
                m_context.key_store().get("customer_acceptance", record.get_mandate_id());
            secret.has_value() && !secret->value().empty() && secret->value().front() == '{') {
            acceptance = secret->value();
        }
        dto.set_acceptance_wire(std::move(acceptance));
        return dto;
    }

    /** @brief MandateRevokedResponse (api_models/mandates.rs:15-33). */
    [[nodiscard]] app::model::mandates::MandateRevokedResponse
    revoked_response(std::string_view mandate_id, std::string_view status) const
    {
        app::model::mandates::MandateRevokedResponse dto{};
        dto.set_mandate_id(std::string{mandate_id});
        app::model::mandates::MandateStatusValue value{};
        value.set_value(std::string{status});
        dto.set_status(std::move(value));
        // error_code / error_message remain empty -> wire accessors emit null, matching
        // upstream's `Option<String>` fields (no skip_serializing_if) on the Ok(_) path.
        return dto;
    }

    [[nodiscard]] RevokeStatus classify_revoke_status(std::string_view status) const noexcept
    {
        if (status == "active" || status == "inactive" || status == "pending") {
            return RevokeStatus::Revocable;
        }
        if (status == "revoked") {
            return RevokeStatus::AlreadyRevoked;
        }
        return RevokeStatus::NotRevocable;
    }

    [[nodiscard]] bool is_card(std::string_view payment_method) const noexcept
    {
        return payment_method.size() == 4 &&
               std::equal(
                   payment_method.begin(),
                   payment_method.end(),
                   "card",
                   [](char a, char b)
                   {
                       return std::tolower(static_cast<unsigned char>(a)) == b;
                   }
               );
    }

    [[nodiscard]] std::string json_quote(std::string_view value) const
    {
        std::string out{"\""};
        for (const char c: value) {
            switch (c) {
                case '"':
                    out += "\\\"";
                    break;
                case '\\':
                    out += "\\\\";
                    break;
                case '\n':
                    out += "\\n";
                    break;
                case '\r':
                    out += "\\r";
                    break;
                case '\t':
                    out += "\\t";
                    break;
                default:
                    if (static_cast<unsigned char>(c) < 0x20) {
                        out += std::format(
                            "\\u{:04x}",
                            static_cast<unsigned>(static_cast<unsigned char>(c))
                        );
                    } else {
                        out.push_back(c);
                    }
            }
        }
        out.push_back('"');
        return out;
    }

    [[nodiscard]] bool matches_created_time(
        std::string_view actual,
        const app::model::mandates::MandateListConstraints& constraints
    ) const
    {
        const auto& exact = constraints.get_created_time();
        const auto& lt = constraints.get_created_time_lt();
        const auto& gt = constraints.get_created_time_gt();
        const auto& lte = constraints.get_created_time_lte();
        const auto& gte = constraints.get_created_time_gte();
        if (!exact && !lt && !gt && !lte && !gte) {
            return true;
        }
        if (actual.empty()) {
            return false;
        }
        if (exact && actual != exact->get_value()) {
            return false;
        }
        if (lt && actual >= lt->get_value()) {
            return false;
        }
        if (gt && actual <= gt->get_value()) {
            return false;
        }
        if (lte && actual > lte->get_value()) {
            return false;
        }
        if (gte && actual < gte->get_value()) {
            return false;
        }
        return true;
    }

private:
    app::context::AppContext& m_context;
};

} // namespace app::handlers::mandates
