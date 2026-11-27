export module app_webhook:verification_runtime_common;

import std;
import core_behavior;
import interfaces;
import core_serde;
import core_error;
import app_context;
import app_dto;
import app_crypto;
import app_connector;
import app_metrics;
import app_store;
import app_keystore;

export namespace app::handlers::verification {

using MerchantConnectorRecord = ::app::store::merchant_connectors::MerchantConnectorRecord;

/** @brief Local stand-in for the upstream `domain::MerchantKeyStore`.
 *
 *  Upstream loads it with the store master key and passes it into every MCA
 *  lookup/update (core/verification/utils.rs:27-32,65-136). Here the master key
 *  is resolved from the in-process KeyStore/secret resolver/config and the
 *  merchant key is derived deterministically; no external KMS is contacted.
 */
class MerchantKeyStore
{
public:
    explicit MerchantKeyStore(std::string key) noexcept :
        m_key{std::move(key)}
    {
    }

    [[nodiscard]] const std::string& key() const noexcept
    {
        return m_key;
    }

private:
    std::string m_key{};
};

/** @brief The key store plus the decrypted MCA, as returned by
 *  `utils::get_apple_pay_merchant_connector_account` (utils.rs:18-63). */
class ApplePayMerchantConnector
{
public:
    MerchantKeyStore key_store{"apple-pay-master-key"};
    MerchantConnectorRecord account{};
    bool details_were_encrypted{false};
};

/** @brief Local mirror of the `services::RequestBuilder` output assembled in
 *  core/verification.rs:43-54 (POST + application/json are fixed by upstream). */
class ApplePayVerificationRequest
{
public:
    void set_method(std::string value)
    {
        m_method = std::move(value);
    }

    [[nodiscard]] const std::string& method() const noexcept
    {
        return m_method;
    }

    void set_url(std::string value)
    {
        m_url = std::move(value);
    }

    [[nodiscard]] const std::string& url() const noexcept
    {
        return m_url;
    }

    void set_content_type(std::string value)
    {
        m_content_type = std::move(value);
    }

    [[nodiscard]] const std::string& content_type() const noexcept
    {
        return m_content_type;
    }

    void set_body(std::string value)
    {
        m_body = std::move(value);
    }

    [[nodiscard]] const std::string& body() const noexcept
    {
        return m_body;
    }

    void set_certificate(std::string value)
    {
        m_certificate = std::move(value);
    }

    [[nodiscard]] const std::string& certificate() const noexcept
    {
        return m_certificate;
    }

    void set_certificate_key(std::string value)
    {
        m_certificate_key = std::move(value);
    }

    [[nodiscard]] const std::string& certificate_key() const noexcept
    {
        return m_certificate_key;
    }

private:
    std::string m_method{"POST"};
    std::string m_url{};
    std::string m_content_type{"application/json"};
    std::string m_body{};
    std::string m_certificate{};
    std::string m_certificate_key{};
};

/** @brief Outcome of the two-level `call_connector_api` result:
 *  `Result<Result<Response, Response>, Report<ApiClientError>>`
 *  (core/verification.rs:56-66). TRANSPORT_ERROR is the outer Err,
 *  CONNECTOR_ERROR the inner Err, SUCCESS the inner Ok. */
enum class AppleCallOutcome : std::uint8_t
{
    SUCCESS,
    CONNECTOR_ERROR,
    TRANSPORT_ERROR,
};

class AppleCallResult
{
public:
    void set_outcome(AppleCallOutcome value) noexcept
    {
        m_outcome = value;
    }

    [[nodiscard]] AppleCallOutcome outcome() const noexcept
    {
        return m_outcome;
    }

    void set_status(std::string value)
    {
        m_status = std::move(value);
    }

