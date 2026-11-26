export module app_routing:gsm_runtime_common;

import std;
import core_serde;
import core_error;
import app_context;
import app_dto;
import app_store;

export namespace app::handlers::gsm {

// Local helpers are a direct port of the four upstream core functions in
// docs/hyperswitch/crates/router/src/core/gsm.rs and the ForeignFrom impls in
// crates/router/src/types/transformers.rs:2055-2186. GsmFeature has one variant
// (`retry`) and GsmFeatureData one variant (`Retry(RetryFeatureData)`), so the
// domain model below collapses those enums while preserving every field exactly.

struct RetryFeatureData
{
    bool step_up_possible{false};
    bool clear_pan_possible{false};
    bool alternate_network_possible{false};
    std::string decision{"do_default"};
};

// hyperswitch_domain_models/src/gsm.rs:4-21 GatewayStatusMap (no `decision`
// field: the decision lives inside feature_data).
struct GatewayStatusMap
{
    std::string connector;
    std::string flow;
    std::string sub_flow;
    std::string code;
    std::string message;
    std::string status;
    std::optional<std::string> router_error;
    std::optional<std::string> unified_code;
    std::optional<std::string> unified_message;
    std::optional<std::string> error_category;
    RetryFeatureData feature_data;
    std::string feature{"retry"};
    std::optional<std::string> standardised_code;
    std::optional<std::string> description;
    std::optional<std::string> user_guidance_message;
};

// hyperswitch_domain_models/src/gsm.rs:23-38 GatewayStatusMappingUpdate.
// Only the fields the upstream core actually derives are represented; the
// redundant diesel `decision`/`step_up_possible`/`clear_pan_possible` columns
// are ignored on read whenever feature_data is present (see try_from at :99-145).
struct GatewayStatusMappingUpdate
{
    std::optional<std::string> status;
    // Upstream passes Some(router_error) unconditionally, so this always overwrites.
    std::optional<std::string> router_error;
    std::optional<std::string> unified_code;
    std::optional<std::string> unified_message;
    std::optional<std::string> error_category;
    std::optional<std::string> standardised_code;
    std::optional<std::string> description;
    std::optional<std::string> user_guidance_message;
    std::optional<RetryFeatureData> feature_data;
    std::string feature{"retry"};
};

constexpr auto kConnectors = std::to_array<std::string_view>(
    {"absa_sanlam",
     "authipay",
     "adyenplatform",
     "stripe_billing_test",
     "phonypay",
     "fauxpay",
     "pretendpay",
     "stripe_test",
     "adyen_test",
     "checkout_test",
     "paypal_test",
     "aci",
     "adyen",
     "affirm",
     "airwallex",
     "amazonpay",
     "archipel",
     "authorizedotnet",
     "bambora",
     "bamboraapac",
     "bankofamerica",
     "barclaycard",
     "betterpayment",
     "billwerk",
     "bitpay",
     "bluesnap",
     "blackhawknetwork",
     "calida",
     "bluecode",
     "boku",
     "braintree",
     "breadpay",
     "cardinal",
     "cashtocode",
     "celero",
     "chargebee",
     "checkbook",
     "checkout",
     "citigate",
     "coinbase",
     "coingate",
     "custombilling",
     "cryptopay",
     "ctp_mastercard",
     "ctp_visa",
     "cybersource",
     "cybersourcedecisionmanager",
     "d24",
     "datatrans",
     "deutschebank",
     "digitalvirgo",
     "dlocal",
     "dwolla",
     "ebanx",
     "ecommpay",
     "envoy",
     "elavon",
     "etisalat",
     "elavon_pg",
     "facilitapay",
     "finix",
     "fiserv",
     "fiservemea",
     "fiservcommercehub",
     "fiuu",
     "flexiti",
     "forte",
     "getnet",
     "gigadat",
     "givepayments",
     "globalpay",
     "globalpayments_heartland",
     "globalpayments_realex",
     "globepay",
     "gocardless",
     "gotyme_sanlam",
     "gpayments",
     "hipay",
     "helcim",
     "hyperpg",
     "hyperswitch_vault",
     "interpayments",
     "inespay",
     "iatapay",
     "ilixium",
     "imerchantsolutions",
     "itaubank",
     "jpmorgan",
     "jpmorgan_orbital",
     "juspay",
     "juspaythreedsserver",
     "klarna",
     "loonio",
     "merchante",
     "mifinity",
     "mollie",
     "moneris",
     "multisafepay",
     "netcetera",
     "nexinets",
     "nexixpay",
     "nmi",
     "nomupay",
     "noon",
     "nordea",
     "novalnet",
     "nuvei",
     "opennode",
     "paybox",
     "paydotcom",
     "payhound",
     "payload",
     "payme",
     "paynearme",
     "payone",
     "paypal",
     "paysafe",
     "paystack",
     "paytm",
     "payconex",
     "payu",
     "peachpayments",
     "payjustnow",
     "payjustnowinstore",
     "phonepe",
     "placetopay",
     "powertranz",
     "prophetpay",
     "rapyd",
     "razorpay",
     "recurly",
     "redsys",
     "revolv3",
     "saferpay",
     "santander",
     "shift4",
     "silverflow",
     "square",
     "stax",
     "stripe",
     "stripebilling",
     "taxjar",
     "threedsecureio",
     "tesouro",
     "tokenex",
     "tokenio",
     "trustly",
     "truelayer",
     "trustpay",
     "trustpayments",
     "tsys",
     "tsys_transit",
     "vgs",
     "volt",
     "wellsfargo",
     "wise",
     "worldline",
     "worldpay",
     "worldpayraft",
     "worldpayvantiv",
     "worldpayxml",
     "worldpaymodular",
     "signifyd",
     "nsure",
     "plaid",
     "riskified",
     "sanlam_payshield",
     "xendit",
     "zen",
     "zift",
     "zsl"}
);

constexpr auto kDecisions = std::to_array<std::string_view>({"retry", "do_default"});
constexpr auto kFeatures = std::to_array<std::string_view>({"retry"});
constexpr auto kErrorCategories = std::to_array<std::string_view>(
    {"frm_decline",
     "processor_downtime",
     "processor_decline_unauthorized",
     "issue_with_payment_method",
     "processor_decline_incorrect_data",
     "hard_decline",
     "soft_decline"}
);
constexpr auto kStandardisedCodes = std::to_array<std::string_view>(
    {"account_closed_or_invalid",
     "authentication_failed",
     "authentication_required",
     "authorization_missing_or_revoked",
     "card_lost_or_stolen",
     "card_not_supported_restricted",
     "cfg_pm_not_enabled_or_misconfigured",
     "compliance_or_sanctions_restriction",
     "configuration_issue",
     "credit_limit_exceeded",
     "currency_or_corridor_not_enabled",
     "do_not_honor",
     "downstream_technical_issue",
     "duplicate_request",
     "generic_unknown_error",
     "incorrect_authentication_code",
     "insufficient_funds",
     "integ_cryptographic_issue",
     "integration_issue",
     "invalid_card_number",
     "invalid_credentials",
     "invalid_cvv",
     "invalid_expiry_date",
     "invalid_state",
     "issuer_unavailable",
     "merchant_inactive",
     "missing_or_invalid_param",
     "operation_not_allowed",
     "payment_cancelled_by_user",
     "payment_method_issue",
     "payment_session_timeout",
     "pm_address_mismatch",
     "psp_acquirer_error",
     "psp_fraud_engine_decline",
     "rate_limit",
     "stored_credential_or_mit_not_enabled",
     "subscription_plan_inactive",
     "suspected_fraud",
     "three_ds_authentication_service_issue",
     "three_ds_configuration_issue",
     "three_ds_data_or_protocol_invalid",
     "transaction_not_permitted",
     "transaction_timed_out",
     "velocity_limit_exceeded",
     "wallet_or_token_config_issue"}
);

/**
 * @brief Shared validation, domain conversion and audit helpers for GSM sub-runtimes.
 */
class GsmRuntimeSupport
{
public:
    explicit GsmRuntimeSupport(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    // ── Request validation (upstream serde rejects unknown enum variants) ──────

    [[nodiscard]] std::optional<core::error::AppError>
    validate_create(const ::app::dto::gsm::GsmCreateRequest& in) const
    {
        if (const auto error = validate_key_fields(
                in.get_connector(),
                in.get_flow(),
                in.get_sub_flow(),
                in.get_code(),
                in.get_message(),
                true
            )) {
            return error;
        }
        if (!valid_decision(in.get_decision())) {
            return invalid_value("decision", in.get_decision());
        }
        if (in.get_status().empty()) {
            return core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing required field: status"
            );
        }
        if (in.get_feature() && !valid_feature(*in.get_feature())) {
            return invalid_value("feature", *in.get_feature());
        }
        if (in.get_error_category() && !valid_error_category(*in.get_error_category())) {
            return invalid_value("error_category", *in.get_error_category());
        }
        if (in.get_standardised_code() && !valid_standardised_code(*in.get_standardised_code())) {
            return invalid_value("standardised_code", *in.get_standardised_code());
        }
        return validate_feature_data(in.get_feature_data());
    }

