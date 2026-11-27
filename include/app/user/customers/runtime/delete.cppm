export module app_user:customers_runtime_delete;

import std;
import core_error;
import core_serde;
import app_context;
import app_store;

import :customers_input;
import :customers_runtime_common;

export namespace app::handlers::customers {

class CustomersDeleteRuntime
{
public:
    explicit CustomersDeleteRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<serde::Value, core::error::AppError>
    delete_customer(const CustomersDeleteRouteInput& input) const
    {
        auto value = delete_core(input);
        if (!value) {
            return std::unexpected{value.error()};
        }
        return std::move(*value);
    }

    [[nodiscard]] std::expected<serde::Value, core::error::AppError>
    delete_customer_v2(const CustomersDeleteRouteInput& input) const
    {
        auto value = delete_core(input);
        if (!value) {
            return std::unexpected{value.error()};
        }
        return std::move(*value);
    }

private:
    [[nodiscard]] std::expected<serde::Value, ::core::error::AppError>
    delete_core(const CustomersDeleteRouteInput& input) const
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
        const std::string merchant_id = input.merchant_id;
        if (!merchant_id.empty() && !existing->record.get_merchant_id().empty() &&
            existing->record.get_merchant_id() != merchant_id) {
            return std::unexpected{
                m_support.upstream_error("AccessForbidden", "customer belongs to another merchant")
            };
        }

        // MandateActive rejection (core/customers.rs:834-840 / 1007-1016).
        for (const auto& mandate:
             repo.find_mandates_by_customer_id(existing->record.get_customer_id())) {
            if (mandate.get_status() == "ACTIVE" || mandate.get_status() == "Active") {
                return std::unexpected{
                    m_support.upstream_error("MandateActive", "customer has an active mandate")
                };
            }
        }

        // Payment-method deletion (core/customers.rs:842-866 / 1018-1079).
        for (const auto& method: repo.list_payment_methods()) {
            if (method.get_customer_id() != existing->record.get_customer_id()) {
                continue;
            }
            if (!merchant_id.empty() && method.get_merchant_id() != merchant_id) {
                continue;
            }
            (void)repo.remove_payment_method(method.get_id());
        }

        // Encrypted `Redacted` values and DeleteStatus::Redacted (core/customers.rs:868-929 /
        // 1081-1185).
        const std::string tombstone = m_support.encrypt_pii(std::string{kRedacted});
        existing->record.set_name(tombstone);
        existing->record.set_email(tombstone);
        existing->record.set_phone(tombstone);
        existing->record.set_description(std::string{kRedacted});
        existing->record.set_metadata("");
        existing->extension.set_tax_registration_id(tombstone);
        existing->extension.set_document_number("");
        existing->extension.set_document_type("");
        existing->extension.set_phone_country_code(std::string{kRedacted});
        existing->extension.set_status("Redacted");
        for (auto& address: existing->addresses) {
            address.set_line1(tombstone);
            address.set_line2(tombstone);
            address.set_line3(tombstone);
            address.set_zip(tombstone);
            address.set_state(tombstone);
            address.set_first_name(tombstone);
            address.set_last_name(tombstone);
            address.set_city(std::string{kRedacted});
            address.set_country(std::string{kRedacted});
            address.set_origin_zip(std::string{kRedacted});
        }
        m_support.persist_update(repo, *existing);

        serde::Value::Object response{};
        if (input.version == ApiVersion::V1) {
            response["customer_id"] = serde::Value{existing->record.get_customer_id()};
        } else {
            response["id"] = serde::Value{
                existing->extension.get_global_id().empty() ? existing->record.get_customer_id()
                                                            : existing->extension.get_global_id()
            };
            const std::string& reference = existing->extension.get_merchant_reference_id();
            response["merchant_reference_id"] =
                reference.empty() ? serde::Value{} : serde::Value{reference};
        }
        response["customer_deleted"] = serde::Value{true};
        response["address_deleted"] = serde::Value{true};
        response["payment_methods_deleted"] = serde::Value{true};
        m_context.metrics().record_flow("customers.redacted");
        m_context.metrics().counter("CUSTOMER_REDACTED").increment();
        m_context.metrics().record_flow("customers.delete");
        m_context.metrics().counter("customers_delete").increment();
        return serde::Value{std::move(response)};
    }

    ::app::context::AppContext& m_context;
    CustomersRuntimeSupport m_support;
};

} // namespace app::handlers::customers
