module;
export module app_handlers:profiles;

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
import app_events;
import app_services;

export namespace app::handlers::profiles {

class ProfilesRouteHandlers
{
public:
    explicit ProfilesRouteHandlers(app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

    void handle_profile_create(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize_operation(req, "profile_create")) {
            send_error(res, send, *auth_error);
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, make_error(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::dto::admin::ProfileCreate;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, make_error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const std::string merchant_id = merchant_from_request(req);
        const std::string org_id = org_from_request(req);
        auto repository = m_context->repository();
        if (!merchant_id.empty()) {
            const auto account = repository.find_merchant_account_by_string(merchant_id);
            // Only enforce the registry when it is populated; an empty directory
            // means no merchant store is available to validate against.
            if (!account && !repository.list_merchant_accounts().empty()) {
                send_error(res, send, make_error(core::error::AppErrorKind::PRECONDITION_FAILED, "merchant account not found"));
                return;
            }
            if (account && !org_id.empty() && !account->get_organization_id().empty()
                && account->get_organization_id() != org_id) {
                send_error(res, send, make_error(core::error::AppErrorKind::FORBIDDEN, "merchant belongs to another organization"));
                return;
            }
        }

        // core/admin.rs:4066-4070 — external-vault configuration guard.
        if (const auto error = validate_external_vault(
                parsed->get_is_external_vault_enabled(),
                parsed->get_external_vault_connector_details().has_value(),
                merchant_id
            )) {
            send_error(res, send, *error);
            return;
        }
        // profiles.rs:22-45 — route-layer request validation.
        if (const auto error = validate_configuration(
                parsed->get_webhook_details(),
                parsed->get_payment_link_config(),
                parsed->get_session_expiry(),
                parsed->get_intent_fulfillment_time()
            )) {
            send_error(res, send, *error);
            return;
        }

        const std::string from_path = profile_id_from_request(req);
        const std::string profile_id = from_path.empty()
            ? m_context->ids().payment_id("profile_")
            : from_path;
        // core/admin.rs:4088-4098 — duplicate insert maps to GenericDuplicateError.
        if (repository.find_profile_by_string(profile_id)) {
            send_error(res, send, make_error(core::error::AppErrorKind::ALREADY_EXISTS, "profile already exists"));
            return;
        }
        std::string profile_name{};
        if (parsed->get_profile_name()) {
            profile_name = *parsed->get_profile_name();
            for (const auto& existing : repository.list_profiles()) {
                if (existing.get_profile_name() == profile_name &&
                    (merchant_id.empty() || existing.get_merchant_id() == merchant_id)) {
                    send_error(res, send, make_error(core::error::AppErrorKind::ALREADY_EXISTS, "profile name already exists"));
                    return;
                }
            }
        }

        auto payment_response_hash_key = parsed->get_payment_response_hash_key();
        if (parsed->get_enable_payment_response_hash().value_or(false) && !payment_response_hash_key) {
            const app::crypto::SecretString secret{m_context->ids().uuid_v4()};
            const app::crypto::KeyedHasher hasher{std::string{secret.value()}};
            payment_response_hash_key = hasher.hash(profile_id);
        }

        app::dto::admin::ProfileResponse response;
        app::dto::admin::ProfileIdValue response_id;
        response_id.set_value(profile_id);
        response.set_profile_id(std::move(response_id));
        app::dto::admin::MerchantIdValue response_merchant;
        response_merchant.set_value(merchant_id);
        response.set_merchant_id(std::move(response_merchant));
        if (parsed->get_profile_name()) {
            response.set_profile_name(*parsed->get_profile_name());
        }
        if (parsed->get_return_url()) {
            response.set_return_url(parsed->get_return_url()->get_value());
        }
        if (parsed->get_enable_payment_response_hash()) {
            response.set_enable_payment_response_hash(*parsed->get_enable_payment_response_hash());
        }
        response.set_payment_response_hash_key(payment_response_hash_key);
        if (parsed->get_redirect_to_merchant_with_http_post()) {
            response.set_redirect_to_merchant_with_http_post(*parsed->get_redirect_to_merchant_with_http_post());
        }
        response.set_metadata(parsed->get_metadata());
        response.set_routing_algorithm(parsed->get_routing_algorithm());
        response.set_frm_routing_algorithm(parsed->get_frm_routing_algorithm());
        response.set_payout_routing_algorithm(parsed->get_payout_routing_algorithm());
        response.set_applepay_verified_domains(parsed->get_applepay_verified_domains());
        response.set_payment_link_config(parsed->get_payment_link_config());
        if (parsed->get_session_expiry()) {
            response.set_session_expiry(std::optional<std::int64_t>{
                static_cast<std::int64_t>(*parsed->get_session_expiry())});
        }
        if (parsed->get_intent_fulfillment_time()) {
            response.set_intent_fulfillment_time(std::optional<std::int64_t>{
                static_cast<std::int64_t>(*parsed->get_intent_fulfillment_time())});
        }
        response.set_tax_connector_id(parsed->get_tax_connector_id());
        response.set_is_tax_connector_enabled(parsed->get_is_tax_connector_enabled());
        response.set_is_network_tokenization_enabled(parsed->get_is_network_tokenization_enabled());
        response.set_is_click_to_pay_enabled(parsed->get_is_click_to_pay_enabled());
        if (parsed->get_is_connector_agnostic_mit_enabled()) {
            response.set_is_connector_agnostic_mit_enabled(*parsed->get_is_connector_agnostic_mit_enabled());
        }

        const auto text = serde::Ser::encode_json(response);
        persist_profile(profile_id, merchant_id, profile_name, text);

        // core/admin.rs:4100-4112 (v1) — unset an existing default profile on create.
        if (!is_v2(req) && !merchant_id.empty()) {
            if (auto account = repository.find_merchant_account_by_string(merchant_id)) {
                if (!account->get_default_profile().empty() && account->get_default_profile() != profile_id) {
                    account->set_default_profile({});
                    (void)repository.update_merchant_account(*account);
                }
            }
        }
        emit_event("create", text);

        m_context->metrics().record_flow("profiles.create");
        m_context->metrics().record_flow("Flow::ProfileCreate");
        m_context->metrics().counter("profiles_create").increment();

        send_json(res, send, text, true);
    }

    void handle_profile_retrieve(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize_operation(req, "profile_retrieve")) {
            send_error(res, send, *auth_error);
            return;
        }

        const std::string profile_id = profile_id_from_request(req);
        if (profile_id.empty()) {
            send_error(res, send, make_error(core::error::AppErrorKind::INVALID_REQUEST, "missing profile id"));
            return;
        }
        const std::string merchant_id = merchant_from_request(req);
        const std::string owner = profile_owner(profile_id);
        // core/admin.rs:4166-4179 — ownership check identical to the DB query.
        if (!merchant_id.empty() && !owner.empty() && owner != merchant_id) {
            send_error(res, send, make_error(core::error::AppErrorKind::NOT_FOUND, "profile not found"));
            return;
        }

        const auto payload = stored_payload(profile_id);
        if (!payload) {
            send_error(res, send, make_error(core::error::AppErrorKind::NOT_FOUND, "profile not found"));
            return;
        }

        m_context->metrics().record_flow("profiles.retrieve");
        m_context->metrics().record_flow("Flow::ProfileRetrieve");
        m_context->metrics().counter("profiles_retrieve").increment();

        send_json(res, send, *payload, true);
    }

    void handle_profile_update(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize_operation(req, "profile_update")) {
            send_error(res, send, *auth_error);
            return;
        }

        const std::string profile_id = profile_id_from_request(req);
        if (profile_id.empty()) {
            send_error(res, send, make_error(core::error::AppErrorKind::INVALID_REQUEST, "missing profile id"));
            return;
        }
        const std::string merchant_id = merchant_from_request(req);
        auto repository = m_context->repository();
        const auto record = repository.find_profile_by_string(profile_id);
        if (!record) {
            send_error(res, send, make_error(core::error::AppErrorKind::NOT_FOUND, "profile not found"));
            return;
        }
        const std::string owner = record->get_merchant_id();
        if (!merchant_id.empty() && !owner.empty() && owner != merchant_id) {
            send_error(res, send, make_error(core::error::AppErrorKind::FORBIDDEN, "profile belongs to another merchant"));
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, make_error(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::dto::admin::ProfileUpdate;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, make_error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        // core/admin.rs:4686-4690 — external-vault configuration guard on update.
        if (const auto error = validate_external_vault(
                parsed->get_is_external_vault_enabled(),
                parsed->get_external_vault_connector_details().has_value(),
                owner
            )) {
            send_error(res, send, *error);
            return;
        }
        // ProfileUpdateBridge (core/admin.rs:4211-4260+) — expiry / destination checks.
        if (const auto error = validate_configuration(
                parsed->get_webhook_details(),
                parsed->get_payment_link_config(),
                parsed->get_session_expiry(),
                parsed->get_intent_fulfillment_time()
            )) {
            send_error(res, send, *error);
            return;
        }
        // core/admin.rs:4700-4725 — decision-engine routing guard.
        if ((parsed->get_routing_algorithm() || parsed->get_payout_routing_algorithm())
            && decision_engine_routing_effective(profile_id)) {
            send_error(res, send, make_error(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "routing_algorithm cannot be set directly for a profile routed by the Decision Engine; use the routing APIs"
            ));
            return;
        }

        app::dto::admin::ProfileResponse response;
        app::dto::admin::ProfileIdValue response_id;
        response_id.set_value(profile_id);
        response.set_profile_id(std::move(response_id));
        app::dto::admin::MerchantIdValue response_merchant;
        response_merchant.set_value(owner.empty() ? merchant_id : owner);
        response.set_merchant_id(std::move(response_merchant));
        if (parsed->get_profile_name()) {
            response.set_profile_name(*parsed->get_profile_name());
        }
        if (parsed->get_return_url()) {
            response.set_return_url(parsed->get_return_url()->get_value());
        }
        if (parsed->get_enable_payment_response_hash()) {
            response.set_enable_payment_response_hash(*parsed->get_enable_payment_response_hash());
        }
        response.set_payment_response_hash_key(parsed->get_payment_response_hash_key());
        if (parsed->get_redirect_to_merchant_with_http_post()) {
            response.set_redirect_to_merchant_with_http_post(*parsed->get_redirect_to_merchant_with_http_post());
        }
        response.set_metadata(parsed->get_metadata());
        response.set_routing_algorithm(parsed->get_routing_algorithm());
        response.set_frm_routing_algorithm(parsed->get_frm_routing_algorithm());
        response.set_payout_routing_algorithm(parsed->get_payout_routing_algorithm());
        response.set_applepay_verified_domains(parsed->get_applepay_verified_domains());
        response.set_extended_card_info_config(parsed->get_extended_card_info_config());
        response.set_payment_link_config(parsed->get_payment_link_config());
        if (parsed->get_session_expiry()) {
            response.set_session_expiry(std::optional<std::int64_t>{
                static_cast<std::int64_t>(*parsed->get_session_expiry())});
        }
        if (parsed->get_intent_fulfillment_time()) {
            response.set_intent_fulfillment_time(std::optional<std::int64_t>{
                static_cast<std::int64_t>(*parsed->get_intent_fulfillment_time())});
        }
        response.set_tax_connector_id(parsed->get_tax_connector_id());
        if (parsed->get_is_connector_agnostic_mit_enabled()) {
            response.set_is_connector_agnostic_mit_enabled(*parsed->get_is_connector_agnostic_mit_enabled());
        }

        const auto text = serde::Ser::encode_json(response);
        persist_profile(profile_id, owner.empty() ? merchant_id : owner,
                        parsed->get_profile_name().value_or(record->get_profile_name()), text);
        // core/admin.rs:4242-4257 — webhook-details merge is persisted as a scoped config row.
        if (parsed->get_webhook_details()) {
            repository.set_config(webhook_config_key(profile_id),
                                  serde::Ser::encode_json(*parsed->get_webhook_details()));
        }
        emit_event("update", text);

        m_context->metrics().record_flow("profiles.update");
        m_context->metrics().record_flow("Flow::ProfileUpdate");
        m_context->metrics().counter("profiles_update").increment();

        send_json(res, send, text, true);
    }

    void handle_profile_delete(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize_operation(req, "profile_delete")) {
            send_error(res, send, *auth_error);
            return;
        }

        const std::string profile_id = profile_id_from_request(req);
        if (profile_id.empty()) {
            send_error(res, send, make_error(core::error::AppErrorKind::INVALID_REQUEST, "missing profile id"));
            return;
        }
        auto repository = m_context->repository();
        const auto record = repository.find_profile_by_string(profile_id);
        if (!record) {
            send_error(res, send, make_error(core::error::AppErrorKind::NOT_FOUND, "profile not found"));
            return;
        }
        const std::string merchant_id = merchant_from_request(req);
        const std::string owner = record->get_merchant_id();
        if (!merchant_id.empty() && !owner.empty() && owner != merchant_id) {
            send_error(res, send, make_error(core::error::AppErrorKind::FORBIDDEN, "profile belongs to another merchant"));
            return;
        }
        forget_profile(profile_id);
        emit_event("delete", profile_id);

        // core/admin.rs:4182-4196 — Json(bool).
        const auto text = std::string{"true"};

        m_context->metrics().record_flow("profiles.delete");
        m_context->metrics().record_flow("Flow::ProfileDelete");
        m_context->metrics().counter("profiles_delete").increment();

        send_json(res, send, text, true);
    }

    void handle_profiles_list(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize_operation(req, "profiles_list")) {
            send_error(res, send, *auth_error);
            return;
        }

        const auto auth = app::auth::AuthContextBuilder{}(req);
        const auto text = profiles_list_json(req, merchant_from_request(req), auth.profile_id());

        m_context->metrics().record_flow("profiles.list");
        m_context->metrics().record_flow("Flow::ProfileList");
        m_context->metrics().counter("profiles_list").increment();

        send_json(res, send, text, true);
    }

