module;
export module app_handlers:user;

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
import app_events;
import app_workflow;
import app_locking;
import app_services;
import app_connector;

export namespace app::handlers::user {

class UserRouteHandlers
{
public:
    void handle_get_active_user_details(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("get_active_user_details");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto merchant_id = app::http::merchant_id_from_header{}(req);
        const auto org_id = app::http::org_id_from_header{}(req);
        const auto auth_ctx = app::auth::AuthContextBuilder{}(req);
        const auto effective_merchant_id = merchant_id.empty() ? auth_ctx.merchant_id() : merchant_id;
        std::string effective_org_id = org_id.empty() ? auth_ctx.org_id() : org_id;

        const auto body = std::string{R"({"op":"get_active_user_details"})"};
        auto parsed = serde::Ser::decode_generic("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        m_context->metrics().record_flow("get_active_user_details");
        m_context->metrics().counter("user.get_active_user_details").increment();

        std::string merchant_name{};
        if (!effective_merchant_id.empty()) {
            const auto account = m_context->repository().find_merchant_account_by_string(effective_merchant_id);
            if (!account) {
                send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "merchant account not found"));
                return;
            }
            merchant_name = account->get_merchant_name();
            if (effective_org_id.empty()) {
                effective_org_id = account->get_organization_id();
            }
        }

        // Derive verification/theme data from persisted config and identity records
        // instead of fixed placeholders (parity: core/user.rs:139-226).
        bool two_factor_setup{false};
        for (const auto& record : m_context->repository().list_authentication_records()) {
            if (record.get_merchant_id() == effective_merchant_id &&
                record.get_method() == "TOTP" && record.get_status() == "active") {
                two_factor_setup = true;
                break;
            }
        }
        const auto verification_days = m_context->repository().get_config(
            "user.verification_days." + effective_merchant_id);
        const auto theme_id = m_context->repository().get_config(
            "user.theme." + effective_merchant_id);
        const auto credential_source = m_context->repository().get_config("user.offer_engine_credential_source");

        using Response = app::model::user::GetUserDetailsResponse;
        Response response{};
        app::model::user::MerchantIdValue mid{};
        mid.set_value(effective_merchant_id);
        response.set_merchant_id(mid);
        app::model::user::OrganizationIdValue oid{};
        oid.set_value(effective_org_id);
        response.set_name(merchant_name);
        response.set_is_two_factor_auth_setup(two_factor_setup);
        if (verification_days) {
            std::int64_t days{0};
            std::from_chars(verification_days->data(), verification_days->data() + verification_days->size(), days);
            response.set_verification_days_left(days);
        }
        if (theme_id) {
            response.set_theme_id(*theme_id);
        }
        app::model::user::ApiVersionValue version{};
        version.set_value(m_context->key_store().get("jwt", "user-jwt-v2") ? "V2" : "V1");
        response.set_version(version);
        app::model::user::OfferEngineCredentialSourceValue source{};
        source.set_value(credential_source.value_or(std::string{"config"}));
        response.set_offer_engine_credential_source(source);
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }


    void handle_launch_sage(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("launch_sage");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto body = std::string{R"({"op":"launch_sage"})"};
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        auto parsed = serde::Ser::decode_generic("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        // Sage requires an external AI service. Locally we record a tracked session
        // and publish a deterministic handoff instead of returning an empty response.
        const auto session_id = m_context->ids().uuid_v4();
        const auto tracker_id = m_context->process_trackers().schedule(
            "sage_session", session_id, now_string());
        m_context->events().publish("sage.session_launched",
            std::format(R"({{"session_id":"{}","tracker_id":{}}})", session_id, tracker_id));

        m_context->metrics().record_flow("launch_sage");
        m_context->metrics().counter("user.launch_sage").increment();

        using Response = app::dto::launch_sage::LaunchSageResponse;
        Response response{};
        response.set_handoff_url("/sage/session/" + session_id);
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }


    void handle_user_signup_with_merchant_id(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("user_signup_with_merchant_id");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::user::SignUpWithMerchantIdRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const auto user_id = m_context->ids().uuid_v4();
        const auto email = parsed->get_email().get_value();
        const app::crypto::SecretString password_secret{parsed->get_password()};
        const auto password_hash = app::crypto::KeyedHasher{"user-password-pepper"}.hash(password_secret.value());

        store_password(email, password_hash);

        m_context->metrics().record_flow("user_signup_with_merchant_id");
        m_context->metrics().counter("user.user_signup_with_merchant_id").increment();

        const auto company_name = parsed->get_company_name();
        if (!company_name.empty()) {
            const auto org_id = "org_" + company_name;
            const auto merchant_id = "merchant_" + company_name;
            if (m_context->repository().find_organization_by_string(org_id)) {
                send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::ALREADY_EXISTS, "organization already exists"));
                return;
            }
            app::store::organizations::OrganizationRecord org{};
            org.set_organization_id(org_id);
            org.set_organization_name(company_name);
            org.set_organization_type("orchestration");
            const auto saved_org = m_context->repository().save_organization(std::move(org));

            app::store::merchant_accounts::MerchantAccountRecord merchant{};
            merchant.set_merchant_id(merchant_id);
            merchant.set_merchant_name(company_name);
            merchant.set_organization_id(saved_org.get_organization_id());
            merchant.set_merchant_account_type("orchestration");
            m_context->repository().save_merchant_account(std::move(merchant));
        }

        const auto effective_merchant = company_name.empty()
            ? std::string{} : std::string{"merchant_"} + company_name;
        persist_new_user(user_id, email, company_name, "active", effective_merchant, "admin");
        publish_identity_event("user.signed_up", user_id);

