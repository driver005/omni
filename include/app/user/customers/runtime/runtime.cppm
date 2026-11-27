export module app_user:customers_runtime;

import std;
import core_behavior;
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

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    create(const CustomerCreateDto& request, const ::core::behavior::CallContext& ctx) const
    {
        return m_create.create(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    retrieve(const ::core::behavior::CallContext& ctx) const
    {
        return m_retrieve.retrieve(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    list(const ::core::behavior::CallContext& ctx) const
    {
        return m_list.list(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    list_with_count(const ::core::behavior::CallContext& ctx) const
    {
        return m_list_with_count.list_with_count(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    update(const CustomerUpdateDto& request, const ::core::behavior::CallContext& ctx) const
    {
        return m_update.update(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    delete_customer(const ::core::behavior::CallContext& ctx) const
    {
        return m_delete.delete_customer(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    get_customer_mandates(const ::core::behavior::CallContext& ctx) const
    {
        return m_mandates.get_customer_mandates(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    create_v2(const CustomerCreateDto& request, const ::core::behavior::CallContext& ctx) const
    {
        return m_create.create_v2(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    retrieve_v2(const ::core::behavior::CallContext& ctx) const
    {
        return m_retrieve.retrieve_v2(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    retrieve_by_merchant_reference_id(const ::core::behavior::CallContext& ctx) const
    {
        return m_retrieve_by_reference.retrieve_by_merchant_reference_id(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    list_v2(const ::core::behavior::CallContext& ctx) const
    {
        return m_list.list_v2(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    list_with_count_v2(const ::core::behavior::CallContext& ctx) const
    {
        return m_list_with_count.list_with_count_v2(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    update_v2(const CustomerUpdateDto& request, const ::core::behavior::CallContext& ctx) const
    {
        return m_update.update_v2(request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    delete_customer_v2(const ::core::behavior::CallContext& ctx) const
    {
        return m_delete.delete_customer_v2(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value> migrate_global_id(
        const std::vector<MigrationOutcome>& request,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_migrate.migrate_global_id(request, ctx);
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