    void handle_profiles_list_at_profile_level(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize_operation(req, "profiles_list_at_profile_level")) {
            send_error(res, send, *auth_error);
            return;
        }

        // profiles.rs:456-491 — restrict the result to the authenticated profile.
        const auto auth = app::auth::AuthContextBuilder{}(req);
        const auto text = profiles_list_json(req, merchant_from_request(req), auth.profile_id());

        m_context->metrics().record_flow("profiles.list_at_profile_level");
        m_context->metrics().record_flow("Flow::ProfileList");
        m_context->metrics().counter("profiles_list_at_profile_level").increment();

        send_json(res, send, text, true);
    }

    void handle_toggle_connector_agnostic_mit(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize_operation(req, "toggle_connector_agnostic_mit")) {
            send_error(res, send, *auth_error);
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, make_error(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        // profiles.rs:501 — ConnectorAgnosticMitChoice body and response shape.
        using Input = app::dto::admin::ConnectorAgnosticMitChoice;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, make_error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const std::string profile_id = profile_id_from_request(req, 1);
        if (profile_id.empty()) {
            send_error(res, send, make_error(core::error::AppErrorKind::INVALID_REQUEST, "missing profile id"));
            return;
        }
        auto repository = m_context->repository();
        const auto record = repository.find_profile_by_string(profile_id);
        if (!record) {
            send_error(res, send, make_error(core::error::AppErrorKind::NOT_FOUND, "profile not found"));
            return;
        }
        const std::string merchant_id = merchant_from_request(req);
        const std::string owner = record->get_merchant_id();
        if (!merchant_id.empty() && !owner.empty() && owner != merchant_id) {
            send_error(res, send, make_error(core::error::AppErrorKind::FORBIDDEN, "profile belongs to another merchant"));
            return;
        }
        // core/admin.rs:4950-4992 — processor/merchant ownership guard.
        const auto auth = app::auth::AuthContextBuilder{}(req);
        if (!auth.merchant_id().empty() && !merchant_id.empty() && auth.merchant_id() != merchant_id) {
            send_error(res, send, make_error(core::error::AppErrorKind::FORBIDDEN, "profile belongs to another merchant"));
            return;
        }

        // core/admin.rs:4968-4982 — conditional write.
        const auto key = mit_config_key(profile_id);
        const auto current = repository.get_config(key);
        const std::string desired = parsed->get_enabled() ? "true" : "false";
        if (!current || *current != desired) {
            repository.set_config(key, desired);
            if (const auto payload = stored_payload(profile_id)) {
                emit_event("toggle_connector_agnostic_mit", *payload);
            }
        }

        const auto text = serde::Ser::encode_json(*parsed);

        m_context->metrics().record_flow("profiles.toggle_connector_agnostic_mit");
        m_context->metrics().record_flow("Flow::ConnectorAgnosticMITToggle");
        m_context->metrics().counter("profiles_toggle_connector_agnostic_mit").increment();

        send_json(res, send, text, true);
    }

    void handle_toggle_extended_card_info(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize_operation(req, "toggle_extended_card_info")) {
            send_error(res, send, *auth_error);
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, make_error(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        // profiles.rs:545 — ExtendedCardInfoChoice body and response shape.
        using Input = app::dto::admin::ExtendedCardInfoChoice;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, make_error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const std::string profile_id = profile_id_from_request(req, 1);
        if (profile_id.empty()) {
            send_error(res, send, make_error(core::error::AppErrorKind::INVALID_REQUEST, "missing profile id"));
            return;
        }
        auto repository = m_context->repository();
        const auto record = repository.find_profile_by_string(profile_id);
        if (!record) {
            send_error(res, send, make_error(core::error::AppErrorKind::NOT_FOUND, "profile not found"));
            return;
        }
        const std::string merchant_id = merchant_from_request(req);
        const std::string owner = record->get_merchant_id();
        if (!merchant_id.empty() && !owner.empty() && owner != merchant_id) {
            send_error(res, send, make_error(core::error::AppErrorKind::FORBIDDEN, "profile belongs to another merchant"));
            return;
        }

        // core/admin.rs:4922-4938 — write when is_none() or the value changes.
        const auto key = card_config_key(profile_id);
        const auto current = repository.get_config(key);
        const std::string desired = parsed->get_enabled() ? "true" : "false";
        if (!current.has_value() || *current != desired) {
            repository.set_config(key, desired);
            if (const auto payload = stored_payload(profile_id)) {
                emit_event("toggle_extended_card_info", *payload);
            }
        }

        const auto text = serde::Ser::encode_json(*parsed);

        m_context->metrics().record_flow("profiles.toggle_extended_card_info");
        m_context->metrics().record_flow("Flow::ExtendedCardInfoToggle");
        m_context->metrics().counter("profiles_toggle_extended_card_info").increment();

        send_json(res, send, text, true);
    }

    void handle_payment_connector_list_profile(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto segments = app::http::RequestPath{}(req);
        const bool upstream_route = segments.count() >= 2
            && segments.at(segments.count() - 2).value_or("") == "profile"
            && segments.at(segments.count() - 1).value_or("") == "connectors";
        const auto operation = upstream_route
            ? std::string_view{"connector_list_profile"}
            : std::string_view{"payment_connector_list_profile"};
        if (const auto auth_error = authorize_operation(req, operation)) {
            send_error(res, send, *auth_error);
            return;
        }

        const auto auth = app::auth::AuthContextBuilder{}(req);
        const auto query = app::http::RequestQuery{}(req);
        std::string profile_id = auth.profile_id();
        if (profile_id.empty()) {
            if (const auto value = query.get("profile_id")) {
                profile_id = *value;
            }
        }
        const std::string merchant_id = merchant_from_request(req);

        std::vector<app::dto::admin::MerchantConnectorListResponse> connectors;
        auto repository = m_context->repository();
        for (const auto& record : repository.list_merchant_connectors()) {
            // core/admin.rs:3070-3077 passes get_disabled = true; the store filter is
            // `get_disabled || account.is_enabled()` (storage_impl/merchant_connector_account.rs:748),
            // so disabled rows are intentionally included here.
            if (!profile_id.empty() && record.get_profile_id() != profile_id) {
                continue;
            }
            if (!merchant_id.empty() && record.get_merchant_id() != merchant_id) {
                continue;
            }
            app::dto::admin::MerchantConnectorListResponse item;
            app::dto::admin::ConnectorTypeValue connector_type;
            connector_type.set_value(record.get_connector_type());
            item.set_connector_type(std::move(connector_type));
            item.set_connector_name(record.get_connector_name());
            app::dto::admin::MerchantConnectorAccountIdValue connector_id;
            connector_id.set_value(record.get_merchant_connector_id());
            item.set_merchant_connector_id(std::move(connector_id));
            app::dto::admin::ProfileIdValue item_profile;
            item_profile.set_value(record.get_profile_id());
            item.set_profile_id(std::move(item_profile));
            app::dto::admin::ConnectorStatusValue status;
            status.set_value(record.get_status());
            item.set_status(std::move(status));
            item.set_test_mode(record.get_test_mode());
            item.set_disabled(record.get_disabled());
            connectors.push_back(std::move(item));
        }

        const auto text = serde::Ser::encode_json(connectors);

        m_context->metrics().record_flow("profiles.payment_connector_list_profile");
        m_context->metrics().record_flow("Flow::ConnectorListProfile");
        m_context->metrics().counter("profiles_payment_connector_list_profile").increment();

        send_json(res, send, text, true);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;

        Router{router, "/v2/profiles"}
            .add_route(
                Route{""}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_profile_create(req, res, std::move(send));
                    })
                    .summary("Hyperswitch profile_create")
            )
            .add_route(
                Route{"/:profile_id"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_profile_retrieve(req, res, std::move(send));
                    })
                    .summary("Hyperswitch profile_retrieve")
            )
            .add_route(
                Route{"/:profile_id"}
                    .put([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_profile_update(req, res, std::move(send));
                    })
                    .summary("Hyperswitch profile_update")
            );

