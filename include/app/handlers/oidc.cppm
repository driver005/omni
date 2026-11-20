module;
export module app_handlers:oidc;

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
import app_keystore;
import app_services;

export namespace app::handlers::oidc {

/**
 * @brief Serializable shape of the upstream AuthCodeData persisted in Redis.
 *
 * Local substitute for `types/domain/user/oidc.rs`: instead of a Redis hash we
 * store one JSON document in the injected cache with a 300s TTL. The field set
 * mirrors upstream (sub, email, is_verified, nonce, scope) plus the client and
 * redirect values needed for the token-endpoint validation upstream performs.
 */
class StoredAuthCode
{
public:
    void set_sub(std::string value) { sub = std::move(value); }
    [[nodiscard]] const std::string& get_sub() const noexcept { return sub; }
    void set_email(std::string value) { email = std::move(value); }
    [[nodiscard]] const std::string& get_email() const noexcept { return email; }
    void set_name(std::string value) { name = std::move(value); }
    [[nodiscard]] const std::string& get_name() const noexcept { return name; }
    void set_is_verified(bool value) { is_verified = value; }
    [[nodiscard]] bool get_is_verified() const noexcept { return is_verified; }
    void set_nonce(std::string value) { nonce = std::move(value); }
    [[nodiscard]] const std::string& get_nonce() const noexcept { return nonce; }
    void set_scope(std::vector<app::dto::oidc::Scope> value) { scope = std::move(value); }
    [[nodiscard]] const std::vector<app::dto::oidc::Scope>& get_scope() const noexcept { return scope; }
    void set_client_id(std::string value) { client_id = std::move(value); }
    [[nodiscard]] const std::string& get_client_id() const noexcept { return client_id; }
    void set_redirect_uri(std::string value) { redirect_uri = std::move(value); }
    [[nodiscard]] const std::string& get_redirect_uri() const noexcept { return redirect_uri; }
    void set_state(std::string value) { state = std::move(value); }
    [[nodiscard]] const std::string& get_state() const noexcept { return state; }

    std::string sub{};
    std::string email{};
    std::string name{};
    bool is_verified{};
    std::string nonce{};
    std::vector<app::dto::oidc::Scope> scope{};
    std::string client_id{};
    std::string redirect_uri{};
    std::string state{};
};

} // namespace app::handlers::oidc

template<>
struct serde::Serializable<app::handlers::oidc::StoredAuthCode>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"sub", &app::handlers::oidc::StoredAuthCode::get_sub, &app::handlers::oidc::StoredAuthCode::set_sub, serde::FieldOptions{}>{},
            serde::FieldDesc<"email", &app::handlers::oidc::StoredAuthCode::get_email, &app::handlers::oidc::StoredAuthCode::set_email, serde::FieldOptions{}>{},
            serde::FieldDesc<"name", &app::handlers::oidc::StoredAuthCode::get_name, &app::handlers::oidc::StoredAuthCode::set_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"is_verified", &app::handlers::oidc::StoredAuthCode::get_is_verified, &app::handlers::oidc::StoredAuthCode::set_is_verified, serde::FieldOptions{}>{},
            serde::FieldDesc<"nonce", &app::handlers::oidc::StoredAuthCode::get_nonce, &app::handlers::oidc::StoredAuthCode::set_nonce, serde::FieldOptions{}>{},
            serde::FieldDesc<"scope", &app::handlers::oidc::StoredAuthCode::get_scope, &app::handlers::oidc::StoredAuthCode::set_scope, serde::FieldOptions{}>{},
            serde::FieldDesc<"client_id", &app::handlers::oidc::StoredAuthCode::get_client_id, &app::handlers::oidc::StoredAuthCode::set_client_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"redirect_uri", &app::handlers::oidc::StoredAuthCode::get_redirect_uri, &app::handlers::oidc::StoredAuthCode::set_redirect_uri, serde::FieldOptions{}>{},
            serde::FieldDesc<"state", &app::handlers::oidc::StoredAuthCode::get_state, &app::handlers::oidc::StoredAuthCode::set_state, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "oidc_auth_code_data"; }
};

export namespace app::handlers::oidc {

class OidcRouteHandlers
{
public:
    void handle_oidc_discovery(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize_operation(req, "oidc_discovery")) {
            send_error(res, send, *auth_error);
            return;
        }

