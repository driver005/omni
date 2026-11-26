export module app_user:user_runtime_clone_connector;

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

class CloneConnectorRuntime
{
public:
    explicit CloneConnectorRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<CloneConnectorResult, core::error::AppError>
    clone_connector(const CloneConnectorInput& input) const
    {
        std::string set_cookie{};


        const auto source_id = input.request.get_source_mca_id().get_value();
        const auto source = m_context.repository().find_merchant_connector_by_string(source_id);
        if (!source) {
            return std::unexpected{m_support.wire_error(
                "UR_58",
                "Invalid Clone Connector Operation: source not found",
                core::error::AppErrorKind::INVALID_REQUEST
            )};
        }
        ::app::store::merchant_connectors::MerchantConnectorRecord clone = *source;
        clone.set_id(0);
        clone.set_merchant_connector_id(m_context.ids().payment_id("mca_"));
        clone.set_profile_id(input.request.get_destination_profile_id().get_value());
        if (input.request.get_connector_label()) {
            clone.set_connector_label(*input.request.get_connector_label());
        }
        clone.set_created_at(m_support.now_string());
        // Capture the cloned row before moving it into the repository; upstream
        // returns the newly created MerchantConnectorResponse on the wire.
        const std::string cloned_id = clone.get_merchant_connector_id();
        const std::string cloned_name = clone.get_connector_name();
        const std::string cloned_type = clone.get_connector_type();
        const std::string cloned_label = clone.get_connector_label();
        const std::string cloned_profile = clone.get_profile_id();
        const std::string cloned_status = clone.get_status();
        m_context.repository().save_merchant_connector(std::move(clone));
        m_support.publish_identity_event("user.connector_cloned", source_id);

        m_context.metrics().record_flow("clone_connector");
        m_context.metrics().counter("user.clone_connector").increment();

        // Upstream api_models::admin::MerchantConnectorResponse wire shape,
        // built as a typed document with insertion order matching the wire order.
        serde::Value::Object object{};
        object["connector_type"] = serde::Value{cloned_type};
        object["connector_name"] = serde::Value{cloned_name};
        object["connector_label"] = serde::Value{cloned_label};
        object["id"] = serde::Value{cloned_id};
        object["profile_id"] = serde::Value{cloned_profile};
        object["connector_account_details"] = ResponseDocument::empty();
        object["status"] = serde::Value{cloned_status};
        return CloneConnectorResult{serde::Value{std::move(object)}, set_cookie};
    }

private:
    ::app::context::AppContext& m_context;
    UserRuntimeSupport m_support;
};

} // namespace app::handlers::user
