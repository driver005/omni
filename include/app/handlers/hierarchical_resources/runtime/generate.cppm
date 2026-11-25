export module app_handlers_hierarchical_resources:runtime_generate;

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

import :input;
import :result;
import :runtime_common;

export namespace app::handlers::hierarchical_resources {

class GenerateHierarchicalResourceRuntime
{
public:
    explicit GenerateHierarchicalResourceRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    /** @brief core:578-603 generate_hierarchical_resource. */
    [[nodiscard]] std::expected<GenerateHierarchicalResourceResult, core::error::AppError>
    generate(const GenerateHierarchicalResourceInput& input) const
    {
        m_context.metrics().record_flow("HierarchicalResourcesGenerate");
        m_context.metrics().counter("hierarchical_resources_generate_total").increment();

        const auto org_key = m_support.ensure_organization_key_store(input.organization_id);
        const auto keys = m_support.generate_ec_keypair_and_csr(org_key);

        // core:179 — id_type::ResourceId::default() -> "res" + alphanumeric.
        const auto resource_id = m_context.ids().payment_id("res");

        // core:155-171 — ApplePayCertificateKeyWrapperData serialized then encrypted
        // with the organization key (keymanager crypto_operation substitute: AesWrap).
        const auto private_key_payload = "{\"payment_processing_certificate_key\":" +
                                         serde::Ser::encode_json(keys.private_key_pem) + "}";
        const auto encrypted_private_key =
            app::crypto::AesWrap{}.encrypt(private_key_payload, org_key);

        ApplePayCertificateData data;
        data.status = m_support.status_csr_generated();
        ResourceData payload;
        payload.data = m_support.encode_apple_pay_certificate_data(data);
        payload.encrypted_data = encrypted_private_key;
        payload.public_key_digest = app::crypto::KeyedHasher{org_key}.hash(keys.private_key_pem);
        payload.merchant_id = input.merchant_id;

        app::store::hierarchical_resources::HierarchicalResourceRecord record{};
        record.set_resource_id(resource_id);
        record.set_scope_id(std::string{input.organization_id});
        record.set_resource_type(std::string{input.resource_type});
        record.set_data(m_support.encode_resource_data(payload));
        record.set_status(data.status);
        record.set_created_at(m_support.created_at_value());
        const auto stored = m_context.repository().create_hierarchical_resource(std::move(record));
        if (!stored) {
            return std::unexpected{stored.error()};
        }

        // core:196-211 — response data is {"apple_pay_certificate":{"csr": <PEM>}}.
        const auto response_data = "{\"" + std::string{input.resource_type} +
                                   "\":{\"csr\":" + serde::Ser::encode_json(keys.csr_pem) + "}}";
        app::dto::hierarchical_resources::GenerateHierarchicalResourceResponse response;
        response.set_id(m_support.resource_id_value(resource_id));
        response.set_data(serde::RawJson{response_data});
        return GenerateHierarchicalResourceResult{std::move(response)};
    }

private:
    app::context::AppContext& m_context;
    HierarchicalResourcesRuntimeSupport m_support;
};

} // namespace app::handlers::hierarchical_resources
