export module app_payment:disputes_runtime_attach_dispute_evidence;

import std;
import interfaces;
import core_router;
import core_serde;
import core_cache;
import core_database;
import core_error;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import app_auth;
import app_http;
import app_metrics;
import app_store;
import app_connector;
import app_workflow;
import app_events;
import app_locking;
import app_services;

import :disputes_input;
import :disputes_result;
import :disputes_runtime_common;

export namespace app::handlers::disputes {

class DisputesAttachDisputeEvidenceRuntime
{
public:
    explicit DisputesAttachDisputeEvidenceRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<::app::dto::files::CreateFileResponse, Error>
    attach_dispute_evidence(const AttachDisputeEvidenceInput& input) const
    {
        m_context.metrics().record_flow("dispute_attach_evidence");
        const auto& scope = input.scope;
        const auto& request = input.request;
        const std::string dispute_id = request.get_dispute_id().value_or(std::string{});
        if (dispute_id.empty()) {
            return std::unexpected{
                core::error::AppErrorBuilder{}(Kind::INVALID_REQUEST, "MissingDisputeId")
            };
        }
        auto dispute =
            m_support.find_dispute_by_processor_merchant_id_dispute_id(scope, dispute_id);
        if (!dispute) {
            return std::unexpected{m_support.dispute_not_found(dispute_id)};
        }
        if (const auto invalid = m_support.validate_profile_id_from_auth_layer(scope, *dispute)) {
            return std::unexpected{*invalid};
        }
        if (m_support.canonical_stage(dispute->get_stage()) != "dispute" ||
            m_support.canonical_status(dispute->get_status()) != "dispute_opened") {
            return std::unexpected{m_support.status_validation_failed(
                std::format(
                    "Evidence cannot be attached because the dispute is in {} stage and has {} "
                    "status",
                    m_support.canonical_stage(dispute->get_stage()),
                    m_support.canonical_status(dispute->get_status())
                )
            )};
        }

        m_context.metrics().counter("disputes_attach_evidence_total").increment();
        const std::string lock_key = std::string{"dispute:attach:"} + dispute_id;
        const auto lock =
            m_context.locks().acquire(lock_key, std::int64_t{30}, "dispute_attach_evidence");
        if (!lock) {
            return std::unexpected{core::error::AppErrorBuilder{}(
                Kind::DUPLICATE_REQUEST,
                "dispute evidence upload already in progress"
            )};
        }

        // files::files_create_core substitute: persist file metadata locally.
        std::string file_id = request.get_file_id().value_or(std::string{});
        if (file_id.empty()) {
            file_id = m_context.ids().payment_id("file_");
        }
        const std::string file_name = request.get_file_name().value_or(file_id);
        ::app::store::files::FileRecord file{};
        file.set_file_id(file_id);
        file.set_merchant_id(
            dispute->get_merchant_id().empty() ? scope.merchant_id : dispute->get_merchant_id()
        );
        file.set_profile_id(
            dispute->get_profile_id().empty() ? scope.profile_id : dispute->get_profile_id()
        );
        file.set_file_name(file_name);
        file.set_file_type(
            request.get_file_type().value_or(std::string{"application/octet-stream"})
        );
        file.set_file_size(request.get_file_size());
        file.set_purpose("dispute_evidence");
        file.set_status("ACTIVE");
        file.set_created_at(m_support.now_string());
        m_context.repository().save_file(file);

        // transformers::update_dispute_evidence — set the matching evidence slot.
        auto evidence = m_support.get_dispute_evidence_vec(dispute_id);
        const auto type =
            m_support.evidence_type_from(request.get_evidence_type().value_or(std::string{}));
        evidence = m_support.update_dispute_evidence(evidence, type, file_id);
        m_context.config_store()
            .put("dispute_evidence", dispute_id, m_support.encode_evidence(evidence));
        m_context.repository().update_dispute(*dispute);
        m_context.locks().release(lock_key);

        ::app::dto::files::CreateFileResponse response{};
        response.set_file_id(file_id);
        return ::app::dto::files::CreateFileResponse{response};
    }

private:
    ::app::context::AppContext& m_context;
    DisputesRuntimeSupport m_support;
};

} // namespace app::handlers::disputes
