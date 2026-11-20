module;
export module app_handlers:authentication;

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

export namespace app::handlers::authentication {

class AuthenticationRouteHandlers
{
public:
    void handle_authentication_create(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto authorized = authorize_operation(req, "authentication_create");
        if (authorized.error) {
            send_error(res, send, *authorized.error);
            return;
        }
        const auto flow = auth_flow_of(authorized.context);

        const auto merchant_id = app::http::merchant_id_from_header{}(req);
        if (merchant_id.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::UNAUTHORIZED, "missing merchant id header"));
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::dto::authentication::AuthenticationCreateRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        // Upstream resolves the business profile from the authenticated merchant.
        // The repository-backed profile / merchant records are the local entities.
        std::string resolved_profile_id{};
        if (parsed->get_profile_id().has_value()) {
            resolved_profile_id = parsed->get_profile_id()->get_value();
            if (m_context->repository().find_profile_by_string(resolved_profile_id).has_value()) {
                m_context->metrics().counter("authentication_profile_resolved").increment();
            } else {
                m_context->metrics().counter("authentication_profile_missing").increment();
            }
        } else {
            m_context->metrics().counter("authentication_profile_default").increment();
        }
        const bool merchant_known =
            m_context->repository().find_merchant_account_by_string(merchant_id).has_value();
        if (merchant_known) {
            m_context->metrics().counter("authentication_merchant_resolved").increment();
        }

        const auto authentication_id_value = m_context->ids().payment_id("authn_");
        const auto client_secret_plain = app::crypto::Base64Url{}.encode(
            app::crypto::KeyedHasher{"authentication_client_secret"}.hash(
                authentication_id_value + "_secret"));
        const auto client_secret_hash = app::crypto::KeyedHasher{"authentication_client_secret"}.hash(
            client_secret_plain);
        remember_client_secret(authentication_id_value, client_secret_plain);

        // authentication_create_core: persist the authentication entity.
        persist_authentication(authentication_id_value, merchant_id, "pending", client_secret_hash);

        // perform_authentication: connector orchestration + process tracker.
        const auto tracker_id = schedule_auth_tracker("create", authentication_id_value, flow);
        const auto outcome = call_connector(
            "authorize", merchant_id, authentication_id_value, body, flow, tracker_id);
        publish_auth_event("authentication.created", authentication_id_value, outcome.reference_id);

        m_context->metrics().counter(std::string{"authentication.auth_flow."} + flow).increment();
        m_context->metrics().record_flow("authentication_create");
        m_context->metrics().counter("authentication_create").increment();

        app::dto::authentication::AuthenticationIdValue authentication_id;
        authentication_id.set_value(authentication_id_value);
        app::dto::authentication::MerchantIdValue merchant_id_value;
        merchant_id_value.set_value(merchant_id);
        app::dto::authentication::AuthenticationStatusValue status;
        status.set_value(auth_status_from(outcome.status));
        app::dto::authentication::PrimitiveDateTimeValue created_at;
        created_at.set_value(created_timestamp(authentication_id_value));

