module;
export module app_handlers:verification;

import std;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import core_error;
import app_auth;
import app_connector;
import app_http;
import app_crypto;
import app_metrics;
import app_store;
import app_keystore;
import app_services;

namespace app::handlers::verification {

using MerchantConnectorRecord = app::store::merchant_connectors::MerchantConnectorRecord;

/** @brief Local serializable shape of the upstream request wire.
 *
 *  `ApplepayMerchantVerificationRequest` (api_models/verifications.rs:13-19) is
 *  snake_case with a plain-string `merchant_connector_account_id` newtype. The
 *  converted app_dto instead carries a `MerchantConnectorAccountIdValue` wrapper
 *  with no serde support, so this local shape restores the exact upstream wire.
 *  The converted wrapper form is intentionally NOT accepted: upstream would
 *  reject it as a deserialization error.
 */
class VerificationRequestWire
{
public:
    void set_domain_names(std::vector<std::string> value) { domain_names = std::move(value); }
    [[nodiscard]] const std::vector<std::string>& get_domain_names() const noexcept { return domain_names; }
    void set_merchant_connector_account_id(std::string value) { merchant_connector_account_id = std::move(value); }
    [[nodiscard]] const std::string& get_merchant_connector_account_id() const noexcept { return merchant_connector_account_id; }
    std::vector<std::string> domain_names{};
    std::string merchant_connector_account_id{};
};

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
    [[nodiscard]] const std::string& key() const noexcept { return m_key; }

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
    void set_method(std::string value) { m_method = std::move(value); }
    [[nodiscard]] const std::string& method() const noexcept { return m_method; }
    void set_url(std::string value) { m_url = std::move(value); }
    [[nodiscard]] const std::string& url() const noexcept { return m_url; }
    void set_content_type(std::string value) { m_content_type = std::move(value); }
    [[nodiscard]] const std::string& content_type() const noexcept { return m_content_type; }
    void set_body(std::string value) { m_body = std::move(value); }
    [[nodiscard]] const std::string& body() const noexcept { return m_body; }
    void set_certificate(std::string value) { m_certificate = std::move(value); }
    [[nodiscard]] const std::string& certificate() const noexcept { return m_certificate; }
    void set_certificate_key(std::string value) { m_certificate_key = std::move(value); }
    [[nodiscard]] const std::string& certificate_key() const noexcept { return m_certificate_key; }

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
    void set_outcome(AppleCallOutcome value) noexcept { m_outcome = value; }
    [[nodiscard]] AppleCallOutcome outcome() const noexcept { return m_outcome; }
    void set_status(std::string value) { m_status = std::move(value); }
    [[nodiscard]] const std::string& status() const noexcept { return m_status; }

private:
    AppleCallOutcome m_outcome{AppleCallOutcome::TRANSPORT_ERROR};
    std::string m_status{};
};

} // namespace app::handlers::verification

// Serialization declared before the handler bodies that use this shape.
template<>
struct serde::Serializable<::app::handlers::verification::VerificationRequestWire>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "domain_names",
                &::app::handlers::verification::VerificationRequestWire::get_domain_names,
                &::app::handlers::verification::VerificationRequestWire::set_domain_names,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_connector_account_id",
                &::app::handlers::verification::VerificationRequestWire::get_merchant_connector_account_id,
                &::app::handlers::verification::VerificationRequestWire::set_merchant_connector_account_id,
                serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "verification_request_wire"; }
};

