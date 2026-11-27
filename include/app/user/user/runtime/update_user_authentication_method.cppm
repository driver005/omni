export module app_user:user_runtime_update_user_authentication_method;

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
import app_locking;
import app_services;

import :user_input;
import :user_result;
import :user_runtime_common;

export namespace app::handlers::user {

class UpdateUserAuthenticationMethodRuntime
{
public:
    explicit UpdateUserAuthenticationMethodRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<std::pair<serde::Value, std::string>, core::error::AppError>
    update_user_authentication_method(const UpdateUserAuthenticationMethodInput& input) const
    {
        std::string set_cookie{};


        // Port of core/user.rs:2863-2945 (update_user_authentication_method).
        // Two request variants: AuthMethod{id,auth_config} updates one record;
        // EmailDomain{owner_id,email_domain} updates every record of that owner.
        std::string auth_id{};
        std::string owner_id{};
        std::string email_domain{};
        if (const auto* object = std::get_if<interfaces::Value::Object>(&input.generic.get())) {
            const auto read_string = [object](std::string_view name)
            {
                std::string out{};
                if (const auto field = object->get(std::string{name})) {
                    if (const auto* text = std::get_if<std::string>(&field.value().get())) {
                        out = *text;
                    }
                }
                return out;
            };
            auth_id = read_string("id");
            owner_id = read_string("owner_id");
            email_domain = read_string("email_domain");
        }

        if (!auth_id.empty()) {
            auto record = m_context.repository().find_authentication_record_by_string(auth_id);
            if (!record) {
                return std::unexpected{m_support.wire_error(
                    "UR_57",
                    "Invalid Auth Method Operation: Auth method not found",
                    core::error::AppErrorKind::INVALID_REQUEST
                )};
            }
            if (!email_domain.empty()) {
                record->set_method(email_domain);
            }
            m_context.repository().update_authentication_record(*record);
        } else if (!owner_id.empty() && !email_domain.empty()) {
            for (auto& record: m_context.repository().list_authentication_records()) {
                if (record.get_user_id() == owner_id) {
                    record.set_method(email_domain);
                    m_context.repository().update_authentication_record(record);
                }
            }
        } else {
            return std::unexpected{m_support.wire_error(
                "UR_44",
                "Invalid user auth method operation",
                core::error::AppErrorKind::INVALID_REQUEST
            )};
        }

        m_context.metrics().record_flow("update_user_authentication_method");
        m_context.metrics().counter("user.update_user_authentication_method").increment();

        // Upstream StatusOk -> bare empty 200 body (services/api.rs:499,730-732).
        return std::pair{ResponseDocument::empty(), set_cookie};
    }

private:
    ::app::context::AppContext& m_context;
    UserRuntimeSupport m_support;
};

} // namespace app::handlers::user
