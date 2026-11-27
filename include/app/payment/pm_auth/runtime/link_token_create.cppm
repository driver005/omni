export module app_payment:pm_auth_runtime_link_token_create;
import core_behavior;

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

import :pm_auth_input;
import :pm_auth_result;
import :pm_auth_runtime_common;

export namespace app::handlers::pm_auth {

class PmAuthLinkTokenCreateRuntime
{
public:
    explicit PmAuthLinkTokenCreateRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    template<class Dto>
    [[nodiscard]] core::behavior::Outcome<::app::dto::pm_auth::LinkTokenCreateResponse>
    link_token_create(const Dto& dto, const core::behavior::CallContext& ctx) const
    {
        const auto input = LinkTokenCreateData{dto, ctx};
        const auto& request = input.request;
        const std::string& payment_id = input.request.get_payment_id().get_value();
        const std::string& payment_method = input.request.get_payment_method().get_value();
        const std::string& payment_method_type =
            input.request.get_payment_method_type().get_value();

        // core/pm_auth.rs:105-110 — verify the stored payment intent and its client secret
        // (state + time-to-live) before minting a link token.
        std::optional<::app::store::payments::PaymentIntentRecord> payment_intent;
        if (const auto intent_error = m_support.verify_payment_intent(
                payment_id,
                input.merchant_id,
                request.get_client_secret().has_value(),
                payment_intent
            )) {
            return std::unexpected{*intent_error};
        }
        if (payment_intent.has_value()) {
            if (const auto age_error = m_support.check_payment_intent_age(payment_id)) {
                return std::unexpected{*age_error};
            }
        }
        if (const auto secret_error =
                m_support.verify_client_secret(payment_id, request.get_client_secret())) {
            return std::unexpected{*secret_error};
        }
        if (request.get_client_secret().has_value()) {
            const auto secret_hash =
                ::app::crypto::KeyedHasher{"pm_auth.client_secret"}.hash_with_domain(
                    ::app::crypto::HashDomain::ClientSecret,
                    *request.get_client_secret()
                );
            m_context.cache().put(
                "pm_auth.client_secret_hash:" + payment_id,
                core::cache::to_cache_value(secret_hash)
            );
        }

        // core/pm_auth.rs:56-93 — the pm_auth_<id> key must exist in Redis (local cache
        // substitute); a missing key is InvalidRequestData, an absent choice is
        // GenericNotFoundError. The MCA pm_auth_config is the local config source.
        const auto choice = m_support.resolve_connector_choice(
            input.merchant_id,
            payment_id,
            payment_method,
            payment_method_type
        );
        if (!choice.key_exists) {
            return std::unexpected{core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "Incorrect payment_id provided in request"
            )};
        }
        std::string connector_name = choice.connector_name;
        std::optional<::app::store::merchant_connectors::MerchantConnectorRecord> mca_record;
        if (const auto mca_error = m_support.validate_merchant_connector(
                input.merchant_id,
                choice.mca_id,
                connector_name,
                mca_record
            )) {
            return std::unexpected{*mca_error};
        }
        if (!choice.matched || connector_name.empty()) {
            return std::unexpected{core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::NOT_FOUND,
                "payment method auth connector name not found"
            )};
        }

        // core/pm_auth.rs:112-129,131-155,224-238 — derive billing country, resolve the MCA
        // auth type and extract BodyKey credentials for the connector router data.
        const std::string billing_country =
            m_support.derive_billing_country(payment_intent, mca_record);
        const ConnectorAuth connector_auth = m_support.resolve_connector_auth(mca_record);

        // core/pm_auth.rs:170-176 — forward HeaderPayload metadata to the connector.
        const auto& client_platform = input.client_platform;
        const auto& app_id = input.app_id;
        const auto& redirect_uri = input.redirect_uri;
        std::string connector_payload =
            "{\"client_name\":\"HyperSwitch\"" + std::string{",\"merchant_id\":"} +
            serde::Ser::encode_json(input.merchant_id) + std::string{",\"payment_id\":"} +
            serde::Ser::encode_json(payment_id) + std::string{",\"payment_method\":"} +
            serde::Ser::encode_json(payment_method) + std::string{",\"payment_method_type\":"} +
            serde::Ser::encode_json(payment_method_type);
        if (request.get_language().has_value()) {
            connector_payload +=
                std::string{",\"language\":"} + serde::Ser::encode_json(*request.get_language());
        }
        if (!client_platform.empty()) {
            connector_payload +=
                std::string{",\"client_platform\":"} + serde::Ser::encode_json(client_platform);
        }
        if (!app_id.empty()) {
            connector_payload +=
                std::string{",\"android_package_name\":"} + serde::Ser::encode_json(app_id);
        }
        if (!redirect_uri.empty()) {
            connector_payload +=
                std::string{",\"redirect_uri\":"} + serde::Ser::encode_json(redirect_uri);
        }
        if (!billing_country.empty()) {
            connector_payload += std::string{",\"country_codes\":["} +
                                 serde::Ser::encode_json(billing_country) + "]";
        }
        if (payment_intent.has_value() && !payment_intent->get_customer_id().empty()) {
            connector_payload += std::string{",\"user_info\":"} +
                                 serde::Ser::encode_json(payment_intent->get_customer_id());
        }
        connector_payload += std::string{",\"connector_auth_type\":"} +
                             serde::Ser::encode_json(connector_auth.scheme);
        connector_payload += "}";

        auto connector_request = ::app::connector::ConnectorRequestBuilder{}
                                     .operation("link_token_create")
                                     .merchant_id(input.merchant_id)
                                     .resource_id(payment_id)
                                     .raw_payload(std::move(connector_payload))();
        std::string link_token;
        if (auto connector = m_context.connector_router().select(payment_id)) {
            const auto connector_response = connector->create_link(connector_request);
            if (const auto connector_error =
                    m_support.connector_failure(connector_response, connector_name)) {
                return std::unexpected{*connector_error};
            }
            link_token = m_support.connector_value(connector_response);
        }
        if (link_token.empty()) {
            // Upstream lets the connector mint the token; ids + crypto provide the
            // deterministic opaque shape when no connector backend returns a payload.
            link_token = m_support.generate_token("lt_", payment_id + "_" + payment_method_type);
        }
        const ::app::crypto::SecretString token_secret{link_token};
        m_context.cache().put(
            std::string{"pm_auth.link_token."} + payment_id,
            core::cache::to_cache_value(m_support.encrypt_secret(token_secret.value()))
        );

        m_context.logger().record_flow("PmAuthLinkTokenCreate");
        m_context.logger().counter("PM_AUTH_LINK_TOKEN_CREATE").increment();

        ::app::dto::pm_auth::LinkTokenCreateResponse response{};
        response.set_link_token(token_secret.value());
        response.set_connector(std::move(connector_name));
        return ::app::dto::pm_auth::LinkTokenCreateResponse{std::move(response)};
    }

private:
    ::app::context::AppContext& m_context;
    PmAuthRuntimeSupport m_support;
};

} // namespace app::handlers::pm_auth
