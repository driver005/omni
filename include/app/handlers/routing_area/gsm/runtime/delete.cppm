export module app_routing:gsm_runtime_delete;

import std;
import core_serde;
import core_error;
import app_context;
import app_dto;
import app_store;
import app_events;
import app_metrics;

import :gsm_input;
import :gsm_result;
import :gsm_runtime_common;

export namespace app::handlers::gsm {

class GsmDeleteRuntime
{
public:
    explicit GsmDeleteRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // ── Core: delete ───────────────────────────────────────────────────────
    // delete_gsm_rule + to_not_found_response (core/gsm.rs:138-163).
    [[nodiscard]] std::expected<DeleteGsmRuleResult, core::error::AppError>
    remove(const DeleteGsmRuleInput& input) const
    {
        const auto& in = input.request;
        if (const auto error = m_support.validate_key_fields(
                in.get_connector(),
                in.get_flow(),
                in.get_sub_flow(),
                in.get_code(),
                in.get_message(),
                false
            )) {
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
        if (!repo.remove_gsm_rule(stored->get_id())) {
            // is_deleted == false -> InternalServerError "Something went wrong"
            // (core/gsm.rs:164-178; api_error_response.rs:58-59).
            return std::unexpected{core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::INTERNAL,
                "Something went wrong"
            )};
        }

        ::app::dto::gsm::GsmDeleteResponse response;
        response.set_gsm_rule_delete(true);
        response.set_connector(in.get_connector());
        response.set_flow(in.get_flow());
        response.set_sub_flow(in.get_sub_flow());
        response.set_code(in.get_code());

        m_context.events().publish("gsm.rule.deleted", m_support.audit_payload("deleted", key));
        m_context.metrics().record_flow("GsmRuleDelete");
        m_context.metrics().counter("GSM_RULE_DELETED").increment();
        return DeleteGsmRuleResult{std::move(response)};
    }

private:
    ::app::context::AppContext& m_context;
    GsmRuntimeSupport m_support;
};

} // namespace app::handlers::gsm