    [[nodiscard]] std::optional<core::error::AppError>
    validate_update(const ::app::dto::gsm::GsmUpdateRequest& in) const
    {
        if (const auto error = validate_key_fields(
                in.get_connector(),
                in.get_flow(),
                in.get_sub_flow(),
                in.get_code(),
                in.get_message(),
                false
            )) {
            return error;
        }
        if (in.get_decision() && !valid_decision(*in.get_decision())) {
            return invalid_value("decision", *in.get_decision());
        }
        if (in.get_feature() && !valid_feature(*in.get_feature())) {
            return invalid_value("feature", *in.get_feature());
        }
        if (in.get_error_category() && !valid_error_category(*in.get_error_category())) {
            return invalid_value("error_category", *in.get_error_category());
        }
        if (in.get_standardised_code() && !valid_standardised_code(*in.get_standardised_code())) {
            return invalid_value("standardised_code", *in.get_standardised_code());
        }
        return validate_feature_data(in.get_feature_data());
    }

    // Upstream GsmCreateRequest/GsmRetrieveRequest use the typed `Connector` enum
    // (api_models/gsm.rs:9,66); GsmUpdateRequest/GsmDeleteRequest use plain String
    // (:80,136) so they are NOT enum-validated there.
    [[nodiscard]] std::optional<core::error::AppError> validate_key_fields(
        std::string_view connector,
        std::string_view flow,
        std::string_view sub_flow,
        std::string_view code,
        std::string_view message,
        bool typed_connector
    ) const
    {
        if (typed_connector && !valid_connector(connector)) {
            return invalid_value("connector", connector);
        }
        if (connector.empty() || flow.empty() || sub_flow.empty() || code.empty() ||
            message.empty()) {
            return core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing required field"
            );
        }
        return std::nullopt;
    }

