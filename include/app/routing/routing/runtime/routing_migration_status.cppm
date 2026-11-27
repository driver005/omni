export module app_routing:routing_runtime_routing_migration_status;

import std;
import core_error;
import core_serde;
import app_context;
import app_model;

import :routing_input;
import :routing_result;
import :routing_runtime_common;

export namespace app::handlers::routing {

class RoutingMigrationStatusRuntime
{
public:
    explicit RoutingMigrationStatusRuntime(::app::context::AppContext& context) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::expected<serde::Value, core::error::AppError>
    routing_migration_status(const RoutingInput& input) const
    {
        const auto limit = input.migration_limit;
        const auto offset = input.migration_offset;

        // list_routing_scope_page substitute: group persisted records by (profile, merchant).
        std::vector<std::pair<std::string, std::string>> scopes;
        std::map<std::string, std::size_t> in_scope_counts;
        for (const auto& record: m_context.repository().list_routing_configs()) {
            if (record.get_profile_id().empty()) {
                continue;
            }
            bool seen = false;
            for (const auto& scope: scopes) {
                if (scope.first == record.get_profile_id()) {
                    seen = true;
                    break;
                }
            }
            if (!seen) {
                scopes.emplace_back(record.get_profile_id(), record.get_merchant_id());
            }
            if (record.get_algorithm() != "default" && record.get_algorithm() != "decision" &&
                record.get_algorithm() != "surcharge" && record.get_algorithm() != "volume_split") {
                ++in_scope_counts[record.get_profile_id()];
            }
        }
        std::ranges::sort(
            scopes,
            [](const auto& a, const auto& b)
            {
                return a.first < b.first;
            }
        );
        const auto begin =
            scopes.begin() +
            static_cast<std::ptrdiff_t>(std::min<std::size_t>(offset, scopes.size()));
        const auto end =
            begin + static_cast<std::ptrdiff_t>(
                        std::min<std::size_t>(limit, static_cast<std::size_t>(scopes.end() - begin))
                    );

        ::app::model::routing::RoutingMigrationStatusResponse response;
        response.set_limit(limit);
        response.set_offset(offset);
        response.set_has_more(static_cast<std::size_t>(end - begin) >= limit);
        ::app::model::routing::RoutingMigrationPageTotals totals;
        totals.set_profiles(static_cast<std::size_t>(end - begin));

        std::vector<::app::model::routing::RoutingMigrationProfileStatus> profiles;
        for (auto it = begin; it != end; ++it) {
            const auto& profile = it->first;
            const auto hs_count = static_cast<std::int64_t>(in_scope_counts[profile]);
            const auto de = m_support.de_rule_ids(profile);
            ::app::model::routing::RoutingMigrationProfileStatus status;
            status.set_profile_id(m_support.wrap<::app::model::routing::ProfileIdValue>(profile));
            status.set_merchant_id(
                m_support.wrap<::app::model::routing::MerchantIdValue>(it->second)
            );
            status.set_rules_hyperswitch(hs_count);
            status.set_rules_decision_engine(static_cast<std::int64_t>(de.size()));
            const auto source =
                m_support.is_cutover(input.merchant_id, profile)
                    ? ::app::model::routing::RoutingResultSource::DecisionEngine
                    : ::app::model::routing::RoutingResultSource::HyperswitchRouting;
            status.set_routing_source(source);
            ::app::model::routing::RoutingMigrationState state{};
            if (de.empty()) {
                state = (source == ::app::model::routing::RoutingResultSource::DecisionEngine)
                            ? ::app::model::routing::RoutingMigrationState::EnabledWithoutRules
                            : ::app::model::routing::RoutingMigrationState::Pending;
            } else if (hs_count > static_cast<std::int64_t>(de.size())) {
                state = ::app::model::routing::RoutingMigrationState::Partial;
            } else if (source == ::app::model::routing::RoutingResultSource::DecisionEngine) {
                state = ::app::model::routing::RoutingMigrationState::Enabled;
            } else {
                state = ::app::model::routing::RoutingMigrationState::Migrated;
            }
            status.set_state(state);
            profiles.push_back(std::move(status));
            switch (state) {
                case ::app::model::routing::RoutingMigrationState::Pending:
                    totals.set_pending(totals.get_pending() + 1);
                    break;
                case ::app::model::routing::RoutingMigrationState::Partial:
                    totals.set_partial(totals.get_partial() + 1);
                    break;
                case ::app::model::routing::RoutingMigrationState::Migrated:
                    totals.set_migrated(totals.get_migrated() + 1);
                    break;
                case ::app::model::routing::RoutingMigrationState::Enabled:
                    totals.set_enabled(totals.get_enabled() + 1);
                    break;
                case ::app::model::routing::RoutingMigrationState::EnabledWithoutRules:
                    totals.set_enabled_without_rules(totals.get_enabled_without_rules() + 1);
                    break;
                case ::app::model::routing::RoutingMigrationState::Unknown:
                    totals.set_unknown(totals.get_unknown() + 1);
                    break;
                case ::app::model::routing::RoutingMigrationState::Diverged:
                    totals.set_diverged(totals.get_diverged() + 1);
                    break;
            }
        }
        response.set_profiles(std::move(profiles));
        response.set_page_totals(totals);
        m_support.track("routing_migration_status");
        return ResponseDocument::from(response);
    }

private:
    ::app::context::AppContext& m_context;
    RoutingRuntimeSupport m_support;
};

} // namespace app::handlers::routing
