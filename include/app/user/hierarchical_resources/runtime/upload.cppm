export module app_user:hierarchical_resources_runtime_upload;

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

import :hierarchical_resources_input;
import :hierarchical_resources_result;
import :hierarchical_resources_runtime_common;

export namespace app::handlers::hierarchical_resources {

class UploadHierarchicalResourceRuntime
{
public:
    explicit UploadHierarchicalResourceRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    /** @brief core:630-668 + ApplePayCertificateResource::upload (:305-367). */
    [[nodiscard]] std::expected<
        ::app::dto::hierarchical_resources::UploadCertificateResponse,
        core::error::AppError>
    upload(const UploadHierarchicalResourceInput& input) const
    {
        m_context.metrics().record_flow("HierarchicalResourcesUpload");
        m_context.metrics().counter("hierarchical_resources_upload_total").increment();

        // core:639-646 — find_resource_scope_id + authorize_scope_id_belongs_to_org.
        const auto stored =
            m_context.repository().find_hierarchical_resource_by_string(input.resource_id);
        if (!stored.has_value()) {
            return std::unexpected{
                m_support.make_error(core::error::AppErrorKind::NOT_FOUND, "resource not found")
            };
        }
        if (!m_support.authorize_scope_id_belongs_to_org(
                stored->get_scope_id(),
                input.organization_id
            )) {
            return std::unexpected{m_support.make_error(
                core::error::AppErrorKind::FORBIDDEN,
                "resource belongs to a different organization"
            )};
        }

        const auto org_key = m_support.ensure_organization_key_store(input.organization_id);
        auto payload = m_support.decode_resource_data(stored->get_data());
        if (!input.merchant_id.empty() && !payload.merchant_id.empty() &&
            payload.merchant_id != input.merchant_id) {
            return std::unexpected{m_support.make_error(
                core::error::AppErrorKind::FORBIDDEN,
                "resource belongs to a different merchant"
            )};
        }

        // core:323 — base64 DER decode + X509::from_der (local OpenSSL substitute).
        auto parsed_cert = m_support.parse_certificate_from_base64_der(input.certificate, org_key);
        if (!parsed_cert) {
            return std::unexpected{std::move(parsed_cert.error())};
        }
        // core:325-336 — cert/key match. The local substitute verifies a structural
        // binding (no EC public-key DER available without OpenSSL); see helper.
        if (!m_support.certificate_matches_private_key(*parsed_cert, payload)) {
            return std::unexpected{m_support.make_error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "Uploaded certificate does not match the generated key"
            )};
        }
        // core:338-341 — CN extraction, rsplit(':')+trim, InvalidRequestData on miss.
        auto identifier = m_support.parse_apple_merchant_identifier(*parsed_cert);
        if (!identifier.has_value() || identifier->empty()) {
            return std::unexpected{m_support.make_error(
                core::error::AppErrorKind::INVALID_REQUEST,
                "Could not parse Apple merchant identifier from certificate"
            )};
        }

        ApplePayCertificateData data;
        data.status = m_support.status_active();
        data.apple_merchant_identifier = *identifier;
        data.payment_processing_certificate = input.certificate;
        payload.data = m_support.encode_apple_pay_certificate_data(data);

        auto updated = *stored;
        updated.set_data(m_support.encode_resource_data(payload));
        updated.set_status(data.status);
        if (!m_context.repository().update_hierarchical_resource(updated)) {
            return std::unexpected{m_support.make_error(
                core::error::AppErrorKind::INTERNAL_ERROR,
                "failed to update resource"
            )};
        }

        ::app::dto::hierarchical_resources::UploadCertificateResponse response;
        response.set_id(m_support.resource_id_value(std::string{input.resource_id}));
        response.set_created_at(m_support.datetime_value(updated.get_created_at()));
        return std::move(response);
    }

private:
    ::app::context::AppContext& m_context;
    HierarchicalResourcesRuntimeSupport m_support;
};

} // namespace app::handlers::hierarchical_resources