    // ── Key / error helpers ─────────────────────────────────────────────────────

    [[nodiscard]] std::string rule_key(
        std::string_view connector,
        std::string_view flow,
        std::string_view sub_flow,
        std::string_view code,
        std::string_view message
    ) const
    {
        std::string key{"gsm.rule:"};
        const auto append_part = [&key](std::string_view part)
        {
            key.push_back('\x1f');
            key.append(part.data(), part.size());
        };
        key.append(connector.data(), connector.size());
        append_part(flow);
        append_part(sub_flow);
        append_part(code);
        append_part(message);
        return key;
    }

    [[nodiscard]] core::error::AppError not_found() const
    {
        return core::error::AppErrorBuilder{}.operator()(
            core::error::AppErrorKind::NOT_FOUND,
            "GSM with given key does not exist in our records"
        );
    }

    // Documented in-process substitute for server_wrap/#[instrument] telemetry:
    // upstream emits Flow metrics + ApiEventsType::Gsm (services/api.rs:420;
    // routes/gsm.rs:31,69,106,144; diesel_models/gsm.rs:161-164); no lock is taken.
    [[nodiscard]] std::string audit_payload(std::string_view operation, std::string_view key) const
    {
        std::string escaped;
        escaped.reserve(key.size());
        for (const char c: key) {
            const auto byte = static_cast<unsigned char>(c);
            if (c == '"' || c == '\\' || byte < 0x20) {
                escaped += std::format("\\u{:04x}", static_cast<unsigned int>(byte));
            } else {
                escaped.push_back(c);
            }
        }
        return std::format("{{\"natural_key\":\"{}\",\"operation\":\"{}\"}}", escaped, operation);
    }