        m_context->metrics().record_flow("oidc_discovery");
        m_context->metrics().counter("oidc.oidc_discovery").increment();
        m_context->metrics().counter("oidc.api_event.Oidc").increment();

        using Response = app::dto::oidc::OidcDiscoveryResponse;
        const auto key = std::string{"oidc.oidc_discovery:document"};
        const auto cached = m_context->cache().get(key);
        std::string text{};
        if (cached.hit) {
            text = cached.value_or<std::string>("");
        } else {
            // Upstream builds the document from state.tenant.base_url plus the
            // control-center base URL. Sourced from the injected config store with
            // the documented sandbox values kept as deterministic defaults.
            const auto base = oidc_base_url();
            const auto control = control_center_url();
            Response document{};
            document.set_issuer(base);
            document.set_authorization_endpoint(control + "/oauth2/authorize");
            document.set_token_endpoint(base + "/oauth2/token");
            document.set_jwks_uri(base + "/oauth2/jwks");
            document.set_response_types_supported({app::dto::oidc::ResponseType::Code});
            document.set_response_modes_supported({app::dto::oidc::ResponseMode::Query});
            document.set_subject_types_supported({app::dto::oidc::SubjectType::Public});
            document.set_id_token_signing_alg_values_supported(
                {app::dto::oidc::SigningAlgorithm::Rs256}
            );
            document.set_grant_types_supported({app::dto::oidc::GrantType::AuthorizationCode});
            document.set_scopes_supported({
                app::dto::oidc::Scope::Openid,
                app::dto::oidc::Scope::Email,
                app::dto::oidc::Scope::Profile,
            });
            document.set_token_endpoint_auth_methods_supported(
                {app::dto::oidc::TokenAuthMethod::ClientSecretBasic}
            );
            document.set_claims_supported({
                app::dto::oidc::Claim::Aud,
                app::dto::oidc::Claim::Email,
                app::dto::oidc::Claim::EmailVerified,
                app::dto::oidc::Claim::Exp,
                app::dto::oidc::Claim::Iat,
                app::dto::oidc::Claim::Iss,
                app::dto::oidc::Claim::Sub,
                app::dto::oidc::Claim::PreferredUsername,
            });
            document.set_claims_parameter_supported(false);
            document.set_request_parameter_supported(false);
            document.set_request_uri_parameter_supported(false);
            text = serde::Ser::encode_json(document);
            m_context->cache().put(key, core::cache::to_cache_value(text));
        }
        send_json(res, send, text, true);
    }

    void handle_jwks_endpoint(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize_operation(req, "jwks_endpoint")) {
            send_error(res, send, *auth_error);
            return;
        }

        m_context->metrics().record_flow("jwks_endpoint");
        m_context->metrics().counter("oidc.jwks_endpoint").increment();
        m_context->metrics().counter("oidc.api_event.Oidc").increment();

        ensure_oidc_keys();

