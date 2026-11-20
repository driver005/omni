module;
export module app_handlers:admin;

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
import app_http;
import app_crypto;
import app_metrics;
import app_store;
import app_locking;
import app_events;
import app_keystore;
import app_services;

export namespace app::handlers::admin {

class AdminRouteHandlers
{
public:
    explicit AdminRouteHandlers(app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

    void handle_organization_create(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_error = authorize_request(req, "organization_create");
        if (auth_error) {
            send_error(res, send, *auth_error);
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::organization::OrganizationCreateRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        for (const auto& existing : m_context->repository().list_organizations()) {
            if (existing.get_organization_name() == parsed->get_organization_name()) {
                send_error(res, send, core::error::AppErrorBuilder{}(
                    core::error::AppErrorKind::ALREADY_EXISTS,
                    "Organization with the given organization_name already exists"
                ));
                return;
            }
        }
        if (const auto duplicate = acquire_operation_lock("organization_create", parsed->get_organization_name())) {
            send_error(res, send, *duplicate);
            return;
        }

        const std::string organization_id{"org_" + uuid_simple()};
        app::store::organizations::OrganizationRecord record;
        record.set_organization_id(organization_id);
        record.set_organization_name(parsed->get_organization_name());
        record.set_organization_details(
            parsed->get_organization_details()
                ? parsed->get_organization_details()->get_value()
                : std::string{}
        );
        record.set_metadata(
            parsed->get_metadata()
                ? parsed->get_metadata()->get_value()
                : std::string{}
        );
        record.set_organization_type("merchant");
        const auto saved = m_context->repository().save_organization(std::move(record));
        release_operation_lock("organization_create", parsed->get_organization_name());

        app::model::organization::OrganizationResponse response;
        app::model::organization::OrganizationIdValue id;
        id.set_value(saved.get_organization_id());
        response.set_organization_id(std::move(id));
        response.set_organization_name(saved.get_organization_name());
        if (!saved.get_organization_details().empty()) {
            app::model::organization::SecretSerdeValueValue details;
            details.set_value(saved.get_organization_details());
            response.set_organization_details(std::move(details));
        }
        if (!saved.get_metadata().empty()) {
            app::model::organization::SecretSerdeValueValue metadata;
            metadata.set_value(saved.get_metadata());
            response.set_metadata(std::move(metadata));
        }
        app::model::organization::OrganizationTypeValue type;
        type.set_value(saved.get_organization_type());
        response.set_organization_type(std::move(type));

        m_context->metrics().record_flow("organization_create");
        m_context->metrics().counter("admin.organization_create").increment();

        auto text = serde::Ser::encode_json(response);
        m_context->events().publish("organization.created", std::string{text});
        send_json(res, send, text, true);
    }

    void handle_organization_update(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_error = authorize_request(req, "organization_update");
        if (auth_error) {
            send_error(res, send, *auth_error);
            return;
        }

        const std::string organization_id{path_part(req, 1)};
        if (organization_id.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_REQUEST, "missing organization id"));
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::organization::OrganizationUpdateRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        auto found = m_context->repository().find_organization_by_string(organization_id);
        if (!found.has_value()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "organization not found"));
            return;
        }
        auto record = std::move(*found);
        if (parsed->get_organization_name().has_value()) {
            record.set_organization_name(*parsed->get_organization_name());
        }
        if (parsed->get_organization_details().has_value()) {
            record.set_organization_details(parsed->get_organization_details()->get_value());
        }
        if (parsed->get_metadata().has_value()) {
            record.set_metadata(parsed->get_metadata()->get_value());
        }
        if (parsed->get_platform_merchant_id().has_value()) {
            record.set_platform_merchant_id(parsed->get_platform_merchant_id()->get_value());
        }
        if (const auto duplicate = acquire_operation_lock("organization_update", organization_id)) {
            send_error(res, send, *duplicate);
            return;
        }
        m_context->repository().update_organization(record);
        release_operation_lock("organization_update", organization_id);

        app::model::organization::OrganizationResponse response;
        app::model::organization::OrganizationIdValue id;
        id.set_value(record.get_organization_id());
        response.set_organization_id(std::move(id));
        response.set_organization_name(record.get_organization_name());
        if (!record.get_organization_details().empty()) {
            app::model::organization::SecretSerdeValueValue details;
            details.set_value(record.get_organization_details());
            response.set_organization_details(std::move(details));
        }
        if (!record.get_metadata().empty()) {
            app::model::organization::SecretSerdeValueValue metadata;
            metadata.set_value(record.get_metadata());
            response.set_metadata(std::move(metadata));
        }

        m_context->metrics().record_flow("organization_update");
        m_context->metrics().counter("admin.organization_update").increment();

        auto text = serde::Ser::encode_json(response);
        m_context->events().publish("organization.updated", std::string{text});
        send_json(res, send, text, true);
    }

