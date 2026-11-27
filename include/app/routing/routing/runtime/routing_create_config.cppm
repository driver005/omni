export module app_routing:routing_runtime_routing_create_config;

import std;
import core_error;
import core_serde;
import app_context;
import app_model;
import core_behavior;

import :routing_result;
import :routing_runtime_common;

export namespace app::handlers::routing {

class RoutingCreateConfigRuntime
{
public:
    explicit RoutingCreateConfigRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value> routing_create_config(
        const ::app::model::routing::RoutingConfigRequest& config_request,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        const auto& typed = config_request;
        // Required fields in upstream order (core:840-873).
        if (typed.get_name().empty()) {
            return std::unexpected{m_support.error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "Name of config not given"
            )};
        }
        if (typed.get_description().empty()) {
            return std::unexpected{m_support.error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "Description of config not given"
            )};
        }
        const auto txn = ctx_transaction_type(ctx);
        auto profile = typed.get_profile_id().get_value().empty()
                           ? ctx.profile_id
                           : typed.get_profile_id().get_value();
        if (profile.empty()) {
            return std::unexpected{m_support.error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "Profile_id not provided"
            )};
        }
        if (const auto validation_error =
                m_support.profile_validation_error(ctx_auth_profile(ctx), profile)) {
            return std::unexpected{
                m_support.error(core::error::AppErrorKind::PRECONDITION_FAILED, *validation_error)
            };
        }

        const auto algorithm_id = m_support.new_routing_id();
        auto config_json = serde::Ser::encode_json(typed);
        // Persist the transaction type on the record (app.rs passes it into the core function).
        if (txn == "payout" && !config_json.empty() && config_json.back() == '}') {
            config_json.pop_back();
            config_json += R"(,"algorithm_for":"payout"})";
        }
        auto record = m_support.config_record(
            ctx.merchant_id,
            algorithm_id,
            typed.get_name(),
            typed.get_description(),
            m_support.algorithm_name(typed.get_algorithm()),
            profile,
            config_json
        );
        record.set_algorithm_id(algorithm_id);
        m_support.upsert_config(std::move(record));

        // v1/v2 response: RoutingDictionaryRecord (foreign_into).
        auto response = m_support.dictionary_record(
            algorithm_id,
            profile,
            typed.get_name(),
            typed.get_description(),
            m_support.algorithm_name(typed.get_algorithm()),
            txn
        );
        m_support.track("routing_create_config");
        return ResponseDocument::from(response);
    }

private:
    ::app::context::AppContext& m_context;
    RoutingRuntimeSupport m_support;
};

} // namespace app::handlers::routing