    [[nodiscard]] const std::string& status() const noexcept
    {
        return m_status;
    }

private:
    AppleCallOutcome m_outcome{AppleCallOutcome::TRANSPORT_ERROR};
    std::string m_status{};
};

/**
 * @brief Shared business helpers for the verification sub-runtimes.
 *
 * Port of core/verification.rs and core/verification/utils.rs plus the local
 * in-process infrastructure substitutes. No HTTP types live here.
 */
class VerificationRuntimeSupport
{
public:
    explicit VerificationRuntimeSupport(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    [[nodiscard]] core::error::AppError
    make_error(core::error::AppErrorKind kind, std::string message) const
    {
        return core::error::AppErrorBuilder{}(kind, std::move(message));
    }

    /** @brief Port of `get_apple_pay_merchant_connector_account`
     *  (core/verification/utils.rs:18-63). */
    [[nodiscard]] core::behavior::Outcome<ApplePayMerchantConnector>
    get_apple_pay_merchant_connector_account(
        std::string_view merchant_id,
        std::string_view merchant_connector_id,
        std::string_view profile_id_from_auth_layer
    ) const
    {
        auto repository = m_context.repository();

        // get_merchant_key_store_by_merchant_id(...).to_not_found_response(
        // MerchantAccountNotFound) — utils.rs:27-32.
        if (!repository.find_merchant_account_by_string(merchant_id)) {
            return std::unexpected(
                make_error(core::error::AppErrorKind::NOT_FOUND, "MerchantAccountNotFound")
            );
        }

        ApplePayMerchantConnector resolved{};
        resolved.key_store = load_merchant_key_store(merchant_id);

        // v1 find_by_...merchant_id_merchant_connector_id / v2 find_by_id both
        // resolve to the repository lookup + merchant check (utils.rs:34-50,52-62).
        auto record = repository.find_merchant_connector_by_string(merchant_connector_id);
        if (!record || record->get_merchant_id() != merchant_id) {
            return std::unexpected(
                make_error(core::error::AppErrorKind::NOT_FOUND, "ResourceIdNotFound")
            );
        }

        // utils.rs:52-57 — validate_profile_id_from_auth_layer.
        if (const auto profile_error = validate_profile_id_from_auth_layer(
                profile_id_from_auth_layer,
                record->get_profile_id()
            )) {
            return std::unexpected(*profile_error);
        }

        // Upstream decrypts the MCA with the key store on read. Local substitute:
        // AesWrap over the persisted value; plaintext (unmarked) values pass
        // through unchanged, marked values are unwrapped with the merchant key.
        const auto details = record->get_connector_account_details();
        resolved.details_were_encrypted = details.starts_with("enc:");
        record->set_connector_account_details(
            decrypt_connector_details(resolved.key_store, details)
        );
        resolved.account = std::move(*record);
        return resolved;
    }

    /** @brief Port of `add_domain_to_apple_pay_merchant_connector_account`
     *  (core/verification/utils.rs:65-136). */
    [[nodiscard]] core::behavior::Outcome<std::vector<std::string>>
    add_domain_to_apple_pay_merchant_connector_account(
        ApplePayMerchantConnector& resolved,
        const std::vector<std::string>& domain_from_req
    ) const
    {
        // utils.rs:71-81 — keep already-verified first, append only new domains.
        auto already_verified_domains =
            decode_domains(resolved.account.get_applepay_verified_domains());
        for (const auto& domain: domain_from_req) {
            if (std::find(
                    already_verified_domains.begin(),
                    already_verified_domains.end(),
                    domain
                ) == already_verified_domains.end()) {
                already_verified_domains.push_back(domain);
            }
        }

        // Upstream re-encrypts the MCA with the key store on update
        // (utils.rs:119-125). Mirror that for marked values only.
        if (resolved.details_were_encrypted) {
            const auto encrypted = ::app::crypto::AesWrap{}.encrypt(
                resolved.account.get_connector_account_details(),
                resolved.key_store.key()
            );
            resolved.account.set_connector_account_details("enc:" + encrypted);
        }
        resolved.account.set_applepay_verified_domains(encode_domains(already_verified_domains));

        // utils.rs:119-133 — InternalServerError on update failure.
        if (!m_context.repository().update_merchant_connector(resolved.account)) {
            return std::unexpected(make_error(
                core::error::AppErrorKind::INTERNAL_ERROR,
                "Failed while updating MerchantConnectorAccount"
            ));
        }
        return already_verified_domains;
    }

    /** @brief Port of `log_applepay_verification_response_if_error`
     *  (core/verification/utils.rs:138-148). Metrics counters are the local
     *  substitute for logger::error!; no secret is ever logged. */
    void log_applepay_verification_response_if_error(const AppleCallResult& response) const
    {
        if (response.outcome() == AppleCallOutcome::TRANSPORT_ERROR) {
            m_context.metrics().counter("applepay_domain_verification.transport_error").increment();
        } else if (response.outcome() == AppleCallOutcome::CONNECTOR_ERROR) {
            m_context.metrics().counter("applepay_domain_verification.connector_error").increment();
        }
    }

    /** @brief Port of `core::utils::validate_profile_id_from_auth_layer`
     *  (core/utils.rs:2889-2911), including both PreconditionFailed messages. */
    [[nodiscard]] std::optional<core::error::AppError> validate_profile_id_from_auth_layer(
        std::string_view auth_profile,
        std::string_view mca_profile
    ) const
    {
        const bool has_auth = !auth_profile.empty();
        const bool has_mca = !mca_profile.empty();
        if (has_auth && has_mca) {
            if (auth_profile == mca_profile) {
                return std::nullopt;
            }
            return make_error(
                core::error::AppErrorKind::PRECONDITION_FAILED,
                "Profile id authentication failed. Please use the correct JWT token"
            );
        }
        if (has_auth && !has_mca) {
            return make_error(
                core::error::AppErrorKind::PRECONDITION_FAILED,
                "Couldn't find profile_id in record for authentication"
            );
        }
        return std::nullopt;
    }

    /** @brief Local `get_master_key` + `get_merchant_key_store_by_merchant_id`
     *  (utils.rs:27-32). */
    [[nodiscard]] MerchantKeyStore load_merchant_key_store(std::string_view merchant_id) const
    {
        std::string master_key{};
        if (const auto key = m_context.key_store().get("master_key", "apple_pay")) {
            master_key = key->value();
        } else if (
            const auto resolved = m_context.secret_resolver().resolve("master_key", "apple_pay")
        ) {
            master_key = *resolved;
        } else if (
            const auto configured =
                m_context.config_store().get_value("applepay_merchant_configs", "master_key")
        ) {
            master_key = *configured;
        } else {
            master_key = "apple-pay-master-key";
        }
        const auto merchant_key = ::app::crypto::KeyedHasher{master_key}.hash_with_domain(
            ::app::crypto::HashDomain::Generic,
            "merchant_key:" + std::string{merchant_id}
        );
        return MerchantKeyStore{merchant_key};
    }

    [[nodiscard]] std::string
    decrypt_connector_details(const MerchantKeyStore& key_store, std::string_view details) const
    {
        constexpr std::string_view marker{"enc:"};
        if (!details.starts_with(marker)) {
            return std::string{details};
        }
        return ::app::crypto::AesWrap{}.decrypt(details.substr(marker.size()), key_store.key());
    }

    /** @brief Local mirror of the RequestBuilder chain in
     *  core/verification.rs:43-54. */
    [[nodiscard]] ApplePayVerificationRequest request_builder(
        const ::app::dto::verifications::ApplepayMerchantVerificationConfigs& configs
    ) const
    {
        ApplePayVerificationRequest request{};
        request.set_method("POST");
        request.set_url(
            m_context.config_store()
                .get_value("applepay_merchant_configs", "applepay_endpoint")
                .value_or(
                    std::string{
                        "https://apple-pay-gateway.apple.com/paymentservices/registerMerchant"
                    }
                )
        );
        request.set_content_type("application/json");
        request.set_body(serde::Ser::encode_json(configs));
        request.set_certificate(m_context.config_store()
                                    .get_value("applepay_merchant_configs", "merchant_cert")
                                    .value_or(std::string{}));
        request.set_certificate_key(m_context.config_store()
                                        .get_value("applepay_merchant_configs", "merchant_cert_key")
                                        .value_or(std::string{}));
        return request;
    }

    /** @brief Local in-process `call_connector_api`
     *  (core/verification.rs:56-62). The configured client certificate/key are
     *  consumed here as the local cert-authenticated transport; no PEM bytes or
     *  secret values are placed in the wire payload, logs or cache keys. */
    [[nodiscard]] AppleCallResult call_connector_api(
        std::string_view merchant_id,
        const ApplePayVerificationRequest& request
    ) const
    {
        AppleCallResult result{};
        auto connector = m_context.connector_router().select(merchant_id);
        if (!connector) {
            result.set_outcome(AppleCallOutcome::TRANSPORT_ERROR);
            return result;
        }

        if (!request.certificate().empty() && !request.certificate_key().empty()) {
            m_context.metrics().counter("applepay.cert_authenticated_transport").increment();
        }

        ::app::connector::ConnectorRequest connector_request{};
        connector_request.set_operation("verify_merchant_creds_for_applepay");
        connector_request.set_merchant_id(std::string{merchant_id});
        connector_request.set_resource_id(request.url());
        connector_request.set_raw_payload(request.body());
        const auto connector_response = connector->authorize(connector_request);
        const auto& status = connector_response.get_status();

        if (status.empty()) {
            result.set_outcome(AppleCallOutcome::TRANSPORT_ERROR);
        } else if (status == "failed" || status == "error" || status == "declined") {
            result.set_outcome(AppleCallOutcome::CONNECTOR_ERROR);
        } else {
            result.set_outcome(AppleCallOutcome::SUCCESS);
        }
        result.set_status(status);
        return result;
    }

    [[nodiscard]] std::string encode_domains(const std::vector<std::string>& domains) const
    {
        return serde::Ser::encode_json(domains);
    }

    [[nodiscard]] std::vector<std::string> decode_domains(std::string_view raw) const
    {
        std::vector<std::string> domains{};
        if (raw.empty()) {
            return domains;
        }
        auto parsed = serde::Ser::decode_generic("application/json", raw);
        if (!parsed) {
            return domains;
        }
        if (const auto* array = std::get_if<interfaces::Value::Array>(&parsed->get())) {
            for (const auto& element: *array) {
                if (const auto* text = std::get_if<std::string>(&element.get())) {
                    domains.push_back(*text);
                }
            }
        }
        return domains;
    }

private:
    ::app::context::AppContext& m_context;
};

} // namespace app::handlers::verification
