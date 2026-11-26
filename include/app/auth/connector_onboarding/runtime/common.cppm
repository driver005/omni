export module app_auth:connector_onboarding_runtime_common;

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
import app_context_core;
import app_http;
import app_crypto;
import app_metrics;
import app_context_core;
import app_connector;
import app_events;
import app_workflow;
import app_keystore;

export namespace app::handlers::connector_onboarding {

/** @brief Local substitute for settings::ConnectorOnboarding + conf.connectors.paypal.
 *
 *  Upstream: configs/settings.rs:1804-1817 (paypal.enabled/client_id/client_secret/
 *  partner_id) and core/connector_onboarding/paypal.rs:18-23 (connectors.paypal.base_url).
 */
struct ConnectorOnboardingConf
{
    bool enabled{true};
    std::string client_id{};
    std::string client_secret{};
    std::string partner_id{};
    std::string base_url{"https://www.paypal.com/"};
};

/** @brief Local substitute for types::ConnectorAuthType (BodyKey / SignatureKey).
 *  Upstream api_models/admin.rs:1582-1597 uses #[serde(tag = "auth_type")].
 */
struct ConnectorAuth
{
    std::string auth_type{};
    std::string api_key{};
    std::string key1{};
    std::string api_secret{};
};

/** @brief Local mirror of api_models::PayPalOnboardingStatus (api_models:37-47). */
struct OnboardingOutcome
{
    std::string_view wire{};
    std::string payer_id{};
    bool success{false};
};

/** @brief Local substitute for types::api::connector_onboarding::paypal::SellerStatusResponse. */
struct PayPalMerchantStatus
{
    std::string merchant_id{};
    std::string details_url{};
};

/**
 * @brief Shared business helpers for the connector_onboarding sub-runtimes.
 *
 * Error construction keeps the upstream variant name in `kind_name` so the wire body
 * stays {"error":KIND,"message":TEXT}. The HTTP status of the upstream variants that
 * differ from the AppErrorKind status is recovered by the handler from `kind_name`
 * (see ConnectorOnboardingInputSupport::status_override_for).
 */
class ConnectorOnboardingRuntimeSupport
{
public:
    explicit ConnectorOnboardingRuntimeSupport(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    // ── Error construction ───────────────────────────────────────────────────────

    [[nodiscard]] core::error::AppError
    local_error(core::error::AppErrorKind kind, std::string message, std::string kind_name) const
    {
        auto error = core::error::AppErrorBuilder{}.operator()(kind, std::move(message));
        error.set_kind_name(std::move(kind_name));
        return error;
    }

    [[nodiscard]] core::error::AppError
    simple_error(core::error::AppErrorKind kind, std::string message) const
    {
        return local_error(
            kind,
            std::move(message),
            std::string{core::error::AppErrorKindName{}(kind)}
        );
    }

    // Upstream ApiErrorResponse::NotImplemented -> AER::NotImplemented (501),
    // IR_0, NotImplementedMessage::Reason Debug appends " is not implemented"
    // (hyperswitch_domain_models/src/errors/api_error_response.rs:176-177,355-371,557-559).
    [[nodiscard]] core::error::AppError not_implemented(std::string_view connector) const
    {
        return local_error(
            core::error::AppErrorKind::INTERNAL_ERROR,
            std::format("Onboarding is not implemented for {} is not implemented", connector),
            "NotImplemented"
        );
    }

    // Upstream ApiErrorResponse::FlowNotSupported -> AER::BadRequest (400), IR_20
    // (api_error_response.rs:240-241,637-639); live message is "{flow} flow not supported".
    [[nodiscard]] core::error::AppError flow_not_supported(std::string_view connector) const
    {
        (void)connector; // carried in upstream's `extra.connector`; the local wire has no extras.
        return local_error(
            core::error::AppErrorKind::INVALID_REQUEST,
            "Connector onboarding flow not supported",
            "FlowNotSupported"
        );
    }

    // Upstream ApiErrorResponse::PreconditionFailed -> AER::BadRequest (400), IR_16
    // (api_error_response.rs:226-227,626-628).
    [[nodiscard]] core::error::AppError precondition_failed(std::string message) const
    {
        return local_error(
            core::error::AppErrorKind::PRECONDITION_FAILED,
            std::move(message),
            "PreconditionFailed"
        );
    }

    [[nodiscard]] core::error::AppError internal_error(std::string message) const
    {
        return local_error(
            core::error::AppErrorKind::INTERNAL_ERROR,
            std::move(message),
            "InternalServerError"
        );
    }

    // ── utils::connector_onboarding helpers ───────────────────────────────────────

    // Upstream utils/connector_onboarding.rs:17-33. The PayPal branch returns a BodyKey
    // auth built from client_secret/client_id; every other connector returns NotImplemented.
    [[nodiscard]] std::expected<ConnectorAuth, core::error::AppError>
    get_connector_auth(std::string_view connector, const ConnectorOnboardingConf& conf) const
    {
        if (is_paypal(connector)) {
            return ConnectorAuth{"BodyKey", conf.client_secret, conf.client_id, std::string{}};
        }
        return std::unexpected{not_implemented(connector)};
    }

    // Upstream utils/connector_onboarding.rs:35-43 -> Option<bool>.
    [[nodiscard]] std::optional<bool>
    is_enabled(std::string_view connector, const ConnectorOnboardingConf& conf) const
    {
        if (is_paypal(connector)) {
            return conf.enabled;
        }
        return std::nullopt;
    }

    // Upstream core/utils.rs:2889-2912. The three-way match is reproduced exactly.
    [[nodiscard]] std::optional<core::error::AppError> validate_profile_id_from_auth_layer(
        std::optional<std::string> auth_profile,
        std::optional<std::string> object_profile
    ) const
    {
        if (auth_profile.has_value() && object_profile.has_value()) {
            if (*auth_profile != *object_profile) {
                return precondition_failed(
                    "Profile id authentication failed. Please use the correct JWT token"
                );
            }
            return std::nullopt;
        }
        if (auth_profile.has_value() && !object_profile.has_value()) {
            return precondition_failed("Couldn't find profile_id in record for authentication");
        }
        return std::nullopt;
    }

    // Upstream utils/connector_onboarding.rs:45-83. The merchant key store is read as the
    // wrap-material source; the local key store only carries that material when seeded, so
    // existence is established through the repository (the stand-in for accounts_store).
    [[nodiscard]] std::optional<core::error::AppError> check_if_connector_exists(
        std::string_view connector_id,
        std::string_view merchant_id,
        std::optional<std::string> profile_id
    ) const
    {
        const auto master_key = m_context.key_store().get("master_key", "merchant_key_store");
        const auto merchant_key_store = m_context.key_store().get("merchant_key", merchant_id);
        (void)master_key;
        (void)merchant_key_store;

        if (merchant_id.empty() ||
            !m_context.repository().find_merchant_account_by_string(merchant_id).has_value()) {
            return simple_error(core::error::AppErrorKind::NOT_FOUND, "merchant account not found");
        }
        if (connector_id.empty()) {
            return simple_error(
                core::error::AppErrorKind::NOT_FOUND,
                std::format("merchant connector account not found: {}", connector_id)
            );
        }
        auto found = m_context.repository().find_merchant_connector_by_string(connector_id);
        if (!found.has_value() || found->get_merchant_id() != merchant_id) {
            return simple_error(
                core::error::AppErrorKind::NOT_FOUND,
                std::format("merchant connector account not found: {}", connector_id)
            );
        }
        std::optional<std::string> object_profile{};
        if (!found->get_profile_id().empty()) {
            object_profile = found->get_profile_id();
        }
        return validate_profile_id_from_auth_layer(profile_id, object_profile);
    }

    // Upstream build_key (utils/connector_onboarding.rs:144-154) with
    // consts::CONNECTOR_ONBOARDING_CONFIG_PREFIX == "onboarding" (consts.rs:107).
    [[nodiscard]] std::string
    build_key(std::string_view connector_id, std::string_view connector) const
    {
        return std::format("{}_{}_{}", "onboarding", normalized_connector(connector), connector_id);
    }

    [[nodiscard]] std::string now_unix_timestamp() const
    {
        const auto epoch = std::chrono::system_clock::now().time_since_epoch();
        return std::to_string(std::chrono::duration_cast<std::chrono::seconds>(epoch).count());
    }

    // Upstream utils/connector_onboarding.rs:85-123: find_config_by_key_optional then
    // update_config_by_key or insert_config on the `configs` table (repository here).
    [[nodiscard]] std::optional<core::error::AppError>
    set_tracking_id_in_configs(std::string_view connector_id, std::string_view connector) const
    {
        const auto key = build_key(connector_id, connector);
        const auto timestamp = now_unix_timestamp();
        auto repository = m_context.repository();
        auto existing = repository.find_config_by_key(key);
        if (existing.has_value()) {
            existing->set_config_value(timestamp);
            if (!repository.update_config(*existing)) {
                return internal_error("Error updating data in configs table");
            }
        } else {
            ::app::store::configs::ConfigRecord created{};
            created.set_config_key(key);
            created.set_config_value(timestamp);
            repository.save_config(std::move(created));
        }
        return std::nullopt;
    }

    // Upstream utils/connector_onboarding.rs:125-142: find_config_by_key_unwrap_or with a
    // freshly minted unix timestamp, then "<connector_id>_<timestamp>" (no insert).
    [[nodiscard]] std::expected<std::string, core::error::AppError>
    get_tracking_id_from_configs(std::string_view connector_id, std::string_view connector) const
    {
        const auto key = build_key(connector_id, connector);
        auto existing = m_context.repository().find_config_by_key(key);
        const auto timestamp =
            existing.has_value() ? existing->get_config_value() : now_unix_timestamp();
        return std::format("{}_{}", connector_id, timestamp);
    }

    // ── core/connector_onboarding/paypal.rs substitutes ───────────────────────────

    [[nodiscard]] std::string build_referral_url(std::string_view base_url) const
    {
        return std::format("{}v2/customer/partner-referrals", base_url);
    }

    // Upstream types/api/connector_onboarding/paypal.rs:112-147 (PartnerReferralRequest::new).
    [[nodiscard]] std::string
    build_referral_request(std::string_view tracking_id, std::string_view return_url) const
    {
        const auto tid = serde::Ser::encode_json(std::string{tracking_id});
        const auto url = serde::Ser::encode_json(std::string{return_url});
        std::string out{};
        out += "{\"tracking_id\":" + tid;
        out += ",\"operations\":[{\"operation\":\"API_INTEGRATION\",\"api_integration_preference\":"
               "{\"rest_api_integration\":{";
        out += "\"integration_method\":\"PAYPAL\",\"integration_type\":\"THIRD_PARTY\",\"third_"
               "party_details\":{\"features\":[";
        out += "\"PAYMENT\",\"REFUND\",\"VAULT\",\"ACCESS_MERCHANT_INFORMATION\",\"BILLING_"
               "AGREEMENT\",\"READ_SELLER_DISPUTE\"]}}}}]";
        out += ",\"products\":[\"PPCP\",\"ADVANCED_VAULTING\"]";
        out += ",\"capabilities\":[\"PAYPAL_WALLET_VAULTING_ADVANCED\"]";
        out += ",\"partner_config_override\":{\"partner_logo_url\":\"https://hyperswitch.io/img/"
               "websiteIcon.svg\",\"return_url\":" +
               url + "}";
        out += ",\"legal_consents\":[{\"type\":\"SHARE_DATA_CONSENT\",\"granted\":true}]}";
        return out;
    }

    // Upstream utils/connector_onboarding/paypal.rs:17-40: get_connector_auth feeds the
    // PayPal access-token call. The injected connector's exchange_token is the in-process
    // substitute for Paypal::get_access_token / the HTTP token endpoint.
    [[nodiscard]] std::expected<std::string, core::error::AppError> generate_access_token(
        std::string_view connector,
        const ConnectorOnboardingConf& conf,
        std::string_view merchant_id
    ) const
    {
        auto auth = get_connector_auth(connector, conf);
        if (!auth) {
            return std::unexpected{std::move(auth.error())};
        }
        if (auto service = m_context.connector_router().select(merchant_id)) {
            const auto payload = std::format(
                R"({{"api_key":{},"key1":{}}})",
                serde::Ser::encode_json(auth->api_key),
                serde::Ser::encode_json(auth->key1)
            );
            auto request = ::app::connector::ConnectorRequestBuilder{}
                               .operation("generate_access_token")
                               .merchant_id(std::string{merchant_id})
                               .resource_id(auth->key1)
                               .raw_payload(payload)();
            const auto response = service->exchange_token(request);
            if (const auto token = response_field_text(response, "access_token")) {
                return *token;
            }
            if (!response.get_reference_id().empty()) {
                return response.get_reference_id();
            }
        }
        return auth->api_key + ":" + auth->key1;
    }

    // Upstream types/.../paypal.rs:235-244: find the HATEOAS link whose rel == "action_url".
    [[nodiscard]] std::expected<std::string, core::error::AppError>
    extract_action_url(std::string_view text) const
    {
        auto parsed = serde::Ser::decode_generic("application/json", text);
        if (parsed.has_value()) {
            const auto& root = parsed->get();
            if (const auto* object = std::get_if<interfaces::Value::Object>(&root)) {
                const auto links = object->get("links");
                if (links.has_value()) {
                    if (const auto* array =
                            std::get_if<interfaces::Value::Array>(&links.value().get())) {
                        for (const auto& link: *array) {
                            if (const auto* link_object =
                                    std::get_if<interfaces::Value::Object>(&link.get())) {
                                if (equals_ignore_case(
                                        child_text(*link_object, "rel"),
                                        "action_url"
                                    )) {
                                    const auto href = child_text(*link_object, "href");
                                    if (!href.empty()) {
                                        return href;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        return std::unexpected{internal_error("Failed to get action_url from paypal response")};
    }

    // Upstream core/connector_onboarding/paypal.rs:40-63. The partner-referral POST is
    // replaced by the injected connector's create_link; the response goes through the same
    // extract_action_url helper as upstream's PartnerReferralResponse.
    [[nodiscard]] std::expected<std::string, core::error::AppError> get_action_url_from_paypal(
        std::string_view tracking_id,
        std::string_view return_url,
        std::string_view merchant_id
    ) const
    {
        const auto conf = connector_onboarding_conf();
        auto token = generate_access_token("paypal", conf, merchant_id);
        if (!token) {
            return std::unexpected{std::move(token.error())};
        }
        const auto referral_url = build_referral_url(conf.base_url);
        const auto body = build_referral_request(tracking_id, return_url);
        if (auto service = m_context.connector_router().select(merchant_id)) {
            auto request = ::app::connector::ConnectorRequestBuilder{}
                               .operation("create_link")
                               .merchant_id(std::string{merchant_id})
                               .resource_id(std::string{tracking_id})
                               .raw_payload(body)();
            const auto response = service->create_link(request);
            if (!response.get_raw_json().empty()) {
                return extract_action_url(response.get_raw_json());
            }
            if (!response.get_reference_id().empty()) {
                return extract_action_url(
                    std::format(
                        R"({{"links":[{{"rel":"action_url","href":{}}}]}})",
                        serde::Ser::encode_json(response.get_reference_id())
                    )
                );
            }
        }
        // Deterministic in-process stand-in for the PayPal partner-referral response.
        (void)referral_url; // request target built upstream as build_referral_url(conf.base_url).
        std::string href =
            std::format("{}bizsignup/entry?tracking_id={}", conf.base_url, tracking_id);
        if (!return_url.empty()) {
            href += std::format("&return_url={}", return_url);
        }
        return extract_action_url(
            std::format(
                R"({{"links":[{{"rel":"action_url","href":{}}}]}})",
                serde::Ser::encode_json(href)
            )
        );
    }

    [[nodiscard]] std::string merchant_onboarding_status_url(
        std::string_view base_url,
        std::string_view partner_id,
        std::string_view tracking_id
    ) const
    {
        return std::format(
            "{}v1/customer/partners/{}/merchant-integrations?tracking_id={}",
            base_url,
            partner_id,
            tracking_id
        );
    }

    // Upstream types/.../paypal.rs:177-186: strip the leading '/' and prefix base_url.
    [[nodiscard]] std::expected<std::string, core::error::AppError>
    extract_merchant_details_url(std::string_view href, std::string_view base_url) const
    {
        if (!href.starts_with('/')) {
            return std::unexpected{
                internal_error("Merchant details not received in onboarding status")
            };
        }
        return std::string{base_url} + std::string{href.substr(1)};
    }

    // Upstream core/connector_onboarding/paypal.rs:117-142. A non-success PayPal status
    // becomes None -> AccountNotFound; the injected connector's sync is the substitute.
    [[nodiscard]] std::expected<std::optional<PayPalMerchantStatus>, core::error::AppError>
    find_paypal_merchant_by_tracking_id(
        std::string_view tracking_id,
        std::string_view access_token,
        const ConnectorOnboardingConf& conf,
        std::string_view merchant_id
    ) const
    {
        (void)access_token;
        const auto url =
            merchant_onboarding_status_url(conf.base_url, conf.partner_id, tracking_id);
        if (auto service = m_context.connector_router().select(merchant_id)) {
            auto request =
                ::app::connector::ConnectorRequestBuilder{}
                    .operation("sync_onboarding_status")
                    .merchant_id(std::string{merchant_id})
                    .resource_id(std::string{tracking_id})
                    .raw_payload(std::format(R"({{"url":{}}})", serde::Ser::encode_json(url)))();
            const auto response = service->sync(request);
            if (response.get_raw_json().empty()) {
                return std::nullopt;
            }
            auto parsed = serde::Ser::decode_generic("application/json", response.get_raw_json());
            if (!parsed.has_value()) {
                return std::nullopt;
            }
            const auto& root = parsed->get();
            const auto* object = std::get_if<interfaces::Value::Object>(&root);
            if (object == nullptr) {
                return std::nullopt;
            }
            const auto merchant = child_text(*object, "merchant_id");
            std::string href{};
            const auto links = object->get("links");
            if (links.has_value()) {
                if (const auto* array =
                        std::get_if<interfaces::Value::Array>(&links.value().get())) {
                    if (!array->empty()) {
                        if (const auto* first =
                                std::get_if<interfaces::Value::Object>(&array->front().get())) {
                            href = child_text(*first, "href");
                        }
                    }
                }
            }
            if (merchant.empty() || href.empty()) {
                return std::nullopt;
            }
            auto details = extract_merchant_details_url(href, conf.base_url);
            if (!details) {
                return std::unexpected{std::move(details.error())};
            }
            return PayPalMerchantStatus{merchant, *details};
        }
        return PayPalMerchantStatus{
            std::string{merchant_id},
            std::format(
                "/v1/customer/partners/{}/merchant-integrations?tracking_id={}",
                conf.partner_id,
                tracking_id
            )
        };
    }

    // Upstream types/.../paypal.rs:188-222 (SellerStatusDetailsResponse::get_eligibility_status).
    [[nodiscard]] OnboardingOutcome get_eligibility_status(
        std::string_view merchant_id,
        bool payments_receivable,
        bool primary_email_confirmed,
        std::string_view ppcp_vetting_status
    ) const
    {
        if (!payments_receivable) {
            return {"payments_not_receivable", std::string{}, false};
        }
        if (!primary_email_confirmed) {
            return {"email_not_verified", std::string{}, false};
        }
        if (equals_ignore_case(ppcp_vetting_status, "denied")) {
            return {"ppcp_custom_denied", std::string{}, false};
        }
        if (!equals_ignore_case(ppcp_vetting_status, "subscribed")) {
            return {"more_permissions_needed", std::string{}, false};
        }
        return {"success", std::string{merchant_id}, true};
    }

    // Upstream core/connector_onboarding/paypal.rs:81-115. The merchant-details GET is
    // replaced by a second injected-connector sync keyed by the extracted details URL.
    [[nodiscard]] std::expected<OnboardingOutcome, core::error::AppError>
    sync_merchant_onboarding_status(
        std::string_view tracking_id,
        std::string_view merchant_id
    ) const
    {
        const auto conf = connector_onboarding_conf();
        auto token = generate_access_token("paypal", conf, merchant_id);
        if (!token) {
            return std::unexpected{std::move(token.error())};
        }
        auto found = find_paypal_merchant_by_tracking_id(tracking_id, *token, conf, merchant_id);
        if (!found) {
            return std::unexpected{std::move(found.error())};
        }
        if (!found->has_value()) {
            return OnboardingOutcome{"account_not_found", std::string{}, false};
        }

        bool payments_receivable = true;
        bool primary_email_confirmed = true;
        std::string vetting_status{"subscribed"};
        if (auto service = m_context.connector_router().select(merchant_id)) {
            auto request = ::app::connector::ConnectorRequestBuilder{}
                               .operation("sync_onboarding_status")
                               .merchant_id(std::string{merchant_id})
                               .resource_id((*found)->details_url)();
            const auto response = service->sync(request);
            auto parsed = serde::Ser::decode_generic("application/json", response.get_raw_json());
            if (parsed.has_value()) {
                if (const auto* object = std::get_if<interfaces::Value::Object>(&parsed->get())) {
                    if (const auto flag = child_bool(*object, "payments_receivable")) {
                        payments_receivable = *flag;
                    }
                    if (const auto flag = child_bool(*object, "primary_email_confirmed")) {
                        primary_email_confirmed = *flag;
                    }
                    const auto products = object->get("products");
                    if (products.has_value()) {
                        if (const auto* array =
                                std::get_if<interfaces::Value::Array>(&products.value().get())) {
                            for (const auto& product: *array) {
                                if (const auto* product_object =
                                        std::get_if<interfaces::Value::Object>(&product.get())) {
                                    if (equals_ignore_case(
                                            child_text(*product_object, "name"),
                                            "PPCP_CUSTOM"
                                        )) {
                                        vetting_status =
                                            child_text(*product_object, "vetting_status");
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        return get_eligibility_status(
            (*found)->merchant_id,
            payments_receivable,
            primary_email_confirmed,
            vetting_status
        );
    }

    // Upstream core/connector_onboarding/paypal.rs:144-201 (update_mca via
    // admin::update_connector). The SignatureKey auth JSON is stored through the repository;
    // AesWrap is the local stand-in for the upstream vault encryption of Secret<serde_json::Value>.
    [[nodiscard]] std::
        expected<::app::store::merchant_connectors::MerchantConnectorRecord, core::error::AppError>
        update_mca(
            std::string_view merchant_id,
            std::string_view profile_id,
            std::string_view connector_id,
            const ConnectorAuth& auth_details
        ) const
    {
        (void)merchant_id;
        (void)profile_id;
        auto found = m_context.repository().find_merchant_connector_by_string(connector_id);
        if (!found.has_value()) {
            return std::unexpected{simple_error(
                core::error::AppErrorKind::NOT_FOUND,
                std::format("merchant connector account not found: {}", connector_id)
            )};
        }
        auto record = std::move(*found);
        const auto auth_json = std::format(
            R"({{"auth_type":"{}","api_key":{},"key1":{},"api_secret":{}}})",
            auth_details.auth_type,
            serde::Ser::encode_json(auth_details.api_key),
            serde::Ser::encode_json(auth_details.key1),
            serde::Ser::encode_json(auth_details.api_secret)
        );
        const auto wrapped = ::app::crypto::AesWrap{}.encrypt(
            auth_json,
            connector_key(record.get_merchant_id(), "paypal")
        );
        record.set_connector_type("payment_processor");
        record.set_connector_account_details(wrapped);
        record.set_disabled(false);
        record.set_status("active");
        m_context.repository().update_merchant_connector(record);
        return record;
    }

    // ── Small parsing / formatting utilities ──────────────────────────────────────

    [[nodiscard]] ConnectorOnboardingConf connector_onboarding_conf() const
    {
        ConnectorOnboardingConf conf{};
        if (const auto value =
                m_context.config_store().get_value("connector_onboarding", "paypal.enabled")) {
            conf.enabled = equals_ignore_case(*value, "true");
        }
        conf.client_id = connector_credential("paypal", "client_id");
        conf.client_secret = connector_credential("paypal", "client_secret");
        if (const auto value =
                m_context.config_store().get_value("connector_onboarding", "paypal.partner_id")) {
            conf.partner_id = *value;
        }
        if (const auto value =
                m_context.config_store().get_value("connectors", "paypal_base_url")) {
            conf.base_url = *value;
        }
        return conf;
    }

    [[nodiscard]] std::string
    connector_credential(std::string_view connector, std::string_view kind) const
    {
        const auto key = std::format("{}.{}", normalized_connector(connector), kind);
        if (const auto value = m_context.config_store().get_value("connector_onboarding", key)) {
            return *value;
        }
        if (const auto secret = m_context.key_store().get("connector_onboarding", key)) {
            return secret->value();
        }
        return {};
    }

    [[nodiscard]] std::string
    connector_key(std::string_view merchant, std::string_view connector) const
    {
        std::string material = std::string{merchant} + ":" + std::string{connector};
        if (const auto master = m_context.key_store().get("merchant_key", merchant)) {
            material += ":" + master->value();
        }
        return ::app::crypto::KeyedHasher{"connector_onboarding.mca.key"}.hash(material);
    }

    [[nodiscard]] bool is_paypal(std::string_view name) const
    {
        return equals_ignore_case(name, "paypal");
    }

    [[nodiscard]] std::string normalized_connector(std::string_view name) const
    {
        std::string out{};
        out.reserve(name.size());
        for (const char ch: name) {
            out.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(ch))));
        }
        return out;
    }

    [[nodiscard]] bool equals_ignore_case(std::string_view lhs, std::string_view rhs) const
    {
        return lhs.size() == rhs.size() &&
               std::equal(
                   lhs.begin(),
                   lhs.end(),
                   rhs.begin(),
                   [](char a, char b)
                   {
                       return std::tolower(static_cast<unsigned char>(a)) ==
                              std::tolower(static_cast<unsigned char>(b));
                   }
               );
    }

    [[nodiscard]] std::string
    child_text(const interfaces::Value::Object& object, std::string_view key) const
    {
        const auto found = object.get(std::string{key});
        if (!found.has_value()) {
            return {};
        }
        return json_text(found.value());
    }

    [[nodiscard]] std::optional<bool>
    child_bool(const interfaces::Value::Object& object, std::string_view key) const
    {
        const auto found = object.get(std::string{key});
        if (!found.has_value()) {
            return std::nullopt;
        }
        const auto& variant = found.value().get();
        if (const auto* flag = std::get_if<bool>(&variant)) {
            return *flag;
        }
        const auto text = json_text(found.value());
        if (!text.empty()) {
            return equals_ignore_case(text, "true");
        }
        return std::nullopt;
    }

    [[nodiscard]] std::string json_text(const interfaces::Value& value) const
    {
        const auto& variant = value.get();
        if (const auto* text = std::get_if<std::string>(&variant)) {
            return *text;
        }
        if (const auto* object = std::get_if<interfaces::Value::Object>(&variant)) {
            if (const auto inner = object->get("value")) {
                const auto& inner_variant = inner.value().get();
                if (const auto* text = std::get_if<std::string>(&inner_variant)) {
                    return *text;
                }
            }
        }
        return {};
    }

    [[nodiscard]] std::optional<std::string> response_field_text(
        const ::app::connector::ConnectorResponse& response,
        std::string_view field
    ) const
    {
        if (response.get_raw_json().empty()) {
            return std::nullopt;
        }
        auto parsed = serde::Ser::decode_generic("application/json", response.get_raw_json());
        if (!parsed.has_value()) {
            return std::nullopt;
        }
        const auto& root = parsed->get();
        const auto* object = std::get_if<interfaces::Value::Object>(&root);
        if (object == nullptr) {
            return std::nullopt;
        }
        const auto found = object->get(std::string{field});
        if (!found.has_value()) {
            return std::nullopt;
        }
        const auto text = json_text(found.value());
        if (text.empty()) {
            return std::nullopt;
        }
        return text;
    }

private:
    ::app::context::AppContext& m_context;
};

} // namespace app::handlers::connector_onboarding
