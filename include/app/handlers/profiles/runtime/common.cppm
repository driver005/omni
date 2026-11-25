export module app_handlers_profiles:runtime_common;

import std;
import core_serde;
import core_cache;
import core_error;
import app_context;
import app_dto;
import app_store;
import app_events;

import :input;
import :result;

export namespace app::handlers::profiles {

/**
 * @brief Shared profile business helpers, validators and store accessors.
 *
 * Every sub-runtime holds one of these; it owns no HTTP types and returns
 * plain values/errors to its caller.
 */
class ProfilesRuntimeSupport
{
public:
    explicit ProfilesRuntimeSupport(app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    using Response = ProfileWireResponse;
    using ListResult = std::expected<std::vector<Response>, core::error::AppError>;
    using CreateInput = app::dto::admin::ProfileCreate;
    using UpdateInput = app::dto::admin::ProfileUpdate;

    // core/admin.rs:4122-4150 — list_profile.
    [[nodiscard]] ListResult list_profile(
        std::string_view merchant_id,
        const std::optional<std::vector<std::string>>& profile_id_list
    ) const
    {
        // core/admin.rs:4127-4133 — master key + key store lookup.
        const auto account = get_merchant_key_store_by_merchant_id(merchant_id, master_key());
        if (!account) {
            return std::unexpected(
                make_error(core::error::AppErrorKind::NOT_FOUND, "merchant account not found")
            );
        }
        // core/admin.rs:4134-4139 — list_profile_by_merchant_id.
        auto profiles = list_profile_by_merchant_id(merchant_id);
        // core/admin.rs:4140 — filter_objects_based_on_profile_id_list.
        profiles = filter_objects_based_on_profile_id_list(profile_id_list, std::move(profiles));
        // core/admin.rs:4143-4145 — foreign_try_from(profile). The typed wire
        // objects go straight to the handler; no JSON text is assembled here.
        return std::move(profiles);
    }

    // core/admin.rs:3576-3826 (v1) / :3830+ (v2) — create_domain_model_from_request.
    [[nodiscard]] std::expected<Response, core::error::AppError>
    create_domain_model_from_request(std::string_view merchant_id, const CreateInput& request) const
    {
        // core/admin.rs:3583-3589 + helpers.rs:8425,8549.
        if (const auto error = validate_session_expiry(request.get_session_expiry())) {
            return std::unexpected(*error);
        }
        if (const auto error =
                validate_intent_fulfillment_expiry(request.get_intent_fulfillment_time())) {
            return std::unexpected(*error);
        }
        // core/admin.rs:3591-3599 — static routing algorithm parse.
        if (request.get_routing_algorithm()) {
            if (const auto error =
                    validate_static_routing_algorithm(*request.get_routing_algorithm())) {
                return std::unexpected(*error);
            }
        }
        // core/admin.rs:3607-3621 — outbound destination validation.
        if (request.get_webhook_details() && request.get_webhook_details()->get_webhook_url()) {
            if (const auto error = validate_webhook_destination(
                    *request.get_webhook_details()->get_webhook_url()
                )) {
                return std::unexpected(*error);
            }
        }
        // core/admin.rs:3698-3704 — merchant country code validation.
        if (request.get_merchant_country_code() &&
            request.get_merchant_country_code()->get_value().empty()) {
            return std::unexpected(make_error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "Invalid merchant country code"
            ));
        }

        auto repository = m_context.repository();
        auto account = repository.find_merchant_account_by_string(merchant_id);
        Response domain;
        const std::string profile_id = m_context.ids().payment_id("profile_");
        // Plain id strings on the profile-owned wire type — upstream emits
        // id_type::MerchantId/ProfileId as plain strings (`admin.rs:2841,2845`).
        domain.set_profile_id(profile_id);
        domain.set_merchant_id(account ? account->get_merchant_id() : std::string{merchant_id});

        // core/admin.rs:3603 — profile_name defaults to "default".
        domain.set_profile_name(request.get_profile_name().value_or("default"));
        // core/admin.rs:3712-3723 — account fallbacks.
        domain.set_return_url(
            request.get_return_url()
                ? std::optional<std::string>{request.get_return_url()->get_value()}
                : std::optional<std::string>{account ? account->get_return_url() : std::string{}}
        );
        domain.set_enable_payment_response_hash(request.get_enable_payment_response_hash().value_or(
            account ? account->get_enable_payment_response_hash() : false
        ));
        // core/admin.rs:3625-3628 — hash key fallback + 64-char secure random.
        std::string hash_key = request.get_payment_response_hash_key().value_or(
            account && !account->get_payment_response_hash_key().empty()
                ? account->get_payment_response_hash_key()
                : random_secure_string(64)
        );
        domain.set_payment_response_hash_key(hash_key);
        domain.set_redirect_to_merchant_with_http_post(
            request.get_redirect_to_merchant_with_http_post().value_or(
                account ? account->get_redirect_to_merchant_with_http_post() : false
            )
        );
        if (request.get_webhook_details()) {
            domain.set_webhook_details(to_webhook_response(*request.get_webhook_details()));
        }
        domain.set_metadata(request.get_metadata());
        // core/admin.rs:3725 — routing_algorithm is intentionally not stored directly.
        domain.set_routing_algorithm(std::nullopt);
        domain.set_intent_fulfillment_time(
            std::optional<std::int64_t>{
                static_cast<std::int64_t>(request.get_intent_fulfillment_time().value_or(900))
            }
        );
        domain.set_frm_routing_algorithm(request.get_frm_routing_algorithm());
        domain.set_payout_routing_algorithm(request.get_payout_routing_algorithm());
        domain.set_applepay_verified_domains(request.get_applepay_verified_domains());
        domain.set_session_expiry(
            std::optional<std::int64_t>{
                static_cast<std::int64_t>(request.get_session_expiry().value_or(900))
            }
        );
        domain.set_payment_link_config(request.get_payment_link_config());
        domain.set_authentication_connector_details(request.get_authentication_connector_details());
        // core/admin.rs:3754-3762 — defaults true/false/false.
        domain.set_use_billing_as_payment_method_billing(
            request.get_use_billing_as_payment_method_billing().value_or(true)
        );
        domain.set_collect_shipping_details_from_wallet_connector(
            request.get_collect_shipping_details_from_wallet_connector().value_or(false)
        );
        domain.set_collect_billing_details_from_wallet_connector(
            request.get_collect_billing_details_from_wallet_connector().value_or(false)
        );
        // core/admin.rs:3751 — created as None; toggled by the dedicated route.
        domain.set_is_connector_agnostic_mit_enabled(
            request.get_is_connector_agnostic_mit_enabled()
        );
        domain.set_payout_link_config(request.get_payout_link_config());
        if (request.get_outgoing_webhook_custom_http_headers()) {
            app::dto::admin::MaskedHeaders headers;
            headers.set_value(*request.get_outgoing_webhook_custom_http_headers());
            domain.set_outgoing_webhook_custom_http_headers(headers);
        }
        domain.set_tax_connector_id(request.get_tax_connector_id());
        domain.set_is_tax_connector_enabled(request.get_is_tax_connector_enabled());
        domain.set_is_network_tokenization_enabled(request.get_is_network_tokenization_enabled());
        domain.set_is_auto_retries_enabled(request.get_is_auto_retries_enabled().value_or(false));
        if (request.get_max_auto_retries_enabled()) {
            domain.set_max_auto_retries_enabled(
                std::optional<std::int16_t>{
                    static_cast<std::int16_t>(*request.get_max_auto_retries_enabled())
                }
            );
        }
        domain.set_always_request_extended_authorization(
            request.get_always_request_extended_authorization()
        );
        domain.set_is_click_to_pay_enabled(request.get_is_click_to_pay_enabled());
        domain.set_authentication_product_ids(request.get_authentication_product_ids());
        domain.set_card_testing_guard_config(request.get_card_testing_guard_config());
        domain.set_is_clear_pan_retries_enabled(
            request.get_is_clear_pan_retries_enabled().value_or(false)
        );
        domain.set_force_3ds_challenge(request.get_force_3ds_challenge().value_or(false));
        domain.set_is_debit_routing_enabled(request.get_is_debit_routing_enabled().value_or(false));
        domain.set_merchant_business_country(request.get_merchant_business_country());
        domain.set_is_iframe_redirection_enabled(request.get_is_iframe_redirection_enabled());
        domain.set_is_pre_network_tokenization_enabled(
            request.get_is_pre_network_tokenization_enabled().value_or(false)
        );
        domain.set_merchant_category_code(request.get_merchant_category_code());
        domain.set_merchant_country_code(request.get_merchant_country_code());
        domain.set_dispute_polling_interval(request.get_dispute_polling_interval());
        domain.set_is_manual_retry_enabled(request.get_is_manual_retry_enabled());
        domain.set_auto_fallback_capture_method(request.get_auto_fallback_capture_method());
        domain.set_always_enable_overcapture(request.get_always_enable_overcapture());
        domain.set_is_external_vault_enabled(request.get_is_external_vault_enabled());
        domain.set_external_vault_connector_details(request.get_external_vault_connector_details());
        domain.set_billing_processor_id(request.get_billing_processor_id());
        domain.set_surcharge_connector_details(request.get_surcharge_connector_details());
        domain.set_is_l2_l3_enabled(request.get_is_l2_l3_enabled().value_or(false));
        domain.set_network_tokenization_credentials(request.get_network_tokenization_credentials());
        domain.set_payment_method_blocking(request.get_payment_method_blocking());
        domain.set_always_collect_billing_details_from_wallet_connector(
            request.get_always_collect_billing_details_from_wallet_connector()
        );
        domain.set_always_collect_shipping_details_from_wallet_connector(
            request.get_always_collect_shipping_details_from_wallet_connector()
        );

        return domain;
    }

