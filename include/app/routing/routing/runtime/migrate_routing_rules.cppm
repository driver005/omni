export module app_routing:routing_runtime_migrate_routing_rules;

import std;
import core_error;
import core_serde;
import app_context;
import app_model;

import :routing_input;
import :routing_result;
import :routing_runtime_common;

export namespace app::handlers::routing {

class MigrateRoutingRulesRuntime
{
public:
    explicit MigrateRoutingRulesRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<serde::Value, core::error::AppError>
    migrate_routing_rules(const RoutingInput& input) const
    {
        const auto& typed = *input.migration_request;
        ::app::model::routing::RuleMigrationResult response;
        ::app::model::routing::RuleMigrationTotals totals;
        totals.set_profiles(typed.get_profile_ids().size());

        std::vector<::app::model::routing::RuleMigrationProfileResult> profiles;
        for (const auto& profile_value: typed.get_profile_ids()) {
            const auto profile = profile_value.get_value();
            ::app::model::routing::RuleMigrationProfileResult result;
            result.set_profile_id(profile_value);
            std::vector<::app::model::routing::RuleMigrationResponse> success;
            std::vector<::app::model::routing::RuleMigrationNotApplicable> not_applicable;

            bool found = false;
            for (const auto& record: m_context.repository().list_routing_configs()) {
                if (record.get_profile_id() != profile ||
                    record.get_config_id().ends_with(":default")) {
                    continue;
                }
                found = true;
                const auto algorithm_id = record.get_config_id();
                if (record.get_algorithm() == "three_ds_decision_rule") {
                    ::app::model::routing::RuleMigrationNotApplicable na;
                    na.set_profile_id(profile_value);
                    na.set_algorithm_id(
                        m_support.wrap<::app::model::routing::RoutingIdValue>(algorithm_id)
                    );
                    na.set_kind(::app::model::routing::RoutingAlgorithmKind::Single);
                    na.set_reason("3DS decision rules are not migrated to the Decision Engine");
                    not_applicable.push_back(std::move(na));
                    continue;
                }
                // Local in-process substitute for DE rule create (external infra).
                m_support.add_de_rule_id(profile, algorithm_id);
                ::app::model::routing::RuleMigrationResponse migrated;
                migrated.set_profile_id(profile_value);
                migrated.set_euclid_algorithm_id(
                    m_support.wrap<::app::model::routing::RoutingIdValue>(algorithm_id)
                );
                migrated.set_decision_engine_algorithm_id(algorithm_id);
                migrated.set_is_active_rule(
                    m_support.active_id_for(input.active_id, profile, "payment") == algorithm_id
                );
                success.push_back(std::move(migrated));
            }
            result.set_success(std::move(success));
            result.set_not_applicable(std::move(not_applicable));
            if (!found) {
                result.set_not_attempted("no routing rules in Hyperswitch");
                totals.set_profiles_not_attempted(totals.get_profiles_not_attempted() + 1);
            }
            totals.set_rules_migrated(totals.get_rules_migrated() + result.get_success().size());
            totals.set_rules_not_applicable(
                totals.get_rules_not_applicable() + result.get_not_applicable().size()
            );
            profiles.push_back(std::move(result));
        }
        response.set_profiles(std::move(profiles));
        response.set_totals(totals);
        m_support.track("migrate_routing_rules");
        return ResponseDocument::from(response);
    }

private:
    ::app::context::AppContext& m_context;
    RoutingRuntimeSupport m_support;
};

} // namespace app::handlers::routing
