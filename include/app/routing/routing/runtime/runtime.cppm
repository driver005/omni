export module app_routing:routing_runtime;

import std;
import core_serde;
import core_error;
import app_context;
import core_behavior;

import :routing_result;
import :routing_runtime_common;
import :routing_runtime_routing_entry;
import :routing_runtime_routing_create_config;
import :routing_runtime_routing_link_config;
import :routing_runtime_routing_retrieve_config;
import :routing_runtime_list_routing_configs;
import :routing_runtime_list_routing_configs_for_profile;
import :routing_runtime_routing_unlink_config;
import :routing_runtime_routing_update_default_config;
import :routing_runtime_routing_retrieve_default_config;
import :routing_runtime_upsert_surcharge_decision_manager_config;
import :routing_runtime_delete_surcharge_decision_manager_config;
import :routing_runtime_retrieve_surcharge_decision_manager_config;
import :routing_runtime_upsert_decision_manager_config;
import :routing_runtime_delete_decision_manager_config;
import :routing_runtime_retrieve_decision_manager_config;
import :routing_runtime_routing_retrieve_linked_config;
import :routing_runtime_routing_retrieve_default_config_for_profiles;
import :routing_runtime_routing_update_default_config_for_profile;
import :routing_runtime_create_success_based_routing;
import :routing_runtime_create_elimination_routing;
import :routing_runtime_contract_based_routing_setup_config;
import :routing_runtime_set_dynamic_routing_volume_split;
import :routing_runtime_get_dynamic_routing_volume_split;
import :routing_runtime_routing_migration_status;
import :routing_runtime_migrate_routing_rules;