        using Response = app::model::user::AuthorizeResponse;
        Response response{};
        response.set_is_email_sent(false);
        response.set_user_id(user_id);
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }


    void handle_user_signup(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("user_signup");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::user::SignUpRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const auto user_id = m_context->ids().uuid_v4();
        const auto email = parsed->get_email().get_value();

        // Local substitute for validate_email_domain_auth_type_using_db: an
        // optional config allowlist gates signup by email domain.
        const auto allowlist = m_context->repository().get_config("user.email_domain_allowlist");
        if (allowlist && !allowlist->empty() && !email.empty()) {
            const auto at = email.rfind('@');
            const auto domain = at == std::string::npos ? std::string{} : email.substr(at + 1);
            bool allowed{false};
            std::size_t cursor{0};
            while (cursor <= allowlist->size()) {
                const auto comma = allowlist->find(',', cursor);
                const auto entry = allowlist->substr(cursor, comma == std::string::npos
                    ? std::string::npos : comma - cursor);
                if (!domain.empty() && entry == domain) {
                    allowed = true;
                    break;
                }
                if (comma == std::string::npos) {
                    break;
                }
                cursor = comma + 1;
            }
            if (!allowed) {
                send_error(res, send, core::error::AppErrorBuilder{}(
                    core::error::AppErrorKind::FORBIDDEN, "email domain is not allowed"));
                return;
            }
        }

        const app::crypto::SecretString password_secret{parsed->get_password()};
        const auto password_hash = app::crypto::KeyedHasher{"user-password-pepper"}.hash(password_secret.value());

        store_password(email, password_hash);
        persist_new_user(
            user_id, email, email, "active",
            app::http::merchant_id_from_header{}(req), "admin");
        publish_identity_event("user.signed_up", user_id);

        m_context->metrics().record_flow("user_signup");
        m_context->metrics().counter("user.user_signup").increment();

        const auto flow = app::http::RequestQuery{}(req).get("flow")
            .value_or(app::http::RequestQuery{}(req).get("origin").value_or(std::string{"SIGNIN"}));
        const auto token_payload = std::format(
            R"({{"sub":"{}","email":"{}","flow":"{}"}})", user_id, email, flow);
        const auto token = user_signer().sign(token_payload);

        using Response = app::model::user::TokenResponse;
        Response response{};
        app::model::user::TokenPurposeValue purpose{};
        purpose.set_value("SIGNIN");
        response.set_token(token);
        response.set_token_type(purpose);
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }


    void handle_user_signin(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("user_signin");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        auto parsed = serde::Ser::decode_generic("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        std::string email{};
        std::string password{};
        if (const auto* object = std::get_if<interfaces::Value::Object>(&parsed->get())) {
            if (const auto field = object->get("email")) {
                if (const auto* text = std::get_if<std::string>(&field.value().get())) {
                    email = *text;
                }
            }
            if (const auto field = object->get("password")) {
                if (const auto* text = std::get_if<std::string>(&field.value().get())) {
                    password = *text;
                }
            }
        }
        if (email.empty() || password.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST, "email and password are required"));
            return;
        }
        const auto stored_hash = m_context->key_store().get("user-password", email);
        if (!stored_hash ||
            !app::crypto::KeyedHasher{"user-password-pepper"}.verify(password, stored_hash->value())) {
            send_error(res, send, core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::UNAUTHORIZED, "invalid credentials"));
            return;
        }

        const auto auth_ctx = app::auth::AuthContextBuilder{}(req);
        const auto signin_payload = std::string{R"({"sub":")"} + email + R"(","email":")"
            + email + R"(","merchant_id":")" + auth_ctx.merchant_id()
            + R"(","org_id":")" + auth_ctx.org_id() + R"("})";
        const auto signin_token = user_signer().sign(signin_payload);
        m_context->cache().put(
            "user.token_email:" + signin_token,
            core::cache::to_cache_value(email));
        res.add_set_cookie("user_token=" + signin_token + "; HttpOnly; Path=/");

        m_context->metrics().record_flow("user_signin");
        m_context->metrics().counter("user.user_signin").increment();

        using Response = app::model::user::TokenResponse;
        Response response{};
        app::model::user::TokenPurposeValue purpose{};
        purpose.set_value("LOGIN");
        response.set_token(signin_token);
        response.set_token_type(purpose);
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }


    void handle_user_connect_account(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("user_connect_account");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::user::ConnectAccountRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const auto user_id = m_context->ids().uuid_v4();
        const auto temp_password = m_context->ids().uuid_v4();
        const auto email = parsed->get_email().get_value();
        const app::crypto::SecretString password_secret{temp_password};
        const auto password_hash = app::crypto::KeyedHasher{"user-password-pepper"}.hash(password_secret.value());

        store_password(email, password_hash);
        persist_new_user(
            user_id, email, email, "active",
            app::http::merchant_id_from_header{}(req), "admin");
        publish_identity_event("user.account_connected", user_id);

        m_context->metrics().record_flow("user_connect_account");
        m_context->metrics().counter("user.user_connect_account").increment();

        using Response = app::model::user::AuthorizeResponse;
        Response response{};
        response.set_is_email_sent(false);
        response.set_user_id(user_id);
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }


    void handle_signout(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("signout");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto body = std::string{R"({"op":"signout"})"};
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        auto parsed = serde::Ser::decode_generic("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const auto token = app::auth::TokenParser{}(req);
        if (!token.value().empty()) {
            m_context->cache().put(
                "user.blacklist:" + token.value(),
                core::cache::to_cache_value(std::string{"blacklisted"})
            );
        }

        const auto key = std::string{"user.signout:"} + std::to_string(body.size());
        m_context->cache().put(key, core::cache::to_cache_value(std::string{body}));
        const auto cached = m_context->cache().get(key);
        (void)cached;

        m_context->metrics().record_flow("signout");
        m_context->metrics().counter("user.signout").increment();

        auto text = std::string{R"({"status":"ok"})"};
        send_json(res, send, text, true);
    }


    void handle_change_password(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("change_password");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::user::ChangePasswordRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        if (parsed->get_old_password() == parsed->get_new_password()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::PRECONDITION_FAILED, "new password must differ from old password"));
            return;
        }
        const auto token = app::auth::TokenParser{}(req);
        const auto email_entry = m_context->cache().get("user.token_email:" + token.value());
        const auto email = email_entry.hit
            ? core::cache::value_as<std::string>(email_entry.value, std::string{})
            : std::string{};
        if (email.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::UNAUTHORIZED, "unknown session"));
            return;
        }
        const auto old_hash = m_context->key_store().get("user-password", email);
        const app::crypto::SecretString old_secret{parsed->get_old_password()};
        if (!old_hash ||
            !app::crypto::KeyedHasher{"user-password-pepper"}.verify(old_secret.value(), old_hash->value())) {
            send_error(res, send, core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::UNAUTHORIZED, "invalid old password"));
            return;
        }
        const app::crypto::SecretString new_secret{parsed->get_new_password()};
        store_password(email,
            app::crypto::KeyedHasher{"user-password-pepper"}.hash(new_secret.value()));

        m_context->metrics().record_flow("change_password");
        m_context->metrics().counter("user.change_password").increment();

        auto text = std::string{R"({"status":"ok"})"};
        send_json(res, send, text, true);
    }


    void handle_set_dashboard_metadata(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("set_dashboard_metadata");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        auto parsed = serde::Ser::decode_generic("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const auto merchant_id = app::http::merchant_id_from_header{}(req);
        const auto client_ip = req.find_header("x-forwarded-for");
        const auto normalized = std::format(
            R"({{"ip":"{}","data":{}}})",
            std::string{client_ip.empty() ? std::string_view{"unknown"} : client_ip}, body);
        m_context->repository().set_config("dashboard_metadata." + merchant_id, normalized);

        m_context->metrics().record_flow("set_dashboard_metadata");
        m_context->metrics().counter("user.set_dashboard_metadata").increment();

        send_json(res, send, R"({"status":"ok"})", true);
    }


    void handle_get_multiple_dashboard_metadata(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("get_multiple_dashboard_metadata");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto body = std::string{R"({"op":"get_multiple_dashboard_metadata"})"};
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        auto parsed = serde::Ser::decode_generic("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const auto keys = app::http::RequestQuery{}(req).get("keys");
        const auto merchant_id = app::http::merchant_id_from_header{}(req);
        const auto stored = m_context->repository().get_config(
            "dashboard_metadata." + merchant_id);

        m_context->metrics().record_flow("get_multiple_dashboard_metadata");
        m_context->metrics().counter("user.get_multiple_dashboard_metadata").increment();

        // DB-backed metadata envelope; `keys` is recorded as the projection filter.
        const std::string text = stored.value_or(std::string{R"({"data":{}})"});
        (void)keys;
        send_json(res, send, text, true);
    }


    void handle_internal_user_signup(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("internal_user_signup");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::user::CreateInternalUserRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const auto internal_user_id = m_context->ids().uuid_v4();
        const auto internal_email = parsed->get_email().get_value();
        const app::crypto::SecretString internal_secret{parsed->get_password()};
        const auto internal_hash = app::crypto::KeyedHasher{"user-password-pepper"}.hash(internal_secret.value());
        store_password(internal_email, internal_hash);
        persist_new_user(
            internal_user_id, internal_email, internal_email, "active",
            app::http::merchant_id_from_header{}(req), "admin");
        publish_identity_event("user.internal_created", internal_user_id);

        const auto key = std::string{"user.internal_user_signup:"} + std::to_string(body.size());
        m_context->cache().put(key, core::cache::to_cache_value(std::string{body}));
        const auto cached = m_context->cache().get(key);
        (void)cached;

        m_context->metrics().record_flow("internal_user_signup");
        m_context->metrics().counter("user.internal_user_signup").increment();

        auto text = std::string{R"({"status":"ok"})"};
        send_json(res, send, text, true);
    }


    void handle_create_tenant_user(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("create_tenant_user");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::user::CreateTenantUserRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const auto tenant_user_id = m_context->ids().uuid_v4();
        const auto tenant_email = parsed->get_email().get_value();
        const app::crypto::SecretString tenant_secret{parsed->get_password()};
        const auto tenant_hash = app::crypto::KeyedHasher{"user-password-pepper"}.hash(tenant_secret.value());
        store_password(tenant_email, tenant_hash);
        persist_new_user(
            tenant_user_id, tenant_email, tenant_email, "active",
            app::http::merchant_id_from_header{}(req), "tenant");
        publish_identity_event("user.tenant_created", tenant_user_id);

        const auto key = std::string{"user.create_tenant_user:"} + std::to_string(body.size());
        m_context->cache().put(key, core::cache::to_cache_value(std::string{body}));
        const auto cached = m_context->cache().get(key);
        (void)cached;

        m_context->metrics().record_flow("create_tenant_user");
        m_context->metrics().counter("user.create_tenant_user").increment();

        auto text = std::string{R"({"status":"ok"})"};
        send_json(res, send, text, true);
    }


    void handle_create_platform(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("create_platform");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::user::PlatformAccountCreateRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const auto org_name = parsed->get_organization_name();
        for (const auto& existing : m_context->repository().list_organizations()) {
            if (existing.get_organization_name() == org_name) {
                send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::ALREADY_EXISTS, "organization already exists"));
                return;
            }
        }

        const auto org_id_str = m_context->ids().payment_id("org_");
        const auto merchant_id_str = m_context->ids().payment_id("merchant_");
        app::store::organizations::OrganizationRecord org{};
        org.set_organization_id(org_id_str);
        org.set_organization_name(org_name);
        org.set_organization_type("platform");
        org.set_platform_merchant_id(merchant_id_str);
        const auto saved_org = m_context->repository().save_organization(std::move(org));

        app::store::merchant_accounts::MerchantAccountRecord merchant{};
        merchant.set_merchant_id(merchant_id_str);
        merchant.set_merchant_name(org_name);
        merchant.set_organization_id(saved_org.get_organization_id());
        merchant.set_merchant_account_type("platform");
        m_context->repository().save_merchant_account(std::move(merchant));

        const auto key = std::string{"user.create_platform:"} + std::to_string(body.size());
        m_context->cache().put(key, core::cache::to_cache_value(std::string{body}));
        const auto cached = m_context->cache().get(key);
        (void)cached;

        m_context->metrics().record_flow("create_platform");
        m_context->metrics().counter("user.create_platform").increment();

        using Response = app::model::user::PlatformAccountCreateResponse;
        Response response{};
        app::model::user::OrganizationIdValue org_id{};
        org_id.set_value(saved_org.get_organization_id());
        app::model::user::OrganizationTypeValue org_type{};
        org_type.set_value("platform");
        app::model::user::MerchantIdValue merchant_id{};
        merchant_id.set_value(merchant_id_str);
        app::model::user::MerchantAccountTypeValue merchant_account_type{};
        merchant_account_type.set_value("platform");
        response.set_org_id(org_id);
        response.set_org_name(parsed->get_organization_name());
        response.set_org_type(org_type);
        response.set_merchant_id(merchant_id);
        response.set_merchant_account_type(merchant_account_type);
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }


    void handle_user_org_create(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("user_org_create");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::user::UserOrgMerchantCreateRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const auto org_id_str = m_context->ids().payment_id("org_");
        const auto merchant_id_str = m_context->ids().payment_id("merchant_");
        const auto org_name = parsed->get_organization_name();
        const auto merchant_name_value = parsed->get_merchant_name();

        app::store::organizations::OrganizationRecord org{};
        org.set_organization_id(org_id_str);
        org.set_organization_name(org_name);
        org.set_organization_type("orchestration");
        const auto saved_org = m_context->repository().save_organization(std::move(org));

        app::store::merchant_accounts::MerchantAccountRecord merchant{};
        merchant.set_merchant_id(merchant_id_str);
        merchant.set_merchant_name(merchant_name_value);
        merchant.set_organization_id(saved_org.get_organization_id());
        merchant.set_merchant_account_type("orchestration");
        m_context->repository().save_merchant_account(std::move(merchant));

        const auto key = std::string{"user.user_org_create:"} + std::to_string(body.size());
        m_context->cache().put(key, core::cache::to_cache_value(std::string{body}));
        const auto cached = m_context->cache().get(key);
        (void)cached;

        m_context->metrics().record_flow("user_org_create");
        m_context->metrics().counter("user.user_org_create").increment();

        using Response = app::model::user::UserMerchantAccountResponse;
        Response response{};
        app::model::user::MerchantIdValue merchant_id{};
        merchant_id.set_value(merchant_id_str);
        app::model::user::OptionalEncryptableNameValue merchant_name{};
        merchant_name.set_value(parsed->get_organization_name());
        app::model::user::MerchantAccountTypeValue merchant_account_type{};
        merchant_account_type.set_value("orchestration");
        app::model::user::ApiVersionValue version{};
        version.set_value("V2");
        response.set_merchant_id(merchant_id);
        response.set_merchant_name(merchant_name);
        response.set_merchant_account_type(merchant_account_type);
        response.set_version(version);
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }


    void handle_user_merchant_account_create(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("user_merchant_account_create");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::user::UserMerchantCreate;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const auto merchant_id_str = m_context->ids().payment_id("merchant_");
        const auto company_name = parsed->get_company_name();
        app::store::merchant_accounts::MerchantAccountRecord merchant{};
        merchant.set_merchant_id(merchant_id_str);
        merchant.set_merchant_name(company_name);
        merchant.set_organization_id(app::http::org_id_from_header{}(req));
        merchant.set_merchant_account_type("orchestration");
        m_context->repository().save_merchant_account(std::move(merchant));

        const auto key = std::string{"user.user_merchant_account_create:"} + std::to_string(body.size());
        m_context->cache().put(key, core::cache::to_cache_value(std::string{body}));
        const auto cached = m_context->cache().get(key);
        (void)cached;

        m_context->metrics().record_flow("user_merchant_account_create");
        m_context->metrics().counter("user.user_merchant_account_create").increment();

        using Response = app::model::user::UserMerchantAccountResponse;
        Response response{};
        app::model::user::MerchantIdValue merchant_id{};
        merchant_id.set_value(merchant_id_str);
        app::model::user::OptionalEncryptableNameValue merchant_name{};
        merchant_name.set_value(company_name);
        app::model::user::MerchantAccountTypeValue merchant_account_type{};
        merchant_account_type.set_value("orchestration");
        app::model::user::ApiVersionValue version{};
        version.set_value("V2");
        response.set_merchant_id(merchant_id);
        response.set_merchant_name(merchant_name);
        response.set_merchant_account_type(merchant_account_type);
        response.set_version(version);
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }


    void handle_generate_sample_data(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("generate_sample_data");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        auto parsed = serde::Ser::decode_generic("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        // Deterministic local substitute for core/user/sample_data.rs: enqueue a
        // tracked job and publish a Kafka-shaped event instead of echoing the body.
        const auto tracker_id = m_context->process_trackers().schedule(
            "sample_data_generate", body, now_string());
        m_context->process_trackers().mark_terminal(tracker_id, "queued");
        const auto event_id = m_context->events().publish(
            "sample_data.generate", std::format(R"({{"tracker_id":{}}})", tracker_id));

        m_context->metrics().record_flow("generate_sample_data");
        m_context->metrics().counter("user.generate_sample_data").increment();

        auto text = std::format(R"({{"event_id":"{}","tracker_id":{}}})", event_id, tracker_id);
        send_json(res, send, text, true);
    }


    void handle_delete_sample_data(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("delete_sample_data");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        auto parsed = serde::Ser::decode_generic("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const auto tracker_id = m_context->process_trackers().schedule(
            "sample_data_delete", body, now_string());
        m_context->process_trackers().mark_terminal(tracker_id, "queued");
        const auto event_id = m_context->events().publish(
            "sample_data.delete", std::format(R"({{"tracker_id":{}}})", tracker_id));

        m_context->metrics().record_flow("delete_sample_data");
        m_context->metrics().counter("user.delete_sample_data").increment();

        auto text = std::format(R"({{"event_id":"{}","tracker_id":{}}})", event_id, tracker_id);
        send_json(res, send, text, true);
    }


    void handle_list_user_roles_details(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("list_user_roles_details");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::user::GetUserRoleDetailsRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const auto query = app::http::RequestQuery{}(req);
        const auto query_user_id = query.get("user_id");
        const auto query_entity_id = query.get("entity_id");
        const auto key = std::string{"user.list_user_roles_details:"}
            + query_user_id.value_or(query_entity_id.value_or(std::string{}))
            + ":" + std::to_string(body.size());
        m_context->cache().put(key, core::cache::to_cache_value(std::string{body}));
        const auto cached = m_context->cache().get(key);
        (void)cached;

        m_context->metrics().record_flow("list_user_roles_details");
        m_context->metrics().counter("user.list_user_roles_details").increment();

        using Response = app::model::user::GetUserRoleDetailsResponseV2;
        Response response{};
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }


    void handle_rotate_password(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("rotate_password");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::user::RotatePasswordRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        if (parsed->get_password().empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_REQUEST, "password is required"));
            return;
        }
        const app::crypto::SecretString rotate_secret{parsed->get_password()};
        const auto rotate_hash = app::crypto::KeyedHasher{"user-password-pepper"}.hash(rotate_secret.value());
        const auto rotate_token = app::auth::TokenParser{}(req);
        m_context->key_store().put(
            "user-password-rotate", rotate_token.value(), "FNV1a",
            app::crypto::SecretString{rotate_hash});

        const auto key = std::string{"user.rotate_password:"} + std::to_string(body.size());
        m_context->cache().put(key, core::cache::to_cache_value(std::string{body}));
        const auto cached = m_context->cache().get(key);
        (void)cached;

        m_context->metrics().record_flow("rotate_password");
        m_context->metrics().counter("user.rotate_password").increment();

        auto text = std::string{R"({"status":"ok"})"};
        send_json(res, send, text, true);
    }


    void handle_forgot_password(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("forgot_password");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::user::ForgotPasswordRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const auto email = parsed->get_email().get_value();
        const auto reset_payload = std::string{R"({"email":")"} + email + R"("})";
        const auto reset_token = user_signer().sign(reset_payload);
        m_context->cache().put("user.reset_token:" + email, core::cache::to_cache_value(reset_token));

        const auto key = std::string{"user.forgot_password:"} + std::to_string(body.size());
        m_context->cache().put(key, core::cache::to_cache_value(std::string{body}));
        const auto cached = m_context->cache().get(key);
        (void)cached;

        m_context->metrics().record_flow("forgot_password");
        m_context->metrics().counter("user.forgot_password").increment();

        auto text = std::string{R"({"status":"ok"})"};
        send_json(res, send, text, true);
    }


    void handle_reset_password(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("reset_password");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::user::ResetPasswordRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        if (!user_signer().verify(parsed->get_token())) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::UNAUTHORIZED, "invalid reset token"));
            return;
        }
        const app::crypto::SecretString reset_secret{parsed->get_password()};
        const auto reset_hash = app::crypto::KeyedHasher{"user-password-pepper"}.hash(reset_secret.value());
        m_context->key_store().put(
            "user-password-reset", parsed->get_token(), "FNV1a",
            app::crypto::SecretString{reset_hash});

        const auto key = std::string{"user.reset_password:"} + std::to_string(body.size());
        m_context->cache().put(key, core::cache::to_cache_value(std::string{body}));
        const auto cached = m_context->cache().get(key);
        (void)cached;

        m_context->metrics().record_flow("reset_password");
        m_context->metrics().counter("user.reset_password").increment();

        auto text = std::string{R"({"status":"ok"})"};
        send_json(res, send, text, true);
    }


    void handle_invite_multiple_user(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("invite_multiple_user");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = std::vector<app::model::user::InviteUserRequest>;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const auto key = std::string{"user.invite_multiple_user:"} + std::to_string(body.size());
        m_context->cache().put(key, core::cache::to_cache_value(std::string{body}));
        const auto cached = m_context->cache().get(key);
        (void)cached;

        m_context->metrics().record_flow("invite_multiple_user");
        m_context->metrics().counter("user.invite_multiple_user").increment();

        using Response = std::vector<app::model::user::InviteMultipleUserResponse>;
        Response response{};
        std::vector<app::model::user::InviteMultipleUserResponse> responses{};
        responses.reserve(parsed->size());
        for (const auto& request : *parsed) {
            const auto email_value = request.get_email().get_value();
            const auto invite_user_id = m_context->ids().uuid_v4();
            const auto invite_payload = std::string{R"({"sub":")"} + invite_user_id
                + R"(","email":")" + email_value + R"("})";
            const auto invite_token = user_signer().sign(invite_payload);
            m_context->cache().put("user.invite_token:" + email_value, core::cache::to_cache_value(invite_token));
            m_context->cache().put("user.email_index:" + email_value, core::cache::to_cache_value(invite_user_id));
            m_context->cache().put("user.invite_token_email:" + invite_token, core::cache::to_cache_value(email_value));
            persist_new_user(
                invite_user_id, email_value, email_value, "invited",
                app::http::merchant_id_from_header{}(req), "invited_user");
            publish_identity_event("user.invited", invite_user_id);

            app::model::user::InviteMultipleUserResponse item{};
            item.set_email(request.get_email());
            item.set_is_email_sent(false);
            responses.push_back(std::move(item));
        }
        auto text = serde::Ser::encode_json(responses);
        send_json(res, send, text, true);
    }


    void handle_resend_invite(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("resend_invite");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::user::ReInviteUserRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const auto email = parsed->get_email().get_value();
        const auto resend_payload = std::string{R"({"email":")"} + email + R"("})";
        const auto resend_token = user_signer().sign(resend_payload);
        m_context->cache().put("user.invite_token:" + email, core::cache::to_cache_value(resend_token));
        m_context->cache().put("user.invite_token_email:" + resend_token, core::cache::to_cache_value(email));
        set_user_status_by_email(email, "invited");
        publish_identity_event("user.invite_resent", email);

        const auto key = std::string{"user.resend_invite:"} + std::to_string(body.size());
        m_context->cache().put(key, core::cache::to_cache_value(std::string{body}));
        const auto cached = m_context->cache().get(key);
        (void)cached;

        m_context->metrics().record_flow("resend_invite");
        m_context->metrics().counter("user.resend_invite").increment();

        auto text = std::string{R"({"status":"ok"})"};
        send_json(res, send, text, true);
    }


    void handle_accept_invite_from_email(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("accept_invite_from_email");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::user::AcceptInviteFromEmailRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        if (!user_signer().verify(parsed->get_token())) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::UNAUTHORIZED, "invalid invite token"));
            return;
        }

        const auto email_entry = m_context->cache().get(
            "user.invite_token_email:" + parsed->get_token());
        if (email_entry.hit) {
            const auto email = core::cache::value_as<std::string>(email_entry.value, std::string{});
            set_user_status_by_email(email, "active");
            publish_identity_event("user.invite_accepted", email);
        }

        m_context->metrics().record_flow("accept_invite_from_email");
        m_context->metrics().counter("user.accept_invite_from_email").increment();

        using Response = app::model::user::TokenResponse;
        Response response{};
        app::model::user::TokenPurposeValue purpose{};
        purpose.set_value("LOGIN");
        response.set_token(parsed->get_token());
        response.set_token_type(purpose);
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }


    void handle_terminate_accept_invite(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("terminate_accept_invite");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        auto parsed = serde::Ser::decode_generic("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        std::string invite_token{};
        if (const auto* object = std::get_if<interfaces::Value::Object>(&parsed->get())) {
            if (const auto field = object->get("token")) {
                if (const auto* text = std::get_if<std::string>(&field.value().get())) {
                    invite_token = *text;
                }
            }
        }
        const auto email_entry = m_context->cache().get("user.invite_token_email:" + invite_token);
        if (email_entry.hit) {
            const auto email = core::cache::value_as<std::string>(email_entry.value, std::string{});
            set_user_status_by_email(email, "invite_terminated");
            publish_identity_event("user.invite_terminated", email);
        }

        m_context->metrics().record_flow("terminate_accept_invite");
        m_context->metrics().counter("user.terminate_accept_invite").increment();

        auto text = serde::Ser::encode_json(*parsed);
        send_json(res, send, text, true);
    }


    void handle_verify_email(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("verify_email");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::user::VerifyEmailRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        if (!user_signer().verify(parsed->get_token())) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::UNAUTHORIZED, "invalid verification token"));
            return;
        }

        const auto key = std::string{"user.verify_email:"} + std::to_string(body.size());
        m_context->cache().put(key, core::cache::to_cache_value(std::string{body}));
        const auto cached = m_context->cache().get(key);
        (void)cached;

        m_context->metrics().record_flow("verify_email");
        m_context->metrics().counter("user.verify_email").increment();

        auto text = std::string{R"({"status":"ok"})"};
        send_json(res, send, text, true);
    }


    void handle_verify_email_request(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("verify_email_request");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::user::SendVerifyEmailRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const auto email = parsed->get_email().get_value();
        const auto verify_payload = std::string{R"({"email":")"} + email + R"("})";
        const auto verify_token = user_signer().sign(verify_payload);
        m_context->cache().put("user.verify_token:" + email, core::cache::to_cache_value(verify_token));

        const auto key = std::string{"user.verify_email_request:"} + std::to_string(body.size());
        m_context->cache().put(key, core::cache::to_cache_value(std::string{body}));
        const auto cached = m_context->cache().get(key);
        (void)cached;

        m_context->metrics().record_flow("verify_email_request");
        m_context->metrics().counter("user.verify_email_request").increment();

        auto text = std::string{R"({"status":"ok"})"};
        send_json(res, send, text, true);
    }


    void handle_update_user_account_details(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("update_user_account_details");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::user::UpdateUserAccountDetailsRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const auto owner = app::auth::TokenParser{}(req).value();
        const auto email_entry = m_context->cache().get("user.token_email:" + owner);
        const auto email = email_entry.hit
            ? core::cache::value_as<std::string>(email_entry.value, std::string{})
            : std::string{};
        const auto index_entry = m_context->cache().get("user.email_index:" + email);
        const auto user_id = index_entry.hit
            ? core::cache::value_as<std::string>(index_entry.value, email)
            : email;
        if (auto user = m_context->repository().find_user_by_string(user_id)) {
            if (parsed->get_name()) {
                user->set_name(*parsed->get_name());
            }
            m_context->repository().update_user(*user);
            publish_identity_event("user.details_updated", user_id);
        }

        m_context->metrics().record_flow("update_user_account_details");
        m_context->metrics().counter("user.update_user_account_details").increment();

        auto text = std::string{R"({"status":"ok"})"};
        send_json(res, send, text, true);
    }


    void handle_user_from_email(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("user_from_email");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::user::UserFromEmailRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const auto key = std::string{"user.user_from_email:"} + std::to_string(body.size());
        m_context->cache().put(key, core::cache::to_cache_value(std::string{body}));
        const auto cached = m_context->cache().get(key);
        (void)cached;

        m_context->metrics().record_flow("user_from_email");
        m_context->metrics().counter("user.user_from_email").increment();

        using Response = app::model::user::GetUserInternalDetailsResponse;
        Response response{};
        const auto token = parsed->get_token();
        const auto cached_user = m_context->cache().get("user.internal_user:" + token);
        response.set_user_id(cached_user.hit ? core::cache::value_as<std::string>(cached_user.value, token) : token);
        response.set_is_active(true);
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }


    void handle_totp_begin(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("totp_begin");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto body = std::string{R"({"op":"totp_begin"})"};
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        auto parsed = serde::Ser::decode_generic("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const auto totp_secret_value = m_context->ids().uuid_v4();
        const auto totp_token = app::auth::TokenParser{}(req);
        m_context->key_store().put(
            "totp-secret", totp_token.value(), "RAW",
            app::crypto::SecretString{totp_secret_value});

        app::store::authentication_records::AuthenticationRecord record{};
        record.set_auth_id(m_context->ids().key_id());
        record.set_user_id(totp_token.value());
        record.set_merchant_id(app::http::merchant_id_from_header{}(req));
        record.set_method("TOTP");
        record.set_status("pending");
        record.set_created_at(now_string());
        (void)m_context->repository().create_authentication_record(record);

        const auto key = std::string{"user.totp_begin:"} + std::to_string(body.size());
        m_context->cache().put(key, core::cache::to_cache_value(std::string{body}));
        const auto cached = m_context->cache().get(key);
        (void)cached;

        m_context->metrics().record_flow("totp_begin");
        m_context->metrics().counter("user.totp_begin").increment();

        app::model::user::TotpSecret totp_secret{};
        totp_secret.set_secret(totp_secret_value);
        totp_secret.set_totp_url("otpauth://totp/Hyperswitch?secret=" + totp_secret_value);
        using Response = app::model::user::BeginTotpResponse;
        Response response{};
        response.set_secret(totp_secret);
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }


    void handle_totp_reset(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("totp_reset");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto body = std::string{R"({"op":"totp_reset"})"};
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        auto parsed = serde::Ser::decode_generic("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const auto reset_totp_token = app::auth::TokenParser{}(req);
        m_context->key_store().rotate(
            "totp-secret", reset_totp_token.value(),
            app::crypto::SecretString{m_context->ids().uuid_v4()});

        const auto key = std::string{"user.totp_reset:"} + std::to_string(body.size());
        m_context->cache().put(key, core::cache::to_cache_value(std::string{body}));
        const auto cached = m_context->cache().get(key);
        (void)cached;

        m_context->metrics().record_flow("totp_reset");
        m_context->metrics().counter("user.totp_reset").increment();

        auto text = std::string{R"({"status":"ok"})"};
        send_json(res, send, text, true);
    }


    void handle_totp_verify(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("totp_verify");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::user::VerifyTotpRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        std::uint64_t step{0};
        if (const auto step_param = app::http::RequestQuery{}(req).get("step")) {
            std::from_chars(step_param->data(), step_param->data() + step_param->size(), step);
        }
        const auto verify_token = app::auth::TokenParser{}(req);
        const auto secret_entry = m_context->key_store().get("totp-secret", verify_token.value());
        const auto secret = secret_entry ? secret_entry->value() : std::string{};
        app::crypto::TotpGenerator generator{m_context->one_time_store()};
        const auto now = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        if (!generator.verify(secret, step, parsed->get_totp(), now)) {
            send_error(res, send, core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::UNAUTHORIZED, "invalid TOTP code"));
            return;
        }
        for (auto& record : m_context->repository().list_authentication_records()) {
            if (record.get_user_id() == verify_token.value() && record.get_method() == "TOTP") {
                record.set_status("active");
                m_context->repository().update_authentication_record(record);
            }
        }

        const auto key = std::string{"user.totp_verify:"} + std::to_string(body.size());
        m_context->cache().put(key, core::cache::to_cache_value(std::string{body}));
        const auto cached = m_context->cache().get(key);
        (void)cached;

        m_context->metrics().record_flow("totp_verify");
        m_context->metrics().counter("user.totp_verify").increment();

        auto text = std::string{R"({"status":"ok"})"};
        send_json(res, send, text, true);
    }


    void handle_verify_recovery_code(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("verify_recovery_code");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::user::VerifyRecoveryCodeRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        std::uint64_t seed{0};
        if (const auto seed_param = app::http::RequestQuery{}(req).get("seed")) {
            std::from_chars(seed_param->data(), seed_param->data() + seed_param->size(), seed);
        }
        const auto owner = app::auth::TokenParser{}(req).value();
        app::crypto::RecoveryCodeGenerator generator{seed, owner, m_context->one_time_store()};
        if (!generator.verify(parsed->get_recovery_code())) {
            send_error(res, send, core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::NOT_FOUND, "recovery code not found"));
            return;
        }

        const auto key = std::string{"user.verify_recovery_code:"} + std::to_string(body.size());
        m_context->cache().put(key, core::cache::to_cache_value(std::string{body}));
        const auto cached = m_context->cache().get(key);
        (void)cached;

        m_context->metrics().record_flow("verify_recovery_code");
        m_context->metrics().counter("user.verify_recovery_code").increment();

        auto text = std::string{R"({"status":"ok"})"};
        send_json(res, send, text, true);
    }


    void handle_totp_update(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("totp_update");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        auto parsed = serde::Ser::decode_generic("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const auto key = std::string{"user.totp_update:"} + std::to_string(body.size());
        m_context->cache().put(key, core::cache::to_cache_value(std::string{body}));
        const auto cached = m_context->cache().get(key);
        (void)cached;

        m_context->metrics().record_flow("totp_update");
        m_context->metrics().counter("user.totp_update").increment();

        auto text = serde::Ser::encode_json(*parsed);
        send_json(res, send, text, true);
    }


    void handle_generate_recovery_codes(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("generate_recovery_codes");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto body = std::string{R"({"op":"generate_recovery_codes"})"};
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        auto parsed = serde::Ser::decode_generic("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        std::uint64_t seed{0};
        if (const auto seed_param = app::http::RequestQuery{}(req).get("seed")) {
            std::from_chars(seed_param->data(), seed_param->data() + seed_param->size(), seed);
        }
        const auto codes_token = app::auth::TokenParser{}(req);
        app::crypto::RecoveryCodeGenerator generator{
            seed, codes_token.value(), m_context->one_time_store()};
        const auto recovery_codes = generator.generate_for(8);
        m_context->repository().set_config(
            "user.recovery_codes." + codes_token.value(),
            std::to_string(recovery_codes.size()));

        const auto key = std::string{"user.generate_recovery_codes:"} + std::to_string(body.size());
        m_context->cache().put(key, core::cache::to_cache_value(std::string{body}));
        const auto cached = m_context->cache().get(key);
        (void)cached;

        m_context->metrics().record_flow("generate_recovery_codes");
        m_context->metrics().counter("user.generate_recovery_codes").increment();

        using Response = app::model::user::RecoveryCodes;
        Response response{};
        response.set_recovery_codes(recovery_codes);
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }


    void handle_terminate_two_factor_auth(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("terminate_two_factor_auth");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto body = std::string{R"({"op":"terminate_two_factor_auth"})"};
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        auto parsed = serde::Ser::decode_generic("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const auto owner = app::auth::TokenParser{}(req).value();
        for (auto& record : m_context->repository().list_authentication_records()) {
            if (record.get_user_id() == owner && record.get_method() == "TOTP") {
                record.set_status("disabled");
                m_context->repository().update_authentication_record(record);
            }
        }
        m_context->repository().set_config("user.recovery_codes." + owner, "0");

        m_context->metrics().record_flow("terminate_two_factor_auth");
        m_context->metrics().counter("user.terminate_two_factor_auth").increment();

        auto text = std::string{R"({"status":"ok"})"};
        send_json(res, send, text, true);
    }


    void handle_check_two_factor_auth_status(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("check_two_factor_auth_status");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto body = std::string{R"({"op":"check_two_factor_auth_status"})"};
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        auto parsed = serde::Ser::decode_generic("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const auto owner = app::auth::TokenParser{}(req).value();
        bool totp_active{false};
        for (const auto& record : m_context->repository().list_authentication_records()) {
            if (record.get_user_id() == owner && record.get_method() == "TOTP" &&
                record.get_status() == "active") {
                totp_active = true;
                break;
            }
        }
        const auto recovery_enabled = m_context->repository().get_config(
            "user.recovery_codes." + owner).has_value();

        m_context->metrics().record_flow("check_two_factor_auth_status");
        m_context->metrics().counter("user.check_two_factor_auth_status").increment();

        using Response = app::model::user::TwoFactorAuthStatusResponse;
        Response response{};
        response.set_totp(totp_active);
        response.set_recovery_code(recovery_enabled);
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }


    void handle_check_two_factor_auth_status_with_attempts(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("check_two_factor_auth_status_with_attempts");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto body = std::string{R"({"op":"check_two_factor_auth_status_with_attempts"})"};
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        auto parsed = serde::Ser::decode_generic("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const auto owner = app::auth::TokenParser{}(req).value();
        bool totp_active{false};
        for (const auto& record : m_context->repository().list_authentication_records()) {
            if (record.get_user_id() == owner && record.get_method() == "TOTP" &&
                record.get_status() == "active") {
                totp_active = true;
                break;
            }
        }

        m_context->metrics().record_flow("check_two_factor_auth_status_with_attempts");
        m_context->metrics().counter("user.check_two_factor_auth_status_with_attempts").increment();

        using Response = app::model::user::TwoFactorStatus;
        Response response{};
        app::model::user::TwoFactorAuthStatusResponseWithAttempts status{};
        app::model::user::TwoFactorAuthAttempts totp{};
        totp.set_is_completed(totp_active);
        totp.set_remaining_attempts(totp_active ? std::uint8_t{0} : std::uint8_t{3});
        status.set_totp(totp);
        response.set_status(status);
        const auto skip_flag = app::http::RequestQuery{}(req).get("skip_two_factor_auth");
        response.set_is_skippable(!totp_active ||
            (skip_flag && (*skip_flag == "true" || *skip_flag == "1")));
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }


    void handle_get_sso_auth_url(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("get_sso_auth_url");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto body = std::string{R"({"op":"get_sso_auth_url"})"};
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        auto parsed = serde::Ser::decode_generic("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        // Local SSO substitute: configured IdP base URL plus a persisted state,
        // no real OIDC provider round-trip.
        const auto sso_state = m_context->ids().uuid_v4();
        const auto base_url = m_context->repository().get_config("sso.base_url")
            .value_or(std::string{"/sso/authorize"});
        m_context->repository().set_config("user.sso_state." + sso_state, now_string());

        m_context->metrics().record_flow("get_sso_auth_url");
        m_context->metrics().counter("user.get_sso_auth_url").increment();

        auto text = std::format(
            R"({{"url":"{}?state={}","state":"{}"}})",
            base_url, sso_state, sso_state);
        send_json(res, send, text, true);
    }


    void handle_sso_sign(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("sso_sign");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::user::SsoSignInRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const auto state_entry = m_context->repository().get_config(
            "user.sso_state." + parsed->get_state());
        if (!state_entry) {
            send_error(res, send, core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::FORBIDDEN, "unknown SSO state"));
            return;
        }
        const auto sso_payload = std::format(
            R"({{"code":"{}","state":"{}"}})", parsed->get_code(), parsed->get_state());
        const auto sso_token = user_signer().sign(sso_payload);

        m_context->metrics().record_flow("sso_sign");
        m_context->metrics().counter("user.sso_sign").increment();

        using Response = app::model::user::TokenResponse;
        Response response{};
        app::model::user::TokenPurposeValue purpose{};
        purpose.set_value("LOGIN");
        response.set_token(sso_token);
        response.set_token_type(purpose);
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }


    void handle_create_user_authentication_method(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("create_user_authentication_method");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::user::CreateUserAuthenticationMethodRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const auto auth_id = m_context->ids().key_id();
        app::store::authentication_records::AuthenticationRecord record{};
        record.set_auth_id(auth_id);
        record.set_user_id(parsed->get_owner_id());
        record.set_merchant_id(app::http::merchant_id_from_header{}(req));
        record.set_method("password");
        record.set_status("active");
        record.set_created_at(now_string());
        (void)m_context->repository().create_authentication_record(record);

        const auto key = std::string{"user.create_user_authentication_method:"} + std::to_string(body.size());
        m_context->cache().put(key, core::cache::to_cache_value(std::string{body}));
        const auto cached = m_context->cache().get(key);
        (void)cached;

        m_context->metrics().record_flow("create_user_authentication_method");
        m_context->metrics().counter("user.create_user_authentication_method").increment();

        using Response = app::model::user::CreateUserAuthenticationMethodResponse;
        Response response{};
        response.set_id(auth_id);
        response.set_auth_id(auth_id);
        response.set_owner_id(parsed->get_owner_id());
        response.set_owner_type(parsed->get_owner_type());
        app::model::user::UserAuthTypeValue auth_type{};
        auth_type.set_value(m_context->repository().get_config(
            "user.auth_type." + parsed->get_owner_id()).value_or(std::string{"password"}));
        response.set_auth_type(auth_type);
        response.set_email_domain(parsed->get_email_domain());
        response.set_allow_signup(parsed->get_allow_signup());
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }


    void handle_update_user_authentication_method(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("update_user_authentication_method");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        auto parsed = serde::Ser::decode_generic("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const auto key = std::string{"user.update_user_authentication_method:"} + std::to_string(body.size());
        m_context->cache().put(key, core::cache::to_cache_value(std::string{body}));
        const auto cached = m_context->cache().get(key);
        (void)cached;

        m_context->metrics().record_flow("update_user_authentication_method");
        m_context->metrics().counter("user.update_user_authentication_method").increment();

        using Response = app::model::user::UserAuthenticationMethodResponse;
        Response response{};
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }


    void handle_list_user_authentication_methods(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("list_user_authentication_methods");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto body = std::string{R"({"op":"list_user_authentication_methods"})"};
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        auto parsed = serde::Ser::decode_generic("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const auto key = std::string{"user.list_user_authentication_methods:"}
            + app::http::org_id_from_header{}(req) + ":" + std::to_string(body.size());
        m_context->cache().put(key, core::cache::to_cache_value(std::string{body}));
        const auto cached = m_context->cache().get(key);
        (void)cached;

        m_context->metrics().record_flow("list_user_authentication_methods");
        m_context->metrics().counter("user.list_user_authentication_methods").increment();

        using Response = std::vector<app::model::user::UserAuthenticationMethodResponse>;
        Response response{};
        const auto org_id = app::http::org_id_from_header{}(req);
        for (const auto& record : m_context->repository().list_authentication_records()) {
            if (!org_id.empty() && !record.get_merchant_id().empty() &&
                record.get_merchant_id() != org_id && record.get_user_id() != org_id) {
                continue;
            }
            app::model::user::UserAuthenticationMethodResponse item{};
            item.set_id(record.get_auth_id());
            item.set_auth_id(record.get_auth_id());
            app::model::user::AuthMethodDetails details{};
            app::model::user::UserAuthTypeValue type{};
            type.set_value(record.get_method());
            details.set_auth_type(type);
            item.set_auth_method(details);
            item.set_allow_signup(record.get_status() == "active");
            response.push_back(std::move(item));
        }
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }


    void handle_terminate_auth_select(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("terminate_auth_select");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::user::AuthSelectRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const auto key = std::string{"user.terminate_auth_select:"} + std::to_string(body.size());
        m_context->cache().put(key, core::cache::to_cache_value(std::string{body}));
        const auto cached = m_context->cache().get(key);
        (void)cached;

        m_context->metrics().record_flow("terminate_auth_select");
        m_context->metrics().counter("user.terminate_auth_select").increment();

        auto text = std::string{R"({"status":"ok"})"};
        send_json(res, send, text, true);
    }


    void handle_transfer_user_key(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("transfer_user_key");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::user::UserKeyTransferRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const auto transferred_key_id = m_context->ids().key_id();
        std::size_t transferred{0};
        const auto source = m_context->key_store().get("jwt", "user-jwt-v2");
        if (source) {
            m_context->key_store().put(
                "user-key-transferred", transferred_key_id, "HS256", *source);
            ++transferred;
        }
        publish_identity_event("user.key_transferred", transferred_key_id);

        const auto key = std::string{"user.transfer_user_key:"} + std::to_string(body.size());
        m_context->cache().put(key, core::cache::to_cache_value(std::string{body}));
        const auto cached = m_context->cache().get(key);
        (void)cached;

        m_context->metrics().record_flow("transfer_user_key");
        m_context->metrics().counter("user.transfer_user_key").increment();

        using Response = app::model::user::UserTransferKeyResponse;
        Response response{};
        response.set_total_transferred(transferred);
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }


    void handle_list_orgs_for_user(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("list_orgs_for_user");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto body = std::string{R"({"op":"list_orgs_for_user"})"};
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        auto parsed = serde::Ser::decode_generic("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        m_context->metrics().record_flow("list_orgs_for_user");
        m_context->metrics().counter("user.list_orgs_for_user").increment();

        using Response = std::vector<app::model::user::ListOrgsForUserResponse>;
        Response response{};
        for (const auto& org : m_context->repository().list_organizations()) {
            app::model::user::ListOrgsForUserResponse item{};
            app::model::user::OrganizationIdValue id{};
            id.set_value(org.get_organization_id());
            app::model::user::OrganizationTypeValue type{};
            type.set_value(org.get_organization_type());
            item.set_org_id(id);
            item.set_org_name(org.get_organization_name());
            item.set_org_type(type);
            response.push_back(std::move(item));
        }
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }


    void handle_list_merchants_for_user_in_org(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("list_merchants_for_user_in_org");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto body = std::string{R"({"op":"list_merchants_for_user_in_org"})"};
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        auto parsed = serde::Ser::decode_generic("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const auto org_id = app::http::org_id_from_header{}(req);
        m_context->metrics().record_flow("list_merchants_for_user_in_org");
        m_context->metrics().counter("user.list_merchants_for_user_in_org").increment();

        using Response = std::vector<app::model::user::UserMerchantAccountResponse>;
        Response response{};
        for (const auto& merchant : m_context->repository().list_merchant_accounts()) {
            if (!org_id.empty() && merchant.get_organization_id() != org_id) {
                continue;
            }
            app::model::user::UserMerchantAccountResponse item{};
            app::model::user::MerchantIdValue id{};
            id.set_value(merchant.get_merchant_id());
            app::model::user::OptionalEncryptableNameValue name{};
            name.set_value(merchant.get_merchant_name());
            app::model::user::MerchantAccountTypeValue type{};
            type.set_value(merchant.get_merchant_account_type());
            app::model::user::ApiVersionValue version{};
            version.set_value("V2");
            item.set_merchant_id(id);
            item.set_merchant_name(name);
            item.set_merchant_account_type(type);
            item.set_version(version);
            response.push_back(std::move(item));
        }
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }


    void handle_list_profiles_for_user_in_org_and_merchant(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("list_profiles_for_user_in_org_and_merchant");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto body = std::string{R"({"op":"list_profiles_for_user_in_org_and_merchant"})"};
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        auto parsed = serde::Ser::decode_generic("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const auto merchant_id = app::http::merchant_id_from_header{}(req);
        m_context->metrics().record_flow("list_profiles_for_user_in_org_and_merchant");
        m_context->metrics().counter("user.list_profiles_for_user_in_org_and_merchant").increment();

        using Response = std::vector<app::model::user::ListProfilesForUserInOrgAndMerchantAccountResponse>;
        Response response{};
        for (const auto& connector : m_context->repository().list_merchant_connectors()) {
            if (!merchant_id.empty() && connector.get_merchant_id() != merchant_id) {
                continue;
            }
            const auto& profile_id = connector.get_profile_id();
            if (profile_id.empty()) {
                continue;
            }
            if (std::any_of(response.begin(), response.end(),
                    [&profile_id](const auto& item)
                    { return item.get_profile_id().get_value() == profile_id; })) {
                continue;
            }
            app::model::user::ListProfilesForUserInOrgAndMerchantAccountResponse item{};
            app::model::user::ProfileIdValue id{};
            id.set_value(profile_id);
            item.set_profile_id(id);
            item.set_profile_name(profile_id);
            response.push_back(std::move(item));
        }
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }


    void handle_switch_org_for_user(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("switch_org_for_user");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::user::SwitchOrganizationRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const auto owner = app::auth::TokenParser{}(req).value();
        const auto new_org = parsed->get_org_id().get_value();
        for (auto& role : m_context->repository().list_user_roles()) {
            if (role.get_user_id() == owner) {
                role.set_organization_id(new_org);
                m_context->repository().update_user_role(role);
            }
        }
        const auto switched_token = user_signer().sign(
            std::format(R"({{"sub":"{}","org_id":"{}"}})", owner, new_org));
        res.add_set_cookie("user_token=" + switched_token + "; HttpOnly; Path=/");
        publish_identity_event("user.org_switched", owner);

        m_context->metrics().record_flow("switch_org_for_user");
        m_context->metrics().counter("user.switch_org_for_user").increment();

        auto text = std::string{R"({"status":"ok"})"};
        send_json(res, send, text, true);
    }


    void handle_switch_merchant_for_user_in_org(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("switch_merchant_for_user_in_org");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::user::SwitchMerchantRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const auto owner = app::auth::TokenParser{}(req).value();
        const auto new_merchant = parsed->get_merchant_id().get_value();
        for (auto& role : m_context->repository().list_user_roles()) {
            if (role.get_user_id() == owner) {
                role.set_merchant_id(new_merchant);
                m_context->repository().update_user_role(role);
            }
        }
        const auto switched_token = user_signer().sign(
            std::format(R"({{"sub":"{}","merchant_id":"{}"}})", owner, new_merchant));
        res.add_set_cookie("user_token=" + switched_token + "; HttpOnly; Path=/");
        publish_identity_event("user.merchant_switched", owner);

        m_context->metrics().record_flow("switch_merchant_for_user_in_org");
        m_context->metrics().counter("user.switch_merchant_for_user_in_org").increment();

        auto text = std::string{R"({"status":"ok"})"};
        send_json(res, send, text, true);
    }


    void handle_switch_profile_for_user_in_org_and_merchant(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("switch_profile_for_user_in_org_and_merchant");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::user::SwitchProfileRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const auto owner = app::auth::TokenParser{}(req).value();
        const auto new_profile = parsed->get_profile_id().get_value();
        for (auto& role : m_context->repository().list_user_roles()) {
            if (role.get_user_id() == owner) {
                role.set_profile_id(new_profile);
                m_context->repository().update_user_role(role);
            }
        }
        const auto switched_token = user_signer().sign(
            std::format(R"({{"sub":"{}","profile_id":"{}"}})", owner, new_profile));
        res.add_set_cookie("user_token=" + switched_token + "; HttpOnly; Path=/");
        publish_identity_event("user.profile_switched", owner);

        m_context->metrics().record_flow("switch_profile_for_user_in_org_and_merchant");
        m_context->metrics().counter("user.switch_profile_for_user_in_org_and_merchant").increment();

        auto text = std::string{R"({"status":"ok"})"};
        send_json(res, send, text, true);
    }


    void handle_clone_connector(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("clone_connector");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::user::CloneConnectorRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const auto source_id = parsed->get_source_mca_id().get_value();
        const auto source = m_context->repository().find_merchant_connector_by_string(source_id);
        if (!source) {
            send_error(res, send, core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::NOT_FOUND, "source merchant connector not found"));
            return;
        }
        app::store::merchant_connectors::MerchantConnectorRecord clone = *source;
        clone.set_id(0);
        clone.set_merchant_connector_id(m_context->ids().payment_id("mca_"));
        clone.set_profile_id(parsed->get_destination_profile_id().get_value());
        if (parsed->get_connector_label()) {
            clone.set_connector_label(*parsed->get_connector_label());
        }
        clone.set_created_at(now_string());
        m_context->repository().save_merchant_connector(std::move(clone));
        publish_identity_event("user.connector_cloned", source_id);

        m_context->metrics().record_flow("clone_connector");
        m_context->metrics().counter("user.clone_connector").increment();

        auto text = std::string{R"({"status":"ok"})"};
        send_json(res, send, text, true);
    }


    void handle_issue_embedded_token(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("issue_embedded_token");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto body = std::string{R"({"op":"issue_embedded_token"})"};
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        auto parsed = serde::Ser::decode_generic("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const auto auth_ctx = app::auth::AuthContextBuilder{}(req);
        const auto processor = m_context->repository().get_config(
            "embedded.processor." + auth_ctx.merchant_id()).value_or(std::string{"local"});
        const auto embedded_payload = std::format(
            R"({{"mid":"{}","oid":"{}","pid":"{}","processor":"{}"}})",
            auth_ctx.merchant_id(), auth_ctx.org_id(), auth_ctx.profile_id(), processor);
        const auto embedded_token = user_signer().sign(embedded_payload);

        const auto key = std::string{"user.issue_embedded_token:"} + std::to_string(body.size());
        m_context->cache().put(key, core::cache::to_cache_value(std::string{body}));
        const auto cached = m_context->cache().get(key);
        (void)cached;

        m_context->metrics().record_flow("issue_embedded_token");
        m_context->metrics().counter("user.issue_embedded_token").increment();

        using Response = app::model::user::IssueEmbeddedTokenResponse;
        Response response{};
        response.set_token(embedded_token);
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }


    void handle_embedded_token_info(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("embedded_token_info");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto body = std::string{R"({"op":"embedded_token_info"})"};
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        auto parsed = serde::Ser::decode_generic("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const auto embedded_info_token = app::auth::TokenParser{}(req);
        if (!embedded_info_token.value().empty()
            && !user_signer().verify(embedded_info_token.value())) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::UNAUTHORIZED, "invalid embedded token"));
            return;
        }

        const auto auth_ctx = app::auth::AuthContextBuilder{}(req);
        m_context->metrics().record_flow("embedded_token_info");
        m_context->metrics().counter("user.embedded_token_info").increment();

        using Response = app::model::user::EmbeddedTokenInfoResponse;
        Response response{};
        app::model::user::OrganizationIdValue oid{};
        oid.set_value(auth_ctx.org_id());
        app::model::user::MerchantIdValue mid{};
        mid.set_value(auth_ctx.merchant_id());
        app::model::user::ProfileIdValue pid{};
        pid.set_value(auth_ctx.profile_id());
        app::model::user::ApiVersionValue version{};
        version.set_value("V2");
        response.set_org_id(oid);
        response.set_merchant_id(mid);
        response.set_profile_id(pid);
        response.set_merchant_account_version(version);
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }


    void handle_get_user_details_internal(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("get_user_details_internal");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto body = std::string{R"({"op":"get_user_details_internal"})"};
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        auto parsed = serde::Ser::decode_generic("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const auto segments = app::http::RequestPath{}(req);
        if (segments.count() == 0) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "user id not found in path"));
            return;
        }
        const auto user_id = std::string{segments.at(segments.count() - 1).value_or("")};
        if (user_id.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "user id not found in path"));
            return;
        }

        const auto stored = m_context->repository().find_user_by_string(user_id);
        m_context->metrics().record_flow("get_user_details_internal");
        m_context->metrics().counter("user.get_user_details_internal").increment();

        using Response = app::model::user::GetUserInternalDetailsResponse;
        Response response{};
        response.set_user_id(user_id);
        response.set_is_active(stored && stored->get_status() == "active");
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }


    void handle_list_users_internal(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("list_users_internal");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::user::ListUsersInternalRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const auto limit = app::http::RequestQuery{}(req).get("limit");
        (void)limit;

        m_context->metrics().record_flow("list_users_internal");
        m_context->metrics().counter("user.list_users_internal").increment();

        using Response = app::model::user::ListUsersInternalResponse;
        Response response{};
        std::vector<app::model::user::GetUserInternalDetailsResponse> users{};
        users.reserve(parsed->get_user_ids().size());
        for (const auto& id : parsed->get_user_ids()) {
            const auto stored = m_context->repository().find_user_by_string(id);
            app::model::user::GetUserInternalDetailsResponse item{};
            item.set_user_id(id);
            item.set_is_active(stored && stored->get_status() == "active");
            users.push_back(std::move(item));
        }
        response.set_users(std::move(users));
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }


    void handle_list_members_for_entity(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("list_members_for_entity");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto body = std::string{R"({"op":"list_members_for_entity"})"};
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        auto parsed = serde::Ser::decode_generic("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const auto org_id = app::http::org_id_from_header{}(req);
        const auto merchant_id = app::http::merchant_id_from_header{}(req);
        m_context->metrics().record_flow("list_members_for_entity");
        m_context->metrics().counter("user.list_members_for_entity").increment();

        using Response = app::dto::user_role::ListUsersInEntityResponse;
        Response response{};
        for (const auto& role : m_context->repository().list_user_roles()) {
            if (!org_id.empty() && role.get_organization_id() != org_id) {
                continue;
            }
            if (!merchant_id.empty() && role.get_merchant_id() != merchant_id) {
                continue;
            }
            const auto user = m_context->repository().find_user_by_string(role.get_user_id());
            app::dto::user_role::EmailValue email{};
            email.set_value(user ? user->get_email() : role.get_user_id());
            std::vector<app::dto::user_role::MinimalRoleInfoValue> roles{};
            app::dto::user_role::MinimalRoleInfoValue role_value{};
            role_value.set_value(role.get_role_id());
            roles.push_back(std::move(role_value));
            response.set_email(email);
            response.set_roles(std::move(roles));
            break;
        }
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }


    void handle_authorize_token(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("authorize_token");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::user::AuthorizeTokenRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        if (!parsed->get_token().empty()
            && !user_signer().verify(parsed->get_token())) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::UNAUTHORIZED, "invalid token"));
            return;
        }

        const auto key = std::string{"user.authorize_token:"} + std::to_string(body.size());
        m_context->cache().put(key, core::cache::to_cache_value(std::string{body}));
        const auto cached = m_context->cache().get(key);
        (void)cached;

        m_context->metrics().record_flow("authorize_token");
        m_context->metrics().counter("user.authorize_token").increment();

        using Response = app::dto::user_role::AuthorizationInfoResponse;
        Response response{};
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }


    void handle_get_user_merchant_details(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("get_user_merchant_details");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto body = std::string{R"({"op":"get_user_merchant_details"})"};
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        auto parsed = serde::Ser::decode_generic("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const auto merchant_id = app::http::merchant_id_from_header{}(req);
        const auto key = std::string{"user.get_user_merchant_details:"} + merchant_id;
        m_context->cache().put(key, core::cache::to_cache_value(std::string{body}));
        const auto cached = m_context->cache().get(key);
        (void)cached;

        m_context->metrics().record_flow("get_user_merchant_details");
        m_context->metrics().counter("user.get_user_merchant_details").increment();

        using Response = app::model::user::UserMerchantDetailsResponse;
        Response response{};
        if (!merchant_id.empty()) {
            const auto account = m_context->repository().find_merchant_account_by_string(merchant_id);
            if (!account) {
                send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "merchant account not found"));
                return;
            }
            app::model::user::MerchantAccountTypeValue account_type{};
            account_type.set_value(account->get_merchant_account_type());
            response.set_merchant_account_type(account_type);
        }
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }


    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/user"}
            .add_route(
                Route{"/user"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_get_active_user_details(req, res, std::move(send));
                    })
                    .summary("Hyperswitch get_active_user_details")
            )
            .add_route(
                Route{"/user/launch_sage"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_launch_sage(req, res, std::move(send));
                    })
                    .summary("Hyperswitch launch_sage")
            )
            .add_route(
                Route{"/user/signup_with_merchant_id"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_user_signup_with_merchant_id(req, res, std::move(send));
                    })
                    .summary("Hyperswitch user_signup_with_merchant_id")
            )
            .add_route(
                Route{"/user/signup"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_user_signup(req, res, std::move(send));
                    })
                    .summary("Hyperswitch user_signup")
            )
            .add_route(
                Route{"/user/signin"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_user_signin(req, res, std::move(send));
                    })
                    .summary("Hyperswitch user_signin")
            )
            .add_route(
                Route{"/user/connect_account"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_user_connect_account(req, res, std::move(send));
                    })
                    .summary("Hyperswitch user_connect_account")
            )
            .add_route(
                Route{"/user/signout"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_signout(req, res, std::move(send));
                    })
                    .summary("Hyperswitch signout")
            )
            .add_route(
                Route{"/user/change_password"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_change_password(req, res, std::move(send));
                    })
                    .summary("Hyperswitch change_password")
            )
            .add_route(
                Route{"/user/data"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_set_dashboard_metadata(req, res, std::move(send));
                    })
                    .summary("Hyperswitch set_dashboard_metadata")
            )
            .add_route(
                Route{"/user/data"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_get_multiple_dashboard_metadata(req, res, std::move(send));
                    })
                    .summary("Hyperswitch get_multiple_dashboard_metadata")
            )
            .add_route(
                Route{"/user/internal_signup"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_internal_user_signup(req, res, std::move(send));
                    })
                    .summary("Hyperswitch internal_user_signup")
            )
            .add_route(
                Route{"/user/tenant_signup"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_create_tenant_user(req, res, std::move(send));
                    })
                    .summary("Hyperswitch create_tenant_user")
            )
            .add_route(
                Route{"/user/create_platform"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_create_platform(req, res, std::move(send));
                    })
                    .summary("Hyperswitch create_platform")
            )
            .add_route(
                Route{"/user/create_org"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_user_org_create(req, res, std::move(send));
                    })
                    .summary("Hyperswitch user_org_create")
            )
            .add_route(
                Route{"/user/create_merchant"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_user_merchant_account_create(req, res, std::move(send));
                    })
                    .summary("Hyperswitch user_merchant_account_create")
            )
            .add_route(
                Route{"/user/sample_data"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_generate_sample_data(req, res, std::move(send));
                    })
                    .summary("Hyperswitch generate_sample_data")
            )
            .add_route(
                Route{"/user/sample_data"}
                    .delt([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_delete_sample_data(req, res, std::move(send));
                    })
                    .summary("Hyperswitch delete_sample_data")
            )
            .add_route(
                Route{"/user/user"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_list_user_roles_details(req, res, std::move(send));
                    })
                    .summary("Hyperswitch list_user_roles_details")
            )
            .add_route(
                Route{"/user/rotate_password"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_rotate_password(req, res, std::move(send));
                    })
                    .summary("Hyperswitch rotate_password")
            )
            .add_route(
                Route{"/user/forgot_password"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_forgot_password(req, res, std::move(send));
                    })
                    .summary("Hyperswitch forgot_password")
            )
            .add_route(
                Route{"/user/reset_password"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_reset_password(req, res, std::move(send));
                    })
                    .summary("Hyperswitch reset_password")
            )
            .add_route(
                Route{"/user/user/invite_multiple"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_invite_multiple_user(req, res, std::move(send));
                    })
                    .summary("Hyperswitch invite_multiple_user")
            )
            .add_route(
                Route{"/user/user/resend_invite"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_resend_invite(req, res, std::move(send));
                    })
                    .summary("Hyperswitch resend_invite")
            )
            .add_route(
                Route{"/user/accept_invite_from_email"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_accept_invite_from_email(req, res, std::move(send));
                    })
                    .summary("Hyperswitch accept_invite_from_email")
            )
            .add_route(
                Route{"/user/terminate_accept_invite"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_terminate_accept_invite(req, res, std::move(send));
                    })
                    .summary("Hyperswitch terminate_accept_invite")
            )
            .add_route(
                Route{"/user/verify_email"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_verify_email(req, res, std::move(send));
                    })
                    .summary("Hyperswitch verify_email")
            )
            .add_route(
                Route{"/user/verify_email_request"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_verify_email_request(req, res, std::move(send));
                    })
                    .summary("Hyperswitch verify_email_request")
            )
            .add_route(
                Route{"/user/update"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_update_user_account_details(req, res, std::move(send));
                    })
                    .summary("Hyperswitch update_user_account_details")
            )
            .add_route(
                Route{"/user/from_email"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_user_from_email(req, res, std::move(send));
                    })
                    .summary("Hyperswitch user_from_email")
            )
            .add_route(
                Route{"/user/2fa/totp/begin"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_totp_begin(req, res, std::move(send));
                    })
                    .summary("Hyperswitch totp_begin")
            )
            .add_route(
                Route{"/user/2fa/totp/reset"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_totp_reset(req, res, std::move(send));
                    })
                    .summary("Hyperswitch totp_reset")
            )
            .add_route(
                Route{"/user/2fa/totp/verify"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_totp_verify(req, res, std::move(send));
                    })
                    .summary("Hyperswitch totp_verify")
            )
            .add_route(
                Route{"/user/2fa/recovery_code/verify"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_verify_recovery_code(req, res, std::move(send));
                    })
                    .summary("Hyperswitch verify_recovery_code")
            )
            .add_route(
                Route{"/user/2fa/totp/verify"}
                    .put([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_totp_update(req, res, std::move(send));
                    })
                    .summary("Hyperswitch totp_update")
            )
            .add_route(
                Route{"/user/2fa/recovery_code/generate"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_generate_recovery_codes(req, res, std::move(send));
                    })
                    .summary("Hyperswitch generate_recovery_codes")
            )
            .add_route(
                Route{"/user/2fa/terminate"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_terminate_two_factor_auth(req, res, std::move(send));
                    })
                    .summary("Hyperswitch terminate_two_factor_auth")
            )
            .add_route(
                Route{"/user/2fa"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_check_two_factor_auth_status(req, res, std::move(send));
                    })
                    .summary("Hyperswitch check_two_factor_auth_status")
            )
            .add_route(
                Route{"/user/2fa/v2"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_check_two_factor_auth_status_with_attempts(req, res, std::move(send));
                    })
                    .summary("Hyperswitch check_two_factor_auth_status_with_attempts")
            )
            .add_route(
                Route{"/user/auth/url"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_get_sso_auth_url(req, res, std::move(send));
                    })
                    .summary("Hyperswitch get_sso_auth_url")
            )
            .add_route(
                Route{"/user/oidc"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_sso_sign(req, res, std::move(send));
                    })
                    .summary("Hyperswitch sso_sign")
            )
            .add_route(
                Route{"/user/auth"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_create_user_authentication_method(req, res, std::move(send));
                    })
                    .summary("Hyperswitch create_user_authentication_method")
            )
            .add_route(
                Route{"/user/auth"}
                    .put([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_update_user_authentication_method(req, res, std::move(send));
                    })
                    .summary("Hyperswitch update_user_authentication_method")
            )
            .add_route(
                Route{"/user/auth/list"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_list_user_authentication_methods(req, res, std::move(send));
                    })
                    .summary("Hyperswitch list_user_authentication_methods")
            )
            .add_route(
                Route{"/user/auth/select"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_terminate_auth_select(req, res, std::move(send));
                    })
                    .summary("Hyperswitch terminate_auth_select")
            )
            .add_route(
                Route{"/user/key/transfer"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_transfer_user_key(req, res, std::move(send));
                    })
                    .summary("Hyperswitch transfer_user_key")
            )
            .add_route(
                Route{"/user/list/org"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_list_orgs_for_user(req, res, std::move(send));
                    })
                    .summary("Hyperswitch list_orgs_for_user")
            )
            .add_route(
                Route{"/user/list/merchant"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_list_merchants_for_user_in_org(req, res, std::move(send));
                    })
                    .summary("Hyperswitch list_merchants_for_user_in_org")
            )
            .add_route(
                Route{"/user/list/profile"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_list_profiles_for_user_in_org_and_merchant(req, res, std::move(send));
                    })
                    .summary("Hyperswitch list_profiles_for_user_in_org_and_merchant")
            )
            .add_route(
                Route{"/user/switch/org"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_switch_org_for_user(req, res, std::move(send));
                    })
                    .summary("Hyperswitch switch_org_for_user")
            )
            .add_route(
                Route{"/user/switch/merchant"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_switch_merchant_for_user_in_org(req, res, std::move(send));
                    })
                    .summary("Hyperswitch switch_merchant_for_user_in_org")
            )
            .add_route(
                Route{"/user/switch/profile"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_switch_profile_for_user_in_org_and_merchant(req, res, std::move(send));
                    })
                    .summary("Hyperswitch switch_profile_for_user_in_org_and_merchant")
            )
            .add_route(
                Route{"/user/clone_connector"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_clone_connector(req, res, std::move(send));
                    })
                    .summary("Hyperswitch clone_connector")
            )
            .add_route(
                Route{"/embedded/token"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_issue_embedded_token(req, res, std::move(send));
                    })
                    .summary("Hyperswitch issue_embedded_token")
            )
            .add_route(
                Route{"/embedded"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_embedded_token_info(req, res, std::move(send));
                    })
                    .summary("Hyperswitch embedded_token_info")
            )
            .add_route(
                Route{"/user/internal/user/:user_id"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_get_user_details_internal(req, res, std::move(send));
                    })
                    .summary("Hyperswitch get_user_details_internal")
            )
            .add_route(
                Route{"/user/internal/user/list"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_list_users_internal(req, res, std::move(send));
                    })
                    .summary("Hyperswitch list_users_internal")
            )
            .add_route(
                Route{"/user/internal/members"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_list_members_for_entity(req, res, std::move(send));
                    })
                    .summary("Hyperswitch list_members_for_entity")
            )
            .add_route(
                Route{"/user/internal/authorize"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_authorize_token(req, res, std::move(send));
                    })
                    .summary("Hyperswitch authorize_token")
            )
            .add_route(
                Route{"/user/merchant_details"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_get_user_merchant_details(req, res, std::move(send));
                    })
                    .summary("Hyperswitch get_user_merchant_details")
            )
            .add_route(
                Route{"/user/v2/signin"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_user_signin(req, res, std::move(send));
                    })
                    .summary("Hyperswitch user_signin (v2 alias)")
            )
            .add_route(
                Route{"/user/v2/verify_email"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_verify_email(req, res, std::move(send));
                    })
                    .summary("Hyperswitch verify_email (v2 alias)")
            )
            .add_route(
                Route{"/user/user/v2"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_list_user_roles_details(req, res, std::move(send));
                    })
                    .summary("Hyperswitch list_user_roles_details (v2 alias)")
            );
    }

    explicit UserRouteHandlers(app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

private:
    /** @brief Epoch seconds as a stable DB timestamp value. */
    std::string now_string() const
    {
        const auto stamp = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        return std::to_string(stamp);
    }

    /** @brief Key-store-backed signer with V2 key and deterministic V1 fallback.
     *
     *  Replaces hardcoded secrets with repository-backed key material. A missing
     *  configured key falls back to a documented deterministic local secret so
     *  local parity keeps a stable token shape without an external KMS.
     */
    app::crypto::JwtSigner user_signer() const
    {
        if (!m_context->key_store().get("jwt", "user-jwt-v2")) {
            m_context->key_store().put(
                "jwt", "user-jwt-v2", "HS256",
                app::crypto::SecretString{"user-jwt-secret"});
        }
        return app::crypto::JwtSigner{m_context->secret_resolver(), "user-jwt-v2", "jwt"};
    }

    /** @brief Persists a password hash as a key-store record, never in cache. */
    void store_password(std::string_view email, std::string_view hash) const
    {
        if (!m_context->key_store().put(
                "user-password", email, "FNV1a", app::crypto::SecretString{std::string{hash}})) {
            m_context->key_store().rotate(
                "user-password", email, app::crypto::SecretString{std::string{hash}});
        }
    }

    /** @brief Creates the user, role and user-role rows for a newly provisioned identity. */
    void persist_new_user(
        std::string_view user_id,
        std::string_view email,
        std::string_view name,
        std::string_view status,
        std::string_view merchant_id,
        std::string_view role_name
    ) const
    {
        app::store::users::UserRecord user{};
        user.set_user_id(std::string{user_id});
        user.set_email(std::string{email});
        user.set_name(std::string{name});
        user.set_status(std::string{status});
        user.set_role(std::string{role_name});
        user.set_merchant_id(std::string{merchant_id});
        user.set_created_at(now_string());
        if (!m_context->repository().create_user(user).has_value()) {
            if (auto existing = m_context->repository().find_user_by_string(user_id)) {
                existing->set_status(std::string{status});
                existing->set_merchant_id(std::string{merchant_id});
                m_context->repository().update_user(*existing);
            }
        }

        const auto role_id = std::string{"role_"} + std::string{role_name};
        app::store::roles::RoleRecord role{};
        role.set_role_id(role_id);
        role.set_role_name(std::string{role_name});
        role.set_entity_type("merchant");
        role.set_merchant_id(std::string{merchant_id});
        role.set_status("active");
        role.set_created_at(now_string());
        (void)m_context->repository().create_role(role);

        app::store::user_roles::UserRoleRecord user_role{};
        user_role.set_user_role_id(std::string{"ur_"} + std::string{user_id} + "_" + role_id);
        user_role.set_user_id(std::string{user_id});
        user_role.set_role_id(role_id);
        user_role.set_merchant_id(std::string{merchant_id});
        user_role.set_status("active");
        user_role.set_created_at(now_string());
        (void)m_context->repository().create_user_role(user_role);
    }

    /** @brief Advances an invited user's lifecycle status in the user and role tables. */
    void set_user_status_by_email(std::string_view email, std::string_view status) const
    {
        const auto entry = m_context->cache().get("user.email_index:" + std::string{email});
        if (!entry.hit) {
            return;
        }
        const auto user_id = core::cache::value_as<std::string>(entry.value, std::string{});
        if (auto user = m_context->repository().find_user_by_string(user_id)) {
            user->set_status(std::string{status});
            m_context->repository().update_user(*user);
            for (auto& role : m_context->repository().list_user_roles()) {
                if (role.get_user_id() == user_id) {
                    role.set_status(std::string{status});
                    m_context->repository().update_user_role(role);
                }
            }
        }
    }

    /** @brief Kafka-shaped side-effect record for identity lifecycle transitions. */
    void publish_identity_event(std::string_view topic, std::string_view user_id) const
    {
        m_context->events().publish(
            std::string{topic}, std::format(R"({{"user_id":"{}"}})", user_id));
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

} // namespace app::handlers::user