        Router{router, "/account/:account_id/business_profile"}
            .add_route(
                Route{""}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_profile_create(req, res, std::move(send));
                    })
                    .summary("Hyperswitch profile_create")
            )
            .add_route(
                Route{""}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_profiles_list(req, res, std::move(send));
                    })
                    .summary("Hyperswitch profiles_list")
            )
            .add_route(
                Route{"/:profile_id"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_profile_retrieve(req, res, std::move(send));
                    })
                    .summary("Hyperswitch profile_retrieve")
            )
            .add_route(
                Route{"/:profile_id"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_profile_update(req, res, std::move(send));
                    })
                    .summary("Hyperswitch profile_update")
            )
            .add_route(
                // delt() is the ApiRoute DELETE binding (core/router/builder.cppm:540-548).
                Route{"/:profile_id"}
                    .delt([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_profile_delete(req, res, std::move(send));
                    })
                    .summary("Hyperswitch profile_delete")
            )
            .add_route(
                Route{"/:profile_id/toggle_extended_card_info"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_toggle_extended_card_info(req, res, std::move(send));
                    })
                    .summary("Hyperswitch toggle_extended_card_info")
            )
            .add_route(
                Route{"/:profile_id/toggle_connector_agnostic_mit"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_toggle_connector_agnostic_mit(req, res, std::move(send));
                    })
                    .summary("Hyperswitch toggle_connector_agnostic_mit")
            );

        Router{router, "/v2/merchant-accounts/:id/profiles"}
            .add_route(
                Route{""}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_profiles_list(req, res, std::move(send));
                    })
                    .summary("Hyperswitch profiles_list")
            );

        Router{router, "/account/:account_id/profile"}
            .add_route(
                Route{""}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_profiles_list_at_profile_level(req, res, std::move(send));
                    })
                    .summary("Hyperswitch profiles_list_at_profile_level")
            );

        Router{router, "/profiles"}
            .add_route(
                Route{"/payment_connector_list_profile"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payment_connector_list_profile(req, res, std::move(send));
                    })
                    .summary("Hyperswitch payment_connector_list_profile")
            );

        // app.rs:2802 — upstream wires /account/{account_id}/profile/connectors
        // to admin::connector_list_profile (profiles.rs:562-597 was unregistered).
        Router{router, "/account/:account_id/profile/connectors"}
            .add_route(
                Route{""}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_payment_connector_list_profile(req, res, std::move(send));
                    })
                    .summary("Hyperswitch connector_list_profile")
            );
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

    [[nodiscard]] core::error::AppError make_error(
        core::error::AppErrorKind kind,
        std::string message
    ) const
    {
        return core::error::AppErrorBuilder{}(kind, std::move(message));
    }

    [[nodiscard]] bool is_v2(interfaces::io::IRequest& req) const
    {
        return app::http::RequestPath{}(req).at(0).value_or("") == "v2";
    }

    [[nodiscard]] std::string path_segment(
        interfaces::io::IRequest& req,
        std::size_t from_end = 0
    ) const
    {
        const app::http::PathSegments segments{app::http::RequestPath{}(req)};
        const std::size_t count{segments.count()};
        if (from_end >= count) {
            return {};
        }
        const auto value = segments.at(count - 1 - from_end);
        return value ? std::string{*value} : std::string{};
    }

    [[nodiscard]] std::string merchant_from_request(interfaces::io::IRequest& req) const
    {
        const app::http::PathSegments segments{app::http::RequestPath{}(req)};
        if (segments.at(0).value_or("") == "v2") {
            if (segments.at(1).value_or("") == "merchant-accounts") {
                return std::string{segments.at(2).value_or("")};
            }
            return app::http::merchant_id_from_header{}(req);
        }
        if (segments.at(0).value_or("") == "profiles") {
            return app::http::merchant_id_from_header{}(req);
        }
        return std::string{segments.at(1).value_or("")};
    }

    [[nodiscard]] std::string org_from_request(interfaces::io::IRequest& req) const
    {
        return app::http::org_id_from_header{}(req);
    }

    [[nodiscard]] std::string profile_id_from_request(
        interfaces::io::IRequest& req,
        std::size_t from_end = 0
    ) const
    {
        const app::http::PathSegments segments{app::http::RequestPath{}(req)};
        const std::size_t count{segments.count()};
        if (from_end == 0 && count >= 3) {
            const auto parent = segments.at(count - 2).value_or("");
            if (parent == "profiles" || parent == "business_profile") {
                return std::string{segments.at(count - 1).value_or("")};
            }
            return {};
        }
        return path_segment(req, from_end);
    }

    [[nodiscard]] std::string payload_config_key(std::string_view profile_id) const
    {
        return std::string{"profiles.payload."} + std::string{profile_id};
    }

    [[nodiscard]] std::string mit_config_key(std::string_view profile_id) const
    {
        return std::string{"profiles.connector_mit."} + std::string{profile_id};
    }

    [[nodiscard]] std::string card_config_key(std::string_view profile_id) const
    {
        return std::string{"profiles.extended_card."} + std::string{profile_id};
    }

    [[nodiscard]] std::string webhook_config_key(std::string_view profile_id) const
    {
        return std::string{"profiles.webhook."} + std::string{profile_id};
    }

    [[nodiscard]] std::optional<std::string> stored_payload(std::string_view profile_id) const
    {
        auto repository = m_context->repository();
        if (const auto value = repository.get_config(payload_config_key(profile_id))) {
            return *value;
        }
        for (const auto& account : repository.list_merchant_accounts()) {
            if (account.get_default_profile() != profile_id) {
                continue;
            }
            app::dto::admin::ProfileResponse response;
            app::dto::admin::ProfileIdValue id;
            id.set_value(std::string{profile_id});
            response.set_profile_id(std::move(id));
            app::dto::admin::MerchantIdValue merchant;
            merchant.set_value(account.get_merchant_id());
            response.set_merchant_id(std::move(merchant));
            response.set_profile_name(account.get_merchant_name());
            return serde::Ser::encode_json(response);
        }
        return std::nullopt;
    }

    [[nodiscard]] std::string profile_owner(std::string_view profile_id) const
    {
        auto repository = m_context->repository();
        if (const auto record = repository.find_profile_by_string(profile_id)) {
            return record->get_merchant_id();
        }
        for (const auto& account : repository.list_merchant_accounts()) {
            if (account.get_default_profile() == profile_id) {
                return account.get_merchant_id();
            }
        }
        return {};
    }

    void persist_profile(
        std::string_view profile_id,
        std::string_view merchant_id,
        std::string_view profile_name,
        std::string_view text
    ) const
    {
        auto repository = m_context->repository();
        repository.set_config(payload_config_key(profile_id), std::string{text});

        auto record = repository.find_profile_by_string(profile_id);
        if (!record) {
            app::store::profiles::ProfileRecord created{};
            created.set_profile_id(std::string{profile_id});
            created.set_merchant_id(std::string{merchant_id});
            created.set_profile_name(std::string{profile_name});
            created.set_type("business");
            created.set_status("active");
            created.set_created_at(now_iso_utc());
            (void)repository.create_profile(std::move(created));
        } else {
            record->set_merchant_id(std::string{merchant_id});
            if (!profile_name.empty()) {
                record->set_profile_name(std::string{profile_name});
            }
            (void)repository.update_profile(*record);
        }
        // Existence marker consumed by profile_acquirer.cppm:267; no business payload in cache.
        m_context->cache().put(
            std::string{"profiles.profile.payload:"} + std::string{profile_id},
            core::cache::to_cache_value(std::string{"1"})
        );
    }

    void forget_profile(std::string_view profile_id) const
    {
        auto repository = m_context->repository();
        if (const auto record = repository.find_profile_by_string(profile_id)) {
            (void)repository.remove_profile(record->get_id());
        }
        for (const auto& key : {
                 payload_config_key(profile_id),
                 mit_config_key(profile_id),
                 card_config_key(profile_id),
                 webhook_config_key(profile_id)
             }) {
            if (const auto config = repository.find_config_by_key(key)) {
                (void)repository.remove_config(config->get_id());
            }
        }
        m_context->cache().erase(std::string{"profiles.profile.payload:"} + std::string{profile_id});
        m_context->cache().erase(std::string{"profiles.profile.owner:"} + std::string{profile_id});
    }

    [[nodiscard]] std::string now_iso_utc() const
    {
        const std::int64_t epoch_seconds = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();
        constexpr std::int64_t seconds_per_day{86400};
        std::int64_t days = epoch_seconds / seconds_per_day;
        std::int64_t seconds_of_day = epoch_seconds % seconds_per_day;
        if (seconds_of_day < 0) {
            seconds_of_day += seconds_per_day;
            --days;
        }
        days += 719468;
        const std::int64_t era = (days >= 0 ? days : days - 146096) / 146097;
        const std::int64_t doe = days - era * 146097;
        const std::int64_t yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
        std::int64_t year = yoe + era * 400;
        const std::int64_t doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
        const std::int64_t mp = (5 * doy + 2) / 153;
        const std::int64_t day = doy - (153 * mp + 2) / 5 + 1;
        const std::int64_t month = mp < 10 ? mp + 3 : mp - 9;
        year += (month <= 2) ? 1 : 0;
        return std::format(
            "{:04d}-{:02d}-{:02d}T{:02d}:{:02d}:{:02d}Z",
            year, month, day,
            seconds_of_day / 3600,
            (seconds_of_day % 3600) / 60,
            seconds_of_day % 60
        );
    }

    [[nodiscard]] std::optional<core::error::AppError> validate_external_vault(
        const std::optional<app::dto::admin::ExternalVaultEnabledValue>& enabled,
        bool has_details,
        std::string_view merchant_id
    ) const
    {
        if (!enabled) {
            return std::nullopt;
        }
        const std::string& value = enabled->get_value();
        const bool flagged = value == "enable" || value == "Enable" || value == "enabled"
                          || value == "Enabled" || value == "true" || value == "1";
        if (!flagged) {
            return std::nullopt;
        }
        if (!has_details) {
            return make_error(core::error::AppErrorKind::VALIDATION_FAILED,
                              "external vault connector details are required");
        }
        // Deterministic substitute: require a configured merchant connector.
        auto repository = m_context->repository();
        for (const auto& connector : repository.list_merchant_connectors()) {
            if (!merchant_id.empty() && connector.get_merchant_id() != merchant_id) {
                continue;
            }
            if (!connector.get_merchant_connector_id().empty()) {
                return std::nullopt;
            }
        }
        return make_error(core::error::AppErrorKind::CONNECTOR_ERROR,
                          "external vault connector is not configured");
    }

    [[nodiscard]] std::optional<core::error::AppError> validate_configuration(
        const std::optional<app::dto::admin::WebhookDetailsRequest>& webhook_details,
        const std::optional<app::dto::admin::BusinessPaymentLinkConfig>& payment_link_config,
        const std::optional<std::uint32_t>& session_expiry,
        const std::optional<std::uint32_t>& intent_fulfillment_time
    ) const
    {
        if (session_expiry && (*session_expiry < 60 || *session_expiry > 7890000)) {
            return make_error(core::error::AppErrorKind::VALIDATION_FAILED,
                              "session_expiry should be between 60(1 min) to 7890000(3 months).");
        }
        if (intent_fulfillment_time && (*intent_fulfillment_time < 60 || *intent_fulfillment_time > 1800)) {
            return make_error(core::error::AppErrorKind::VALIDATION_FAILED,
                              "intent_fulfillment_time should be between 60(1 min) to 1800(30 mins).");
        }
        if (webhook_details && webhook_details->get_webhook_url()) {
            const std::string& url = *webhook_details->get_webhook_url();
            if (!url.starts_with("http://") && !url.starts_with("https://")) {
                return make_error(core::error::AppErrorKind::VALIDATION_FAILED,
                                  "webhook_url is not valid");
            }
        }
        if (payment_link_config) {
            if (payment_link_config->get_domain_name() && payment_link_config->get_domain_name()->empty()) {
                return make_error(core::error::AppErrorKind::VALIDATION_FAILED,
                                  "payment_link_config.domain_name is not valid");
            }
            if (payment_link_config->get_allowed_domains()) {
                for (const auto& domain : *payment_link_config->get_allowed_domains()) {
                    if (domain.empty()) {
                        return make_error(core::error::AppErrorKind::VALIDATION_FAILED,
                                          "payment_link_config.allowed_domains is not valid");
                    }
                }
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] bool decision_engine_routing_effective(std::string_view profile_id) const
    {
        const auto flag = m_context->config_store().get_value("decision_engine", "routing_effective");
        const bool enabled = flag && (*flag == "true" || *flag == "1");
        constexpr std::string_view program =
            R"({"rules":[{"if_condition":{"field":"connector","op":"eq","value":"decision_engine"},"action":"reject"}],"default_action":"allow"})";
        const std::string context_json = std::format(
            R"({{"amount":"0","currency":"USD","payment_method":"card","country":"","connector":"{}"}})",
            enabled ? "decision_engine" : "local"
        );
        const auto decision = m_context->decision_engine().evaluate(program, context_json);
        if (decision.get_action() == "reject") {
            return true;
        }
        auto repository = m_context->repository();
        return repository.find_routing_config_by_algorithm_id_and_profile_id(
            "decision_engine", profile_id
        ).has_value();
    }

    void emit_event(std::string_view operation, std::string_view payload) const
    {
        (void)m_context->events().publish(
            std::string{"profiles."} + std::string{operation}, std::string{payload}
        );
    }

    [[nodiscard]] std::string join_json_array(const std::vector<std::string>& items) const
    {
        std::string out{"["};
        for (std::size_t i{0}; i < items.size(); ++i) {
            if (i != 0) {
                out.push_back(',');
            }
            out += items[i];
        }
        out.push_back(']');
        return out;
    }

    [[nodiscard]] std::string profiles_list_json(
        interfaces::io::IRequest& req,
        std::string_view merchant_id,
        std::string_view auth_profile_filter
    ) const
    {
        const auto query = app::http::RequestQuery{}(req);
        const auto id_filter = query.get("profile_id");
        auto name_filter = query.get("profile_name");
        if (!name_filter) {
            name_filter = query.get("business_profile_name");
        }

        std::vector<std::string> items{};
        auto repository = m_context->repository();
        for (const auto& record : repository.list_profiles()) {
            if (!merchant_id.empty() && record.get_merchant_id() != merchant_id) {
                continue;
            }
            const std::string& id = record.get_profile_id();
            if (!auth_profile_filter.empty() && id != auth_profile_filter) {
                continue;
            }
            if (id_filter && id != *id_filter) {
                continue;
            }
            const auto payload = stored_payload(id);
            if (!payload) {
                continue;
            }
            if (name_filter && payload->find(*name_filter) == std::string::npos) {
                continue;
            }
            items.push_back(*payload);
        }

        if (items.empty()) {
            for (const auto& account : repository.list_merchant_accounts()) {
                if (!merchant_id.empty() && account.get_merchant_id() != merchant_id) {
                    continue;
                }
                const auto& default_profile = account.get_default_profile();
                if (default_profile.empty()) {
                    continue;
                }
                if (!auth_profile_filter.empty() && default_profile != auth_profile_filter) {
                    continue;
                }
                if (id_filter && default_profile != *id_filter) {
                    continue;
                }
                app::dto::admin::ProfileResponse response;
                app::dto::admin::ProfileIdValue id;
                id.set_value(default_profile);
                response.set_profile_id(std::move(id));
                app::dto::admin::MerchantIdValue merchant;
                merchant.set_value(account.get_merchant_id());
                response.set_merchant_id(std::move(merchant));
                response.set_profile_name(account.get_merchant_name());
                items.push_back(serde::Ser::encode_json(response));
            }
        }

        return join_json_array(items);
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

} // namespace app::handlers::profiles
