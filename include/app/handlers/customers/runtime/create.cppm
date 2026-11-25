export module app_handlers_customers:runtime_create;

import std;
import core_error;
import core_serde;
import app_context;
import app_store;

import :input;
import :result;
import :runtime_common;

export namespace app::handlers::customers {

class CustomersCreateRuntime
{
public:
    explicit CustomersCreateRuntime(app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<CustomersCreateResult, ::core::error::AppError>
    create(const CustomersCreateRouteInput& input) const
    {
        auto value = create_core(input);
        if (!value) {
            return std::unexpected{value.error()};
        }
        return CustomersCreateResult{std::move(*value)};
    }

    [[nodiscard]] std::expected<CustomersCreateV2Result, ::core::error::AppError>
    create_v2(const CustomersCreateRouteInput& input) const
    {
        auto value = create_core(input);
        if (!value) {
            return std::unexpected{value.error()};
        }
        return CustomersCreateV2Result{std::move(*value)};
    }

private:
    [[nodiscard]] std::expected<serde::Value, ::core::error::AppError>
    create_core(const CustomersCreateRouteInput& input) const
    {
        const std::string merchant_id = input.merchant_id;
        // MerchantReferenceIdForCustomer::verify_if_merchant_reference_not_present*
        // (core/customers.rs:89-104 / 472-569).
        const std::string reference =
            input.version == ApiVersion::V1
                ? input.request.customer_id.value_or(m_support.generate_merchant_reference_id())
                : input.request.merchant_reference_id.value_or(
                      input.request.customer_id.value_or(m_support.generate_merchant_reference_id())
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
        aggregate.record.set_customer_id(input.version == ApiVersion::V1 ? reference : global_id);
        aggregate.record.set_merchant_id(merchant_id);
        aggregate.record.set_created_at(m_support.now_iso8601());
        if (input.request.name) {
            aggregate.record.set_name(m_support.encrypt_pii(*input.request.name));
        }
        if (input.request.email) {
            aggregate.record.set_email(m_support.encrypt_pii(*input.request.email));
        }
        if (input.request.phone) {
            aggregate.record.set_phone(m_support.encrypt_pii(*input.request.phone));
        }
        if (input.request.description) {
            aggregate.record.set_description(*input.request.description);
        }
        if (input.request.metadata_raw) {
            aggregate.record.set_metadata(*input.request.metadata_raw);
        }

        aggregate.extension.set_global_id(global_id);
        aggregate.extension.set_merchant_reference_id(reference);
        aggregate.extension.set_version(input.version == ApiVersion::V1 ? "v1" : "v2");
        aggregate.extension.set_status("Active");
        if (input.request.phone_country_code) {
            aggregate.extension.set_phone_country_code(*input.request.phone_country_code);
        }
        if (input.request.tax_registration_id) {
            aggregate.extension.set_tax_registration_id(
                m_support.encrypt_pii(*input.request.tax_registration_id)
            );
        }
        if (input.request.document_details) {
            aggregate.extension.set_document_type(
                input.request.document_details->document_type.value_or("")
            );
            aggregate.extension.set_document_number(
                m_support.encrypt_pii(input.request.document_details->document_number.value_or(""))
            );
        }
        if (input.request.default_payment_method_id) {
            aggregate.extension.set_default_payment_method_id(
                *input.request.default_payment_method_id
            );
        }

        // AddressStructForDbEntry::encrypt_customer_address_and_set_to_db
        // (core/customers.rs:423-470).
        if (input.version == ApiVersion::V1 && input.request.address) {
            aggregate.addresses.push_back(
                m_support.to_address_record(*input.request.address, "v1")
            );
        } else if (input.version == ApiVersion::V2) {
            if (input.request.billing_address) {
                aggregate.addresses.push_back(
                    m_support.to_address_record(*input.request.billing_address, "billing")
                );
            }
            if (input.request.shipping_address) {
                aggregate.addresses.push_back(
                    m_support.to_address_record(*input.request.shipping_address, "shipping")
                );
            }
        }
        m_support.persist_create(repo, aggregate);

        serde::Value document =
            m_support.customer_document(aggregate, input.request, std::nullopt, input.version);
        m_context.metrics().record_flow("customers.create");
        m_context.metrics().counter("customers_create").increment();
        return document;
    }

    app::context::AppContext& m_context;
    CustomersRuntimeSupport m_support;
};

} // namespace app::handlers::customers