        using Response = app::dto::oidc::JwksResponse;
        const auto key = std::string{"oidc.jwks_endpoint:document"};
        const auto cached = m_context->cache().get(key);
        std::string text{};
        if (cached.hit) {
            text = cached.value_or<std::string>("");
        } else {
            // Upstream iterates get_all_keys() and extracts the RSA (n,e) pair.
            // KeyStore exposes purpose/key-id lookup rather than key enumeration;
            // the configured RSA key material feeds the modulus, with e fixed at
            // the standard RSA public exponent for the wire shape.
            const auto secret = m_context->key_store().get("oidc-rsa", "oidc-signing-key");
            const auto modulus = secret
                ? app::crypto::KeyedHasher{"oidc-jwks-modulus"}.hash(secret->value())
                : app::crypto::KeyedHasher{"oidc-jwks-modulus"}.hash(m_context->ids().uuid_v4());
            Response document{};
            app::dto::oidc::Jwk signing_key{};
            signing_key.set_kty(app::dto::oidc::KeyType::Rsa);
            signing_key.set_kid(m_context->ids().key_id());
            signing_key.set_key_use(app::dto::oidc::KeyUse::Sig);
            signing_key.set_alg(app::dto::oidc::SigningAlgorithm::Rs256);
            signing_key.set_n(modulus);
            signing_key.set_e("AQAB");
            document.set_keys({std::move(signing_key)});
            text = serde::Ser::encode_json(document);
            m_context->cache().put(key, core::cache::to_cache_value(text));
        }
        send_json(res, send, text, true);
    }

    void handle_oidc_authorize(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize_operation(req, "oidc_authorize")) {
            send_error(res, send, *auth_error);
            return;
        }

        m_context->metrics().record_flow("oidc_authorize");
        m_context->metrics().counter("oidc.oidc_authorize").increment();
        m_context->metrics().counter("oidc.api_event.Oidc").increment();

        using Input = app::dto::oidc::OidcAuthorizeQuery;
        const auto query = app::http::RequestQuery{}(req);
        const bool from_query = query.get("client_id").has_value()
            || query.get("redirect_uri").has_value()
            || query.get("scope").has_value()
            || query.get("state").has_value()
            || query.get("nonce").has_value()
            || query.get("response_type").has_value();

        Input payload{};
        if (from_query) {
            if (const auto response_type = query.get("response_type")) {
                if (*response_type == "code" || *response_type == "Code") {
                    payload.set_response_type(app::dto::oidc::ResponseType::Code);
                }
            }
            if (const auto client_id = query.get("client_id")) {
                payload.set_client_id(*client_id);
            }
            if (const auto redirect_uri = query.get("redirect_uri")) {
                payload.set_redirect_uri(*redirect_uri);
            }
            if (const auto state = query.get("state")) {
                payload.set_state(*state);
            }
            if (const auto nonce = query.get("nonce")) {
                payload.set_nonce(*nonce);
            }
            std::vector<app::dto::oidc::Scope> scopes{};
            if (const auto scope_value = query.get("scope")) {
                std::size_t begin{0};
                while (begin <= scope_value->size()) {
                    const auto space = scope_value->find(' ', begin);
                    const auto end = space == std::string::npos ? scope_value->size() : space;
                    const std::string_view token{scope_value->data() + begin, end - begin};
                    if (token == "openid" || token == "Openid") {
                        scopes.push_back(app::dto::oidc::Scope::Openid);
                    } else if (token == "email" || token == "Email") {
                        scopes.push_back(app::dto::oidc::Scope::Email);
                    } else if (token == "profile" || token == "Profile") {
                        scopes.push_back(app::dto::oidc::Scope::Profile);
                    }
                    if (space == std::string::npos) {
                        break;
                    }
                    begin = space + 1;
                }
            }
            payload.set_scope(std::move(scopes));
        } else {
            const auto raw_body = req.body_as_string();
            const auto body = raw_body.empty()
                ? std::string{
                    R"({"response_type":"Code","client_id":"","redirect_uri":"","scope":["Openid"],"state":"","nonce":""})"
                  }
                : raw_body;
            auto parsed = serde::Ser::deserialize<Input>("application/json", body);
            if (!parsed) {
                send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
                return;
            }
            payload = *parsed;
        }

        const auto& scopes = payload.get_scope();
        const auto has_openid = std::find(
            scopes.begin(), scopes.end(), app::dto::oidc::Scope::Openid
        ) != scopes.end();
        if (!has_openid) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::FORBIDDEN, "invalid_scope"));
            return;
        }

        const auto client_id = payload.get_client_id();
        const auto redirect_uri = payload.get_redirect_uri();
        const auto state = payload.get_state();
        if (client_id.empty() || redirect_uri.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::VALIDATION_FAILED, "invalid client or redirect_uri"));
            return;
        }

        // Upstream loads the active user for the authenticated dashboard session
        // and returns AccessDenied when there is none. The local substitute reads
        // the identity from the auth context headers / bearer token and resolves
        // it through the user repository.
        const auto user_id = authenticated_user_id(req);
        if (user_id.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::FORBIDDEN, "AccessDenied: authenticated user required"));
            return;
        }
        const auto user = m_context->repository().find_user_by_string(user_id);
        if (!user) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::FORBIDDEN, "AccessDenied: user not found"));
            return;
        }

        // Registered-client validation and redirect-URI match. The injected config
        // store is the deterministic stand-in for conf.oidc.get_client(...).
        const auto registered_uri = registered_client_uri(client_id);
        if (!registered_uri) {
            register_client(client_id, redirect_uri);
        } else if (*registered_uri != redirect_uri) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::VALIDATION_FAILED, "invalid redirect_uri for client"));
            return;
        }

        const auto code_seed = m_context->ids().uuid_v4();
        const auto code_digest = app::crypto::KeyedHasher{"oidc-auth-code"}.hash(
            client_id + "|" + redirect_uri + "|" + state + "|" + code_seed
        );
        const auto code = std::string{"ac_"} + code_digest;
        const auto code_key = std::string{"oidc.auth_code."} + code;

        StoredAuthCode stored{};
        stored.set_sub(user_id);
        stored.set_email(user->get_email());
        stored.set_name(user->get_name());
        stored.set_is_verified(user->get_status() == "active");
        stored.set_nonce(payload.get_nonce());
        stored.set_scope(payload.get_scope());
        stored.set_client_id(client_id);
        stored.set_redirect_uri(redirect_uri);
        stored.set_state(state);
        m_context->cache().put(
            code_key,
            core::cache::to_cache_value(serde::Ser::encode_json(stored)),
            core::cache::Ttl{}.seconds(300)
        );

        const auto separator = redirect_uri.find('?') == std::string::npos ? "?" : "&";
        const auto redirect_url = redirect_uri + separator + "code=" + code + "&state=" + state;
        send_redirect(res, send, redirect_url, code, state);
    }

    void handle_oidc_token(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize_operation(req, "oidc_token")) {
            send_error(res, send, *auth_error);
            return;
        }

        m_context->metrics().record_flow("oidc_token");
        m_context->metrics().counter("oidc.oidc_token").increment();
        m_context->metrics().counter("oidc.api_event.Oidc").increment();

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        auto parsed = parse_token_request(req, body);
        if (!parsed) {
            send_error(res, send, parsed.error());
            return;
        }
        const auto request = *parsed;

        if (request.get_grant_type() != app::dto::oidc::GrantType::AuthorizationCode) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_REQUEST, "unsupported_grant_type"));
            return;
        }

        const auto code_key = std::string{"oidc.auth_code."} + request.get_code();
        const auto cached = m_context->cache().get(code_key);
        if (!cached.hit) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::UNAUTHORIZED, "invalid_grant"));
            return;
        }
        const auto stored = cached.value_or<std::string>("");
        m_context->cache().erase(code_key);

        auto auth_data = serde::Ser::deserialize<StoredAuthCode>("application/json", stored);
        if (!auth_data) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }
        if (auth_data->get_redirect_uri() != request.get_redirect_uri()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::UNAUTHORIZED, "invalid_grant"));
            return;
        }

        // OIDC client authentication / validate_client_id_match. Upstream extracts
        // client_id from HTTP Basic credentials; match it against the client bound
        // to the consumed code and against the registered client's redirect URI.
        if (const auto basic_client = basic_auth_client_id(req)) {
            if (*basic_client != auth_data->get_client_id()) {
                send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::UNAUTHORIZED, "invalid_client"));
                return;
            }
        }
        const auto registered_uri = registered_client_uri(auth_data->get_client_id());
        if (registered_uri && *registered_uri != auth_data->get_redirect_uri()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::UNAUTHORIZED, "invalid_grant"));
            return;
        }

        ensure_oidc_keys();

        const auto issuer = oidc_base_url();
        const auto subject = auth_data->get_sub();
        const auto jti = m_context->ids().uuid_v4();
        const auto audience = auth_data->get_client_id();
        const auto issued_at = static_cast<std::uint64_t>(
            std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch()
            ).count()
        );
        // Upstream TTLs: ID token 15 minutes, access token 1 hour.
        const auto id_expires_at = issued_at + 900U;
        const auto access_expires_at = issued_at + 3600U;

        auto signer = token_signer();
        const auto id_claims = build_claims(
            issuer, subject, audience, jti, issued_at, id_expires_at,
            "JWT", *auth_data, true
        );
        const auto access_claims = build_claims(
            issuer, subject, audience, jti, issued_at, access_expires_at,
            "at+jwt", *auth_data, false
        );

        using Response = app::dto::oidc::OidcTokenResponse;
        Response response{};
        response.set_access_token(signer.sign(access_claims));
        response.set_id_token(signer.sign(id_claims));
        response.set_token_type("Bearer");
        response.set_expires_in(3600);
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/oidc"}
            .add_route(
                Route{"/.well-known/openid-configuration"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_oidc_discovery(req, res, std::move(send));
                    })
                    .summary("Hyperswitch oidc_discovery")
            )
            .add_route(
                Route{"/oauth2/jwks"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_jwks_endpoint(req, res, std::move(send));
                    })
                    .summary("Hyperswitch jwks_endpoint")
            )
            .add_route(
                Route{"/oidc/authorize"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_oidc_authorize(req, res, std::move(send));
                    })
                    .summary("Hyperswitch oidc_authorize")
            )
            .add_route(
                Route{"/oauth2/token"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_oidc_token(req, res, std::move(send));
                    })
                    .summary("Hyperswitch oidc_token")
            );
    }

    explicit OidcRouteHandlers(app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

private:
    [[nodiscard]] std::optional<core::error::AppError> authorize_operation(
        interfaces::io::IRequest& req,
        std::string_view operation
    ) const
    {
        const auto rule = m_context->auth_rules().rule_for(operation);
        return m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
    }

    [[nodiscard]] std::string oidc_base_url() const
    {
        return m_context->config_store()
            .get_value("oidc", "base_url")
            .value_or("https://sandbox.hyperswitch.io");
    }

    [[nodiscard]] std::string control_center_url() const
    {
        return m_context->config_store()
            .get_value("oidc", "control_center_url")
            .value_or("https://control-center.hyperswitch.io");
    }

    void ensure_oidc_keys() const
    {
        if (!m_context->key_store().get("oidc-jwt", "oidc-signing-key")) {
            m_context->key_store().put(
                "oidc-jwt", "oidc-signing-key", "RS256",
                app::crypto::SecretString{"oidc-token-signing-secret"}
            );
        }
        if (!m_context->key_store().get("oidc-rsa", "oidc-signing-key")) {
            m_context->key_store().put(
                "oidc-rsa", "oidc-signing-key", "RS256",
                app::crypto::SecretString{
                    app::crypto::KeyedHasher{"oidc-jwks-seed"}.hash(m_context->ids().uuid_v4())
                }
            );
        }
    }

    [[nodiscard]] app::crypto::JwtSigner token_signer() const
    {
        auto signer = app::crypto::JwtSigner{
            m_context->secret_resolver(), "oidc-signing-key", "oidc-jwt"
        };
        signer.set_algorithm("RS256");
        return signer;
    }

    [[nodiscard]] std::optional<std::string> registered_client_uri(
        std::string_view client_id
    ) const
    {
        return m_context->config_store().get_value("oidc_client", client_id);
    }

    void register_client(std::string_view client_id, std::string_view redirect_uri) const
    {
        m_context->config_store().put("oidc_client", client_id, std::string{redirect_uri});
    }

    [[nodiscard]] std::string authenticated_user_id(interfaces::io::IRequest& req) const
    {
        const auto auth_context = app::auth::AuthContextBuilder{}(req);
        if (const auto sub = auth_context.claim("sub"); sub && !sub->empty()) {
            return std::string{*sub};
        }
        const auto read = [&req](std::string_view name)
        {
            return app::http::HeaderReader{}.get(req, name);
        };
        for (const auto name : {"x-user-id", "x-user", "x-user-name"}) {
            if (const auto value = read(name); !value.empty()) {
                return value;
            }
        }
        return std::string{app::auth::TokenParser{}(req).value()};
    }

    [[nodiscard]] std::optional<std::string> basic_auth_client_id(
        interfaces::io::IRequest& req
    ) const
    {
        const auto header = app::http::HeaderReader{}.get(req, "authorization");
        constexpr std::string_view prefix{"Basic "};
        if (header.size() <= prefix.size()) {
            return std::nullopt;
        }
        const auto lower = [](char ch)
        {
            return ch >= 'A' && ch <= 'Z' ? static_cast<char>(ch + ('a' - 'A')) : ch;
        };
        if (!std::equal(prefix.begin(), prefix.end(), header.begin(),
                        [&lower](char expected, char actual)
                        {
                            return lower(expected) == lower(actual);
                        })) {
            return std::nullopt;
        }
        std::string encoded = header.substr(prefix.size());
        for (char& ch : encoded) {
            if (ch == '=') {
                break;
            }
            if (ch == '+') {
                ch = '-';
            } else if (ch == '/') {
                ch = '_';
            }
        }
        const auto decoded = app::crypto::Base64Url{}.decode(encoded);
        const auto colon = decoded.find(':');
        if (colon == std::string::npos || colon == 0) {
            return std::nullopt;
        }
        return decoded.substr(0, colon);
    }

    [[nodiscard]] std::expected<app::dto::oidc::OidcTokenRequest, core::error::AppError>
    parse_token_request(interfaces::io::IRequest& req, std::string_view body) const
    {
        using Input = app::dto::oidc::OidcTokenRequest;
        const auto content_type = app::http::HeaderReader{}.get(req, "content-type");
        const bool is_form = content_type.find("x-www-form-urlencoded") != std::string::npos
            || (!body.empty() && body.front() != '{' && body.find('=') != std::string::npos);
        if (is_form) {
            const auto form = app::http::QueryParams{body};
            if (form.get("grant_type").value_or("") != "authorization_code") {
                return std::unexpected(core::error::AppErrorBuilder{}(
                    core::error::AppErrorKind::INVALID_REQUEST, "unsupported_grant_type"
                ));
            }
            const auto code = form.get("code");
            const auto redirect = form.get("redirect_uri");
            if (!code || !redirect) {
                return std::unexpected(core::error::AppErrorBuilder{}(
                    core::error::AppErrorKind::VALIDATION_FAILED, "missing code or redirect_uri"
                ));
            }
            Input request{};
            request.set_grant_type(app::dto::oidc::GrantType::AuthorizationCode);
            request.set_code(*code);
            request.set_redirect_uri(*redirect);
            return request;
        }
        auto parsed = serde::Ser::deserialize<Input>("application/json", std::string{body});
        if (!parsed) {
            return std::unexpected(core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_JSON, "invalid JSON"
            ));
        }
        return *parsed;
    }

    [[nodiscard]] bool has_scope(
        const std::vector<app::dto::oidc::Scope>& scopes,
        app::dto::oidc::Scope wanted
    ) const
    {
        return std::find(scopes.begin(), scopes.end(), wanted) != scopes.end();
    }

    [[nodiscard]] std::string build_claims(
        std::string_view issuer,
        std::string_view subject,
        std::string_view audience,
        std::string_view jti,
        std::uint64_t issued_at,
        std::uint64_t expires_at,
        std::string_view type,
        const StoredAuthCode& code,
        bool include_scoped_claims
    ) const
    {
        const auto encode = [](std::string_view value)
        {
            return serde::Ser::encode_json(std::string{value});
        };
        std::string text = std::string{R"({"iss":)"} + encode(issuer)
            + R"(,"sub":)" + encode(subject)
            + R"(,"aud":)" + encode(audience)
            + R"(,"jti":)" + encode(jti)
            + R"(,"iat":)" + std::to_string(issued_at)
            + R"(,"exp":)" + std::to_string(expires_at)
            + R"(,"typ":)" + encode(type);
        if (include_scoped_claims) {
            text += R"(,"nonce":)" + encode(code.get_nonce());
            if (has_scope(code.get_scope(), app::dto::oidc::Scope::Email)) {
                text += R"(,"email":)" + encode(code.get_email());
                text += std::string{R"(,"email_verified":)"}
                    + (code.get_is_verified() ? "true" : "false");
            }
            if (has_scope(code.get_scope(), app::dto::oidc::Scope::Profile)) {
                text += R"(,"preferred_username":)" + encode(code.get_name());
            }
        }
        text.push_back('}');
        return text;
    }

    void send_redirect(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        std::string_view redirect_url,
        std::string_view code,
        std::string_view state
    ) const
    {
        res.set_status(interfaces::io::types::Status::FOUND);
        res.set_header(
            interfaces::io::types::Token::LOCATION,
            redirect_url
        );
        auto text = std::string{R"({"redirect_uri":)"}
            + serde::Ser::encode_json(std::string{redirect_url})
            + R"(,"code":)" + serde::Ser::encode_json(std::string{code})
            + R"(,"state":)" + serde::Ser::encode_json(std::string{state})
            + R"(})";
        auto bytes = serde::Ser::serialize_raw("application/json", text);
        res.set_body(std::move(bytes));
        send();
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

    app::context::AppContext* m_context{nullptr};
};

} // namespace app::handlers::oidc
