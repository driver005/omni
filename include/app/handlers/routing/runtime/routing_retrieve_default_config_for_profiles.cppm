export module app_handlers_routing:runtime_routing_retrieve_default_config_for_profiles;

import std;
import core_error;
import core_serde;
import app_context;
import app_model;

import :input;
import :result;
import :runtime_common;

export namespace app::handlers::routing {

class RoutingRetrieveDefaultConfigForProfilesRuntime
{
public:
    explicit RoutingRetrieveDefaultConfigForProfilesRuntime(
        app::context::AppContext& context
    ) noexcept :
        m_context{context},
        m_support{context}
    {
    }

    [[nodiscard]] std::
        expected<RoutingRetrieveDefaultConfigForProfilesResult, core::error::AppError>
        routing_retrieve_default_config_for_profiles(const RoutingInput& input) const
    {
        std::vector<app::model::routing::ProfileDefaultRoutingConfig> response;
        const auto merchant = input.merchant_id;
        std::vector<std::string> profiles;
        for (const auto& record: m_context.repository().list_routing_configs()) {
            if (!merchant.empty() && record.get_merchant_id() != merchant) {
                continue;
            }
            if (record.get_config_id().ends_with(":default") && !record.get_profile_id().empty()) {
                profiles.push_back(record.get_profile_id());
            }
        }
        std::ranges::sort(profiles);
        profiles.erase(std::unique(profiles.begin(), profiles.end()), profiles.end());
        for (const auto& profile: profiles) {
            app::model::routing::ProfileDefaultRoutingConfig item;
            item.set_profile_id(m_support.wrap<app::model::routing::ProfileIdValue>(profile));
            item.set_connectors(m_support.default_config(profile));
            response.push_back(std::move(item));
        }
        m_support.track("routing_retrieve_default_config_for_profiles");
        return RoutingRetrieveDefaultConfigForProfilesResult{ResponseDocument::from(response)};
    }

private:
    app::context::AppContext& m_context;
    RoutingRuntimeSupport m_support;
};

} // namespace app::handlers::routing
