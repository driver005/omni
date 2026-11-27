export module app_user:customers_runtime_update;

import std;
import core_behavior;
import core_error;
import core_serde;
import app_context;
import app_store;

import :customers_input;
import :customers_runtime_common;

export namespace app::handlers::customers {

class CustomersUpdateRuntime
{
public:
    explicit CustomersUpdateRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    update(const CustomerUpdateDto& request,
        const ::core::behavior::CallContext& ctx) const
    {
        auto value = update_core(request, ctx);
        if (!value) {
            return std::unexpected{value.error()};
        }
        return std::move(*value);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    update_v2(const CustomerUpdateDto& request,
        const ::core::behavior::CallContext& ctx) const
    {
        auto value = update_core(request, ctx);
        if (!value) {
            return std::unexpected{value.error()};
        }
        return std::move(*value);
    }

private:
    [[nodiscard]] core::behavior::Outcome<serde::Value>
    update_core(const CustomerUpdateDto& request,
        const ::core::behavior::CallContext& ctx) const
    {
        auto repo = m_context.repository();
        auto existing = ctx_version(ctx) == ApiVersion::V1
                            ? m_support.locate_customer(repo, ctx_string(ctx,"merchant_id"), ctx_string(ctx,"id"))
                            : m_support.locate_global_customer(repo, ctx_string(ctx,"id"));
        if (!existing) {
            return std::unexpected{
                m_support.upstream_error("CustomerNotFound", "customer not found")
            };
        }

        // VerifyIdForUpdateCustomer::verify_id_and_get_customer_object
        // (core/customers.rs:1385-1424).
        if (ctx_version(ctx) == ApiVersion::V1 && !m_support.is_global_customer_id_format(ctx_string(ctx,"id")) &&
            !existing->record.get_customer_id().empty() &&
            existing->record.get_customer_id() != ctx_string(ctx,"id")) {
            return std::unexpected{
                m_support.upstream_error("InvalidRequestData", "customer id mismatch")
            };
        }

        // CustomerUpdateBridge::create_domain_model_from_request / CustomerUpdate::Update
        // (core/customers.rs:1426-1684) — only fields present in the request are applied.
        const CustomerUpdateDto& effective = request;
        if (request.name) {
            existing->record.set_name(m_support.encrypt_pii(*request.name));
        }
        if (request.email) {
            existing->record.set_email(m_support.encrypt_pii(*request.email));
        }
        if (request.phone) {
            existing->record.set_phone(m_support.encrypt_pii(*request.phone));
        }
        if (request.description) {
            existing->record.set_description(*request.description);
        }
        if (request.metadata_raw) {
            existing->record.set_metadata(*request.metadata_raw);
        }
        if (request.phone_country_code) {
            existing->extension.set_phone_country_code(*request.phone_country_code);
        }
        if (request.tax_registration_id) {
            existing->extension.set_tax_registration_id(
                m_support.encrypt_pii(*request.tax_registration_id)
            );
        }
        if (request.document_details) {
            existing->extension.set_document_type(
                request.document_details->document_type.value_or("")
            );
            existing->extension.set_document_number(
                m_support.encrypt_pii(request.document_details->document_number.value_or(""))
            );
        }
        if (request.default_payment_method_id) {
            existing->extension.set_default_payment_method_id(
                *request.default_payment_method_id
            );
        }
        // AddressStructForDbUpdate::update_address_if_sent (core/customers.rs:1287-1365).
        if (ctx_version(ctx) == ApiVersion::V1 && effective.address) {
            m_support.upsert_address(repo, *existing, *effective.address, "v1");
        } else if (ctx_version(ctx) == ApiVersion::V2) {
            if (effective.billing_address) {
                m_support.upsert_address(repo, *existing, *effective.billing_address, "billing");
            }
            if (effective.shipping_address) {
                m_support.upsert_address(repo, *existing, *effective.shipping_address, "shipping");
            }
        }
        m_support.persist_update(repo, *existing);

        serde::Value document =
            m_support.customer_document(*existing, std::nullopt, request, ctx_version(ctx));
        m_context.metrics().record_flow("customers.update");
        m_context.metrics().counter("customers_update").increment();
        return document;
    }

    ::app::context::AppContext& m_context;
    CustomersRuntimeSupport m_support;
};

} // namespace app::handlers::customers