    // ── Domain conversions (ForeignFrom / ForeignInto) ──────────────────────────

    [[nodiscard]] RetryFeatureData
    retry_from_dto(const ::app::dto::gsm::RetryFeatureData& data) const
    {
        return RetryFeatureData{
            data.get_step_up_possible(),
            data.get_clear_pan_possible(),
            data.get_alternate_network_possible(),
            data.get_decision()
        };
    }

    [[nodiscard]] ::app::dto::gsm::GsmFeatureData retry_to_dto(const RetryFeatureData& data) const
    {
        ::app::dto::gsm::RetryFeatureData dto;
        dto.set_step_up_possible(data.step_up_possible);
        dto.set_clear_pan_possible(data.clear_pan_possible);
        dto.set_alternate_network_possible(data.alternate_network_possible);
        dto.set_decision(data.decision);
        ::app::dto::gsm::GsmFeatureData feature_data;
        feature_data.set_retry(std::move(dto));
        return feature_data;
    }

    // GsmFeatureData::foreign_from(&GsmCreateRequest) (transformers.rs:2080-2097).
    [[nodiscard]] RetryFeatureData
    inferred_feature_data_from_create(const ::app::dto::gsm::GsmCreateRequest& in) const
    {
        return RetryFeatureData{
            in.get_step_up_possible(),
            in.get_clear_pan_possible(),
            false, // alternate_network_possible defaults to false
            in.get_decision()
        };
    }

    // ForeignFrom<GsmCreateRequest> for GatewayStatusMap (transformers.rs:2055-2078).
    [[nodiscard]] GatewayStatusMap
    gateway_map_from_create(const ::app::dto::gsm::GsmCreateRequest& in) const
    {
        GatewayStatusMap domain;
        domain.connector = in.get_connector();
        domain.flow = in.get_flow();
        domain.sub_flow = in.get_sub_flow();
        domain.code = in.get_code();
        domain.message = in.get_message();
        domain.status = in.get_status();
        domain.router_error = in.get_router_error();
        domain.unified_code = in.get_unified_code();
        domain.unified_message = in.get_unified_message();
        domain.error_category = in.get_error_category();
        domain.feature_data = in.get_feature_data()
                                  ? retry_from_dto(in.get_feature_data()->get_retry())
                                  : inferred_feature_data_from_create(in);
        domain.feature = in.get_feature() ? *in.get_feature() : "retry";
        domain.standardised_code = in.get_standardised_code();
        domain.description = in.get_description();
        domain.user_guidance_message = in.get_user_guidance_message();
        return domain;
    }