    // core/admin.rs:4211-4470 — ProfileUpdateBridge::get_update_profile_object.
    // Upstream converts the request into domain::ProfileGeneralUpdate whose
    // Option fields mean "no change"; applying them onto the stored domain
    // profile reproduces the upstream DB merge exactly.
    [[nodiscard]] std::expected<Response, core::error::AppError>
    get_update_profile_object(const Response& stored, const UpdateInput& request) const
    {
        // core/admin.rs:4218-4224.
        if (const auto error = validate_session_expiry(request.get_session_expiry())) {
            return std::unexpected(*error);
        }
        if (const auto error =
                validate_intent_fulfillment_expiry(request.get_intent_fulfillment_time())) {
            return std::unexpected(*error);
        }
        // core/admin.rs:4226-4240 — outbound destination validation.
        if (request.get_webhook_details() && request.get_webhook_details()->get_webhook_url()) {
            if (const auto error = validate_webhook_destination(
                    *request.get_webhook_details()->get_webhook_url()
                )) {
                return std::unexpected(*error);
            }
        }
        // core/admin.rs:4259-4267 — static routing algorithm parse.
        if (request.get_routing_algorithm()) {
            if (const auto error =
                    validate_static_routing_algorithm(*request.get_routing_algorithm())) {
                return std::unexpected(*error);
            }
        }
        // core/admin.rs:4269-4277, :4315-4323 — payment/payout link config validate.
        if (request.get_payment_link_config()) {
            if (const auto error =
                    validate_payment_link_config(*request.get_payment_link_config())) {
                return std::unexpected(*error);
            }
        }
        if (request.get_payout_link_config()) {
            if (const auto error =
                    validate_payout_link_config(request.get_payout_link_config()->get_config())) {
                return std::unexpected(*error);
            }
        }
        // core/admin.rs:4385-4391 — merchant country code validation.
        if (request.get_merchant_country_code() &&
            request.get_merchant_country_code()->get_value().empty()) {
            return std::unexpected(make_error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "Invalid merchant country code"
            ));
        }