export namespace app::handlers::verification {

class VerificationRouteHandlers
{
public:
    /** @brief Route handler for upstream `apple_pay_merchant_registration`
     *  (routes/verification.rs:11-58 v1, :60-107 v2). */
    void handle_apple_pay_merchant_registration(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send,
        bool v2
    )
    {
        const auto auth = app::auth::AuthContextBuilder{}(req);
        if (const auto auth_error = authorize_verification(auth, "apple_pay_merchant_registration", v2)) {
            send_error(res, send, *auth_error);
            return;
        }

        // The merchant id is the last path segment on both the v1 and v2 routes.
        const auto segments = app::http::RequestPath{}(req);
        const std::string merchant_id = segments.count() > 0
            ? std::string{segments.at(segments.count() - 1).value_or(std::string_view{})}
            : std::string{};
        if (merchant_id.empty()) {
            send_error(res, send, upstream_error(core::error::AppErrorKind::NOT_FOUND, "ResourceIdNotFound"));
            return;
        }

        // server_wrap closure (routes/verification.rs:29-40 / 78-89): the
        // authenticated processor account id must equal the path merchant id,
        // otherwise ApiErrorResponse::Unauthorized.
        const auto authenticated = authenticated_merchant_id(auth, req);
        if (authenticated.empty() || authenticated != merchant_id) {
            send_error(res, send, upstream_error(core::error::AppErrorKind::UNAUTHORIZED, "Unauthorized"));
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, upstream_error(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        // web::Json<ApplepayMerchantVerificationRequest> is strict: the plain
        // snake_case wire is the only accepted shape.
        using Input = ::app::handlers::verification::VerificationRequestWire;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, upstream_error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        auto result = verify_merchant_creds_for_applepay(
            merchant_id,
            std::string{auth.profile_id()},
            parsed->get_domain_names(),
            parsed->get_merchant_connector_account_id()
        );
        if (!result.has_value()) {
            send_error(res, send, result.error());
            return;
        }

        // #[instrument(fields(flow = ?Flow::Verification))] on the route.
        m_context->metrics().record_flow("Verification");
        send_json(res, send, result.value(), true);
    }

    /** @brief Route handler for upstream `retrieve_apple_pay_verified_domains`
     *  (routes/verification.rs:109-170). */
    void handle_retrieve_apple_pay_verified_domains(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send,
        bool v2
    )
    {
        const auto auth = app::auth::AuthContextBuilder{}(req);
        if (const auto auth_error = authorize_verification(auth, "retrieve_apple_pay_verified_domains", v2)) {
            send_error(res, send, *auth_error);
            return;
        }

        // Query<ApplepayGetVerifiedDomainsParam>: both snake_case fields are
        // required by the upstream model (api_models/verifications.rs:28-34).
        const auto query = app::http::RequestQuery{}(req);
        const auto merchant_param = query.get("merchant_id");
        const auto connector_param = query.get("merchant_connector_account_id");
        if (!merchant_param || merchant_param->empty()) {
            send_error(res, send, upstream_error(core::error::AppErrorKind::INVALID_REQUEST, "merchant_id query parameter required"));
            return;
        }
        if (!connector_param || connector_param->empty()) {
            send_error(res, send, upstream_error(core::error::AppErrorKind::INVALID_REQUEST, "merchant_connector_account_id query parameter required"));
            return;
        }
        const auto merchant_id = *merchant_param;
        const auto merchant_connector_id = *connector_param;

        const auto authenticated = authenticated_merchant_id(auth, req);
        if (authenticated.empty() || authenticated != merchant_id) {
            send_error(res, send, upstream_error(core::error::AppErrorKind::UNAUTHORIZED, "Unauthorized"));
            return;
        }

        auto result = get_verified_apple_domains_with_mid_mca_id(
            merchant_id,
            std::string{auth.profile_id()},
            merchant_connector_id
        );
        if (!result.has_value()) {
            send_error(res, send, result.error());
            return;
        }

        m_context->metrics().record_flow("Verification");
        send_json(res, send, result.value(), true);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;

        // Upstream v1 scope is /verify (app.rs:2870), v2 scope is /v2/verify
        // (app.rs:2886). Sub-paths use the upstream underscores/hyphens.
        Router{router, "verify"}
            .add_router(Router{router, "apple_pay"}.add_route(
                Route{":merchant_id"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_apple_pay_merchant_registration(req, res, std::move(send), false);
                    })
                    .summary("Hyperswitch apple_pay_merchant_registration")
            ))
            .add_route(
                Route{"applepay_verified_domains"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_retrieve_apple_pay_verified_domains(req, res, std::move(send), false);
                    })
                    .summary("Hyperswitch retrieve_apple_pay_verified_domains")
            );

        Router{router, "v2/verify"}
            .add_router(Router{router, "apple-pay"}.add_route(
                Route{":merchant_id"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_apple_pay_merchant_registration(req, res, std::move(send), true);
                    })
                    .summary("Hyperswitch apple_pay_merchant_registration (v2)")
            ))
            .add_route(
                Route{"applepay-verified-domains"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_retrieve_apple_pay_verified_domains(req, res, std::move(send), true);
                    })
                    .summary("Hyperswitch retrieve_apple_pay_verified_domains (v2)")
            );
    }

    explicit VerificationRouteHandlers(app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

private:
    // ── core/verification.rs ──────────────────────────────────────────────────

    /** @brief Port of `verify_merchant_creds_for_applepay`
     *  (core/verification.rs:11-93). */
    [[nodiscard]] std::expected<std::string, core::error::AppError>
    verify_merchant_creds_for_applepay(
        std::string_view merchant_id,
        std::string_view profile_id,
        const std::vector<std::string>& domain_names,
        std::string_view merchant_connector_id
    ) const
    {
        // core/verification.rs:18-24 — resolve key store + MCA first.
        auto account = get_apple_pay_merchant_connector_account(merchant_id, merchant_connector_id, profile_id);
        if (!account.has_value()) {
            return std::unexpected(account.error());
        }

        // core/verification.rs:26-41. APPLEPAY_INTERNAL_MERCHANT_NAME is
        // "Applepay_merchant" (:9); both identifier fields share the configured
        // common_merchant_identifier (:28-31).
        const auto common_identifier = m_context->config_store()
                                           .get_value("applepay_merchant_configs", "common_merchant_identifier")
                                           .value_or(std::string{"Applepay_merchant"});
        app::dto::verifications::ApplepayMerchantVerificationConfigs configs{};
        configs.set_domain_names(domain_names);
        configs.set_encrypt_to(common_identifier);
        configs.set_partner_internal_merchant_identifier(common_identifier);
        configs.set_partner_merchant_name("Applepay_merchant");

        // core/verification.rs:43-54 — POST, JSON content type, cert + cert key.
        // The in-process transport is a local substitute for the Apple HTTPS
        // gateway; the wire body and certificate references are upstream-exact.
        const auto request = request_builder(configs);

        // core/verification.rs:56-63.
        const auto response = call_connector_api(merchant_id, request);
        log_applepay_verification_response_if_error(response);

        // core/verification.rs:65-66 — outer client error maps to
        // InternalServerError (the original error is already logged above).
        if (response.outcome() == AppleCallOutcome::TRANSPORT_ERROR) {
            return std::unexpected(upstream_error(core::error::AppErrorKind::INTERNAL_ERROR, "InternalServerError"));
        }

        // core/verification.rs:69-92 — Ok persists the domains; Err returns
        // InvalidRequestData{"Applepay verification Failed"}.
        if (response.outcome() == AppleCallOutcome::CONNECTOR_ERROR) {
            return std::unexpected(upstream_error(core::error::AppErrorKind::INVALID_REQUEST, "Applepay verification Failed"));
        }

        auto added = add_domain_to_apple_pay_merchant_connector_account(*account, domain_names);
        if (!added.has_value()) {
            return std::unexpected(added.error());
        }

        app::dto::verifications::ApplepayMerchantResponse response_dto{};
        response_dto.set_status_message("Applepay verification Completed");
        return serde::Ser::encode_json(response_dto);
    }

    /** @brief Port of `get_verified_apple_domains_with_mid_mca_id`
     *  (core/verification.rs:95-119). */
    [[nodiscard]] std::expected<std::string, core::error::AppError>
    get_verified_apple_domains_with_mid_mca_id(
        std::string_view merchant_id,
        std::string_view profile_id,
        std::string_view merchant_connector_id
    ) const
    {
        auto account = get_apple_pay_merchant_connector_account(merchant_id, merchant_connector_id, profile_id);
        if (!account.has_value()) {
            return std::unexpected(account.error());
        }

        // `.applepay_verified_domains.unwrap_or_default()` (:112-114).
        app::dto::verifications::ApplepayVerifiedDomainsResponse response{};
        response.set_verified_domains(decode_domains(account->account.get_applepay_verified_domains()));
        return serde::Ser::encode_json(response);
    }

    // ── core/verification/utils.rs ───────────────────────────────────────────

    /** @brief Port of `get_apple_pay_merchant_connector_account`
     *  (core/verification/utils.rs:18-63). */
    [[nodiscard]] std::expected<ApplePayMerchantConnector, core::error::AppError>
    get_apple_pay_merchant_connector_account(
        std::string_view merchant_id,
        std::string_view merchant_connector_id,
        std::string_view profile_id_from_auth_layer
    ) const
    {
        auto repository = m_context->repository();

        // get_merchant_key_store_by_merchant_id(...).to_not_found_response(
        // MerchantAccountNotFound) — utils.rs:27-32.
        if (!repository.find_merchant_account_by_string(merchant_id)) {
            return std::unexpected(upstream_error(core::error::AppErrorKind::NOT_FOUND, "MerchantAccountNotFound"));
        }

        ApplePayMerchantConnector resolved{};
        resolved.key_store = load_merchant_key_store(merchant_id);

        // v1 find_by_...merchant_id_merchant_connector_id / v2 find_by_id both
        // resolve to the repository lookup + merchant check (utils.rs:34-50,52-62).
        auto record = repository.find_merchant_connector_by_string(merchant_connector_id);
        if (!record || record->get_merchant_id() != merchant_id) {
            return std::unexpected(upstream_error(core::error::AppErrorKind::NOT_FOUND, "ResourceIdNotFound"));
        }

        // utils.rs:52-57 — validate_profile_id_from_auth_layer.
        if (const auto profile_error = validate_profile_id_from_auth_layer(
                profile_id_from_auth_layer, record->get_profile_id()
            )) {
            return std::unexpected(*profile_error);
        }

        // Upstream decrypts the MCA with the key store on read. Local substitute:
        // AesWrap over the persisted value; plaintext (unmarked) values pass
        // through unchanged, marked values are unwrapped with the merchant key.
        const auto details = record->get_connector_account_details();
        resolved.details_were_encrypted = details.starts_with("enc:");
        record->set_connector_account_details(decrypt_connector_details(resolved.key_store, details));
        resolved.account = std::move(*record);
        return resolved;
    }

    /** @brief Port of `add_domain_to_apple_pay_merchant_connector_account`
     *  (core/verification/utils.rs:65-136). */
    [[nodiscard]] std::expected<std::vector<std::string>, core::error::AppError>
    add_domain_to_apple_pay_merchant_connector_account(
        ApplePayMerchantConnector& resolved,
        const std::vector<std::string>& domain_from_req
    ) const
    {
        // utils.rs:71-81 — keep already-verified first, append only new domains.
        auto already_verified_domains = decode_domains(resolved.account.get_applepay_verified_domains());
        for (const auto& domain : domain_from_req) {
            if (std::find(already_verified_domains.begin(), already_verified_domains.end(), domain)
                == already_verified_domains.end()) {
                already_verified_domains.push_back(domain);
            }
        }

        // Upstream re-encrypts the MCA with the key store on update
        // (utils.rs:119-125). Mirror that for marked values only.
        if (resolved.details_were_encrypted) {
            const auto encrypted = app::crypto::AesWrap{}.encrypt(
                resolved.account.get_connector_account_details(), resolved.key_store.key()
            );
            resolved.account.set_connector_account_details("enc:" + encrypted);
        }
        resolved.account.set_applepay_verified_domains(encode_domains(already_verified_domains));

        // utils.rs:119-133 — InternalServerError on update failure.
        if (!m_context->repository().update_merchant_connector(resolved.account)) {
            return std::unexpected(upstream_error(
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
            m_context->metrics().counter("applepay_domain_verification.transport_error").increment();
        } else if (response.outcome() == AppleCallOutcome::CONNECTOR_ERROR) {
            m_context->metrics().counter("applepay_domain_verification.connector_error").increment();
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
            return upstream_error(
                core::error::AppErrorKind::PRECONDITION_FAILED,
                "Profile id authentication failed. Please use the correct JWT token"
            );
        }
        if (has_auth && !has_mca) {
            return upstream_error(
                core::error::AppErrorKind::PRECONDITION_FAILED,
                "Couldn't find profile_id in record for authentication"
            );
        }
        return std::nullopt;
    }

    // ── local in-process infrastructure substitutes ─────────────────────────

    /** @brief Local `get_master_key` + `get_merchant_key_store_by_merchant_id`
     *  (utils.rs:27-32). */
    [[nodiscard]] MerchantKeyStore load_merchant_key_store(std::string_view merchant_id) const
    {
        std::string master_key{};
        if (const auto key = m_context->key_store().get("master_key", "apple_pay")) {
            master_key = key->value();
        } else if (const auto resolved = m_context->secret_resolver().resolve("master_key", "apple_pay")) {
            master_key = *resolved;
        } else if (const auto configured = m_context->config_store().get_value("applepay_merchant_configs", "master_key")) {
            master_key = *configured;
        } else {
            master_key = "apple-pay-master-key";
        }
        const auto merchant_key = app::crypto::KeyedHasher{master_key}.hash_with_domain(
            app::crypto::HashDomain::Generic,
            "merchant_key:" + std::string{merchant_id}
        );
        return MerchantKeyStore{merchant_key};
    }

    [[nodiscard]] std::string decrypt_connector_details(
        const MerchantKeyStore& key_store,
        std::string_view details
    ) const
    {
        constexpr std::string_view marker{"enc:"};
        if (!details.starts_with(marker)) {
            return std::string{details};
        }
        return app::crypto::AesWrap{}.decrypt(details.substr(marker.size()), key_store.key());
    }

    /** @brief Local mirror of the RequestBuilder chain in
     *  core/verification.rs:43-54. */
    [[nodiscard]] ApplePayVerificationRequest request_builder(
        const app::dto::verifications::ApplepayMerchantVerificationConfigs& configs
    ) const
    {
        ApplePayVerificationRequest request{};
        request.set_method("POST");
        request.set_url(
            m_context->config_store()
                .get_value("applepay_merchant_configs", "applepay_endpoint")
                .value_or(std::string{"https://apple-pay-gateway.apple.com/paymentservices/registerMerchant"})
        );
        request.set_content_type("application/json");
        request.set_body(serde::Ser::encode_json(configs));
        request.set_certificate(
            m_context->config_store()
                .get_value("applepay_merchant_configs", "merchant_cert")
                .value_or(std::string{})
        );
        request.set_certificate_key(
            m_context->config_store()
                .get_value("applepay_merchant_configs", "merchant_cert_key")
                .value_or(std::string{})
        );
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
        auto connector = m_context->connector_router().select(merchant_id);
        if (!connector) {
            result.set_outcome(AppleCallOutcome::TRANSPORT_ERROR);
            return result;
        }

        if (!request.certificate().empty() && !request.certificate_key().empty()) {
            m_context->metrics().counter("applepay.cert_authenticated_transport").increment();
        }

        app::connector::ConnectorRequest connector_request{};
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

    // ── auth / shared helpers ─────────────────────────────────────────────────

    /** @brief Port of the `auth_type` branches in routes/verification.rs.
     *
     *  `is_jwt_auth(headers)` selects JWTAuth (ProfileAccountWrite /
     *  MerchantAccountRead, allow_connected: true, allow_platform: false);
     *  otherwise HeaderAuth(ApiKeyAuth) on v1 or V2ApiKeyAuth on v2 is used,
     *  both with allow_connected_scope_operation=false and
     *  allow_platform_self_operation=false. */
    [[nodiscard]] std::optional<core::error::AppError> authorize_verification(
        const app::auth::AuthContext& auth,
        std::string_view operation,
        bool v2
    ) const
    {
        const bool is_jwt = !auth.token().is_empty(); // is_jwt_auth: Bearer prefix
        const auto permission = operation == "apple_pay_merchant_registration"
                                    ? app::auth::AppPermission::PROFILE_WRITE
                                    : app::auth::AppPermission::MERCHANT_READ;

        if (!is_jwt) {
            // Connected-scope operations are denied for both API-key branches.
            if (auth.claim(app::auth::kClaimConnectedMerchantId).has_value()) {
                return upstream_error(core::error::AppErrorKind::UNAUTHORIZED, "Unauthorized");
            }
            const auto scheme = v2 ? app::auth::AuthScheme::V2_API_KEY
                                   : app::auth::AuthScheme::API_KEY;
            const auto rule = app::auth::RouteAuthRuleBuilder{}
                                  .scheme(scheme)
                                  .permission(permission)
                                  .allow_connected(false)
                                  .allow_platform(false)
                                  .build();
            // The local authorizer requires a credential for API_KEY /
            // V2_API_KEY rules. Upstream authenticates these branches from the
            // api-key header, so carry that header as the local credential when
            // no Bearer token was supplied (the JWT branch always has one).
            auto effective = auth;
            if (effective.token().is_empty()) {
                if (const auto api_key = effective.claim(app::auth::kClaimApiKey);
                    api_key && !api_key->empty()) {
                    effective.set_token(app::auth::BearerToken{std::string{*api_key}});
                }
            }
            return m_context->authorizer().authorize_route(effective, rule);
        }

        const auto rule = app::auth::RouteAuthRuleBuilder{}
                              .scheme(app::auth::AuthScheme::JWT)
                              .permission(permission)
                              .allow_connected(true)
                              .allow_platform(false)
                              .build();
        return m_context->authorizer().authorize_route(auth, rule);
    }

    [[nodiscard]] std::string authenticated_merchant_id(
        const app::auth::AuthContext& auth,
        interfaces::io::IRequest& req
    ) const
    {
        // For JWT allow_connected=true the effective processor account is the
        // connected merchant when acting on its behalf.
        if (!auth.merchant_id().empty()) {
            return auth.merchant_id();
        }
        if (const auto connected = auth.claim(app::auth::kClaimConnectedMerchantId)) {
            if (!connected->empty()) {
                return std::string{*connected};
            }
        }
        return app::http::merchant_id_from_header{}(req);
    }

    [[nodiscard]] static core::error::AppError upstream_error(
        core::error::AppErrorKind kind,
        std::string message
    )
    {
        return core::error::AppErrorBuilder{}(kind, std::move(message));
    }

    void send_error(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        core::error::AppError error
    ) const
    {
        auto text = serde::Ser::encode_json(error);
        auto bytes = serde::Ser::serialize_raw("application/json", text);
        res.set_status(core::error::AppErrorStatus{}(error.get_kind()));
        res.set_body(std::move(bytes));
        send();
    }

    void send_json(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        std::string_view text,
        bool ok
    ) const
    {
        auto bytes = serde::Ser::serialize_raw("application/json", text);
        res.set_status(ok ? interfaces::io::types::Status::OK
                          : interfaces::io::types::Status::BAD_REQUEST);
        res.set_body(std::move(bytes));
        send();
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
            for (const auto& element : *array) {
                if (const auto* text = std::get_if<std::string>(&element.get())) {
                    domains.push_back(*text);
                }
            }
        }
        return domains;
    }

    app::context::AppContext* m_context{nullptr};
};

} // namespace app::handlers::verification
