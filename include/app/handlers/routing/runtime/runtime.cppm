export module app_handlers_routing:runtime;

import std;
import core_error;
import app_context;

import :input;
import :result;
import :runtime_common;
import :runtime_routing_entry;
import :runtime_routing_create_config;
import :runtime_routing_link_config;
import :runtime_routing_retrieve_config;
import :runtime_list_routing_configs;
import :runtime_list_routing_configs_for_profile;
import :runtime_routing_unlink_config;
import :runtime_routing_update_default_config;
import :runtime_routing_retrieve_default_config;
import :runtime_upsert_surcharge_decision_manager_config;
import :runtime_delete_surcharge_decision_manager_config;
import :runtime_retrieve_surcharge_decision_manager_config;
import :runtime_upsert_decision_manager_config;
import :runtime_delete_decision_manager_config;
import :runtime_retrieve_decision_manager_config;
import :runtime_routing_retrieve_linked_config;
import :runtime_routing_retrieve_default_config_for_profiles;
import :runtime_routing_update_default_config_for_profile;
import :runtime_create_success_based_routing;
import :runtime_create_elimination_routing;
import :runtime_contract_based_routing_setup_config;
import :runtime_set_dynamic_routing_volume_split;
import :runtime_get_dynamic_routing_volume_split;
import :runtime_routing_migration_status;
import :runtime_migrate_routing_rules;

export namespace app::handlers::routing {

class RoutingRuntime
{
public:
    explicit RoutingRuntime(app::context::AppContext& context) noexcept :
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

    [[nodiscard]] std::expected<RoutingEntryResult, core::error::AppError>
    routing_entry(const RoutingInput& input) const
    {
        return m_routing_entry.routing_entry(input);
    }

    [[nodiscard]] std::expected<RoutingCreateConfigResult, core::error::AppError>
    routing_create_config(const RoutingInput& input) const
    {
        return m_routing_create_config.routing_create_config(input);
    }

    [[nodiscard]] std::expected<RoutingLinkConfigResult, core::error::AppError>
    routing_link_config(const RoutingInput& input) const
    {
        return m_routing_link_config.routing_link_config(input);
    }

    [[nodiscard]] std::expected<RoutingRetrieveConfigResult, core::error::AppError>
    routing_retrieve_config(const RoutingInput& input) const
    {
        return m_routing_retrieve_config.routing_retrieve_config(input);
    }

    [[nodiscard]] std::expected<ListRoutingConfigsResult, core::error::AppError>
    list_routing_configs(const RoutingInput& input) const
    {
        return m_list_routing_configs.list_routing_configs(input);
    }

    [[nodiscard]] std::expected<ListRoutingConfigsForProfileResult, core::error::AppError>
    list_routing_configs_for_profile(const RoutingInput& input) const
    {
        return m_list_routing_configs_for_profile.list_routing_configs_for_profile(input);
    }

    [[nodiscard]] std::expected<RoutingUnlinkConfigResult, core::error::AppError>
    routing_unlink_config(const RoutingInput& input) const
    {
        return m_routing_unlink_config.routing_unlink_config(input);
    }

    [[nodiscard]] std::expected<RoutingUpdateDefaultConfigResult, core::error::AppError>
    routing_update_default_config(const RoutingInput& input) const
    {
        return m_routing_update_default_config.routing_update_default_config(input);
    }

    [[nodiscard]] std::expected<RoutingRetrieveDefaultConfigResult, core::error::AppError>
    routing_retrieve_default_config(const RoutingInput& input) const
    {
        return m_routing_retrieve_default_config.routing_retrieve_default_config(input);
    }

    [[nodiscard]] std::expected<UpsertSurchargeDecisionManagerConfigResult, core::error::AppError>
    upsert_surcharge_decision_manager_config(const RoutingInput& input) const
    {
        return m_upsert_surcharge_decision_manager_config.upsert_surcharge_decision_manager_config(
            input
        );
    }