    void handle_convert_organization_to_platform(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_error = authorize_request(req, "convert_organization_to_platform");
        if (auth_error) {
            send_error(res, send, *auth_error);
            return;
        }

        const std::string organization_id{path_part(req, 2)};
        if (organization_id.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_REQUEST, "missing organization id"));
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::model::organization::ConvertOrganizationToPlatformRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        if (m_context->config_store().get_value("platform", "enabled").value_or("true") == "false") {
            send_error(res, send, core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "Platform feature is not enabled"
            ));
            return;
        }
        auto found = m_context->repository().find_organization_by_string(organization_id);
        if (!found.has_value()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "organization not found"));
            return;
        }
        if (found->get_organization_type() == "platform" || !found->get_platform_merchant_id().empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::PRECONDITION_FAILED,
                "Organization is already of type Platform"
            ));
            return;
        }
        if (const auto duplicate = acquire_operation_lock("convert_organization_to_platform", organization_id)) {
            send_error(res, send, *duplicate);
            return;
        }
        auto record = std::move(*found);
        record.set_organization_type("platform");
        const std::string platform_merchant_id{"merchant_" + uuid_simple()};
        record.set_platform_merchant_id(platform_merchant_id);
        m_context->repository().update_organization(record);

        std::string merchant_name{parsed->get_platform_merchant_name().value_or("")};
        if (merchant_name.empty()) {
            merchant_name = record.get_organization_name() + " Platform";
        }
        app::store::merchant_accounts::MerchantAccountRecord platform_account;
        platform_account.set_merchant_id(platform_merchant_id);
        platform_account.set_merchant_name(merchant_name);
        platform_account.set_organization_id(record.get_organization_id());
        platform_account.set_parent_merchant_id(record.get_organization_id());
        platform_account.set_merchant_account_type("platform");
        m_context->repository().save_merchant_account(std::move(platform_account));
        m_context->events().publish(
            "merchant_account.created",
            std::format("{{\"merchant_id\":\"{}\",\"organization_id\":\"{}\"}}",
                        platform_merchant_id, record.get_organization_id())
        );
        release_operation_lock("convert_organization_to_platform", organization_id);

        app::model::organization::ConvertOrganizationToPlatformResponse response;
        app::model::organization::OrganizationIdValue id;
        id.set_value(record.get_organization_id());
        response.set_organization_id(std::move(id));
        app::model::organization::OrganizationTypeValue type;
        type.set_value(record.get_organization_type());
        response.set_organization_type(std::move(type));
        app::model::organization::MerchantIdValue platform_id;
        platform_id.set_value(record.get_platform_merchant_id());
        response.set_platform_merchant_id(std::move(platform_id));

        m_context->metrics().record_flow("convert_organization_to_platform");
        m_context->metrics().counter("admin.convert_organization_to_platform").increment();

        auto text = serde::Ser::encode_json(response);
        m_context->events().publish("organization.converted_to_platform", std::string{text});
        send_json(res, send, text, true);
    }

    void handle_merchant_account_create(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_error = authorize_request(req, "merchant_account_create");
        if (auth_error) {
            send_error(res, send, *auth_error);
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::dto::admin::MerchantAccountCreate;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        std::string merchant_id{parsed->get_merchant_id().get_value()};
        if (merchant_id.empty()) {
            merchant_id = "merchant_" + uuid_simple();
        }
        if (m_context->repository().find_merchant_account_by_string(merchant_id).has_value()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::ALREADY_EXISTS, "merchant account already exists"));
            return;
        }
        if (const auto duplicate = acquire_operation_lock("merchant_account_create", merchant_id)) {
            send_error(res, send, *duplicate);
            return;
        }

        std::string organization_id{app::http::org_id_from_header{}(req)};
        if (organization_id.empty() && parsed->get_organization_id().has_value()) {
            organization_id = parsed->get_organization_id()->get_value();
        }

        const std::string simple_uuid{uuid_simple()};
        const std::string env_prefix{
            m_context->config_store().get_value("env", "prefix").value_or("test")
        };
        const app::crypto::SecretString publishable_key{
            std::format("pk_{}_{}", env_prefix, simple_uuid)
        };
        const std::string locker_id{simple_uuid};
        const std::string response_hash{
            app::crypto::KeyedHasher{"admin.publishable_key"}.hash(publishable_key.value())
        };
        const std::string aes_key{m_context->ids().key_id()};
        m_context->key_store().put(
            "merchant_aes", merchant_id, "AES-256", app::crypto::SecretString{aes_key}
        );
        m_context->key_store().put(
            "merchant_locker",
            merchant_id,
            "AES-256-wrap",
            app::crypto::SecretString{app::crypto::AesWrap{}.encrypt(locker_id, aes_key)}
        );

        app::store::merchant_accounts::MerchantAccountRecord record;
        record.set_merchant_id(merchant_id);
        record.set_merchant_name(parsed->get_merchant_name().value_or(""));
        record.set_organization_id(organization_id);
        if (parsed->get_parent_merchant_id().has_value()) {
            record.set_parent_merchant_id(parsed->get_parent_merchant_id()->get_value());
        }
        if (parsed->get_return_url().has_value()) {
            record.set_return_url(parsed->get_return_url()->get_value());
        }
        record.set_publishable_key(publishable_key.value());
        record.set_locker_id(locker_id);
        record.set_payment_response_hash_key(response_hash);
        record.set_sub_merchants_enabled(parsed->get_sub_merchants_enabled().value_or(false));
        record.set_enable_payment_response_hash(parsed->get_enable_payment_response_hash().value_or(false));
        record.set_redirect_to_merchant_with_http_post(
            parsed->get_redirect_to_merchant_with_http_post().value_or(false)
        );
        const auto saved = m_context->repository().save_merchant_account(std::move(record));
        release_operation_lock("merchant_account_create", merchant_id);

        app::dto::admin::MerchantAccountResponse response;
        response.set_merchant_id(merchant_id_value(saved.get_merchant_id()));
        response.set_merchant_name(encryptable_name(saved.get_merchant_name()));
        response.set_organization_id(organization_id_value(saved.get_organization_id()));
        if (!saved.get_return_url().empty()) {
            response.set_return_url(saved.get_return_url());
        }
        if (!saved.get_publishable_key().empty()) {
            response.set_publishable_key(saved.get_publishable_key());
        }
        if (!saved.get_locker_id().empty()) {
            response.set_locker_id(saved.get_locker_id());
        }
        response.set_sub_merchants_enabled(saved.get_sub_merchants_enabled());
        response.set_enable_payment_response_hash(saved.get_enable_payment_response_hash());
        response.set_redirect_to_merchant_with_http_post(
            saved.get_redirect_to_merchant_with_http_post()
        );
        if (!saved.get_payment_response_hash_key().empty()) {
            response.set_payment_response_hash_key(saved.get_payment_response_hash_key());
        }

        m_context->metrics().record_flow("merchant_account_create");
        m_context->metrics().counter("admin.merchant_account_create").increment();

        auto text = serde::Ser::encode_json(response);
        m_context->events().publish("merchant_account.created", std::string{text});
        send_json(res, send, text, true);
    }

    void handle_update_merchant_account(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_error = authorize_request(req, "update_merchant_account");
        if (auth_error) {
            send_error(res, send, *auth_error);
            return;
        }

        const std::string merchant_id{path_part(req, 1)};
        if (merchant_id.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_REQUEST, "missing merchant id"));
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::dto::admin::MerchantAccountUpdate;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        auto found = m_context->repository().find_merchant_account_by_string(merchant_id);
        if (!found.has_value()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "merchant account not found"));
            return;
        }
        auto record = std::move(*found);
        if (parsed->get_merchant_name().has_value()) {
            record.set_merchant_name(*parsed->get_merchant_name());
        }
        if (parsed->get_return_url().has_value()) {
            record.set_return_url(parsed->get_return_url()->get_value());
        }
        if (parsed->get_publishable_key().has_value()) {
            record.set_publishable_key(*parsed->get_publishable_key());
        }
        if (parsed->get_locker_id().has_value()) {
            record.set_locker_id(*parsed->get_locker_id());
        }
        if (parsed->get_sub_merchants_enabled().has_value()) {
            record.set_sub_merchants_enabled(*parsed->get_sub_merchants_enabled());
        }
        if (parsed->get_enable_payment_response_hash().has_value()) {
            record.set_enable_payment_response_hash(*parsed->get_enable_payment_response_hash());
        }
        if (parsed->get_redirect_to_merchant_with_http_post().has_value()) {
            record.set_redirect_to_merchant_with_http_post(
                *parsed->get_redirect_to_merchant_with_http_post()
            );
        }
        if (parsed->get_payment_response_hash_key().has_value()) {
            record.set_payment_response_hash_key(*parsed->get_payment_response_hash_key());
        }
        if (const auto duplicate = acquire_operation_lock("update_merchant_account", merchant_id)) {
            send_error(res, send, *duplicate);
            return;
        }
        m_context->repository().update_merchant_account(record);
        release_operation_lock("update_merchant_account", merchant_id);

        app::dto::admin::MerchantAccountResponse response;
        response.set_merchant_id(merchant_id_value(record.get_merchant_id()));
        response.set_merchant_name(encryptable_name(record.get_merchant_name()));
        response.set_organization_id(organization_id_value(record.get_organization_id()));
        if (!record.get_return_url().empty()) {
            response.set_return_url(record.get_return_url());
        }
        if (!record.get_publishable_key().empty()) {
            response.set_publishable_key(record.get_publishable_key());
        }
        if (!record.get_locker_id().empty()) {
            response.set_locker_id(record.get_locker_id());
        }
        response.set_sub_merchants_enabled(record.get_sub_merchants_enabled());

        m_context->metrics().record_flow("update_merchant_account");
        m_context->metrics().counter("admin.update_merchant_account").increment();

        auto text = serde::Ser::encode_json(response);
        m_context->events().publish("merchant_account.updated", std::string{text});
        send_json(res, send, text, true);
    }

    void handle_connector_create(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_error = authorize_request(req, "connector_create");
        if (auth_error) {
            send_error(res, send, *auth_error);
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::dto::admin::MerchantConnectorCreate;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        std::string merchant_id{app::http::merchant_id_from_header{}(req)};
        if (merchant_id.empty() && path_has(req, "account")) {
            merchant_id = path_part(req, 2);
        }

        const std::string connector_name{parsed->get_connector_name().get_value()};
        if (const auto invalid = validate_connector_create(merchant_id, connector_name)) {
            send_error(res, send, *invalid);
            return;
        }

        const std::string connector_id{"mca_" + uuid_simple()};
        if (m_context->repository().find_merchant_connector_by_string(connector_id).has_value()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::ALREADY_EXISTS, "connector already exists"));
            return;
        }
        if (const auto duplicate = acquire_operation_lock("connector_create", connector_id)) {
            send_error(res, send, *duplicate);
            return;
        }
        m_context->key_store().put(
            "connector_data_key",
            connector_id,
            "AES-256",
            app::crypto::SecretString{m_context->ids().key_id()}
        );

        const std::string connector_type{parsed->get_connector_type().get_value()};
        const std::string account_plaintext{
            parsed->get_connector_account_details()
                ? parsed->get_connector_account_details()->get_value()
                : std::string{}
        };
        const app::crypto::SecretString account_secret{account_plaintext};
        const std::string account_ciphertext{
            app::crypto::AesWrap{}.encrypt(
                account_secret.value(),
                connector_encryption_key(merchant_id, connector_id)
            )
        };

        app::store::merchant_connectors::MerchantConnectorRecord record;
        record.set_merchant_connector_id(connector_id);
        record.set_merchant_id(merchant_id);
        record.set_connector_name(connector_name);
        record.set_connector_type(connector_type);
        record.set_connector_label(parsed->get_connector_label().value_or(""));
        record.set_profile_id(parsed->get_profile_id().get_value());
        record.set_connector_account_details(account_ciphertext);
        record.set_metadata(
            parsed->get_metadata()
                ? parsed->get_metadata()->get_value()
                : std::string{}
        );
        record.set_status(
            parsed->get_status()
                ? parsed->get_status()->get_value()
                : std::string{}
        );
        record.set_disabled(parsed->get_disabled().value_or(false));
        const auto saved = m_context->repository().save_merchant_connector(std::move(record));
        release_operation_lock("connector_create", connector_id);

        app::dto::admin::MerchantConnectorResponse response;
        response.set_connector_type(connector_type_value(saved.get_connector_type()));
        response.set_connector_name(connector_value(saved.get_connector_name()));
        response.set_connector_label(saved.get_connector_label());
        response.set_id(connector_account_id(saved.get_merchant_connector_id()));
        response.set_profile_id(profile_id(saved.get_profile_id()));
        response.set_connector_account_details(secret_value(saved.get_connector_account_details()));
        response.set_disabled(saved.get_disabled());
        if (!saved.get_metadata().empty()) {
            response.set_metadata(secret_value(saved.get_metadata()));
        }
        if (!saved.get_status().empty()) {
            response.set_status(connector_status_value(saved.get_status()));
        }
        if (parsed->get_connector_webhook_details().has_value()) {
            response.set_connector_webhook_details(*parsed->get_connector_webhook_details());
        }
        if (parsed->get_payment_methods_enabled().has_value()) {
            response.set_payment_methods_enabled(*parsed->get_payment_methods_enabled());
        }

        m_context->metrics().record_flow("connector_create");
        m_context->metrics().counter("admin.connector_create").increment();

        auto text = serde::Ser::encode_json(response);
        m_context->events().publish("merchant_connector.created", std::string{text});
        send_json(res, send, text, true);
    }

    void handle_connector_update(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_error = authorize_request(req, "connector_update");
        if (auth_error) {
            send_error(res, send, *auth_error);
            return;
        }

        const std::string connector_id{path_part(req, 1)};
        if (connector_id.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_REQUEST, "missing connector id"));
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::dto::admin::MerchantConnectorUpdate;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        auto found = m_context->repository().find_merchant_connector_by_string(connector_id);
        if (!found.has_value()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "connector not found"));
            return;
        }
        if (const auto duplicate = acquire_operation_lock("connector_update", connector_id)) {
            send_error(res, send, *duplicate);
            return;
        }
        auto record = std::move(*found);
        if (parsed->get_connector_label().has_value()) {
            record.set_connector_label(*parsed->get_connector_label());
        }
        if (parsed->get_metadata().has_value()) {
            record.set_metadata(parsed->get_metadata()->get_value());
        }
        if (parsed->get_disabled().has_value()) {
            record.set_disabled(*parsed->get_disabled());
        }
        if (parsed->get_status().has_value()) {
            record.set_status(parsed->get_status()->get_value());
        }
        if (parsed->get_connector_account_details().has_value()) {
            const app::crypto::SecretString plaintext{
                parsed->get_connector_account_details()->get_value()
            };
            record.set_connector_account_details(
                app::crypto::AesWrap{}.encrypt(
                    plaintext.value(),
                    connector_encryption_key(record.get_merchant_id(), connector_id)
                )
            );
        }
        m_context->repository().update_merchant_connector(record);
        release_operation_lock("connector_update", connector_id);

        app::dto::admin::MerchantConnectorResponse response;
        response.set_connector_type(connector_type_value(record.get_connector_type()));
        response.set_connector_name(connector_value(record.get_connector_name()));
        response.set_connector_label(record.get_connector_label());
        response.set_id(connector_account_id(record.get_merchant_connector_id()));
        response.set_profile_id(profile_id(record.get_profile_id()));
        response.set_connector_account_details(secret_value(record.get_connector_account_details()));
        response.set_disabled(record.get_disabled());
        if (!record.get_metadata().empty()) {
            response.set_metadata(secret_value(record.get_metadata()));
        }
        if (!record.get_status().empty()) {
            response.set_status(connector_status_value(record.get_status()));
        }

        m_context->metrics().record_flow("connector_update");
        m_context->metrics().counter("admin.connector_update").increment();

        auto text = serde::Ser::encode_json(response);
        m_context->events().publish("merchant_connector.updated", std::string{text});
        send_json(res, send, text, true);
    }

    void handle_merchant_account_toggle_kv(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_error = authorize_request(req, "merchant_account_toggle_kv");
        if (auth_error) {
            send_error(res, send, *auth_error);
            return;
        }

        const std::string merchant_id{path_part(req, 2)};
        if (merchant_id.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_REQUEST, "missing merchant id"));
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::dto::admin::ToggleKVRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        auto found = m_context->repository().find_merchant_account_by_string(merchant_id);
        if (!found.has_value()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "merchant account not found"));
            return;
        }

        if (const auto blocked = kv_soft_kill_guard()) {
            send_error(res, send, *blocked);
            return;
        }
        if (const auto duplicate = acquire_operation_lock("merchant_account_toggle_kv", merchant_id)) {
            send_error(res, send, *duplicate);
            return;
        }
        const bool kv_enabled{parsed->get_kv_enabled()};
        m_context->config_store().put(
            "merchant_kv_scheme",
            found->get_merchant_id(),
            kv_enabled ? "kv" : "database"
        );
        release_operation_lock("merchant_account_toggle_kv", merchant_id);

        app::dto::admin::ToggleKVResponse response;
        response.set_merchant_id(merchant_id_value(found->get_merchant_id()));
        response.set_kv_enabled(kv_enabled);

        m_context->metrics().record_flow("merchant_account_toggle_kv");
        m_context->metrics().counter("admin.merchant_account_toggle_kv").increment();

        auto text = serde::Ser::encode_json(response);
        m_context->events().publish("merchant_account.kv_toggled", std::string{text});
        send_json(res, send, text, true);
    }

    void handle_merchant_account_toggle_all_kv(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_error = authorize_request(req, "merchant_account_toggle_all_kv");
        if (auth_error) {
            send_error(res, send, *auth_error);
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::dto::admin::ToggleAllKVRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        if (const auto blocked = kv_soft_kill_guard()) {
            send_error(res, send, *blocked);
            return;
        }
        if (const auto duplicate = acquire_operation_lock("merchant_account_toggle_all_kv", "all")) {
            send_error(res, send, *duplicate);
            return;
        }
        const bool kv_enabled{parsed->get_kv_enabled()};
        const std::string target_scheme{kv_enabled ? "kv" : "database"};
        std::size_t total_updated{0};
        for (const auto& record : m_context->repository().list_merchant_accounts()) {
            const auto current = m_context->config_store().get_value(
                "merchant_kv_scheme", record.get_merchant_id()
            );
            if (current.has_value() && *current == target_scheme) {
                continue;
            }
            m_context->config_store().put(
                "merchant_kv_scheme", record.get_merchant_id(), target_scheme
            );
            ++total_updated;
        }
        release_operation_lock("merchant_account_toggle_all_kv", "all");

        app::dto::admin::ToggleAllKVResponse response;
        response.set_total_updated(total_updated);
        response.set_kv_enabled(kv_enabled);

        m_context->metrics().record_flow("merchant_account_toggle_all_kv");
        m_context->metrics().counter("admin.merchant_account_toggle_all_kv").increment();

        auto text = serde::Ser::encode_json(response);
        m_context->events().publish("merchant_account.kv_toggled_all", std::string{text});
        send_json(res, send, text, true);
    }

    void handle_merchant_account_transfer_keys(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_error = authorize_request(req, "merchant_account_transfer_keys");
        if (auth_error) {
            send_error(res, send, *auth_error);
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::dto::admin::MerchantKeyTransferRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        if (const auto duplicate = acquire_operation_lock("merchant_account_transfer_keys", "all")) {
            send_error(res, send, *duplicate);
            return;
        }
        const std::size_t offset{parsed->get_from()};
        const std::size_t limit{parsed->get_limit()};
        std::size_t total_transferred{0};
        const auto accounts = m_context->repository().list_merchant_accounts();
        for (std::size_t index{offset}; index < accounts.size() && total_transferred < limit; ++index) {
            auto record = accounts[index];
            const std::string new_key{m_context->ids().key_id()};
            const app::crypto::SecretString wrapped{
                app::crypto::AesWrap{}.encrypt(new_key, connector_key(record.get_merchant_id(), new_key))
            };
            record.set_payment_response_hash_key(
                app::crypto::KeyedHasher{new_key}.hash(wrapped.value())
            );
            if (!m_context->key_store().rotate(
                    "merchant_aes", record.get_merchant_id(),
                    app::crypto::SecretString{new_key}
                )) {
                m_context->key_store().put(
                    "merchant_aes", record.get_merchant_id(), "AES-256",
                    app::crypto::SecretString{new_key}
                );
            }
            if (m_context->repository().update_merchant_account(record)) {
                ++total_transferred;
            }
        }
        release_operation_lock("merchant_account_transfer_keys", "all");

        app::dto::admin::TransferKeyResponse response;
        response.set_total_transferred(total_transferred);

        m_context->metrics().record_flow("merchant_account_transfer_keys");
        m_context->metrics().counter("admin.merchant_account_transfer_keys").increment();

        auto text = serde::Ser::encode_json(response);
        m_context->events().publish("merchant_account.keys_transferred", std::string{text});
        send_json(res, send, text, true);
    }

    void handle_connector_webhook_register(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_error = authorize_request(req, "connector_webhook_register");
        if (auth_error) {
            send_error(res, send, *auth_error);
            return;
        }

        const std::string connector_id{path_part(req, 1)};
        if (connector_id.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_REQUEST, "missing connector id"));
            return;
        }
        std::string merchant_id{app::http::merchant_id_from_header{}(req)};
        if (merchant_id.empty() && path_has(req, "account")) {
            merchant_id = path_part(req, 4);
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::dto::admin::WebhookDetailsRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        std::string connector_name{connector_id};
        const auto connector = m_context->repository().find_merchant_connector_by_string(connector_id);
        if (connector.has_value() && !connector->get_connector_name().empty()) {
            connector_name = connector->get_connector_name();
        }

        if (const auto duplicate = acquire_operation_lock("connector_webhook_register", connector_id)) {
            send_error(res, send, *duplicate);
            return;
        }
        const app::crypto::SecretString password{parsed->get_webhook_password().value_or("")};
        app::store::connector_webhooks::ConnectorWebhookRecord record;
        record.set_connector_webhook_id(connector_id);
        record.set_connector(connector_name);
        record.set_merchant_id(merchant_id);
        record.set_event_type(parsed->get_webhook_version().value_or(""));
        record.set_scope(parsed->get_webhook_url().value_or(""));
        record.set_secret(
            app::crypto::AesWrap{}.encrypt(
                password.value(),
                connector_encryption_key(merchant_id, connector_id)
            )
        );
        record.set_status("active");
        const auto saved = m_context->repository().save_connector_webhook(std::move(record));
        release_operation_lock("connector_webhook_register", connector_id);

        app::dto::admin::WebhookDetailsResponse response;
        if (!saved.get_event_type().empty()) {
            response.set_webhook_version(saved.get_event_type());
        }
        if (!saved.get_scope().empty()) {
            response.set_webhook_url(saved.get_scope());
        }
        const std::string decrypted_password{
            app::crypto::AesWrap{}.decrypt(
                saved.get_secret(),
                connector_encryption_key(saved.get_merchant_id(), connector_id)
            )
        };
        if (!decrypted_password.empty()) {
            response.set_webhook_password(decrypted_password);
        }
        response.set_webhook_username(parsed->get_webhook_username());
        response.set_payment_created_enabled(parsed->get_payment_created_enabled());
        response.set_payment_succeeded_enabled(parsed->get_payment_succeeded_enabled());
        response.set_payment_failed_enabled(parsed->get_payment_failed_enabled());
        response.set_payment_statuses_enabled(parsed->get_payment_statuses_enabled());
        response.set_refund_statuses_enabled(parsed->get_refund_statuses_enabled());
        response.set_payout_statuses_enabled(parsed->get_payout_statuses_enabled());
        response.set_dispute_statuses_enabled(parsed->get_dispute_statuses_enabled());
        response.set_mandate_statuses_enabled(parsed->get_mandate_statuses_enabled());
        response.set_invoice_statuses_enabled(parsed->get_invoice_statuses_enabled());

        m_context->metrics().record_flow("connector_webhook_register");
        m_context->metrics().counter("admin.connector_webhook_register").increment();

        auto text = serde::Ser::encode_json(response);
        const std::string event_id{
            m_context->events().publish("connector_webhook.registered", std::string{text})
        };
        if (!saved.get_scope().empty()) {
            m_context->webhook_delivery().enqueue(event_id, saved.get_scope());
        }
        send_json(res, send, text, true);
    }

    void handle_organization_retrieve(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_error = authorize_request(req, "organization_retrieve");
        if (auth_error) {
            send_error(res, send, *auth_error);
            return;
        }

        const std::string organization_id{path_part(req, 1)};
        if (organization_id.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_REQUEST, "missing organization id"));
            return;
        }
        const auto found = m_context->repository().find_organization_by_string(organization_id);
        if (!found.has_value()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "organization not found"));
            return;
        }

        app::model::organization::OrganizationResponse response;
        app::model::organization::OrganizationIdValue id;
        id.set_value(found->get_organization_id());
        response.set_organization_id(std::move(id));
        response.set_organization_name(found->get_organization_name());
        if (!found->get_organization_details().empty()) {
            app::model::organization::SecretSerdeValueValue details;
            details.set_value(found->get_organization_details());
            response.set_organization_details(std::move(details));
        }
        if (!found->get_metadata().empty()) {
            app::model::organization::SecretSerdeValueValue metadata;
            metadata.set_value(found->get_metadata());
            response.set_metadata(std::move(metadata));
        }
        if (!found->get_organization_type().empty()) {
            app::model::organization::OrganizationTypeValue type;
            type.set_value(found->get_organization_type());
            response.set_organization_type(std::move(type));
        }
        if (!found->get_created_at().empty()) {
            app::model::organization::PrimitiveDateTimeValue created;
            created.set_value(found->get_created_at());
            response.set_created_at(std::move(created));
        }

        m_context->metrics().record_flow("organization_retrieve");
        m_context->metrics().counter("admin.organization_retrieve").increment();

        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void handle_retrieve_merchant_account(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_error = authorize_request(req, "retrieve_merchant_account");
        if (auth_error) {
            send_error(res, send, *auth_error);
            return;
        }

        const std::string merchant_id{path_part(req, 1)};
        if (merchant_id.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_REQUEST, "missing merchant id"));
            return;
        }
        const auto found = m_context->repository().find_merchant_account_by_string(merchant_id);
        if (!found.has_value()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "merchant account not found"));
            return;
        }

        app::dto::admin::MerchantAccountResponse response;
        response.set_merchant_id(merchant_id_value(found->get_merchant_id()));
        response.set_merchant_name(encryptable_name(found->get_merchant_name()));
        response.set_organization_id(organization_id_value(found->get_organization_id()));
        if (!found->get_return_url().empty()) {
            response.set_return_url(found->get_return_url());
        }
        if (!found->get_publishable_key().empty()) {
            response.set_publishable_key(found->get_publishable_key());
        }
        if (!found->get_locker_id().empty()) {
            response.set_locker_id(found->get_locker_id());
        }
        response.set_sub_merchants_enabled(found->get_sub_merchants_enabled());

        m_context->metrics().record_flow("retrieve_merchant_account");
        m_context->metrics().counter("admin.retrieve_merchant_account").increment();

        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void handle_merchant_account_list(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_error = authorize_request(req, "merchant_account_list");
        if (auth_error) {
            send_error(res, send, *auth_error);
            return;
        }

        std::string organization_id{
            app::http::RequestQuery{}(req).get("organization_id").value_or("")
        };
        if (organization_id.empty() && path_has(req, "organizations")) {
            organization_id = path_part(req, 2);
        }
        const std::string merchant_filter{
            app::http::RequestQuery{}(req).get("merchant_id").value_or("")
        };
        const std::size_t offset{size_query(req, "offset", 0)};
        const std::size_t limit{size_query(req, "limit", 0)};

        std::vector<app::dto::admin::MerchantAccountResponse> response;
        const auto accounts = m_context->repository().list_merchant_accounts();
        std::size_t emitted{0};
        for (const auto& account : accounts) {
            if (!organization_id.empty() && account.get_organization_id() != organization_id) {
                continue;
            }
            if (!merchant_filter.empty() && account.get_merchant_id() != merchant_filter) {
                continue;
            }
            if (emitted < offset) {
                ++emitted;
                continue;
            }
            if (limit != 0 && response.size() >= limit) {
                break;
            }
            app::dto::admin::MerchantAccountResponse item;
            item.set_merchant_id(merchant_id_value(account.get_merchant_id()));
            item.set_merchant_name(encryptable_name(account.get_merchant_name()));
            item.set_organization_id(organization_id_value(account.get_organization_id()));
            if (!account.get_publishable_key().empty()) {
                item.set_publishable_key(account.get_publishable_key());
            }
            response.push_back(std::move(item));
        }

        m_context->metrics().record_flow("merchant_account_list");
        m_context->metrics().counter("admin.merchant_account_list").increment();

        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void handle_delete_merchant_account(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_error = authorize_request(req, "delete_merchant_account");
        if (auth_error) {
            send_error(res, send, *auth_error);
            return;
        }

        const std::string merchant_id{path_part(req, 1)};
        if (merchant_id.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_REQUEST, "missing merchant id"));
            return;
        }
        const auto found = m_context->repository().find_merchant_account_by_string(merchant_id);
        if (!found.has_value()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "merchant account not found"));
            return;
        }
        const bool deleted = m_context->repository().remove_merchant_account(found->get_id());

        app::dto::admin::MerchantAccountDeleteResponse response;
        response.set_merchant_id(merchant_id_value(found->get_merchant_id()));
        response.set_deleted(deleted);

        m_context->metrics().record_flow("delete_merchant_account");
        m_context->metrics().counter("admin.delete_merchant_account").increment();

        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void handle_connector_retrieve(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_error = authorize_request(req, "connector_retrieve");
        if (auth_error) {
            send_error(res, send, *auth_error);
            return;
        }

        const std::string connector_id{path_part(req, 1)};
        if (connector_id.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_REQUEST, "missing connector id"));
            return;
        }
        const auto found = m_context->repository().find_merchant_connector_by_string(connector_id);
        if (!found.has_value()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "connector not found"));
            return;
        }

        const std::string decrypted{
            app::crypto::AesWrap{}.decrypt(
                found->get_connector_account_details(),
                connector_encryption_key(found->get_merchant_id(), connector_id)
            )
        };

        app::dto::admin::MerchantConnectorResponse response;
        response.set_connector_type(connector_type_value(found->get_connector_type()));
        response.set_connector_name(connector_value(found->get_connector_name()));
        response.set_connector_label(found->get_connector_label());
        response.set_id(connector_account_id(found->get_merchant_connector_id()));
        response.set_profile_id(profile_id(found->get_profile_id()));
        response.set_connector_account_details(secret_value(decrypted));
        response.set_disabled(found->get_disabled());
        if (!found->get_metadata().empty()) {
            response.set_metadata(secret_value(found->get_metadata()));
        }
        if (!found->get_status().empty()) {
            response.set_status(connector_status_value(found->get_status()));
        }

        m_context->metrics().record_flow("connector_retrieve");
        m_context->metrics().counter("admin.connector_retrieve").increment();

        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void handle_connector_list(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_error = authorize_request(req, "connector_list");
        if (auth_error) {
            send_error(res, send, *auth_error);
            return;
        }

        std::string merchant_id{app::http::merchant_id_from_header{}(req)};
        if (merchant_id.empty() && path_has(req, "account")) {
            merchant_id = path_part(req, 2);
        }
        std::string profile_id_filter{
            app::http::RequestQuery{}(req).get("profile_id").value_or("")
        };
        if (profile_id_filter.empty() && path_has(req, "profiles")) {
            profile_id_filter = path_part(req, 2);
        }

        std::vector<app::dto::admin::MerchantConnectorListResponse> response;
        for (const auto& record : m_context->repository().list_merchant_connectors()) {
            if (!merchant_id.empty() && record.get_merchant_id() != merchant_id) {
                continue;
            }
            if (!profile_id_filter.empty() && record.get_profile_id() != profile_id_filter) {
                continue;
            }
            app::dto::admin::MerchantConnectorListResponse item;
            item.set_connector_type(connector_type_value(record.get_connector_type()));
            item.set_connector_name(record.get_connector_name());
            item.set_connector_label(record.get_connector_label());
            item.set_merchant_connector_id(connector_account_id(record.get_merchant_connector_id()));
            item.set_profile_id(profile_id(record.get_profile_id()));
            item.set_disabled(record.get_disabled());
            item.set_status(connector_status_value(record.get_status()));
            response.push_back(std::move(item));
        }

        m_context->metrics().record_flow("connector_list");
        m_context->metrics().counter("admin.connector_list").increment();

        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void handle_connector_list_profile(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_error = authorize_request(req, "connector_list_profile");
        if (auth_error) {
            send_error(res, send, *auth_error);
            return;
        }

        std::string merchant_id{app::http::merchant_id_from_header{}(req)};
        if (merchant_id.empty() && path_has(req, "account")) {
            merchant_id = path_part(req, 3);
        }

        std::vector<app::dto::admin::MerchantConnectorListResponse> response;
        for (const auto& record : m_context->repository().list_merchant_connectors()) {
            if (!merchant_id.empty() && record.get_merchant_id() != merchant_id) {
                continue;
            }
            app::dto::admin::MerchantConnectorListResponse item;
            item.set_connector_type(connector_type_value(record.get_connector_type()));
            item.set_connector_name(record.get_connector_name());
            item.set_connector_label(record.get_connector_label());
            item.set_merchant_connector_id(connector_account_id(record.get_merchant_connector_id()));
            item.set_profile_id(profile_id(record.get_profile_id()));
            item.set_disabled(record.get_disabled());
            item.set_status(connector_status_value(record.get_status()));
            response.push_back(std::move(item));
        }

        m_context->metrics().record_flow("connector_list_profile");
        m_context->metrics().counter("admin.connector_list_profile").increment();

        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void handle_connector_delete(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_error = authorize_request(req, "connector_delete");
        if (auth_error) {
            send_error(res, send, *auth_error);
            return;
        }

        const std::string connector_id{path_part(req, 1)};
        if (connector_id.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_REQUEST, "missing connector id"));
            return;
        }
        const auto found = m_context->repository().find_merchant_connector_by_string(connector_id);
        if (!found.has_value()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "connector not found"));
            return;
        }
        const bool deleted = m_context->repository().remove_merchant_connector(found->get_id());

        app::dto::admin::MerchantConnectorDeleteResponse response;
        response.set_merchant_id(merchant_id_value(found->get_merchant_id()));
        response.set_merchant_connector_id(connector_account_id(found->get_merchant_connector_id()));
        response.set_deleted(deleted);

        m_context->metrics().record_flow("connector_delete");
        m_context->metrics().counter("admin.connector_delete").increment();

        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void handle_merchant_account_kv_status(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_error = authorize_request(req, "merchant_account_kv_status");
        if (auth_error) {
            send_error(res, send, *auth_error);
            return;
        }

        const std::string merchant_id{path_part(req, 2)};
        if (merchant_id.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_REQUEST, "missing merchant id"));
            return;
        }
        const auto found = m_context->repository().find_merchant_account_by_string(merchant_id);
        if (!found.has_value()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "merchant account not found"));
            return;
        }
        const bool kv_enabled{
            m_context->config_store()
                .get_value("merchant_kv_scheme", found->get_merchant_id())
                .value_or("database") == "kv"
        };

        app::dto::admin::ToggleKVResponse response;
        response.set_merchant_id(merchant_id_value(found->get_merchant_id()));
        response.set_kv_enabled(kv_enabled);

        m_context->metrics().record_flow("merchant_account_kv_status");
        m_context->metrics().counter("admin.merchant_account_kv_status").increment();

        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void handle_retrieve_connector_webhook(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto auth_error = authorize_request(req, "retrieve_connector_webhook");
        if (auth_error) {
            send_error(res, send, *auth_error);
            return;
        }

        const std::string connector_id{path_part(req, 1)};
        if (connector_id.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_REQUEST, "missing connector id"));
            return;
        }
        std::string merchant_id{app::http::merchant_id_from_header{}(req)};
        if (merchant_id.empty() && path_has(req, "account")) {
            merchant_id = path_part(req, 4);
        }

        const auto found = m_context->repository().find_connector_webhook_by_string(connector_id);
        if (!found.has_value()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "connector webhook not found"));
            return;
        }

        const std::string decrypted{
            app::crypto::AesWrap{}.decrypt(
                found->get_secret(),
                connector_encryption_key(found->get_merchant_id(), connector_id)
            )
        };

        app::dto::admin::WebhookDetailsResponse response;
        if (!found->get_event_type().empty()) {
            response.set_webhook_version(found->get_event_type());
        }
        if (!found->get_scope().empty()) {
            response.set_webhook_url(found->get_scope());
        }
        if (!decrypted.empty()) {
            response.set_webhook_password(decrypted);
        }

        m_context->metrics().record_flow("retrieve_connector_webhook");
        m_context->metrics().counter("admin.retrieve_connector_webhook").increment();

        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/admin"}
            .add_route(
                Route{"/organization"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_organization_create(req, res, std::move(send));
                    })
                    .summary("Hyperswitch organization_create")
            )
            .add_route(
                Route{"/v2/organizations"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_organization_create(req, res, std::move(send));
                    })
                    .summary("Hyperswitch organization_create")
            )
            .add_route(
                Route{"/organization/:id"}
                    .put([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_organization_update(req, res, std::move(send));
                    })
                    .summary("Hyperswitch organization_update")
            )
            .add_route(
                Route{"/v2/organizations/:id"}
                    .put([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_organization_update(req, res, std::move(send));
                    })
                    .summary("Hyperswitch organization_update")
            )
            .add_route(
                Route{"/organization/:id"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_organization_retrieve(req, res, std::move(send));
                    })
                    .summary("Hyperswitch organization_retrieve")
            )
            .add_route(
                Route{"/v2/organizations/:id"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_organization_retrieve(req, res, std::move(send));
                    })
                    .summary("Hyperswitch organization_retrieve")
            )
            .add_route(
                Route{"/organization/:id/convert_to_platform"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_convert_organization_to_platform(req, res, std::move(send));
                    })
                    .summary("Hyperswitch convert_organization_to_platform")
            )
            .add_route(
                Route{"/accounts"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_merchant_account_create(req, res, std::move(send));
                    })
                    .summary("Hyperswitch merchant_account_create")
            )
            .add_route(
                Route{"/v2/merchant-accounts"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_merchant_account_create(req, res, std::move(send));
                    })
                    .summary("Hyperswitch merchant_account_create")
            )
            .add_route(
                Route{"/accounts/:id"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_retrieve_merchant_account(req, res, std::move(send));
                    })
                    .summary("Hyperswitch retrieve_merchant_account")
            )
            .add_route(
                Route{"/v2/merchant-accounts/:id"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_retrieve_merchant_account(req, res, std::move(send));
                    })
                    .summary("Hyperswitch retrieve_merchant_account")
            )
            .add_route(
                Route{"/accounts/list"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_merchant_account_list(req, res, std::move(send));
                    })
                    .summary("Hyperswitch merchant_account_list")
            )
            .add_route(
                Route{"/v2/organizations/:id/merchant-accounts"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_merchant_account_list(req, res, std::move(send));
                    })
                    .summary("Hyperswitch merchant_account_list")
            )
            .add_route(
                Route{"/accounts/:id"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_update_merchant_account(req, res, std::move(send));
                    })
                    .summary("Hyperswitch update_merchant_account")
            )
            .add_route(
                Route{"/v2/merchant-accounts/:id"}
                    .put([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_update_merchant_account(req, res, std::move(send));
                    })
                    .summary("Hyperswitch update_merchant_account")
            )
            .add_route(
                Route{"/accounts/:id"}
                    .delt([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_delete_merchant_account(req, res, std::move(send));
                    })
                    .summary("Hyperswitch delete_merchant_account")
            )
            .add_route(
                Route{"/account/:merchant_id/connectors"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_connector_create(req, res, std::move(send));
                    })
                    .summary("Hyperswitch connector_create")
            )
            .add_route(
                Route{"/v2/connector-accounts"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_connector_create(req, res, std::move(send));
                    })
                    .summary("Hyperswitch connector_create")
            )
            .add_route(
                Route{"/account/:merchant_id/connectors/:merchant_connector_id"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_connector_retrieve(req, res, std::move(send));
                    })
                    .summary("Hyperswitch connector_retrieve")
            )
            .add_route(
                Route{"/v2/connector-accounts/:id"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_connector_retrieve(req, res, std::move(send));
                    })
                    .summary("Hyperswitch connector_retrieve")
            )
            .add_route(
                Route{"/account/:merchant_id/connectors"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_connector_list(req, res, std::move(send));
                    })
                    .summary("Hyperswitch connector_list")
            )
            .add_route(
                Route{"/v2/profiles/:profile_id/connector-accounts"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_connector_list(req, res, std::move(send));
                    })
                    .summary("Hyperswitch connector_list")
            )
            .add_route(
                Route{"/account/:account_id/profile/connectors"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_connector_list_profile(req, res, std::move(send));
                    })
                    .summary("Hyperswitch connector_list_profile")
            )
            .add_route(
                Route{"/account/:merchant_id/connectors/:merchant_connector_id"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_connector_update(req, res, std::move(send));
                    })
                    .summary("Hyperswitch connector_update")
            )
            .add_route(
                Route{"/v2/connector-accounts/:id"}
                    .put([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_connector_update(req, res, std::move(send));
                    })
                    .summary("Hyperswitch connector_update")
            )
            .add_route(
                Route{"/account/:merchant_id/connectors/:merchant_connector_id"}
                    .delt([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_connector_delete(req, res, std::move(send));
                    })
                    .summary("Hyperswitch connector_delete")
            )
            .add_route(
                Route{"/v2/connector-accounts/:id"}
                    .delt([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_connector_delete(req, res, std::move(send));
                    })
                    .summary("Hyperswitch connector_delete")
            )
            .add_route(
                Route{"/accounts/:id/kv"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_merchant_account_toggle_kv(req, res, std::move(send));
                    })
                    .summary("Hyperswitch merchant_account_toggle_kv")
            )
            .add_route(
                Route{"/v2/merchant-accounts/:id/kv"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_merchant_account_toggle_kv(req, res, std::move(send));
                    })
                    .summary("Hyperswitch merchant_account_toggle_kv")
            )
            .add_route(
                Route{"/accounts/kv"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_merchant_account_toggle_all_kv(req, res, std::move(send));
                    })
                    .summary("Hyperswitch merchant_account_toggle_all_kv")
            )
            .add_route(
                Route{"/accounts/:id/kv"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_merchant_account_kv_status(req, res, std::move(send));
                    })
                    .summary("Hyperswitch merchant_account_kv_status")
            )
            .add_route(
                Route{"/v2/merchant-accounts/:id/kv"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_merchant_account_kv_status(req, res, std::move(send));
                    })
                    .summary("Hyperswitch merchant_account_kv_status")
            )
            .add_route(
                Route{"/accounts/transfer"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_merchant_account_transfer_keys(req, res, std::move(send));
                    })
                    .summary("Hyperswitch merchant_account_transfer_keys")
            )
            .add_route(
                Route{"/account/:merchant_id/connectors/webhooks/:merchant_connector_id"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_connector_webhook_register(req, res, std::move(send));
                    })
                    .summary("Hyperswitch connector_webhook_register")
            )
            .add_route(
                Route{"/account/:merchant_id/connectors/webhooks/:merchant_connector_id"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_retrieve_connector_webhook(req, res, std::move(send));
                    })
                    .summary("Hyperswitch retrieve_connector_webhook")
            );
    }

