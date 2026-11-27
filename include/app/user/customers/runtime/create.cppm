export module app_user:customers_runtime_create;

import std;
import core_behavior;
import core_error;
import core_serde;
import app_context;
import app_store;

import :customers_input;
import :customers_runtime_common;

export namespace app::handlers::customers {

class CustomersCreateRuntime
{
public:
    explicit CustomersCreateRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    create(const CustomerCreateDto& request,
        const ::core::behavior::CallContext& ctx) const
    {
        auto value = create_core(request, ctx);
        if (!value) {
            return std::unexpected{value.error()};
        }
        return std::move(*value);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    create_v2(const CustomerCreateDto& request,
        const ::core::behavior::CallContext& ctx) const
    {
        auto value = create_core(request, ctx);
        if (!value) {
            return std::unexpected{value.error()};
        }
        return std::move(*value);
    }

private:
    [[nodiscard]] core::behavior::Outcome<serde::Value>
    create_core(const CustomerCreateDto& request,
        const ::core::behavior::CallContext& ctx) const
    {
        const std::string merchant_id = ctx_string(ctx,"merchant_id");
        // MerchantReferenceIdForCustomer::verify_if_merchant_reference_not_present*
        // (core/customers.rs:89-104 / 472-569).
        const std::string reference =
            ctx_version(ctx) == ApiVersion::V1
                ? request.customer_id.value_or(m_support.generate_merchant_reference_id())
                : request.merchant_reference_id.value_or(
                      request.customer_id.value_or(m_support.generate_merchant_reference_id())
                  );
        if (reference.empty()) {
            return std::unexpected{
                m_support.upstream_error("InternalServerError", "missing customer reference")
            };
        }
        auto repo = m_context.repository();
        if (m_support.find_by_reference(repo, reference, merchant_id)) {
            return std::unexpected{
                m_support.upstream_error("CustomerAlreadyExists", "customer already exists")
            };
        }

        // CustomerCreateBridge::create_domain_model_from_request (core/customers.rs:152-421).
        CustomerAggregate aggregate{};
        const std::string global_id = m_support.generate_global_customer_id();
        aggregate.record.set_customer_id(ctx_version(ctx) == ApiVersion::V1 ? reference : global_id);
        aggregate.record.set_merchant_id(merchant_id);
        aggregate.record.set_created_at(m_support.now_iso8601());
        if (request.name) {
            aggregate.record.set_name(m_support.encrypt_pii(*request.name));
        }
        if (request.email) {
            aggregate.record.set_email(m_support.encrypt_pii(*request.email));
        }
        if (request.phone) {
            aggregate.record.set_phone(m_support.encrypt_pii(*request.phone));
        }
        if (request.description) {
            aggregate.record.set_description(*request.description);
        }
        if (request.metadata_raw) {
            aggregate.record.set_metadata(*request.metadata_raw);
        }

        aggregate.extension.set_global_id(global_id);
        aggregate.extension.set_merchant_reference_id(reference);
        aggregate.extension.set_version(ctx_version(ctx) == ApiVersion::V1 ? "v1" : "v2");
        aggregate.extension.set_status("Active");
        if (request.phone_country_code) {
            aggregate.extension.set_phone_country_code(*request.phone_country_code);
        }
        if (request.tax_registration_id) {
            aggregate.extension.set_tax_registration_id(
                m_support.encrypt_pii(*request.tax_registration_id)
            );
        }
        if (request.document_details) {
            aggregate.extension.set_document_type(
                request.document_details->document_type.value_or("")
            );
            aggregate.extension.set_document_number(
                m_support.encrypt_pii(request.document_details->document_number.value_or(""))
            );
        }
        if (request.default_payment_method_id) {
            aggregate.extension.set_default_payment_method_id(
                *request.default_payment_method_id
            );
        }

        // AddressStructForDbEntry::encrypt_customer_address_and_set_to_db
        // (core/customers.rs:423-470).
        if (ctx_version(ctx) == ApiVersion::V1 && request.address) {
            aggregate.addresses.push_back(
                m_support.to_address_record(*request.address, "v1")
            );
        } else if (ctx_version(ctx) == ApiVersion::V2) {
            if (request.billing_address) {
                aggregate.addresses.push_back(
                    m_support.to_address_record(*request.billing_address, "billing")
                );
            }
            if (request.shipping_address) {
                aggregate.addresses.push_back(
                    m_support.to_address_record(*request.shipping_address, "shipping")
                );
            }
        }
        m_support.persist_create(repo, aggregate);

        serde::Value document =
            m_support.customer_document(aggregate, request, std::nullopt, ctx_version(ctx));
        m_context.metrics().record_flow("customers.create");
        m_context.metrics().counter("customers_create").increment();
        return document;
    }

    ::app::context::AppContext& m_context;
    CustomersRuntimeSupport m_support;
};

} // namespace app::handlers::customers
