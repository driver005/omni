export module app_routing:gsm_runtime_create;

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

class GsmCreateRuntime
{
public:
    explicit GsmCreateRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    // ── Core: create ───────────────────────────────────────────────────────
    // Upstream gsm_rule.foreign_into() then add_gsm_rule (core/gsm.rs:14-24,
    // transformers.rs:2055-2078).
    [[nodiscard]] std::expected<CreateGsmRuleResult, core::error::AppError>
    create(const CreateGsmRuleInput& input) const
    {
        const auto& in = input.request;
        if (const auto error = m_support.validate_create(in)) {
            return std::unexpected{*error};
        }

        const auto key = m_support.rule_key(
            in.get_connector(),
            in.get_flow(),
            in.get_sub_flow(),
            in.get_code(),
            in.get_message()
        );
        const auto domain = m_support.gateway_map_from_create(in);

        auto repo = m_context.repository();
        auto created = repo.create_gsm_rule(m_support.record_from_domain(key, domain));
        if (!created) {
            // to_duplicate_response: DB unique violation -> GenericDuplicateError
            // (core/gsm.rs:21-23; db/gsm.rs:63-68). Upstream maps that error to
            // AER::BadRequest (HTTP 400), api_error_response.rs:687-689.
            if (created.error().get_kind() == core::error::AppErrorKind::ALREADY_EXISTS) {
                return std::unexpected{core::error::AppErrorBuilder{}.operator()(
                    core::error::AppErrorKind::INVALID_REQUEST,
                    "GSM with given key already exists in our records"
                )};
            }
            return std::unexpected{created.error()};
        }

        // Domain record -> GsmResponse via foreign_into (transformers.rs:2155-2186).
        const auto persisted = m_support.domain_from_record(*created);
        const auto response = m_support.response_from_domain(persisted ? *persisted : domain);

        m_context.events().publish("gsm.rule.created", m_support.audit_payload("created", key));
        m_context.metrics().record_flow("GsmRuleCreate");
        m_context.metrics().counter("GSM_RULE_CREATED").increment();
        return CreateGsmRuleResult{std::move(response)};
    }

private:
    ::app::context::AppContext& m_context;
    GsmRuntimeSupport m_support;
};

} // namespace app::handlers::gsm