        using Response = app::dto::authentication::AuthenticationResponse;
        Response response{};
        response.set_authentication_id(authentication_id);
        response.set_merchant_id(merchant_id_value);
        response.set_status(status);
        response.set_client_secret(std::optional<std::string>{client_secret_plain});
        response.set_amount(parsed->get_amount());
        response.set_currency(parsed->get_currency());
        response.set_authentication_connector(parsed->get_authentication_connector());
        response.set_force_3ds_challenge(parsed->get_force_3ds_challenge());
        response.set_return_url(parsed->get_return_url());
        response.set_created_at(created_at);
        if (!resolved_profile_id.empty()) {
            app::dto::authentication::ProfileIdValue profile_value;
            profile_value.set_value(resolved_profile_id);
            response.set_profile_id(profile_value);
        } else {
            response.set_profile_id(parsed->get_profile_id());
        }
        response.set_psd2_sca_exemption_type(parsed->get_psd2_sca_exemption_type());
        response.set_acquirer_details(parsed->get_acquirer_details());
        response.set_profile_acquirer_id(parsed->get_profile_acquirer_id());
        response.set_customer_details(encrypted_customer_details(parsed->get_customer_details()));
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void handle_authentication_eligibility(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto authorized = authorize_operation(req, "authentication_eligibility");
        if (authorized.error) {
            send_error(res, send, *authorized.error);
            return;
        }
        const auto flow = auth_flow_of(authorized.context);

        const auto path = app::http::RequestPath{}(req);
        const auto id_segment = path.at(1);
        if (!id_segment.has_value()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::INVALID_REQUEST, "authentication id missing in path"));
            return;
        }
        const std::string authentication_id_value{*id_segment};

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::dto::authentication::AuthenticationEligibilityRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        // Upstream check_sdk_auth_and_get_auth overrides the body secret with the
        // credential extracted from the SDK authorization layer.
        parsed->set_client_secret(effective_client_secret(req, body, parsed->get_client_secret()));
        if (const auto secret_error = verify_client_secret(authentication_id_value, parsed->get_client_secret())) {
            send_error(res, send, *secret_error);
            return;
        }

        track_record_fetch("eligibility", authentication_id_value);
        const auto tracker_id = schedule_auth_tracker("eligibility", authentication_id_value, flow);
        const auto outcome = call_connector(
            "verify", merchant_for(req), authentication_id_value, body, flow, tracker_id);
        publish_auth_event("authentication.eligibility", authentication_id_value, outcome.reference_id);

        m_context->metrics().counter(std::string{"authentication.auth_flow."} + flow).increment();
        m_context->metrics().record_flow("authentication_eligibility");
        m_context->metrics().counter("authentication_eligibility").increment();

        app::dto::authentication::AuthenticationIdValue authentication_id;
        authentication_id.set_value(authentication_id_value);
        app::dto::authentication::ProfileIdValue profile_id;
        if (parsed->get_profile_id()) {
            profile_id = *parsed->get_profile_id();
        }
        app::dto::authentication::AuthenticationStatusValue status;
        status.set_value(auth_status_from(outcome.status));
        app::dto::authentication::UrlValue url;
        url.set_value(std::string{"/authentication/"} + authentication_id_value + "/" + outcome.reference_id);
        app::dto::authentication::MethodValue method;
        method.set_value("GET");
        app::dto::authentication::NextAction next_action;
        next_action.set_url(url);
        next_action.set_http_method(method);
        app::dto::authentication::OptionalEncryptableEmailValue email;
        if (parsed->get_email()) {
            email.set_value(parsed->get_email()->get_value());
        }