    // ForeignFrom<(&GsmUpdateRequest, GatewayStatusMap)> for
    // (GsmFeature, GsmFeatureData) (transformers.rs:2099-2153).
    [[nodiscard]] std::pair<std::string, RetryFeatureData> infer_feature_info(
        const ::app::dto::gsm::GsmUpdateRequest& in,
        const GatewayStatusMap& existing
    ) const
    {
        // get_decision() is Retry | DoDefault -> GsmFeature::Retry for both.
        const std::string db_inferred_feature{"retry"};
        const std::string feature = in.get_feature().value_or(db_inferred_feature);
        const RetryFeatureData& db = existing.feature_data;
        (void)feature; // Retry is the only GsmFeature variant.
        return {
            "retry",
            RetryFeatureData{
                in.get_step_up_possible().value_or(db.step_up_possible),
                in.get_clear_pan_possible().value_or(db.clear_pan_possible),
                db.alternate_network_possible,
                in.get_decision().value_or(db.decision)
            }
        };
    }

    // GatewayStatusMappingUpdate built in core/gsm.rs:98-128.
    [[nodiscard]] GatewayStatusMappingUpdate gateway_update_from(
        const ::app::dto::gsm::GsmUpdateRequest& in,
        const std::pair<std::string, RetryFeatureData>& inferred
    ) const
    {
        GatewayStatusMappingUpdate update;
        update.status = in.get_status();
        // Upstream always passes Some(router_error), even when it is None.
        update.router_error = in.get_router_error();
        update.unified_code = in.get_unified_code();
        update.unified_message = in.get_unified_message();
        update.error_category = in.get_error_category();
        update.standardised_code = in.get_standardised_code();
        update.description = in.get_description();
        update.user_guidance_message = in.get_user_guidance_message();
        update.feature_data = in.get_feature_data()
                                  ? std::optional<RetryFeatureData>{retry_from_dto(
                                        in.get_feature_data()->get_retry()
                                    )}
                                  : std::optional<RetryFeatureData>{inferred.second};
        update.feature = in.get_feature().value_or(inferred.first);
        return update;
    }

    // Applies the diesel changeset to the loaded domain record. Plain Option
    // fields leave the column untouched when absent; router_error always
    // overwrites. The derived step_up/clear_pan/decision columns are ignored
    // because feature_data is always present (domain try_from :99-145).
    [[nodiscard]] GatewayStatusMap
    apply_update(const GatewayStatusMap& existing, const GatewayStatusMappingUpdate& update) const
    {
        GatewayStatusMap result = existing;
        if (update.status) {
            result.status = *update.status;
        }
        result.router_error = update.router_error;
        if (update.unified_code) {
            result.unified_code = update.unified_code;
        }
        if (update.unified_message) {
            result.unified_message = update.unified_message;
        }
        if (update.error_category) {
            result.error_category = update.error_category;
        }
        if (update.standardised_code) {
            result.standardised_code = update.standardised_code;
        }
        if (update.description) {
            result.description = update.description;
        }
        if (update.user_guidance_message) {
            result.user_guidance_message = update.user_guidance_message;
        }
        result.feature = update.feature;
        if (update.feature_data) {
            result.feature_data = *update.feature_data;
        }
        return result;
    }

    // ForeignFrom<GatewayStatusMap> for GsmResponse (transformers.rs:2155-2186).
    [[nodiscard]] ::app::dto::gsm::GsmResponse
    response_from_domain(const GatewayStatusMap& domain) const
    {
        ::app::dto::gsm::GsmResponse response;
        response.set_connector(domain.connector);
        response.set_flow(domain.flow);
        response.set_sub_flow(domain.sub_flow);
        response.set_code(domain.code);
        response.set_message(domain.message);
        response.set_decision(domain.feature_data.decision);
        response.set_status(domain.status);
        response.set_router_error(domain.router_error);
        response.set_step_up_possible(domain.feature_data.step_up_possible);
        response.set_unified_code(domain.unified_code);
        response.set_unified_message(domain.unified_message);
        response.set_error_category(domain.error_category);
        response.set_clear_pan_possible(domain.feature_data.clear_pan_possible);
        response.set_feature(domain.feature);
        response.set_feature_data(retry_to_dto(domain.feature_data));
        response.set_standardised_code(domain.standardised_code);
        response.set_description(domain.description);
        response.set_user_guidance_message(domain.user_guidance_message);
        return response;
    }