    [[nodiscard]] std::expected<DeleteSurchargeDecisionManagerConfigResult, core::error::AppError>
    delete_surcharge_decision_manager_config(const RoutingInput& input) const
    {
        return m_delete_surcharge_decision_manager_config.delete_surcharge_decision_manager_config(
            input
        );
    }

    [[nodiscard]] std::expected<RetrieveSurchargeDecisionManagerConfigResult, core::error::AppError>
    retrieve_surcharge_decision_manager_config(const RoutingInput& input) const
    {
        return m_retrieve_surcharge_decision_manager_config
            .retrieve_surcharge_decision_manager_config(input);
    }

    [[nodiscard]] std::expected<UpsertDecisionManagerConfigResult, core::error::AppError>
    upsert_decision_manager_config(const RoutingInput& input) const
    {
        return m_upsert_decision_manager_config.upsert_decision_manager_config(input);
    }

    [[nodiscard]] std::expected<DeleteDecisionManagerConfigResult, core::error::AppError>
    delete_decision_manager_config(const RoutingInput& input) const
    {
        return m_delete_decision_manager_config.delete_decision_manager_config(input);
    }

    [[nodiscard]] std::expected<RetrieveDecisionManagerConfigResult, core::error::AppError>
    retrieve_decision_manager_config(const RoutingInput& input) const
    {
        return m_retrieve_decision_manager_config.retrieve_decision_manager_config(input);
    }

    [[nodiscard]] std::expected<RoutingRetrieveLinkedConfigResult, core::error::AppError>
    routing_retrieve_linked_config(const RoutingInput& input) const
    {
        return m_routing_retrieve_linked_config.routing_retrieve_linked_config(input);
    }

    [[nodiscard]] std::
        expected<RoutingRetrieveDefaultConfigForProfilesResult, core::error::AppError>
        routing_retrieve_default_config_for_profiles(const RoutingInput& input) const
    {
        return m_routing_retrieve_default_config_for_profiles
            .routing_retrieve_default_config_for_profiles(input);
    }

    [[nodiscard]] std::expected<RoutingUpdateDefaultConfigForProfileResult, core::error::AppError>
    routing_update_default_config_for_profile(const RoutingInput& input) const
    {
        return m_routing_update_default_config_for_profile
            .routing_update_default_config_for_profile(input);
    }

    [[nodiscard]] std::expected<CreateSuccessBasedRoutingResult, core::error::AppError>
    create_success_based_routing(const RoutingInput& input) const
    {
        return m_create_success_based_routing.create_success_based_routing(input);
    }

    [[nodiscard]] std::expected<CreateEliminationRoutingResult, core::error::AppError>
    create_elimination_routing(const RoutingInput& input) const
    {
        return m_create_elimination_routing.create_elimination_routing(input);
    }

    [[nodiscard]] std::expected<ContractBasedRoutingSetupConfigResult, core::error::AppError>
    contract_based_routing_setup_config(const RoutingInput& input) const
    {
        return m_contract_based_routing_setup_config.contract_based_routing_setup_config(input);
    }

    [[nodiscard]] std::expected<SetDynamicRoutingVolumeSplitResult, core::error::AppError>
    set_dynamic_routing_volume_split(const RoutingInput& input) const
    {
        return m_set_dynamic_routing_volume_split.set_dynamic_routing_volume_split(input);
    }

    [[nodiscard]] std::expected<GetDynamicRoutingVolumeSplitResult, core::error::AppError>
    get_dynamic_routing_volume_split(const RoutingInput& input) const
    {
        return m_get_dynamic_routing_volume_split.get_dynamic_routing_volume_split(input);
    }

    [[nodiscard]] std::expected<RoutingMigrationStatusResult, core::error::AppError>
    routing_migration_status(const RoutingInput& input) const
    {
        return m_routing_migration_status.routing_migration_status(input);
    }

    [[nodiscard]] std::expected<MigrateRoutingRulesResult, core::error::AppError>
    migrate_routing_rules(const RoutingInput& input) const
    {
        return m_migrate_routing_rules.migrate_routing_rules(input);
    }

private:
    app::context::AppContext& m_context;
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
