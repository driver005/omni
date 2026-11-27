export module app_routing:routing_runtime_upsert_surcharge_decision_manager_config;

import std;
import core_error;
import core_serde;
import app_context;
import app_dto;
import core_behavior;

import :routing_result;
import :routing_runtime_common;

export namespace app::handlers::routing {

class UpsertSurchargeDecisionManagerConfigRuntime
{
public:
    explicit UpsertSurchargeDecisionManagerConfigRuntime(
        ::app::context::AppContext& context
    ) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value> upsert_surcharge_decision_manager_config(
        const ::app::dto::surcharge_decision_configs::SurchargeDecisionConfigReq& request,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        const auto& typed = request;
        if (!typed.get_algorithm()) {
            return std::unexpected{m_support.error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "Program for config not given"
            )};
        }
        const auto program = typed.get_algorithm()->get_value();
        if (!m_support.valid_program(program)) {
            return std::unexpected{
                m_support.error(core::error::AppErrorKind::INVALID_REQUEST, "Invalid Request Data")
            };
        }

        const auto profile = ctx.profile_id;
        const auto key = m_support.decision_key(profile, "surcharge");
        const auto existing = m_support.find_config(key);
        ::app::dto::surcharge_decision_configs::SurchargeDecisionManagerRecord record;
        const auto timestamp = m_support.now_unix();
        if (existing) {
            auto previous = serde::Ser::deserialize<
                ::app::dto::surcharge_decision_configs::SurchargeDecisionManagerRecord>(
                "application/json",
                existing->get_config_json()
            );
            record = previous ? std::move(*previous) : record;
            if (typed.get_name()) {
                record.set_name(*typed.get_name());
            }
            record.set_modified_at(timestamp);
        } else {
            if (!typed.get_name()) {
                return std::unexpected{m_support.error(
                    core::error::AppErrorKind::INVALID_REQUEST,
                    "name of the config not found"
                )};
            }
            record.set_name(*typed.get_name());
            record.set_created_at(timestamp);
            record.set_modified_at(timestamp);
        }
        record.set_algorithm(typed.get_algorithm().value());
        record.set_merchant_surcharge_configs(typed.get_merchant_surcharge_configs());

        auto stored = m_support.config_record(
            ctx.merchant_id,
            key,
            record.get_name(),
            "surcharge decision manager",
            "surcharge",
            profile,
            serde::Ser::encode_json(record)
        );
        m_support.upsert_config(std::move(stored));
        m_support.set_active_algo_ref(profile, "surcharge_config_algo_id", key);
        m_support.track("upsert_surcharge_decision_manager_config");
        return ResponseDocument::from(record);
    }

private:
    ::app::context::AppContext& m_context;
    RoutingRuntimeSupport m_support;
};

} // namespace app::handlers::routing
