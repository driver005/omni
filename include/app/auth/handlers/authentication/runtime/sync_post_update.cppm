export module app_auth:authentication_runtime_sync_post_update;

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
import app_context_core;
import app_http;
import app_crypto;
import app_metrics;
import app_context_core;
import app_connector;
import app_events;
import app_workflow;
import app_keystore;

import :authentication_input;
import :authentication_result;
import :authentication_runtime_common;

export namespace app::handlers::authentication {

class AuthenticationSyncPostUpdateRuntime
{
public:
    explicit AuthenticationSyncPostUpdateRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // ───────────────────────── Core: post-sync ─────────────────────────
    // unified_authentication_service.rs:2386-2496 authentication_post_sync_core.
    [[nodiscard]] std::expected<AuthenticationSyncPostUpdateResult, core::error::AppError>
    sync_post_update(const AuthenticationSyncPostUpdateInput& input) const
    {
        auto record = m_support.find_authentication(input.authentication_id);
        if (!record.has_value()) {
            return std::unexpected{m_support.not_found_error(input.authentication_id)};
        }
        // :2413 ensure_not_terminal_status(trans_status).
        if (const auto err = m_support.ensure_not_terminal_status(*record)) {
            return std::unexpected{*err};
        }
        // :2436-2466 post_authentication + update_trackers.
        const auto outcome = m_support.perform_post_authentication(*record, AuthFlowKind::Merchant);
        m_support.update_trackers_from_outcome(*record, outcome);
        m_support.save_authentication(*record);
        m_support.update_authentication_status(input.authentication_id, record->get_status());
        m_context.metrics().record_flow("AuthenticationSyncPostUpdate");

        // :2481-2495 get_handle_response_url_for_modular_authentication -> JsonForRedirection.
        // Wire shape: HTTP 302 + RedirectionResponse JSON (api_models/payments.rs:10344-10350).
        const std::string return_url = record->get_return_url().empty()
                                           ? std::string{"/authentication/"} + input.merchant_id +
                                                 "/" + input.authentication_id + "/redirect"
                                           : record->get_return_url();
        const std::string status{m_support.trans_status_wire(record->get_trans_status())};
        // Vec<(String, Secret<String>)> wire shape:
        // [["status","Y"],["authentication_id","..."],...].
        const auto pair_value = [](std::string_view key, std::string_view value)
        {
            serde::Value::Array pair{};
            pair.emplace_back(std::string{key});
            pair.emplace_back(std::string{value});
            return serde::Value{std::move(pair)};
        };
        serde::Value::Array params{};
        params.push_back(pair_value("status", status));
        params.push_back(pair_value("authentication_id", input.authentication_id));
        params.push_back(pair_value("authentication_client_secret", record->get_client_secret()));
        params.push_back(pair_value("amount", record->get_amount()));
        params.push_back(pair_value("manual_retry_allowed", "false"));
        JsonObject body{};
        body.str("return_url", return_url);
        body.val("params", serde::Value{std::move(params)});
        body.str(
            "return_url_with_query_params",
            return_url + "?" +
                m_support.query_from_params(
                    status,
                    input.authentication_id,
                    record->get_client_secret(),
                    record->get_amount()
                )
        );
        body.str("http_method", "GET");
        body.val("headers", serde::Value{serde::Value::Array{}});
        const std::string url_with_query = return_url + "?" +
                                           m_support.query_from_params(
                                               status,
                                               input.authentication_id,
                                               record->get_client_secret(),
                                               record->get_amount()
                                           );
        return AuthenticationSyncPostUpdateResult{body.done(), url_with_query};
    }

private:
    ::app::context::AppContext& m_context;
    AuthenticationRuntimeSupport m_support;
};

} // namespace app::handlers::authentication
