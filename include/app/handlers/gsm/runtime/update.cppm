export module app_handlers_gsm:runtime_update;

import std;
import core_serde;
import core_error;
import app_context;
import app_dto;
import app_store;
import app_events;
import app_metrics;

import :input;
import :result;
import :runtime_common;

export namespace app::handlers::gsm {

class GsmUpdateRuntime
{
public:
    explicit GsmUpdateRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // ── Core: update ───────────────────────────────────────────────────────
    // find_gsm_rule + to_not_found_response (core/gsm.rs:60-71).
    [[nodiscard]] std::expected<UpdateGsmRuleResult, core::error::AppError>
    update(const UpdateGsmRuleInput& input) const
    {
        const auto& in = input.request;
        if (const auto error = m_support.validate_update(in)) {
            return std::unexpected{*error};
        }

        const auto key = m_support.rule_key(
            in.get_connector(),
            in.get_flow(),
            in.get_sub_flow(),
            in.get_code(),
            in.get_message()
        );
        auto repo = m_context.repository();
        const auto stored = repo.find_gsm_rule(key);
        if (!stored) {
            return std::unexpected{m_support.not_found()};
        }
        const auto existing_opt = m_support.domain_from_record(*stored);
        if (!existing_opt) {
            return std::unexpected{core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::INTERNAL,
                "stored GSM rule could not be decoded"
            )};
        }
        const auto existing = *existing_opt;

        // <(GsmFeature, GsmFeatureData)>::foreign_from((&request, db_record))
        // (core/gsm.rs:73-76; transformers.rs:2099-2153).
        const auto inferred = m_support.infer_feature_info(in, existing);
        // GatewayStatusMappingUpdate (core/gsm.rs:98-128).
        const auto update = m_support.gateway_update_from(in, inferred);
        // TrackedGsmInterface::update_gsm_rule returns the stored domain record,
        // mapped back via foreign_into (core/gsm.rs:129-134).
        const auto updated = m_support.apply_update(existing, update);

        auto record = m_support.record_from_domain(key, updated);
        record.set_id(stored->get_id());
        if (!repo.update_gsm_rule(record)) {
            // to_not_found_response + attach_printable("Failed while updating Gsm rule")
            // (core/gsm.rs:129-133).
            return std::unexpected{m_support.not_found()};
        }

        const auto response = m_support.response_from_domain(updated);

        m_context.events().publish("gsm.rule.updated", m_support.audit_payload("updated", key));
        m_context.metrics().record_flow("GsmRuleUpdate");
        m_context.metrics().counter("GSM_RULE_UPDATED").increment();
        return UpdateGsmRuleResult{std::move(response)};
    }

private:
    app::context::AppContext& m_context;
    GsmRuntimeSupport m_support;
};

} // namespace app::handlers::gsm