    [[nodiscard]] GatewayStatusMap
    domain_from_response(const ::app::dto::gsm::GsmResponse& response) const
    {
        GatewayStatusMap domain;
        domain.connector = response.get_connector();
        domain.flow = response.get_flow();
        domain.sub_flow = response.get_sub_flow();
        domain.code = response.get_code();
        domain.message = response.get_message();
        domain.status = response.get_status();
        domain.router_error = response.get_router_error();
        domain.unified_code = response.get_unified_code();
        domain.unified_message = response.get_unified_message();
        domain.error_category = response.get_error_category();
        domain.feature = response.get_feature();
        domain.standardised_code = response.get_standardised_code();
        domain.description = response.get_description();
        domain.user_guidance_message = response.get_user_guidance_message();
        if (response.get_feature_data()) {
            domain.feature_data = retry_from_dto(response.get_feature_data()->get_retry());
        } else {
            // Legacy records with null feature_data (domain try_from :108-125).
            domain.feature_data = RetryFeatureData{
                response.get_step_up_possible(),
                response.get_clear_pan_possible(),
                false,
                response.get_decision()
            };
        }
        return domain;
    }

    [[nodiscard]] ::app::store::gsm_rules::GsmRuleRecord
    record_from_domain(std::string_view key, const GatewayStatusMap& domain) const
    {
        ::app::store::gsm_rules::GsmRuleRecord record;
        record.set_natural_key(std::string{key});
        record.set_merchant_id({});
        record.set_rule_name(std::format("{}/{}", domain.connector, domain.flow));
        record.set_decision(domain.feature_data.decision);
        record.set_program_json(serde::Ser::encode_json(response_from_domain(domain)));
        record.set_status(domain.status);
        record.set_created_at({});
        return record;
    }

    [[nodiscard]] std::optional<GatewayStatusMap>
    domain_from_record(const ::app::store::gsm_rules::GsmRuleRecord& record) const
    {
        if (record.get_program_json().empty()) {
            return std::nullopt;
        }
        auto response = serde::Ser::deserialize<::app::dto::gsm::GsmResponse>(
            "application/json",
            record.get_program_json()
        );
        if (!response) {
            return std::nullopt;
        }
        return domain_from_response(*response);
    }

private:
    template<std::size_t N>
    [[nodiscard]] bool
    one_of(std::string_view value, const std::array<std::string_view, N>& allowed) const
    {
        return std::ranges::find(allowed, value) != allowed.end();
    }

    [[nodiscard]] bool valid_connector(std::string_view value) const
    {
        return one_of(value, kConnectors);
    }

    [[nodiscard]] bool valid_decision(std::string_view value) const
    {
        return one_of(value, kDecisions);
    }

    [[nodiscard]] bool valid_feature(std::string_view value) const
    {
        return one_of(value, kFeatures);
    }

    [[nodiscard]] bool valid_error_category(std::string_view value) const
    {
        return one_of(value, kErrorCategories);
    }

    [[nodiscard]] bool valid_standardised_code(std::string_view value) const
    {
        return one_of(value, kStandardisedCodes);
    }

    [[nodiscard]] core::error::AppError
    invalid_value(std::string_view field, std::string_view value) const
    {
        return core::error::AppErrorBuilder{}.operator()(
            core::error::AppErrorKind::INVALID_REQUEST,
            std::format("invalid value provided: {} ({})", field, value)
        );
    }

    [[nodiscard]] std::optional<core::error::AppError>
    validate_feature_data(const std::optional<::app::dto::gsm::GsmFeatureData>& feature_data) const
    {
        if (feature_data && !valid_decision(feature_data->get_retry().get_decision())) {
            return invalid_value("feature_data.decision", feature_data->get_retry().get_decision());
        }
        return std::nullopt;
    }

    ::app::context::AppContext& m_context;
};

} // namespace app::handlers::gsm
