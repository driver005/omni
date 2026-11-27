export module app_user:customers_runtime;

import std;
import core_error;
import core_serde;
import app_context;

import :customers_input;
import :customers_runtime_common;
import :customers_runtime_create;
import :customers_runtime_retrieve;
import :customers_runtime_retrieve_by_merchant_reference_id;
import :customers_runtime_list;
import :customers_runtime_list_with_count;
import :customers_runtime_update;
import :customers_runtime_delete;
import :customers_runtime_get_customer_mandates;
import :customers_runtime_migrate_global_id;

export namespace app::handlers::customers {

class CustomersRuntime
{
public:
    explicit CustomersRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_create{context},
        m_retrieve{context},
        m_retrieve_by_reference{context},
        m_list{context},
        m_list_with_count{context},
        m_update{context},
        m_delete{context},
        m_mandates{context},
        m_migrate{context}
    {
    }

    [[nodiscard]] std::expected<serde::Value, core::error::AppError>
    create(const CustomersCreateRouteInput& input) const
    {
        return m_create.create(input);
    }

    [[nodiscard]] std::expected<serde::Value, core::error::AppError>
    retrieve(const CustomersRetrieveRouteInput& input) const
    {
        return m_retrieve.retrieve(input);
    }

    [[nodiscard]] std::expected<serde::Value, core::error::AppError>
    list(const CustomersListRouteInput& input) const
    {
        return m_list.list(input);
    }

    [[nodiscard]] std::expected<serde::Value, core::error::AppError>
    list_with_count(const CustomersListWithCountRouteInput& input) const
    {
        return m_list_with_count.list_with_count(input);
    }

    [[nodiscard]] std::expected<serde::Value, core::error::AppError>
    update(const CustomersUpdateRouteInput& input) const
    {
        return m_update.update(input);
    }

    [[nodiscard]] std::expected<serde::Value, core::error::AppError>
    delete_customer(const CustomersDeleteRouteInput& input) const
    {
        return m_delete.delete_customer(input);
    }

    [[nodiscard]] std::expected<serde::Value, core::error::AppError>
    get_customer_mandates(const CustomersMandatesRouteInput& input) const
    {
        return m_mandates.get_customer_mandates(input);
    }

    [[nodiscard]] std::expected<serde::Value, core::error::AppError>
    create_v2(const CustomersCreateRouteInput& input) const
    {
        return m_create.create_v2(input);
    }

    [[nodiscard]] std::expected<serde::Value, core::error::AppError>
    retrieve_v2(const CustomersRetrieveRouteInput& input) const
    {
        return m_retrieve.retrieve_v2(input);
    }

    [[nodiscard]] std::expected<serde::Value, core::error::AppError>
    retrieve_by_merchant_reference_id(const CustomersRetrieveByReferenceRouteInput& input) const
    {
        return m_retrieve_by_reference.retrieve_by_merchant_reference_id(input);
    }

    [[nodiscard]] std::expected<serde::Value, core::error::AppError>
    list_v2(const CustomersListRouteInput& input) const
    {
        return m_list.list_v2(input);
    }

    [[nodiscard]] std::expected<serde::Value, core::error::AppError>
    list_with_count_v2(const CustomersListWithCountRouteInput& input) const
    {
        return m_list_with_count.list_with_count_v2(input);
    }

    [[nodiscard]] std::expected<serde::Value, core::error::AppError>
    update_v2(const CustomersUpdateRouteInput& input) const
    {
        return m_update.update_v2(input);
    }

    [[nodiscard]] std::expected<serde::Value, core::error::AppError>
    delete_customer_v2(const CustomersDeleteRouteInput& input) const
    {
        return m_delete.delete_customer_v2(input);
    }

    [[nodiscard]] std::expected<serde::Value, core::error::AppError>
    migrate_global_id(const CustomersMigrateRouteInput& input) const
    {
        return m_migrate.migrate_global_id(input);
    }

private:
    ::app::context::AppContext& m_context;
    CustomersCreateRuntime m_create;
    CustomersRetrieveRuntime m_retrieve;
    CustomersRetrieveByReferenceRuntime m_retrieve_by_reference;
    CustomersListRuntime m_list;
    CustomersListWithCountRuntime m_list_with_count;
    CustomersUpdateRuntime m_update;
    CustomersDeleteRuntime m_delete;
    CustomersMandatesRuntime m_mandates;
    CustomersMigrateRuntime m_migrate;
};

} // namespace app::handlers::customers