        Response merged = stored;
        apply_update_fields(merged, request);
        // core/admin.rs:4242-4257 — webhook-details merge (v1).
        if (request.get_webhook_details()) {
            auto base = to_webhook_request(
                merged.get_webhook_details().value_or(app::dto::admin::WebhookDetailsResponse{})
            );
            overlay_webhook(base, *request.get_webhook_details());
            merged.set_webhook_details(to_webhook_response(base));
        }
        return merged;
    }

    // Apply only the Option fields that are present (domain::ProfileGeneralUpdate
    // semantics — core/admin.rs:4393-4469).
    void apply_update_fields(Response& target, const UpdateInput& u) const
    {
        if (u.get_profile_name()) {
            target.set_profile_name(*u.get_profile_name());
        }
        if (u.get_return_url()) {
            target.set_return_url(u.get_return_url()->get_value());
        }
        if (u.get_enable_payment_response_hash()) {
            target.set_enable_payment_response_hash(*u.get_enable_payment_response_hash());
        }
        if (u.get_payment_response_hash_key()) {
            target.set_payment_response_hash_key(*u.get_payment_response_hash_key());
        }
        if (u.get_redirect_to_merchant_with_http_post()) {
            target.set_redirect_to_merchant_with_http_post(
                *u.get_redirect_to_merchant_with_http_post()
            );
        }
        if (u.get_metadata()) {
            target.set_metadata(*u.get_metadata());
        }
        if (u.get_routing_algorithm()) {
            target.set_routing_algorithm(*u.get_routing_algorithm());
        }
        if (u.get_intent_fulfillment_time()) {
            target.set_intent_fulfillment_time(
                std::optional<std::int64_t>{
                    static_cast<std::int64_t>(*u.get_intent_fulfillment_time())
                }
            );
        }
        if (u.get_frm_routing_algorithm()) {
            target.set_frm_routing_algorithm(*u.get_frm_routing_algorithm());
        }
        if (u.get_payout_routing_algorithm()) {
            target.set_payout_routing_algorithm(*u.get_payout_routing_algorithm());
        }
        if (u.get_applepay_verified_domains()) {
            target.set_applepay_verified_domains(*u.get_applepay_verified_domains());
        }
        if (u.get_session_expiry()) {
            target.set_session_expiry(
                std::optional<std::int64_t>{static_cast<std::int64_t>(*u.get_session_expiry())}
            );
        }
        if (u.get_payment_link_config()) {
            target.set_payment_link_config(*u.get_payment_link_config());
        }
        if (u.get_authentication_connector_details()) {
            target.set_authentication_connector_details(*u.get_authentication_connector_details());
        }
        if (u.get_extended_card_info_config()) {
            target.set_extended_card_info_config(*u.get_extended_card_info_config());
        }
        if (u.get_use_billing_as_payment_method_billing()) {
            target.set_use_billing_as_payment_method_billing(
                *u.get_use_billing_as_payment_method_billing()
            );
        }
        if (u.get_collect_shipping_details_from_wallet_connector()) {
            target.set_collect_shipping_details_from_wallet_connector(
                *u.get_collect_shipping_details_from_wallet_connector()
            );
        }
        if (u.get_collect_billing_details_from_wallet_connector()) {
            target.set_collect_billing_details_from_wallet_connector(
                *u.get_collect_billing_details_from_wallet_connector()
            );
        }
        if (u.get_always_collect_shipping_details_from_wallet_connector()) {
            target.set_always_collect_shipping_details_from_wallet_connector(
                *u.get_always_collect_shipping_details_from_wallet_connector()
            );
        }
        if (u.get_always_collect_billing_details_from_wallet_connector()) {
            target.set_always_collect_billing_details_from_wallet_connector(
                *u.get_always_collect_billing_details_from_wallet_connector()
            );
        }
        if (u.get_always_request_extended_authorization()) {
            target.set_always_request_extended_authorization(
                *u.get_always_request_extended_authorization()
            );
        }
        if (u.get_is_connector_agnostic_mit_enabled()) {
            target.set_is_connector_agnostic_mit_enabled(
                *u.get_is_connector_agnostic_mit_enabled()
            );
        }
        if (u.get_payout_link_config()) {
            target.set_payout_link_config(*u.get_payout_link_config());
        }
        if (u.get_outgoing_webhook_custom_http_headers()) {
            app::dto::admin::MaskedHeaders headers;
            headers.set_value(*u.get_outgoing_webhook_custom_http_headers());
            target.set_outgoing_webhook_custom_http_headers(headers);
        }
        if (u.get_tax_connector_id()) {
            target.set_tax_connector_id(*u.get_tax_connector_id());
        }
        if (u.get_is_tax_connector_enabled()) {
            target.set_is_tax_connector_enabled(*u.get_is_tax_connector_enabled());
        }
        if (u.get_is_network_tokenization_enabled()) {
            target.set_is_network_tokenization_enabled(*u.get_is_network_tokenization_enabled());
        }
        if (u.get_is_auto_retries_enabled()) {
            target.set_is_auto_retries_enabled(*u.get_is_auto_retries_enabled());
        }
        if (u.get_max_auto_retries_enabled()) {
            target.set_max_auto_retries_enabled(
                std::optional<std::int16_t>{
                    static_cast<std::int16_t>(*u.get_max_auto_retries_enabled())
                }
            );
        }
        if (u.get_is_click_to_pay_enabled()) {
            target.set_is_click_to_pay_enabled(*u.get_is_click_to_pay_enabled());
        }
        if (u.get_authentication_product_ids()) {
            target.set_authentication_product_ids(*u.get_authentication_product_ids());
        }
        if (u.get_card_testing_guard_config()) {
            target.set_card_testing_guard_config(*u.get_card_testing_guard_config());
        }
        if (u.get_is_clear_pan_retries_enabled()) {
            target.set_is_clear_pan_retries_enabled(*u.get_is_clear_pan_retries_enabled());
        }
        if (u.get_force_3ds_challenge()) {
            target.set_force_3ds_challenge(*u.get_force_3ds_challenge());
        }
        if (u.get_is_debit_routing_enabled()) {
            target.set_is_debit_routing_enabled(*u.get_is_debit_routing_enabled());
        }
        if (u.get_merchant_business_country()) {
            target.set_merchant_business_country(*u.get_merchant_business_country());
        }
        if (u.get_is_iframe_redirection_enabled()) {
            target.set_is_iframe_redirection_enabled(*u.get_is_iframe_redirection_enabled());
        }
        if (u.get_is_pre_network_tokenization_enabled()) {
            target.set_is_pre_network_tokenization_enabled(
                *u.get_is_pre_network_tokenization_enabled()
            );
        }
        if (u.get_merchant_category_code()) {
            target.set_merchant_category_code(*u.get_merchant_category_code());
        }
        if (u.get_merchant_country_code()) {
            target.set_merchant_country_code(*u.get_merchant_country_code());
        }
        if (u.get_dispute_polling_interval()) {
            target.set_dispute_polling_interval(*u.get_dispute_polling_interval());
        }
        if (u.get_is_manual_retry_enabled()) {
            target.set_is_manual_retry_enabled(*u.get_is_manual_retry_enabled());
        }
        if (u.get_auto_fallback_capture_method()) {
            target.set_auto_fallback_capture_method(*u.get_auto_fallback_capture_method());
        }
        if (u.get_always_enable_overcapture()) {
            target.set_always_enable_overcapture(*u.get_always_enable_overcapture());
        }
        if (u.get_is_external_vault_enabled()) {
            target.set_is_external_vault_enabled(*u.get_is_external_vault_enabled());
        }
        if (u.get_external_vault_connector_details()) {
            target.set_external_vault_connector_details(*u.get_external_vault_connector_details());
        }
        if (u.get_billing_processor_id()) {
            target.set_billing_processor_id(*u.get_billing_processor_id());
        }
        if (u.get_surcharge_connector_details()) {
            target.set_surcharge_connector_details(*u.get_surcharge_connector_details());
        }
        if (u.get_is_l2_l3_enabled()) {
            target.set_is_l2_l3_enabled(*u.get_is_l2_l3_enabled());
        }
        if (u.get_network_tokenization_credentials()) {
            target.set_network_tokenization_credentials(*u.get_network_tokenization_credentials());
        }
        if (u.get_payment_method_blocking()) {
            target.set_payment_method_blocking(*u.get_payment_method_blocking());
        }
    }