        using Response = app::dto::authentication::AuthenticationEligibilityResponse;
        Response response{};
        response.set_authentication_id(authentication_id);
        response.set_profile_id(profile_id);
        response.set_status(status);
        response.set_next_action(next_action);
        response.set_eligibility_response_params(
            app::dto::authentication::EligibilityResponseParams::ThreeDsData);
        response.set_email(email);
        response.set_billing(parsed->get_billing());
        response.set_shipping(parsed->get_shipping());
        response.set_browser_information(parsed->get_browser_information());
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void handle_authentication_authenticate(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto authorized = authorize_operation(req, "authentication_authenticate");
        if (authorized.error) {
            send_error(res, send, *authorized.error);
            return;
        }
        const auto flow = auth_flow_of(authorized.context);

        const auto path = app::http::RequestPath{}(req);
        const auto id_segment = path.at(1);
        if (!id_segment.has_value()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::INVALID_REQUEST, "authentication id missing in path"));
            return;
        }
        const std::string authentication_id_value{*id_segment};

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::dto::authentication::AuthenticationAuthenticateRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        parsed->set_client_secret(effective_client_secret(req, body, parsed->get_client_secret()));
        if (const auto secret_error = verify_client_secret(authentication_id_value, parsed->get_client_secret())) {
            send_error(res, send, *secret_error);
            return;
        }

        // perform_authentication: 3DS connector AReq call and tracker lifecycle.
        track_record_fetch("authenticate", authentication_id_value);
        const auto tracker_id = schedule_auth_tracker("authenticate", authentication_id_value, flow);
        const auto outcome = call_connector(
            "verify", merchant_for(req), authentication_id_value, body, flow, tracker_id);
        publish_auth_event("authentication.authenticated", authentication_id_value, outcome.reference_id);

        m_context->metrics().counter(std::string{"authentication.auth_flow."} + flow).increment();
        m_context->metrics().record_flow("authentication_authenticate");
        m_context->metrics().counter("authentication_authenticate").increment();

        app::dto::authentication::AuthenticationIdValue authentication_id;
        authentication_id.set_value(authentication_id_value);
        app::dto::authentication::AuthenticationStatusValue status;
        status.set_value(auth_status_from(outcome.status));
        app::dto::authentication::TransactionStatusValue transaction_status;
        transaction_status.set_value(outcome.status);
        app::dto::authentication::AuthenticationConnectorsValue connector_value;
        connector_value.set_value("local");

        using Response = app::dto::authentication::AuthenticationAuthenticateResponse;
        Response response{};
        response.set_authentication_id(authentication_id);
        response.set_status(status);
        response.set_transaction_status(transaction_status);
        response.set_three_ds_requestor_url(
            std::string{"/authentication/3ds/"} + outcome.reference_id);
        response.set_three_ds_server_transaction_id(outcome.reference_id);
        response.set_authentication_value(std::optional<std::string>{cavv_from(outcome.reference_id)});
        response.set_authentication_connector(connector_value);
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void handle_authentication_eligibility_check(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto authorized = authorize_operation(req, "authentication_eligibility_check");
        if (authorized.error) {
            send_error(res, send, *authorized.error);
            return;
        }
        const auto flow = auth_flow_of(authorized.context);

        const auto path = app::http::RequestPath{}(req);
        const auto id_segment = path.at(1);
        if (!id_segment.has_value()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::INVALID_REQUEST, "authentication id missing in path"));
            return;
        }
        const std::string authentication_id_value{*id_segment};

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::dto::authentication::AuthenticationEligibilityCheckRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        parsed->set_client_secret(effective_client_secret(req, body, parsed->get_client_secret()));
        if (const auto secret_error = verify_client_secret(authentication_id_value, parsed->get_client_secret())) {
            send_error(res, send, *secret_error);
            return;
        }

        // perform_pre_authentication: 3DS method / DDC connector call.
        track_record_fetch("eligibility_check", authentication_id_value);
        const auto tracker_id = schedule_auth_tracker("eligibility_check", authentication_id_value, flow);
        const auto outcome = call_connector(
            "verify", merchant_for(req), authentication_id_value, body, flow, tracker_id);
        publish_auth_event("authentication.eligibility_checked", authentication_id_value, outcome.reference_id);

        m_context->metrics().counter(std::string{"authentication.auth_flow."} + flow).increment();
        m_context->metrics().record_flow("authentication_eligibility_check");
        m_context->metrics().counter("authentication_eligibility_check").increment();

        app::dto::authentication::AuthenticationIdValue authentication_id;
        authentication_id.set_value(authentication_id_value);
        using Response = app::dto::authentication::AuthenticationEligibilityCheckResponse;
        Response response{};
        response.set_authentication_id(authentication_id);
        response.set_sdk_next_action(auth_status_from(outcome.status) == "failed"
            ? app::dto::authentication::AuthenticationSdkNextAction::Deny
            : app::dto::authentication::AuthenticationSdkNextAction::Proceed);
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void handle_authentication_retrieve_eligibility_check(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto authorized = authorize_operation(req, "authentication_retrieve_eligibility_check");
        if (authorized.error) {
            send_error(res, send, *authorized.error);
            return;
        }
        const auto flow = auth_flow_of(authorized.context);

        const auto path = app::http::RequestPath{}(req);
        const auto id_segment = path.at(1);
        if (!id_segment.has_value()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::INVALID_REQUEST, "authentication id missing in path"));
            return;
        }
        const std::string authentication_id_value{*id_segment};

        const auto input_json = std::string{R"({"authentication_id":{"value":})"}
            + serde::Ser::encode_json(authentication_id_value) + "}}";
        using Input = app::dto::authentication::AuthenticationRetrieveEligibilityCheckRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", input_json);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        // retrieve_eligibility_check_core: DB fetch of the authentication record.
        track_record_fetch("retrieve_eligibility_check", authentication_id_value);
        const auto tracker_id = schedule_auth_tracker("retrieve_eligibility_check", authentication_id_value, flow);
        m_context->process_trackers().mark_terminal(tracker_id, authentication_id_value);

        m_context->metrics().counter(std::string{"authentication.auth_flow."} + flow).increment();
        m_context->metrics().record_flow("authentication_retrieve_eligibility_check");
        m_context->metrics().counter("authentication_retrieve_eligibility_check").increment();

        using Response = app::dto::authentication::AuthenticationRetrieveEligibilityCheckResponse;
        Response response{};
        response.set_authentication_id(parsed->get_authentication_id());
        response.set_eligibility_check_data(
            app::dto::authentication::AuthenticationEligibilityCheckResponseData::ClickToPayEnrollmentStatus
        );
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void handle_authentication_sync(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto authorized = authorize_operation(req, "authentication_sync");
        if (authorized.error) {
            send_error(res, send, *authorized.error);
            return;
        }
        const auto flow = auth_flow_of(authorized.context);

        const auto path = app::http::RequestPath{}(req);
        const auto merchant_segment = path.at(1);
        const auto id_segment = path.at(2);
        if (!merchant_segment.has_value() || !id_segment.has_value()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::INVALID_REQUEST, "merchant id or authentication id missing in path"));
            return;
        }
        const std::string merchant_id_value{*merchant_segment};
        const std::string authentication_id_value{*id_segment};

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::dto::authentication::AuthenticationSyncRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        parsed->set_client_secret(effective_client_secret(req, body, parsed->get_client_secret()));
        if (const auto secret_error = verify_client_secret(authentication_id_value, parsed->get_client_secret())) {
            send_error(res, send, *secret_error);
            return;
        }

        const bool merchant_known =
            m_context->repository().find_merchant_account_by_string(merchant_id_value).has_value();
        if (merchant_known) {
            m_context->metrics().counter("authentication_merchant_resolved").increment();
        }

        // authentication_sync_core: DB fetch of the persisted authentication record.
        std::string created_at_value = created_timestamp(authentication_id_value);
        const auto record = m_context->repository().find_authentication_record_by_string(authentication_id_value);
        if (record.has_value()) {
            created_at_value = record->get_created_at().empty() ? created_at_value : record->get_created_at();
            m_context->metrics().counter("authentication_sync_record_found").increment();
        } else {
            m_context->metrics().counter("authentication_sync_record_missing").increment();
        }

        // perform_post_authentication: sync connector call + tracker.
        const auto tracker_id = schedule_auth_tracker("sync", authentication_id_value, flow);
        const auto outcome = call_connector(
            "sync", merchant_id_value, authentication_id_value, body, flow, tracker_id);
        publish_auth_event("authentication.synced", authentication_id_value, outcome.reference_id);

        m_context->metrics().counter(std::string{"authentication.auth_flow."} + flow).increment();
        m_context->metrics().record_flow("authentication_sync");
        m_context->metrics().counter("authentication_sync").increment();

        app::dto::authentication::AuthenticationIdValue authentication_id;
        authentication_id.set_value(authentication_id_value);
        app::dto::authentication::MerchantIdValue merchant_id;
        merchant_id.set_value(merchant_id_value);
        app::dto::authentication::AuthenticationStatusValue status;
        status.set_value(auth_status_from(outcome.status));
        app::dto::authentication::MinorUnitValue amount;
        amount.set_value("0");
        app::dto::authentication::CurrencyValue currency;
        currency.set_value("USD");
        app::dto::authentication::PrimitiveDateTimeValue created_at;
        created_at.set_value(created_at_value);
        app::dto::authentication::ProfileIdValue profile_id;
        profile_id.set_value("");
        app::dto::authentication::OptionalEncryptableEmailValue email;
        email.set_value("");

        using Response = app::dto::authentication::AuthenticationSyncResponse;
        Response response{};
        response.set_authentication_id(authentication_id);
        response.set_merchant_id(merchant_id);
        response.set_status(status);
        response.set_client_secret(parsed->get_client_secret());
        response.set_amount(amount);
        response.set_currency(currency);
        response.set_created_at(created_at);
        response.set_profile_id(profile_id);
        response.set_email(email);
        response.set_threeds_server_transaction_id(outcome.reference_id);
        response.set_connector_authentication_id(outcome.reference_id);
        response.set_three_ds_method_data(std::optional<std::string>{cavv_from(outcome.reference_id)});
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void handle_authentication_sync_post_update(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto authorized = authorize_operation(req, "authentication_sync_post_update");
        if (authorized.error) {
            send_error(res, send, *authorized.error);
            return;
        }

        const auto path = app::http::RequestPath{}(req);
        const auto merchant_segment = path.at(1);
        const auto id_segment = path.at(2);
        if (!merchant_segment.has_value() || !id_segment.has_value()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::INVALID_REQUEST, "merchant id or authentication id missing in path"));
            return;
        }
        const std::string merchant_id_value{*merchant_segment};
        const std::string authentication_id_value{*id_segment};

        if (std::string_view{authorized.context.merchant_id()} != merchant_id_value) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::FORBIDDEN, "merchant id does not match request path"));
            return;
        }
        const auto flow = auth_flow_of(authorized.context);

        const auto input_json = std::string{R"({"authentication_id":{"value":})"}
            + serde::Ser::encode_json(authentication_id_value) + "}}";
        using Input = app::dto::authentication::AuthenticationSyncPostUpdateRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", input_json);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        // perform_post_authentication: terminal-status update, post-auth connector
        // call and CAVV retrieval (deterministic substitute) from the vault.
        const auto tracker_id = schedule_auth_tracker("sync_post_update", authentication_id_value, flow);
        const auto outcome = call_connector(
            "sync", merchant_id_value, authentication_id_value, input_json, flow, tracker_id);
        update_authentication_status(authentication_id_value, auth_status_from(outcome.status));
        publish_auth_event("authentication.post_sync", authentication_id_value, cavv_from(outcome.reference_id));

        m_context->metrics().counter(std::string{"authentication.auth_flow."} + flow).increment();
        m_context->metrics().record_flow("authentication_sync_post_update");
        m_context->metrics().counter("authentication_sync_post_update").increment();

        auto text = serde::Ser::encode_json(*parsed);
        send_json(res, send, text, true);
    }

    void handle_authentication_session_token(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto authorized = authorize_operation(req, "authentication_session_token");
        if (authorized.error) {
            send_error(res, send, *authorized.error);
            return;
        }
        const auto flow = auth_flow_of(authorized.context);

        const auto path = app::http::RequestPath{}(req);
        const auto id_segment = path.at(1);
        if (!id_segment.has_value()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::INVALID_REQUEST, "authentication id missing in path"));
            return;
        }
        const std::string authentication_id_value{*id_segment};

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::dto::authentication::AuthenticationSessionTokenRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        parsed->set_client_secret(effective_client_secret(req, body, parsed->get_client_secret()));
        if (const auto secret_error = verify_client_secret(authentication_id_value, parsed->get_client_secret())) {
            send_error(res, send, *secret_error);
            return;
        }

        // authentication_session_core: session-token generation from connector.
        track_record_fetch("session_token", authentication_id_value);
        const auto tracker_id = schedule_auth_tracker("session_token", authentication_id_value, flow);
        const auto outcome = call_connector(
            "verify", merchant_for(req), authentication_id_value, body, flow, tracker_id);
        publish_auth_event("authentication.session_token", authentication_id_value, outcome.reference_id);

        m_context->metrics().counter(std::string{"authentication.auth_flow."} + flow).increment();
        m_context->metrics().record_flow("authentication_session_token");
        m_context->metrics().counter("authentication_session_token").increment();

        app::dto::authentication::AuthenticationIdValue authentication_id;
        authentication_id.set_value(authentication_id_value);
        using Response = app::dto::authentication::AuthenticationSessionResponse;
        Response response{};
        response.set_authentication_id(authentication_id);
        response.set_session_token({app::dto::authentication::AuthenticationSessionToken::ClickToPay});
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/authentication"}
            .add_route(
                Route{""}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_authentication_create(req, res, std::move(send));
                    })
                    .summary("Hyperswitch authentication_create")
            );
        Router{router, "/authentication"}
            .add_route(
                Route{"/:authentication_id/eligibility"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_authentication_eligibility(req, res, std::move(send));
                    })
                    .summary("Hyperswitch authentication_eligibility")
            );
        Router{router, "/authentication"}
            .add_route(
                Route{"/:authentication_id/authenticate"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_authentication_authenticate(req, res, std::move(send));
                    })
                    .summary("Hyperswitch authentication_authenticate")
            );
        Router{router, "/authentication"}
            .add_route(
                Route{"/:authentication_id/eligibility-check"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_authentication_eligibility_check(req, res, std::move(send));
                    })
                    .summary("Hyperswitch authentication_eligibility_check")
            );
        Router{router, "/authentication"}
            .add_route(
                Route{"/:authentication_id/eligibility-check"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_authentication_retrieve_eligibility_check(req, res, std::move(send));
                    })
                    .summary("Hyperswitch authentication_retrieve_eligibility_check")
            );
        Router{router, "/authentication"}
            .add_route(
                Route{"/:merchant_id/:authentication_id/redirect"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_authentication_sync_post_update(req, res, std::move(send));
                    })
                    .summary("Hyperswitch authentication_sync_post_update")
            );
        Router{router, "/authentication"}
            .add_route(
                Route{"/:merchant_id/:authentication_id/redirect"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_authentication_sync_post_update(req, res, std::move(send));
                    })
                    .summary("Hyperswitch authentication_sync_post_update")
            );
        Router{router, "/authentication"}
            .add_route(
                Route{"/:merchant_id/:authentication_id/sync"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_authentication_sync(req, res, std::move(send));
                    })
                    .summary("Hyperswitch authentication_sync")
            );
        Router{router, "/authentication"}
            .add_route(
                Route{"/:authentication_id/enabled_authn_methods_token"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_authentication_session_token(req, res, std::move(send));
                    })
                    .summary("Hyperswitch authentication_session_token")
            );
    }

    explicit AuthenticationRouteHandlers(app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

private:
    struct AuthorizedRequest
    {
        std::optional<core::error::AppError> error{};
        app::auth::AuthContext context{};
    };

    struct ConnectorOutcome
    {
        std::string status{};
        std::string reference_id{};
        std::string message{};
    };

    [[nodiscard]] AuthorizedRequest authorize_operation(
        interfaces::io::IRequest& req,
        std::string_view operation
    ) const
    {
        const auto rule = m_context->auth_rules().rule_for(operation);
        auto context = app::auth::AuthContextBuilder{}(req);
        auto denied = m_context->authorizer().authorize_route(context, rule);
        return AuthorizedRequest{std::move(denied), std::move(context)};
    }

    [[nodiscard]] std::string auth_flow_of(const app::auth::AuthContext& context) const
    {
        if (context.claim(app::auth::kClaimPublishableKey).has_value() ||
            context.claim(app::auth::kClaimClientSecret).has_value()) {
            return "SDK";
        }
        if (context.claim(app::auth::kClaimApiKey).has_value()) {
            return "API_KEY";
        }
        if (!context.token().is_empty()) {
            return "JWT";
        }
        if (!context.merchant_id().empty()) {
            return "MERCHANT_ID";
        }
        return "NO_AUTH";
    }

    [[nodiscard]] std::string merchant_for(interfaces::io::IRequest& req) const
    {
        return app::http::merchant_id_from_header{}(req);
    }

    [[nodiscard]] std::optional<std::string> effective_client_secret(
        interfaces::io::IRequest& req,
        std::string_view body,
        const std::optional<std::string>& from_dto
    ) const
    {
        const auto context = app::auth::AuthContextBuilder{}(req, body);
        if (const auto claim = context.claim(app::auth::kClaimClientSecret);
            claim.has_value() && !claim->empty()) {
            return std::string{*claim};
        }
        return from_dto;
    }

    [[nodiscard]] std::string auth_status_from(std::string_view connector_status) const
    {
        if (connector_status.contains("authorized") || connector_status.contains("success") ||
            connector_status.contains("synced") || connector_status.contains("verified")) {
            return "success";
        }
        if (connector_status.contains("denied") || connector_status.contains("failed")) {
            return "failed";
        }
        return "pending";
    }

    [[nodiscard]] std::string cavv_from(std::string_view reference_id) const
    {
        return app::crypto::Base64Url{}.encode(
            app::crypto::KeyedHasher{"authentication_cavv"}.hash(reference_id));
    }

    [[nodiscard]] std::string created_timestamp(std::string_view authentication_id) const
    {
        const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(
                                 std::chrono::system_clock::now().time_since_epoch()
                             ).count();
        return std::to_string(seconds) + ":" + std::string{authentication_id};
    }

    void remember_client_secret(std::string_view authentication_id, std::string_view plain) const
    {
        const app::crypto::KeyedHasher hasher{"authentication_client_secret"};
        m_context->cache().put(
            "authentication.client_secret_hash:" + std::string{authentication_id},
            core::cache::to_cache_value(hasher.hash(plain)));
    }

    [[nodiscard]] std::optional<core::error::AppError> verify_client_secret(
        std::string_view authentication_id,
        const std::optional<std::string>& presented
    ) const
    {
        if (!presented.has_value() || presented->empty()) {
            return std::nullopt;
        }
        std::string expected{};
        const auto cache_key = std::string{"authentication.client_secret_hash:"}
                             + std::string{authentication_id};
        const auto cached = m_context->cache().get(cache_key);
        if (cached.hit) {
            expected = cached.value_or<std::string>("");
        } else {
            const auto record = m_context->repository().find_authentication_record_by_string(authentication_id);
            if (record.has_value()) {
                expected = record->get_challenge();
            }
        }
        if (expected.empty()) {
            return std::nullopt;
        }
        const app::crypto::KeyedHasher hasher{"authentication_client_secret"};
        if (!hasher.verify(*presented, expected)) {
            return core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::FORBIDDEN, "invalid authentication client secret");
        }
        return std::nullopt;
    }

    void persist_authentication(
        std::string_view authentication_id,
        std::string_view merchant_id,
        std::string_view status,
        std::string_view challenge_hash
    ) const
    {
        app::store::authentication_records::AuthenticationRecord record{};
        record.set_auth_id(std::string{authentication_id});
        record.set_merchant_id(std::string{merchant_id});
        record.set_method("3DS");
        record.set_status(std::string{status});
        record.set_challenge(std::string{challenge_hash});
        const auto stamp = now_seconds();
        record.set_created_at(stamp);
        record.set_expires_at(stamp);
        m_context->repository().save_authentication_record(std::move(record));
    }

    void update_authentication_status(std::string_view authentication_id, std::string_view status) const
    {
        auto record = m_context->repository().find_authentication_record_by_string(authentication_id);
        if (!record.has_value()) {
            m_context->metrics().counter("authentication_status_record_missing").increment();
            return;
        }
        record->set_status(std::string{status});
        m_context->repository().update_authentication_record(*record);
        m_context->metrics().counter("authentication_status_updated").increment();
    }

    [[nodiscard]] std::optional<app::dto::authentication::CustomerDetailsValue> encrypted_customer_details(
        const std::optional<app::dto::authentication::CustomerDetailsValue>& input
    ) const
    {
        if (!input.has_value()) {
            return std::nullopt;
        }
        constexpr std::string_view purpose{"authentication"};
        constexpr std::string_view key_id{"customer_details"};
        std::string wrap_key{"authentication_customer_details_local_wrap_key"};
        const auto existing = m_context->key_store().get(purpose, key_id);
        if (existing.has_value()) {
            wrap_key = existing->value();
        } else {
            (void)m_context->key_store().put(
                purpose, key_id, "LOCAL_XOR", app::crypto::SecretString{wrap_key}, 0);
        }
        app::crypto::AesWrap wrap{};
        const std::string wrapped = wrap.encrypt(input->get_value(), wrap_key);
        app::dto::authentication::CustomerDetailsValue output{};
        output.set_value(app::crypto::Base64Url{}.encode(wrapped));
        return output;
    }

    [[nodiscard]] std::int64_t schedule_auth_tracker(
        std::string_view operation,
        std::string_view authentication_id,
        std::string_view flow
    ) const
    {
        std::string payload{std::string{operation}};
        payload += ":";
        payload += authentication_id;
        payload += ":auth_flow=";
        payload += flow;
        return m_context->process_trackers().schedule(
            "authentication_" + std::string{operation}, std::move(payload), "0");
    }

    [[nodiscard]] ConnectorOutcome call_connector(
        std::string_view operation,
        std::string_view merchant_id,
        std::string_view resource_id,
        std::string_view payload,
        std::string_view flow,
        std::int64_t tracker_id
    ) const
    {
        app::connector::ConnectorRequest request{};
        request.set_operation(std::string{operation});
        request.set_merchant_id(std::string{merchant_id});
        request.set_resource_id(std::string{resource_id});
        request.set_raw_payload(std::string{payload});

        auto connector = m_context->connector_router().select_for(request);
        if (connector) {
            app::connector::ConnectorResponse response =
                operation == "sync" ? connector->sync(request)
                : operation == "verify" ? connector->verify(request)
                : connector->authorize(request);
            m_context->process_trackers().update_status(tracker_id, "RUNNING");
            m_context->process_trackers().mark_terminal(tracker_id, response.get_reference_id());
            m_context->metrics().record_flow(std::string{"authentication.connector."} + std::string{operation});
            m_context->metrics().counter("authentication_connector_call").increment();
            return ConnectorOutcome{response.get_status(), response.get_reference_id(), response.get_message()};
        }

        // Deterministic local substitute: no connector backend registered.
        const auto digest = app::crypto::KeyedHasher{"authentication_connector"}.hash(
            std::string{operation} + ":" + std::string{resource_id} + ":" + std::string{flow});
        m_context->process_trackers().mark_terminal(tracker_id, digest);
        m_context->metrics().counter("authentication_connector_unavailable").increment();
        return ConnectorOutcome{"local_pending", "authn_" + digest, "local deterministic substitute"};
    }

    void publish_auth_event(
        std::string_view topic,
        std::string_view authentication_id,
        std::string_view detail
    ) const
    {
        std::string payload{"{\"authentication_id\":\""};
        payload += authentication_id;
        payload += "\",\"detail\":\"";
        payload += detail;
        payload += "\"}";
        (void)m_context->events().publish(std::string{topic}, std::move(payload));
    }

    void track_record_fetch(std::string_view operation, std::string_view authentication_id) const
    {
        if (m_context->repository().find_authentication_record_by_string(authentication_id).has_value()) {
            m_context->metrics().counter(std::string{"authentication_record_found."} + std::string{operation}).increment();
        } else {
            m_context->metrics().counter(std::string{"authentication_record_missing."} + std::string{operation}).increment();
        }
    }

    [[nodiscard]] std::string now_seconds() const
    {
        return std::to_string(std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());
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

} // namespace app::handlers::authentication
