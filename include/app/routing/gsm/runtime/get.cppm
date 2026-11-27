export module app_routing:gsm_runtime_get;

import std;
import core_serde;
import core_error;
import app_context;
import app_dto;
import app_store;
import app_events;
import app_metrics;
import core_behavior;

import :gsm_result;
import :gsm_runtime_common;

export namespace app::handlers::gsm {

class GsmGetRuntime
{
public:
    explicit GsmGetRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // ── Core: retrieve ─────────────────────────────────────────────────────
    // find_gsm_rule + to_not_found_response (core/gsm.rs:28-46).
    [[nodiscard]] core::behavior::Outcome<::app::dto::gsm::GsmResponse>
    get(const ::app::dto::gsm::GsmRetrieveRequest& in, const ::core::behavior::CallContext&) const
    {
        if (const auto error = m_support.validate_key_fields(
                in.get_connector(),
                in.get_flow(),
                in.get_sub_flow(),
                in.get_code(),
                in.get_message(),
                true
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
        const auto domain = m_support.domain_from_record(*stored);
        if (!domain) {
            return std::unexpected{core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::INTERNAL,
                "stored GSM rule could not be decoded"
            )};
        }

        const auto response = m_support.response_from_domain(*domain);

        m_context.events().publish("gsm.rule.retrieved", m_support.audit_payload("retrieved", key));
        m_context.metrics().record_flow("GsmRuleRetrieve");
        m_context.metrics().counter("GSM_RULE_RETRIEVED").increment();
        return std::move(response);
    }

private:
    ::app::context::AppContext& m_context;
    GsmRuntimeSupport m_support;
};

} // namespace app::handlers::gsm
