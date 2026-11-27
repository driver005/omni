export module app_routing:routing_runtime_upsert_decision_manager_config;

import std;
import core_error;
import core_serde;
import app_context;
import app_dto;
import core_behavior;

import :routing_result;
import :routing_runtime_common;

export namespace app::handlers::routing {

class UpsertDecisionManagerConfigRuntime
{
public:
    explicit UpsertDecisionManagerConfigRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value> upsert_decision_manager_config(
        const serde::Value& body,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        const auto name = m_support.generic_string(body, "name");
        const auto program = m_support.generic_string(body, "program", "algorithm");
        if (program.empty()) {
            return std::unexpected{m_support.error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "Program for config not given"
            )};
        }
        if (!m_support.valid_program(program)) {
            return std::unexpected{
                m_support.error(core::error::AppErrorKind::INVALID_REQUEST, "Invalid Request Data")
            };
        }

        const auto profile = ctx.profile_id;
        if (ctx_is_v2(ctx)) {
            if (const auto validation_error =
                    m_support.profile_validation_error(ctx_auth_profile(ctx), profile)) {
                return std::unexpected{m_support.error(
                    core::error::AppErrorKind::PRECONDITION_FAILED,
                    *validation_error
                )};
            }
        }
        const auto key = m_support.decision_key(profile, "decision");
        const auto existing = m_support.find_config(key);
        ::app::dto::conditional_configs::DecisionManagerRecord record;
        const auto timestamp = m_support.now_unix();
        if (existing) {
            auto previous =
                serde::Ser::deserialize<::app::dto::conditional_configs::DecisionManagerRecord>(
                    "application/json",
                    existing->get_config_json()
                );
            record = previous ? std::move(*previous) : record;
            record.set_modified_at(timestamp);
        } else {
            if (name.empty()) {
                return std::unexpected{m_support.error(
                    core::error::AppErrorKind::INVALID_REQUEST,
                    "name of the config not found"
                )};
            }
            record.set_name(name);
            record.set_created_at(timestamp);
            record.set_modified_at(timestamp);
        }
        if (!name.empty()) {
            record.set_name(name);
        }
        record.set_program(m_support.wrap<::app::dto::conditional_configs::ProgramValue>(program));

        auto stored = m_support.config_record(
            ctx.merchant_id,
            key,
            record.get_name(),
            "decision manager",
            "decision",
            profile,
            serde::Ser::encode_json(record)
        );
        m_support.upsert_config(std::move(stored));
        m_support.set_active_algo_ref(profile, "config_algo_id", key);
        m_support.track("upsert_decision_manager_config");
        return ResponseDocument::from(record);
    }

private:
    ::app::context::AppContext& m_context;
    RoutingRuntimeSupport m_support;
};

} // namespace app::handlers::routing