private:
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

    [[nodiscard]] std::optional<core::error::AppError> authorize_request(
        interfaces::io::IRequest& req,
        std::string_view operation
    ) const
    {
        const auto context = app::auth::AuthContextBuilder{}(req);
        const auto rule = m_context->auth_rules().rule_for(operation);
        return m_context->authorizer().authorize_route(context, rule);
    }

    [[nodiscard]] std::string operation_lock_key(
        std::string_view operation,
        std::string_view resource
    ) const
    {
        return std::format("admin.{}.{}", operation, resource);
    }

    [[nodiscard]] std::optional<core::error::AppError> acquire_operation_lock(
        std::string_view operation,
        std::string_view resource
    ) const
    {
        const auto acquired = m_context->locks().acquire(
            operation_lock_key(operation, resource),
            std::int64_t{10},
            "admin-handler"
        );
        if (!acquired.has_value()) {
            return core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::DUPLICATE_REQUEST,
                "duplicate admin operation in progress"
            );
        }
        return std::nullopt;
    }

    void release_operation_lock(std::string_view operation, std::string_view resource) const
    {
        m_context->locks().release(operation_lock_key(operation, resource));
    }

    [[nodiscard]] bool is_vault_connector(std::string_view connector_name) const noexcept
    {
        return connector_name == "vgs" || connector_name == "hyperswitch_vault" ||
               connector_name == "tokenex";
    }

    [[nodiscard]] std::optional<core::error::AppError> validate_connector_create(
        std::string_view merchant_id,
        std::string_view connector_name
    ) const
    {
        if (connector_name.empty()) {
            return core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "missing connector name"
            );
        }
        const auto account = m_context->repository().find_merchant_account_by_string(merchant_id);
        if (!account.has_value()) {
            return core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::NOT_FOUND,
                "merchant account not found"
            );
        }
        const bool dummy_enabled{
            m_context->config_store().get_value("dummy_connector", "enabled").value_or("false") == "true"
        };
        if (connector_name == "dummy" && !dummy_enabled) {
            return core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "Invalid connector name"
            );
        }
        const bool vault = is_vault_connector(connector_name);
        const std::string account_type{account->get_merchant_account_type()};
        if (account_type == "connected" && vault) {
            return core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "Vault connectors can only be configured by platform or standard merchants, not connected merchants"
            );
        }
        if (account_type == "platform" && !vault) {
            return core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "Platform merchants can only configure vault connectors"
            );
        }
        if (m_context->config_store().is_killed(connector_name)) {
            return core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::CONNECTOR_ERROR,
                std::format("connector killed: {}", connector_name)
            );
        }
        return std::nullopt;
    }

    [[nodiscard]] std::string connector_encryption_key(
        std::string_view merchant,
        std::string_view connector
    ) const
    {
        const auto stored = m_context->key_store().get("connector_data_key", connector);
        return stored.has_value() ? stored->value() : connector_key(merchant, connector);
    }

    [[nodiscard]] std::optional<core::error::AppError> kv_soft_kill_guard() const
    {
        if (m_context->config_store().get_value("kv", "soft_kill").value_or("false") == "true") {
            return core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::PRECONDITION_FAILED,
                "KV soft-kill mode blocks storage scheme changes"
            );
        }
        return std::nullopt;
    }

    [[nodiscard]] std::string path_part(interfaces::io::IRequest& req, std::size_t from_end) const
    {
        const auto segments = app::http::RequestPath{}(req);
        if (from_end == 0 || from_end > segments.count()) {
            return {};
        }
        const auto part = segments.at(segments.count() - from_end);
        return part.has_value() ? std::string{*part} : std::string{};
    }

    [[nodiscard]] bool path_has(interfaces::io::IRequest& req, std::string_view expected) const
    {
        const auto segments = app::http::RequestPath{}(req);
        for (std::size_t i{0}; i < segments.count(); ++i) {
            const auto part = segments.at(i);
            if (part.has_value() && *part == expected) {
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] std::size_t size_query(
        interfaces::io::IRequest& req,
        std::string_view key,
        std::size_t fallback
    ) const
    {
        const auto raw = app::http::RequestQuery{}(req).get(key);
        if (!raw.has_value() || raw->empty()) {
            return fallback;
        }
        std::size_t value{fallback};
        const auto result = std::from_chars(raw->data(), raw->data() + raw->size(), value);
        return result.ec == std::errc{} ? value : fallback;
    }

    [[nodiscard]] std::string uuid_simple() const
    {
        std::string value{m_context->ids().uuid_v4()};
        std::erase(value, '-');
        return value;
    }

    [[nodiscard]] std::string connector_key(std::string_view merchant, std::string_view connector) const
    {
        return app::crypto::KeyedHasher{"admin.connector.key"}.hash(
            std::string{merchant} + ":" + std::string{connector}
        );
    }

    [[nodiscard]] app::dto::admin::MerchantIdValue merchant_id_value(std::string value) const
    {
        app::dto::admin::MerchantIdValue out;
        out.set_value(std::move(value));
        return out;
    }

    [[nodiscard]] app::dto::admin::OrganizationIdValue organization_id_value(std::string value) const
    {
        app::dto::admin::OrganizationIdValue out;
        out.set_value(std::move(value));
        return out;
    }

    [[nodiscard]] app::dto::admin::ConnectorValue connector_value(std::string value) const
    {
        app::dto::admin::ConnectorValue out;
        out.set_value(std::move(value));
        return out;
    }

    [[nodiscard]] app::dto::admin::ConnectorTypeValue connector_type_value(std::string value) const
    {
        app::dto::admin::ConnectorTypeValue out;
        out.set_value(std::move(value));
        return out;
    }

    [[nodiscard]] app::dto::admin::ConnectorStatusValue connector_status_value(std::string value) const
    {
        app::dto::admin::ConnectorStatusValue out;
        out.set_value(std::move(value));
        return out;
    }

    [[nodiscard]] app::dto::admin::SecretSerdeValueValue secret_value(std::string value) const
    {
        app::dto::admin::SecretSerdeValueValue out;
        out.set_value(std::move(value));
        return out;
    }

    [[nodiscard]] app::dto::admin::MerchantConnectorAccountIdValue connector_account_id(std::string value) const
    {
        app::dto::admin::MerchantConnectorAccountIdValue out;
        out.set_value(std::move(value));
        return out;
    }

    [[nodiscard]] app::dto::admin::ProfileIdValue profile_id(std::string value) const
    {
        app::dto::admin::ProfileIdValue out;
        out.set_value(std::move(value));
        return out;
    }

    [[nodiscard]] app::dto::admin::OptionalEncryptableNameValue encryptable_name(std::string value) const
    {
        app::dto::admin::OptionalEncryptableNameValue out;
        out.set_value(std::move(value));
        return out;
    }

    app::context::AppContext* m_context{nullptr};
};

} // namespace app::handlers::admin
