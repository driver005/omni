export module app_user:customers_runtime_update;

import std;
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

    [[nodiscard]] std::expected<serde::Value, core::error::AppError>
    update(const CustomersUpdateRouteInput& input) const
    {
        auto value = update_core(input);
        if (!value) {
            return std::unexpected{value.error()};
        }
        return std::move(*value);
    }

    [[nodiscard]] std::expected<serde::Value, core::error::AppError>
    update_v2(const CustomersUpdateRouteInput& input) const
    {
        auto value = update_core(input);
        if (!value) {
            return std::unexpected{value.error()};
        }
        return std::move(*value);
    }

private:
    [[nodiscard]] std::expected<serde::Value, ::core::error::AppError>
    update_core(const CustomersUpdateRouteInput& input) const
    {
        auto repo = m_context.repository();
        auto existing = input.version == ApiVersion::V1
                            ? m_support.locate_customer(repo, input.merchant_id, input.id)
                            : m_support.locate_global_customer(repo, input.id);
        if (!existing) {
            return std::unexpected{
                m_support.upstream_error("CustomerNotFound", "customer not found")
            };
        }

        // VerifyIdForUpdateCustomer::verify_id_and_get_customer_object
        // (core/customers.rs:1385-1424).
        if (input.version == ApiVersion::V1 && !m_support.is_global_customer_id_format(input.id) &&
            !existing->record.get_customer_id().empty() &&
            existing->record.get_customer_id() != input.id) {
            return std::unexpected{
                m_support.upstream_error("InvalidRequestData", "customer id mismatch")
            };
        }

        // CustomerUpdateBridge::create_domain_model_from_request / CustomerUpdate::Update
        // (core/customers.rs:1426-1684) — only fields present in the request are applied.
        const CustomerUpdateInput& effective = input.request;
        if (input.request.name) {
            existing->record.set_name(m_support.encrypt_pii(*input.request.name));
        }
        if (input.request.email) {
            existing->record.set_email(m_support.encrypt_pii(*input.request.email));
        }
        if (input.request.phone) {
            existing->record.set_phone(m_support.encrypt_pii(*input.request.phone));
        }
        if (input.request.description) {
            existing->record.set_description(*input.request.description);
        }
        if (input.request.metadata_raw) {
            existing->record.set_metadata(*input.request.metadata_raw);
        }
        if (input.request.phone_country_code) {
            existing->extension.set_phone_country_code(*input.request.phone_country_code);
        }
        if (input.request.tax_registration_id) {
            existing->extension.set_tax_registration_id(
                m_support.encrypt_pii(*input.request.tax_registration_id)
            );
        }
        if (input.request.document_details) {
            existing->extension.set_document_type(
                input.request.document_details->document_type.value_or("")
            );
            existing->extension.set_document_number(
                m_support.encrypt_pii(input.request.document_details->document_number.value_or(""))
            );
        }
        if (input.request.default_payment_method_id) {
            existing->extension.set_default_payment_method_id(
                *input.request.default_payment_method_id
            );
        }
        // AddressStructForDbUpdate::update_address_if_sent (core/customers.rs:1287-1365).
        if (input.version == ApiVersion::V1 && effective.address) {
            m_support.upsert_address(repo, *existing, *effective.address, "v1");
        } else if (input.version == ApiVersion::V2) {
            if (effective.billing_address) {
                m_support.upsert_address(repo, *existing, *effective.billing_address, "billing");
            }
            if (effective.shipping_address) {
                m_support.upsert_address(repo, *existing, *effective.shipping_address, "shipping");
            }
        }
        m_support.persist_update(repo, *existing);

        serde::Value document =
            m_support.customer_document(*existing, std::nullopt, input.request, input.version);
        m_context.metrics().record_flow("customers.update");
        m_context.metrics().counter("customers_update").increment();
        return document;
    }

    ::app::context::AppContext& m_context;
    CustomersRuntimeSupport m_support;
};

} // namespace app::handlers::customers