export namespace app::handlers::routing {

class RoutingRuntime
{
public:
    explicit RoutingRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_routing_entry{context},
        m_routing_create_config{context},
        m_routing_link_config{context},
        m_routing_retrieve_config{context},
        m_list_routing_configs{context},
        m_list_routing_configs_for_profile{context},
        m_routing_unlink_config{context},
        m_routing_update_default_config{context},
        m_routing_retrieve_default_config{context},
        m_upsert_surcharge_decision_manager_config{context},
        m_delete_surcharge_decision_manager_config{context},
        m_retrieve_surcharge_decision_manager_config{context},
        m_upsert_decision_manager_config{context},
        m_delete_decision_manager_config{context},
        m_retrieve_decision_manager_config{context},
        m_routing_retrieve_linked_config{context},
        m_routing_retrieve_default_config_for_profiles{context},
        m_routing_update_default_config_for_profile{context},
        m_create_success_based_routing{context},
        m_create_elimination_routing{context},
        m_contract_based_routing_setup_config{context},
        m_set_dynamic_routing_volume_split{context},
        m_get_dynamic_routing_volume_split{context},
        m_routing_migration_status{context},
        m_migrate_routing_rules{context}
    {
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    routing_entry(const ::core::behavior::CallContext& ctx) const
    {
        return m_routing_entry.routing_entry(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value> routing_create_config(
        const ::app::model::routing::RoutingConfigRequest& config_request,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_routing_create_config.routing_create_config(config_request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value> routing_link_config(
        const std::optional<::app::model::routing::RoutingLinkWrapper>& link,
        const std::optional<::app::model::routing::RoutingActivatePayload>& activate,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_routing_link_config.routing_link_config(link, activate, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    routing_retrieve_config(const ::core::behavior::CallContext& ctx) const
    {
        return m_routing_retrieve_config.routing_retrieve_config(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    list_routing_configs(const ::core::behavior::CallContext& ctx) const
    {
        return m_list_routing_configs.list_routing_configs(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    list_routing_configs_for_profile(const ::core::behavior::CallContext& ctx) const
    {
        return m_list_routing_configs_for_profile.list_routing_configs_for_profile(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value> routing_unlink_config(
        const std::optional<::app::model::routing::RoutingConfigRequest>& config_request,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_routing_unlink_config.routing_unlink_config(config_request, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value> routing_update_default_config(
        const Connectors& connectors,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_routing_update_default_config.routing_update_default_config(connectors, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    routing_retrieve_default_config(const ::core::behavior::CallContext& ctx) const
    {
        return m_routing_retrieve_default_config.routing_retrieve_default_config(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value> upsert_surcharge_decision_manager_config(
        const ::app::dto::surcharge_decision_configs::SurchargeDecisionConfigReq& request,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_upsert_surcharge_decision_manager_config.upsert_surcharge_decision_manager_config(
            request,
            ctx
        );
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    delete_surcharge_decision_manager_config(const ::core::behavior::CallContext& ctx) const
    {
        return m_delete_surcharge_decision_manager_config.delete_surcharge_decision_manager_config(
            ctx
        );
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    retrieve_surcharge_decision_manager_config(const ::core::behavior::CallContext& ctx) const
    {
        return m_retrieve_surcharge_decision_manager_config
            .retrieve_surcharge_decision_manager_config(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value> upsert_decision_manager_config(
        const serde::Value& body,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_upsert_decision_manager_config.upsert_decision_manager_config(body, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    delete_decision_manager_config(const ::core::behavior::CallContext& ctx) const
    {
        return m_delete_decision_manager_config.delete_decision_manager_config(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    retrieve_decision_manager_config(const ::core::behavior::CallContext& ctx) const
    {
        return m_retrieve_decision_manager_config.retrieve_decision_manager_config(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    routing_retrieve_linked_config(const ::core::behavior::CallContext& ctx) const
    {
        return m_routing_retrieve_linked_config.routing_retrieve_linked_config(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    routing_retrieve_default_config_for_profiles(const ::core::behavior::CallContext& ctx) const
    {
        return m_routing_retrieve_default_config_for_profiles
            .routing_retrieve_default_config_for_profiles(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value> routing_update_default_config_for_profile(
        const Connectors& connectors,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_routing_update_default_config_for_profile
            .routing_update_default_config_for_profile(connectors, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value> create_success_based_routing(
        const std::optional<serde::Value>& body,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_create_success_based_routing.create_success_based_routing(body, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value> create_elimination_routing(
        const std::optional<serde::Value>& body,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_create_elimination_routing.create_elimination_routing(body, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value> contract_based_routing_setup_config(
        const std::optional<serde::Value>& body,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_contract_based_routing_setup_config.contract_based_routing_setup_config(body, ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    set_dynamic_routing_volume_split(const ::core::behavior::CallContext& ctx) const
    {
        return m_set_dynamic_routing_volume_split.set_dynamic_routing_volume_split(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    get_dynamic_routing_volume_split(const ::core::behavior::CallContext& ctx) const
    {
        return m_get_dynamic_routing_volume_split.get_dynamic_routing_volume_split(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value>
    routing_migration_status(const ::core::behavior::CallContext& ctx) const
    {
        return m_routing_migration_status.routing_migration_status(ctx);
    }

    [[nodiscard]] core::behavior::Outcome<serde::Value> migrate_routing_rules(
        const ::app::model::routing::RuleMigrationRequest& request,
        const ::core::behavior::CallContext& ctx
    ) const
    {
        return m_migrate_routing_rules.migrate_routing_rules(request, ctx);
    }

private:
    ::app::context::AppContext& m_context;
    RoutingEntryRuntime m_routing_entry;
    RoutingCreateConfigRuntime m_routing_create_config;
    RoutingLinkConfigRuntime m_routing_link_config;
    RoutingRetrieveConfigRuntime m_routing_retrieve_config;
    ListRoutingConfigsRuntime m_list_routing_configs;
    ListRoutingConfigsForProfileRuntime m_list_routing_configs_for_profile;
    RoutingUnlinkConfigRuntime m_routing_unlink_config;
    RoutingUpdateDefaultConfigRuntime m_routing_update_default_config;
    RoutingRetrieveDefaultConfigRuntime m_routing_retrieve_default_config;
    UpsertSurchargeDecisionManagerConfigRuntime m_upsert_surcharge_decision_manager_config;
    DeleteSurchargeDecisionManagerConfigRuntime m_delete_surcharge_decision_manager_config;
    RetrieveSurchargeDecisionManagerConfigRuntime m_retrieve_surcharge_decision_manager_config;
    UpsertDecisionManagerConfigRuntime m_upsert_decision_manager_config;
    DeleteDecisionManagerConfigRuntime m_delete_decision_manager_config;
    RetrieveDecisionManagerConfigRuntime m_retrieve_decision_manager_config;
    RoutingRetrieveLinkedConfigRuntime m_routing_retrieve_linked_config;
    RoutingRetrieveDefaultConfigForProfilesRuntime m_routing_retrieve_default_config_for_profiles;
    RoutingUpdateDefaultConfigForProfileRuntime m_routing_update_default_config_for_profile;
    CreateSuccessBasedRoutingRuntime m_create_success_based_routing;
    CreateEliminationRoutingRuntime m_create_elimination_routing;
    ContractBasedRoutingSetupConfigRuntime m_contract_based_routing_setup_config;
    SetDynamicRoutingVolumeSplitRuntime m_set_dynamic_routing_volume_split;
    GetDynamicRoutingVolumeSplitRuntime m_get_dynamic_routing_volume_split;
    RoutingMigrationStatusRuntime m_routing_migration_status;
    MigrateRoutingRulesRuntime m_migrate_routing_rules;
};

} // namespace app::handlers::routing