    // core/admin.rs:4016-4059 — external vault may only be configured on a
    // platform merchant (replaces the earlier connector-existence substitute).
    [[nodiscard]] std::optional<core::error::AppError>
    validate_external_vault_config_for_merchant_account_type(
        std::string_view merchant_account_type,
        const std::optional<app::dto::admin::ExternalVaultEnabledValue>& enabled,
        bool has_details
    ) const
    {
        const bool flagged =
            enabled && (enabled->get_value() == "Enable" || enabled->get_value() == "enable" ||
                        enabled->get_value() == "Enabled" || enabled->get_value() == "enabled" ||
                        enabled->get_value() == "true" || enabled->get_value() == "1");
        const bool configuring = flagged || has_details;
        if (configuring && merchant_account_type == "connected") {
            return make_error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "External vault can only be configured for platform merchant, not for connected "
                "merchants"
            );
        }
        return std::nullopt;
    }

    // core/payments/helpers.rs:8425-8433.
    [[nodiscard]] std::optional<core::error::AppError>
    validate_session_expiry(const std::optional<std::uint32_t>& expiry) const
    {
        if (expiry && (*expiry < 60 || *expiry > 7'890'000)) {
            return make_error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "session_expiry should be between 60(1 min) to 7890000(3 months)."
            );
        }
        return std::nullopt;
    }

    // core/payments/helpers.rs:8549-8562.
    [[nodiscard]] std::optional<core::error::AppError>
    validate_intent_fulfillment_expiry(const std::optional<std::uint32_t>& expiry) const
    {
        if (expiry && (*expiry < 60 || *expiry > 1'800)) {
            return make_error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "intent_fulfillment_time should be between 60(1 min) to 1800(30 mins)."
            );
        }
        return std::nullopt;
    }

    // api_models/admin.rs:3759-3795 — strict host domain + wildcard allowed
    // domains + nested default/business-specific config validation.
    [[nodiscard]] std::optional<core::error::AppError>
    validate_payment_link_config(const app::dto::admin::BusinessPaymentLinkConfig& config) const
    {
        if (config.get_domain_name() && !validate_strict_domain(*config.get_domain_name())) {
            return make_error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "Invalid host domain name received in payment_link_config"
            );
        }
        if (config.get_allowed_domains()) {
            for (const auto& domain: *config.get_allowed_domains()) {
                if (!validate_wildcard_domain(domain)) {
                    return make_error(
                        core::error::AppErrorKind::INVALID_REQUEST,
                        "Invalid allowed domain names received in payment_link_config"
                    );
                }
            }
        }
        return std::nullopt;
    }

    // core/admin.rs:4315-4323 — BusinessGenericLinkConfig::validate for the
    // nested payout_link_config.config value.
    [[nodiscard]] std::optional<core::error::AppError>
    validate_payout_link_config(const app::dto::admin::BusinessGenericLinkConfig& config) const
    {
        if (config.get_domain_name() && !validate_strict_domain(*config.get_domain_name())) {
            return make_error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "Invalid host domain name received in payout_link_config"
            );
        }
        for (const auto& domain: config.get_allowed_domains()) {
            if (!validate_wildcard_domain(domain)) {
                return make_error(
                    core::error::AppErrorKind::INVALID_REQUEST,
                    "Invalid allowed domain names received in payout_link_config"
                );
            }
        }
        return std::nullopt;
    }

    // core/admin.rs:4232-4240 / :4517-4525 — outbound_destination::validate_destination
    // is a network-bound resolver upstream; local substitute checks scheme and
    // the configured bypass-host list.
    [[nodiscard]] std::optional<core::error::AppError>
    validate_webhook_destination(std::string_view url) const
    {
        if (!url.starts_with("http://") && !url.starts_with("https://")) {
            return make_error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "webhook_url is not valid"
            );
        }
        const auto bypass = m_context.config_store().get_value("proxy", "bypass_proxy_hosts");
        if (bypass && !bypass->empty()) {
            const std::string host = host_of(url);
            std::string list{*bypass};
            std::size_t start{0};
            while (start <= list.size()) {
                const auto comma = list.find(',', start);
                const std::string entry = list.substr(
                    start,
                    comma == std::string::npos ? std::string::npos : comma - start
                );
                if (!entry.empty() && host.ends_with(entry)) {
                    return std::nullopt;
                }
                if (comma == std::string::npos) {
                    break;
                }
                start = comma + 1;
            }
        }
        return std::nullopt;
    }

    // core/admin.rs:4259-4267 — StaticRoutingAlgorithm parse_value guard. The
    // local substitute checks the JSON value is a well-formed object before
    // accepting a direct routing write.
    [[nodiscard]] std::optional<core::error::AppError>
    validate_static_routing_algorithm(const serde::Value& value) const
    {
        const auto text = serde::Ser::encode_json(value);
        if (text.empty() || text == "null" || text.front() != '{') {
            return make_error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "Invalid routing algorithm given"
            );
        }
        return std::nullopt;
    }

    // core/admin.rs:4700-4725 — decision-engine routing effectiveness. Local
    // substitute: config flag + decision engine rule evaluation + routing-config
    // repository lookup with the profile dimension.
    [[nodiscard]] bool decision_engine_routing_effective(std::string_view profile_id) const
    {
        const auto flag =
            m_context.config_store().get_value("decision_engine", "routing_effective");
        const bool enabled = flag && (*flag == "true" || *flag == "1");
        constexpr std::string_view program =
            R"({"rules":[{"if_condition":{"field":"connector","op":"eq","value":"decision_engine"},"action":"reject"}],"default_action":"allow"})";
        const std::string context_json = std::format(
            R"({{"amount":"0","currency":"USD","payment_method":"card","country":"","connector":"{}"}})",
            enabled ? "decision_engine" : "local"
        );
        if (m_context.decision_engine().evaluate(program, context_json).get_action() == "reject") {
            return true;
        }
        return m_context.repository()
            .find_routing_config_by_algorithm_id_and_profile_id("decision_engine", profile_id)
            .has_value();
    }

    // ==================================================================
    // accounts_store / master-key / key-store substitutes (repository-backed).
    // ==================================================================

    [[nodiscard]] std::string master_key() const
    {
        return m_context.config_store().get_value("keys", "master").value_or("");
    }

    // Upstream get_merchant_key_store_by_merchant_id: a missing merchant maps
    // to MerchantAccountNotFound.
    [[nodiscard]] std::optional<app::store::merchant_accounts::MerchantAccountRecord>
    get_merchant_key_store_by_merchant_id(std::string_view merchant_id, std::string_view key) const
    {
        (void)key; // master key decrypts the upstream key store; local store is plaintext.
        return m_context.repository().find_merchant_account_by_string(merchant_id);
    }

    [[nodiscard]] std::optional<core::error::AppError> insert_business_profile(
        std::string_view profile_id,
        std::string_view merchant_id,
        std::string_view profile_name,
        const Response& domain
    ) const
    {
        auto repository = m_context.repository();
        app::store::profiles::ProfileRecord record{};
        record.set_profile_id(std::string{profile_id});
        record.set_merchant_id(std::string{merchant_id});
        record.set_profile_name(std::string{profile_name});
        record.set_type("business");
        record.set_status("active");
        record.set_created_at(now_iso_utc());
        if (!repository.create_profile(std::move(record))) {
            return make_error(
                core::error::AppErrorKind::ALREADY_EXISTS,
                std::format("Business Profile with the profile_id {} already exists", profile_id)
            );
        }
        repository.set_config(payload_config_key(profile_id), serde::Ser::encode_json(domain));
        // Existence marker consumed by profile_acquirer.cppm:267.
        m_context.cache().put(
            std::string{"profiles.profile.payload:"} + std::string{profile_id},
            core::cache::to_cache_value(std::string{"1"})
        );
        return std::nullopt;
    }

    [[nodiscard]] std::optional<Response> find_business_profile_by_merchant_id_profile_id(
        std::string_view merchant_id,
        std::string_view profile_id
    ) const
    {
        auto repository = m_context.repository();
        const auto record = repository.find_profile_by_string(profile_id);
        if (!record) {
            return std::nullopt;
        }
        if (!merchant_id.empty() && record->get_merchant_id() != merchant_id) {
            return std::nullopt;
        }
        return deserialize_profile(record->get_profile_id());
    }

    [[nodiscard]] std::optional<Response>
    find_business_profile_by_profile_id(std::string_view profile_id) const
    {
        auto repository = m_context.repository();
        const auto record = repository.find_profile_by_string(profile_id);
        if (!record) {
            return std::nullopt;
        }
        return deserialize_profile(record->get_profile_id());
    }

    [[nodiscard]] std::optional<Response> deserialize_profile(std::string_view profile_id) const
    {
        const auto payload = m_context.repository().get_config(payload_config_key(profile_id));
        if (!payload) {
            return std::nullopt;
        }
        const auto parsed = serde::Ser::deserialize<Response>("application/json", *payload);
        if (parsed) {
            return *parsed;
        }
        return std::nullopt;
    }

    [[nodiscard]] bool update_profile_by_profile_id(
        std::string_view profile_id,
        std::string_view merchant_id,
        const Response& domain
    ) const
    {
        auto repository = m_context.repository();
        auto record = repository.find_profile_by_string(profile_id);
        if (!record) {
            return false;
        }
        if (!merchant_id.empty()) {
            record->set_merchant_id(std::string{merchant_id});
        }
        record->set_profile_name(domain.get_profile_name());
        if (!repository.update_profile(*record)) {
            return false;
        }
        repository.set_config(payload_config_key(profile_id), serde::Ser::encode_json(domain));
        m_context.cache().put(
            std::string{"profiles.profile.payload:"} + std::string{profile_id},
            core::cache::to_cache_value(std::string{"1"})
        );
        return true;
    }

    [[nodiscard]] bool delete_profile_by_profile_id_merchant_id(
        std::string_view profile_id,
        std::string_view merchant_id
    ) const
    {
        auto repository = m_context.repository();
        const auto record = repository.find_profile_by_string(profile_id);
        if (!record) {
            return false;
        }
        if (!merchant_id.empty() && record->get_merchant_id() != merchant_id) {
            return false;
        }
        if (!repository.remove_profile(record->get_id())) {
            return false;
        }
        for (const auto& key:
             {payload_config_key(profile_id),
              std::string{"profiles.connector_mit."} + std::string{profile_id},
              card_config_key(profile_id),
              webhook_config_key(profile_id)}) {
            if (const auto config = repository.find_config_by_key(key)) {
                (void)repository.remove_config(config->get_id());
            }
        }
        m_context.cache().erase(std::string{"profiles.profile.payload:"} + std::string{profile_id});
        return true;
    }

    [[nodiscard]] std::vector<Response>
    list_profile_by_merchant_id(std::string_view merchant_id) const
    {
        std::vector<Response> out{};
        for (const auto& record: m_context.repository().list_profiles()) {
            if (!merchant_id.empty() && record.get_merchant_id() != merchant_id) {
                continue;
            }
            if (const auto profile = deserialize_profile(record.get_profile_id())) {
                out.push_back(*profile);
            }
        }
        return out;
    }

    [[nodiscard]] std::vector<Response> filter_objects_based_on_profile_id_list(
        const std::optional<std::vector<std::string>>& ids,
        std::vector<Response> profiles
    ) const
    {
        if (!ids) {
            return profiles;
        }
        std::vector<Response> out{};
        for (auto& profile: profiles) {
            const auto& id = profile.get_profile_id();
            if (std::find(ids->begin(), ids->end(), id) != ids->end()) {
                out.push_back(std::move(profile));
            }
        }
        return out;
    }

    [[nodiscard]] std::string payload_config_key(std::string_view profile_id) const
    {
        return std::string{"profiles.payload."} + std::string{profile_id};
    }

    [[nodiscard]] std::string webhook_config_key(std::string_view profile_id) const
    {
        return std::string{"profiles.webhook."} + std::string{profile_id};
    }

    [[nodiscard]] std::string card_config_key(std::string_view profile_id) const
    {
        return std::string{"profiles.extended_card."} + std::string{profile_id};
    }

    [[nodiscard]] std::string random_secure_string(std::size_t length) const
    {
        std::string out{};
        while (out.size() < length) {
            std::string part{m_context.ids().uuid_v4()};
            std::erase(part, '-');
            out += part;
        }
        out.resize(length);
        return out;
    }

    [[nodiscard]] bool validate_strict_domain(std::string_view domain) const
    {
        if (domain.empty() || domain.size() > 253) {
            return false;
        }
        std::size_t start{0};
        while (start < domain.size()) {
            const auto dot = domain.find('.', start);
            const auto label = domain.substr(
                start,
                dot == std::string_view::npos ? std::string_view::npos : dot - start
            );
            if (label.empty() || label.size() > 63 || label.front() == '-' || label.back() == '-') {
                return false;
            }
            for (const char c: label) {
                if (!std::isalnum(static_cast<unsigned char>(c)) && c != '-') {
                    return false;
                }
            }
            if (dot == std::string_view::npos) {
                break;
            }
            start = dot + 1;
        }
        return true;
    }

    [[nodiscard]] bool validate_wildcard_domain(std::string_view domain) const
    {
        if (domain.starts_with("*.")) {
            domain.remove_prefix(2);
        }
        return validate_strict_domain(domain);
    }

    [[nodiscard]] std::string host_of(std::string_view url) const
    {
        const auto scheme = url.find("://");
        std::size_t start = scheme == std::string_view::npos ? 0 : scheme + 3;
        const auto slash = url.find('/', start);
        const auto end = slash == std::string_view::npos ? url.size() : slash;
        return std::string{url.substr(start, end - start)};
    }

    [[nodiscard]] app::dto::admin::WebhookDetailsResponse
    to_webhook_response(const app::dto::admin::WebhookDetailsRequest& request) const
    {
        return convert_dto<app::dto::admin::WebhookDetailsResponse>(request);
    }

    [[nodiscard]] app::dto::admin::WebhookDetailsRequest
    to_webhook_request(const app::dto::admin::WebhookDetailsResponse& response) const
    {
        return convert_dto<app::dto::admin::WebhookDetailsRequest>(response);
    }

    template<class To, class From>
    [[nodiscard]] To convert_dto(const From& from) const
    {
        const auto text = serde::Ser::encode_json(from);
        const auto result = serde::Ser::deserialize<To>("application/json", text);
        return result ? *result : To{};
    }

    void overlay_webhook(
        app::dto::admin::WebhookDetailsRequest& base,
        const app::dto::admin::WebhookDetailsRequest& incoming
    ) const
    {
        if (incoming.get_webhook_version()) {
            base.set_webhook_version(incoming.get_webhook_version());
        }
        if (incoming.get_webhook_username()) {
            base.set_webhook_username(incoming.get_webhook_username());
        }
        if (incoming.get_webhook_password()) {
            base.set_webhook_password(incoming.get_webhook_password());
        }
        if (incoming.get_webhook_url()) {
            base.set_webhook_url(incoming.get_webhook_url());
        }
        if (incoming.get_payment_created_enabled()) {
            base.set_payment_created_enabled(incoming.get_payment_created_enabled());
        }
        if (incoming.get_payment_succeeded_enabled()) {
            base.set_payment_succeeded_enabled(incoming.get_payment_succeeded_enabled());
        }
        if (incoming.get_payment_failed_enabled()) {
            base.set_payment_failed_enabled(incoming.get_payment_failed_enabled());
        }
        if (incoming.get_payment_statuses_enabled()) {
            base.set_payment_statuses_enabled(incoming.get_payment_statuses_enabled());
        }
        if (incoming.get_refund_statuses_enabled()) {
            base.set_refund_statuses_enabled(incoming.get_refund_statuses_enabled());
        }
        if (incoming.get_payout_statuses_enabled()) {
            base.set_payout_statuses_enabled(incoming.get_payout_statuses_enabled());
        }
        if (incoming.get_dispute_statuses_enabled()) {
            base.set_dispute_statuses_enabled(incoming.get_dispute_statuses_enabled());
        }
        if (incoming.get_mandate_statuses_enabled()) {
            base.set_mandate_statuses_enabled(incoming.get_mandate_statuses_enabled());
        }
        if (incoming.get_invoice_statuses_enabled()) {
            base.set_invoice_statuses_enabled(incoming.get_invoice_statuses_enabled());
        }
    }

    [[nodiscard]] std::string now_iso_utc() const
    {
        const std::int64_t epoch_seconds = std::chrono::duration_cast<std::chrono::seconds>(
                                               std::chrono::system_clock::now().time_since_epoch()
        )
                                               .count();
        constexpr std::int64_t seconds_per_day{86'400};
        std::int64_t days = epoch_seconds / seconds_per_day;
        std::int64_t seconds_of_day = epoch_seconds % seconds_per_day;
        if (seconds_of_day < 0) {
            seconds_of_day += seconds_per_day;
            --days;
        }
        days += 719'468;
        const std::int64_t era = (days >= 0 ? days : days - 146'096) / 146'097;
        const std::int64_t doe = days - era * 146'097;
        const std::int64_t yoe = (doe - doe / 1'460 + doe / 36'524 - doe / 146'096) / 365;
        std::int64_t year = yoe + era * 400;
        const std::int64_t doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
        const std::int64_t mp = (5 * doy + 2) / 153;
        const std::int64_t day = doy - (153 * mp + 2) / 5 + 1;
        const std::int64_t month = mp < 10 ? mp + 3 : mp - 9;
        year += (month <= 2) ? 1 : 0;
        return std::format(
            "{:04d}-{:02d}-{:02d}T{:02d}:{:02d}:{:02d}Z",
            year,
            month,
            day,
            seconds_of_day / 3'600,
            (seconds_of_day % 3'600) / 60,
            seconds_of_day % 60
        );
    }

    void emit_event(std::string_view operation, std::string_view payload) const
    {
        (void)m_context.events().publish(
            std::string{"profiles."} + std::string{operation},
            std::string{payload}
        );
    }

    [[nodiscard]] core::error::AppError
    make_error(core::error::AppErrorKind kind, std::string message) const
    {
        return core::error::AppErrorBuilder{}(kind, std::move(message));
    }

    [[nodiscard]] core::error::AppError profile_not_found(std::string_view profile_id) const
    {
        return make_error(
            core::error::AppErrorKind::NOT_FOUND,
            std::format("Profile with id {} not found", profile_id)
        );
    }

private:
    app::context::AppContext& m_context;
};

} // namespace app::handlers::profiles
