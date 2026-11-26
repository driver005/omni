export module app_user:user_runtime_transfer_user_key;

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

class TransferUserKeyRuntime
{
public:
    explicit TransferUserKeyRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<TransferUserKeyResult, core::error::AppError>
    transfer_user_key(const TransferUserKeyInput& input) const
    {
        std::string set_cookie{};


        const auto transferred_key_id = m_context.ids().key_id();
        std::size_t transferred{0};
        const auto source = m_context.key_store().get("jwt", "user-jwt-v2");
        if (source) {
            m_context.key_store().put("user-key-transferred", transferred_key_id, "HS256", *source);
            ++transferred;
        }
        m_support.publish_identity_event("user.key_transferred", transferred_key_id);

        const auto key = std::string{"user.transfer_user_key:"} +
                         std::to_string(input.request_context.body.size());
        m_context.cache().put(
            key,
            core::cache::to_cache_value(std::string{input.request_context.body})
        );
        const auto cached = m_context.cache().get(key);
        (void)cached;

        m_context.metrics().record_flow("transfer_user_key");
        m_context.metrics().counter("user.transfer_user_key").increment();

        using Response = ::app::model::user::UserTransferKeyResponse;
        Response response{};
        response.set_total_transferred(transferred);
        auto document = ResponseDocument::from(response);
        return TransferUserKeyResult{document, set_cookie};
    }

private:
    ::app::context::AppContext& m_context;
    UserRuntimeSupport m_support;
};

} // namespace app::handlers::user
