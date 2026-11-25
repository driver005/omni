export module app_handlers_user_role:runtime_common;

import std;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import core_cache;
import core_error;
import app_auth;
import app_http;
import app_crypto;
import app_metrics;
import app_store;
import app_events;
import app_keystore;

import :input;
import :result;

export namespace app::handlers::user_role {

namespace role_tables {

    // ── Local transcription of upstream common_enums / authorization static data ──
    // Ported from `common_enums/src/enums.rs:10095-10210,11225-11230`,
    // `services/authorization/permission_groups.rs:18-362,364-428`,
    // `services/authorization/info.rs:8-103`, `services/authorization/permissions.rs:4-206`.

    struct PermGroupDef
    {
        std::string_view name{};
        std::string_view parent{};
        std::string_view scope{};       // "read"/"write"
        std::string_view category{};    // "dashboard"/"orchestration"/"recon"
        std::string_view description{}; // empty => hidden from auth-info
    };

    inline const std::vector<PermGroupDef>& perm_groups()
    {
        static const std::vector<PermGroupDef> table{
            // name, parent, scope, category, description ('' hidden)
            {"operations_view",
             "operations",
             "read",
             "orchestration",
             "View Payments, Refunds, Payouts, Mandates, Disputes and Customers"},
            {"operations_manage",
             "operations",
             "write",
             "orchestration",
             "Create, modify and delete Payments, Refunds, Payouts, Mandates, Disputes and "
             "Customers"},
            {"connectors_view",
             "connectors",
             "read",
             "orchestration",
             "View connected Payment Processors, Payout Processors and Fraud & Risk Manager "
             "details"},
            {"connectors_manage",
             "connectors",
             "write",
             "orchestration",
             "Create, modify and delete connectors like Payment Processors, Payout Processors "
             "and Fraud & Risk Manager"},
            {"workflows_view",
             "workflows",
             "read",
             "orchestration",
             "View Routing, 3DS Decision Manager, Surcharge Decision Manager"},
            {"workflows_manage",
             "workflows",
             "write",
             "orchestration",
             "Create, modify and delete Routing, 3DS Decision Manager, Surcharge Decision "
             "Manager"},
            {"analytics_view", "analytics", "read", "orchestration", "View Analytics"},
            {"users_view", "users", "read", "dashboard", "View Users"},
            {"users_manage", "users", "write", "dashboard", "Manage and invite Users to the Team"},
            {"account_view", "account", "read", "orchestration", "View Merchant Details"},
            {"account_manage",
             "account",
             "write",
             "orchestration",
             "Create, modify and delete Merchant Details like api keys, webhooks, etc"},
            {"webhooks_view", "webhook", "read", "orchestration", ""},
            {"webhooks_manage", "webhook", "write", "orchestration", ""},
            {"api_keys_view", "api_keys", "read", "orchestration", ""},
            {"api_keys_manage", "api_keys", "write", "orchestration", ""},
            {"clone_connector_manage",
             "clone_connector",
             "write",
             "orchestration",
             "Clone a connector and its configuration"},
            {"theme_view", "theme", "read", "orchestration", "View Themes"},
            {"theme_manage", "theme", "write", "orchestration", "Manage Themes"},
            {"configurations_view",
             "configurations",
             "read",
             "orchestration",
             "View Configurations"},
            {"configurations_manage",
             "configurations",
             "write",
             "orchestration",
             "Create, modify and delete Configurations"},
            {"recon_sources_view",
             "recon_sources",
             "read",
             "recon",
             "View recon ingestion and transformation configs and files"},
            {"recon_sources_manage",
             "recon_sources",
             "write",
             "recon",
             "Create and edit recon ingestion and transformation configs and download files"},
            {"recon_exceptions_view",
             "recon_exceptions",
             "read",
             "recon",
             "Investigate Exceptions and view resolutions"},
            {"recon_exceptions_manage",
             "recon_exceptions",
             "write",
             "recon",
             "Investigate and resolve recon exceptions"},
            {"recon_transactions_view",
             "recon_transactions",
             "read",
             "recon",
             "View recon staging entries and transactions"},
            {"recon_transactions_manage",
             "recon_transactions",
             "write",
             "recon",
             "View and edit recon staging entries and transactions"},
            {"recon_rules_view", "recon_rules", "read", "recon", "View reconciliation rules"},
            {"recon_rules_manage",
             "recon_rules",
             "write",
             "recon",
             "Create and edit reconciliation rules"},
            {"recon_api_key_view", "recon_api_key", "read", "recon", "View recon API keys"},
            {"recon_api_key_manage",
             "recon_api_key",
             "write",
             "recon",
             "Create, update and revoke recon API keys"},
            {"recon_accounts_view",
             "recon_accounts",
             "read",
             "recon",
             "View recon ledger accounts and balances"},
            {"recon_accounts_manage",
             "recon_accounts",
             "write",
             "recon",
             "Create and edit recon ledger accounts"},
            {"recon_audit_view", "recon_audit", "read", "recon", "View the recon audit trail"},
            {"offers_view", "offers", "read", "orchestration", "View Offers"},
            {"offers_manage",
             "offers",
             "write",
             "orchestration",
             "Create, modify and delete Offers"},
            {"alerts_view", "alerts", "read", "orchestration", ""},
            {"alerts_manage", "alerts", "write", "orchestration", ""},
            {"monitoring_view", "monitoring", "read", "orchestration", ""},
            {"monitoring_manage", "monitoring", "write", "orchestration", ""},
        };
        return table;
    }

    inline const PermGroupDef* perm_group(std::string_view name)
    {
        for (const auto& def: perm_groups()) {
            if (def.name == name) {
                return &def;
            }
        }
        return nullptr;
    }

    inline bool is_internal_perm_group(std::string_view name)
    {
        return name == "alerts_view" || name == "alerts_manage" || name == "monitoring_view" ||
               name == "monitoring_manage";
    }

    // Upstream `PermissionGroupExt::accessible_groups` (permission_groups.rs:97-197).
    inline std::vector<std::string> accessible_groups(std::string_view group)
    {
        auto make = [](std::initializer_list<const char*> values)
        {
            std::vector<std::string> out{};
            for (const char* v: values) {
                out.emplace_back(v);
            }
            return out;
        };
        if (group == "operations_view") {
            return make({"operations_view", "connectors_view"});
        }
        if (group == "operations_manage") {
            return make({"operations_view", "operations_manage", "connectors_view"});
        }
        if (group == "connectors_view") {
            return make({"connectors_view"});
        }
        if (group == "connectors_manage") {
            return make({"connectors_view", "connectors_manage"});
        }
        if (group == "workflows_view") {
            return make({"workflows_view", "connectors_view"});
        }
        if (group == "workflows_manage") {
            return make({"workflows_view", "workflows_manage", "connectors_view"});
        }
        if (group == "analytics_view") {
            return make({"analytics_view", "operations_view"});
        }
        if (group == "users_view") {
            return make({"users_view"});
        }
        if (group == "users_manage") {
            return make({"users_view", "users_manage"});
        }
        if (group == "account_view") {
            return make({"account_view"});
        }
        if (group == "account_manage") {
            return make({"account_view", "account_manage"});
        }
        if (group == "webhooks_view") {
            return make({"webhooks_view", "account_view"});
        }
        if (group == "webhooks_manage") {
            return make({"webhooks_view", "webhooks_manage", "account_view"});
        }
        if (group == "api_keys_view") {
            return make({"api_keys_view", "account_view"});
        }
        if (group == "api_keys_manage") {
            return make({"api_keys_view", "api_keys_manage", "account_view"});
        }
        if (group == "clone_connector_manage") {
            return make({"clone_connector_manage", "connectors_manage", "connectors_view"});
        }
        if (group == "theme_view") {
            return make({"theme_view", "account_view"});
        }
        if (group == "theme_manage") {
            return make({"theme_manage", "account_view"});
        }
        if (group == "configurations_view") {
            return make({"configurations_view"});
        }
        if (group == "configurations_manage") {
            return make({"configurations_view", "configurations_manage"});
        }
        if (group == "recon_sources_view") {
            return make({"recon_sources_view", "recon_transactions_view", "recon_rules_view"});
        }
        if (group == "recon_sources_manage") {
            return make(
                {"recon_sources_manage",
                 "recon_sources_view",
                 "recon_transactions_view",
                 "recon_rules_view"}
            );
        }
        if (group == "recon_exceptions_view") {
            return make({"recon_exceptions_view", "recon_transactions_view", "recon_rules_view"});
        }
        if (group == "recon_exceptions_manage") {
            return make(
                {"recon_exceptions_manage",
                 "recon_exceptions_view",
                 "recon_sources_view",
                 "recon_transactions_view",
                 "recon_rules_view"}
            );
        }
        if (group == "recon_transactions_view") {
            return make({"recon_transactions_view", "recon_rules_view"});
        }
        if (group == "recon_transactions_manage") {
            return make(
                {"recon_transactions_manage", "recon_transactions_view", "recon_rules_view"}
            );
        }
        if (group == "recon_rules_view") {
            return make({"recon_rules_view", "recon_transactions_view"});
        }
        if (group == "recon_rules_manage") {
            return make({"recon_rules_manage", "recon_rules_view", "recon_transactions_view"});
        }
        if (group == "recon_api_key_view") {
            return make({"recon_api_key_view"});
        }
        if (group == "recon_api_key_manage") {
            return make({"recon_api_key_view", "recon_api_key_manage"});
        }
        if (group == "recon_accounts_view") {
            return make({"recon_accounts_view"});
        }
        if (group == "recon_accounts_manage") {
            return make({"recon_accounts_view", "recon_accounts_manage"});
        }
        if (group == "recon_audit_view") {
            return make({"recon_audit_view"});
        }
        if (group == "offers_view") {
            return make({"offers_view"});
        }
        if (group == "offers_manage") {
            return make({"offers_view", "offers_manage"});
        }
        if (group == "alerts_view") {
            return make({"alerts_view"});
        }
        if (group == "alerts_manage") {
            return make({"alerts_view", "alerts_manage"});
        }
        if (group == "monitoring_view") {
            return make({"monitoring_view"});
        }
        if (group == "monitoring_manage") {
            return make({"monitoring_view", "monitoring_manage"});
        }
        return make({});
    }

    // Upstream `ParentGroupExt::resources` (permission_groups.rs:302-326).
    inline std::vector<std::string_view> parent_resources(std::string_view parent)
    {
        auto make = [](std::initializer_list<const char*> values)
        {
            std::vector<std::string_view> out{};
            for (const char* v: values) {
                out.emplace_back(v);
            }
            return out;
        };
        if (parent == "operations") {
            return make(
                {"payment",
                 "payment_link",
                 "refund",
                 "mandate",
                 "dispute",
                 "customer",
                 "payout",
                 "report",
                 "account"}
            );
        }
        if (parent == "connectors") {
            return make({"connector", "account"});
        }
        if (parent == "workflows") {
            return make(
                {"routing",
                 "three_ds_decision_manager",
                 "surcharge_decision_manager",
                 "account",
                 "revenue_recovery"}
            );
        }
        if (parent == "analytics") {
            return make({"analytics", "report", "account"});
        }
        if (parent == "users") {
            return make({"user", "account"});
        }
        if (parent == "account") {
            return make({"account", "api_key", "webhook_event"});
        }
        if (parent == "webhook") {
            return make({"webhook_event"});
        }
        if (parent == "api_keys") {
            return make({"api_key"});
        }
        if (parent == "clone_connector") {
            return make({"clone_connector"});
        }
        if (parent == "theme") {
            return make({"theme"});
        }
        if (parent == "configurations") {
            return make({"superposition_config"});
        }
        if (parent == "recon_sources") {
            return make({"recon_ingestion", "recon_transformation", "account"});
        }
        if (parent == "recon_exceptions") {
            return make({"recon_exception", "account"});
        }
        if (parent == "recon_transactions") {
            return make({"recon_staging_entry", "recon_transaction", "account"});
        }
        if (parent == "recon_rules") {
            return make({"recon_rule", "account"});
        }
        if (parent == "recon_api_key") {
            return make({"recon_api_key"});
        }
        if (parent == "recon_accounts") {
            return make({"recon_account"});
        }
        if (parent == "recon_audit") {
            return make({"recon_audit"});
        }
        if (parent == "offers") {
            return make({"offers"});
        }
        if (parent == "alerts") {
            return make({"alert"});
        }
        if (parent == "monitoring") {
            return make({"monitoring"});
        }
        return make({});
    }

    // Upstream `get_resource_name` (permissions.rs:145-187).
    inline std::optional<std::string_view>
    resource_name(std::string_view res, std::string_view entity)
    {
        if (res == "payment") {
            return "Payments";
        }
        if (res == "payment_link") {
            return "Payment Links";
        }
        if (res == "refund") {
            return "Refunds";
        }
        if (res == "dispute") {
            return "Disputes";
        }
        if (res == "mandate") {
            return "Mandates";
        }
        if (res == "customer") {
            return "Customers";
        }
        if (res == "payout") {
            return "Payouts";
        }
        if (res == "api_key") {
            return "Api Keys";
        }
        if (res == "connector") {
            return "Payment Processors, Payout Processors, 3ds Authenticators, Fraud & Risk "
                   "Managers, PM Auth Processor,Tax Processors, Billing Processors, Vault "
                   "Processors";
        }
        if (res == "routing") {
            return "Routing";
        }
        if (res == "subscription") {
            return "Subscription";
        }
        if (res == "revenue_recovery") {
            return "Revenue Recovery";
        }
        if (res == "three_ds_decision_manager") {
            return "3DS Decision Manager";
        }
        if (res == "surcharge_decision_manager") {
            return "Surcharge Decision Manager";
        }
        if (res == "analytics") {
            return "Insights, Payments, Refunds, Authentication, Routing";
        }
        if (res == "report") {
            return "Operation Reports";
        }
        if (res == "user") {
            return "Users";
        }
        if (res == "webhook_event") {
            return "Webhook Events";
        }
        if (res == "account") {
            if (entity == "profile") {
                return "Business Profile Account";
            }
            if (entity == "merchant") {
                return "Merchant Account";
            }
            if (entity == "organization") {
                return "Organization Account";
            }
            return "Tenant Account";
        }
        if (res == "theme") {
            return "Themes";
        }
        if (res == "clone_connector") {
            return "Clone Connectors";
        }
        if (res == "recon_ingestion") {
            return "Recon Ingestion Configs";
        }
        if (res == "recon_transformation") {
            return "Recon Transformation Configs";
        }
        if (res == "recon_exception") {
            return "Recon Exception Management";
        }
        if (res == "recon_staging_entry") {
            return "Recon Staging Entries";
        }
        if (res == "recon_transaction") {
            return "Recon Transactions";
        }
        if (res == "recon_rule") {
            return "Recon Rules";
        }
        if (res == "recon_api_key") {
            return "Recon API Keys";
        }
        if (res == "recon_account") {
            return "Recon Ledger Accounts";
        }
        if (res == "recon_audit") {
            return "Recon Audit Trail";
        }
        if (res == "superposition_config") {
            return "Superposition Configs";
        }
        if (res == "offers") {
            return "Offers";
        }
        // Alert / Monitoring have no resource name (upstream None).
        return std::nullopt;
    }

    // Upstream `Resource::entities()` (permissions.rs:4-143) + `filter_resources_by_entity_type`
    // (permissions.rs:196-206). Entity rank: tenant=3, organization=2, merchant=1, profile=0.
    inline int entity_rank(std::string_view entity)
    {
        if (entity == "tenant") {
            return 3;
        }
        if (entity == "organization") {
            return 2;
        }
        if (entity == "merchant") {
            return 1;
        }
        return 0; // profile
    }

    inline bool resource_allowed_for_entity(std::string_view res, std::string_view entity)
    {
        const int rank = entity_rank(entity);
        if (res == "account") {
            return true; // profile, merchant, organization, tenant
        }
        if (res == "analytics" || res == "report") {
            return rank <= 2; // profile, merchant, organization
        }
        if (res == "theme") {
            return rank >= 2; // organization (and tenant)
        }
        if (res == "revenue_recovery" || res == "recon_ingestion" ||
            res == "recon_transformation" || res == "recon_exception" ||
            res == "recon_staging_entry" || res == "recon_transaction" || res == "recon_rule" ||
            res == "recon_account" || res == "recon_audit" || res == "superposition_config" ||
            res == "offers") {
            return rank <= 0; // profile only
        }
        if (res == "mandate" || res == "customer" || res == "api_key" ||
            res == "surcharge_decision_manager" || res == "clone_connector" ||
            res == "recon_api_key" || res == "alert" || res == "monitoring") {
            return rank <= 1; // profile, merchant
        }
        // payment, payment_link, refund, dispute, payout, connector, routing, subscription,
        // three_ds_decision_manager, user, webhook_event: profile & merchant
        return rank <= 1;
    }

    inline std::optional<std::vector<std::string>> filter_resources_by_entity(
        const std::vector<std::string_view>& resources,
        std::string_view entity
    )
    {
        std::vector<std::string> out{};
        for (const auto res: resources) {
            if (resource_allowed_for_entity(res, entity)) {
                out.emplace_back(res);
            }
        }
        if (out.empty()) {
            return std::nullopt;
        }
        return out;
    }

    // Upstream `ParentGroupExt::get_descriptions_for_groups` (permission_groups.rs:328-354).
    inline std::vector<ParentGroupDescriptionWire>
    descriptions_for_groups(std::string_view entity, const std::vector<std::string>& groups)
    {
        std::vector<ParentGroupDescriptionWire> out{};
        for (const auto& def: perm_groups()) {
            if (std::find(groups.begin(), groups.end(), std::string{def.name}) == groups.end()) {
                continue;
            }
            const auto filtered = filter_resources_by_entity(parent_resources(def.parent), entity);
            if (!filtered) {
                continue;
            }
            std::string description{};
            bool first{true};
            for (const auto& res: *filtered) {
                const auto name = resource_name(res, entity);
                if (!name) {
                    continue;
                }
                if (!first) {
                    description += ", ";
                }
                first = false;
                description += *name;
            }
            if (description.empty()) {
                continue;
            }
            // scopes = all permission groups with this parent (upstream uses a HashSet).
            std::vector<std::string> scopes{};
            for (const auto& candidate: perm_groups()) {
                if (candidate.parent == def.parent &&
                    std::find(scopes.begin(), scopes.end(), std::string{candidate.name}) ==
                        scopes.end()) {
                    scopes.emplace_back(candidate.name);
                }
            }
            ParentGroupDescriptionWire item{};
            item.set_name(std::string{def.parent});
            item.set_description(std::move(description));
            item.set_scopes(std::move(scopes));
            out.push_back(std::move(item));
        }
        return out;
    }

    // Upstream `permission_groups_to_parent_group_info` (utils/user_role.rs:567-600).
    inline std::vector<ParentGroupInfoWire> permission_groups_to_parent_group_info(
        const std::vector<std::string>& groups,
        std::string_view entity
    )
    {
        std::vector<std::string> parents{};
        std::map<std::string, std::vector<std::string>> scopes_by_parent{};
        for (const auto& name: groups) {
            const auto* def = perm_group(name);
            if (def == nullptr) {
                continue;
            }
            const std::string parent{def->parent};
            if (std::find(parents.begin(), parents.end(), parent) == parents.end()) {
                parents.push_back(parent);
            }
            auto& scopes = scopes_by_parent[parent];
            const std::string scope{def->scope};
            if (std::find(scopes.begin(), scopes.end(), scope) == scopes.end()) {
                scopes.push_back(scope);
            }
        }
        std::vector<ParentGroupInfoWire> out{};
        for (const auto& parent: parents) {
            const auto filtered = filter_resources_by_entity(parent_resources(parent), entity);
            if (!filtered) {
                continue;
            }
            ParentGroupInfoWire item{};
            item.set_name(parent);
            item.set_resources(*filtered);
            item.set_scopes(scopes_by_parent[parent]);
            out.push_back(std::move(item));
        }
        return out;
    }

    // Upstream `get_accessible_product_categories` (db/domain/role.rs).
    inline std::vector<std::string> accessible_product_categories(std::string_view product_type)
    {
        if (product_type == "recon") {
            return {"dashboard", "recon"};
        }
        return {"dashboard", "orchestration"};
    }

    // ── Local RoleInfo model (upstream `services/authorization/roles.rs:11-155`) ──
    struct RoleInfo
    {
        std::string role_id{};
        std::string role_name{};
        std::vector<std::string> groups{}; // raw groups (upstream RoleInfo::groups)
        std::string scope{};               // role_scope: organization/merchant/profile
        std::string entity_type{};         // tenant/organization/merchant/profile
        bool is_invitable{true};
        bool is_deletable{true};
        bool is_updatable{true};
        bool is_internal{false};
        std::string product_type_filter{}; // "" / "orchestration" / "recon"

        // Upstream `RoleInfo::get_permission_groups` (roles.rs:34-41): expand accessible
        // groups, dedup via HashSet (order irrelevant for the wire on our side).
        [[nodiscard]] std::vector<std::string> permission_groups() const
        {
            return permission_groups_from(groups);
        }

        [[nodiscard]] std::vector<std::string>
        permission_groups_from(const std::vector<std::string>& source) const
        {
            std::vector<std::string> out{};
            for (const auto& group: source) {
                for (auto&& expanded: accessible_groups(group)) {
                    if (std::find(out.begin(), out.end(), expanded) == out.end()) {
                        out.push_back(std::move(expanded));
                    }
                }
            }
            return out;
        }

        [[nodiscard]] std::string effective_product_type() const
        {
            if (entity_type == "organization" || entity_type == "tenant") {
                return {};
            }
            return product_type_filter.empty() ? std::string{"orchestration"} : product_type_filter;
        }
    };

    // Upstream predefined roles table (predefined_roles.rs:8-706). Transcribed in
    // process; exact id/name/entity/flags/groups preserved for the role logic.
    struct PredefinedRoleSeed
    {
        const char* id;
        const char* name;
        const char* entity;
        const char* scope;
        const char* product_type;
        bool invitable;
        bool deletable;
        bool updatable;
        bool internal;
        std::vector<std::string_view> groups;
    };

    inline const std::vector<PredefinedRoleSeed>& predefined_role_seeds()
    {
        static const std::vector<PredefinedRoleSeed> seeds{
            {"internal_admin",
             "internal_admin",
             "merchant",
             "organization",
             "",
             false,
             false,
             false,
             true,
             std::vector<std::string_view>{
                 "recon_api_key_view",
                 "recon_api_key_manage",
                 "recon_accounts_view",
                 "recon_accounts_manage",
                 "recon_audit_view",
                 "operations_view",
                 "operations_manage",
                 "connectors_view",
                 "connectors_manage",
                 "workflows_view",
                 "workflows_manage",
                 "analytics_view",
                 "users_view",
                 "users_manage",
                 "account_view",
                 "account_manage",
                 "recon_sources_view",
                 "recon_sources_manage",
                 "recon_exceptions_view",
                 "recon_exceptions_manage",
                 "recon_transactions_view",
                 "recon_transactions_manage",
                 "recon_rules_view",
                 "recon_rules_manage",
                 "alerts_view",
                 "alerts_manage",
                 "monitoring_view",
                 "monitoring_manage"
             }},
            {"internal_view_only",
             "internal_view_only",
             "merchant",
             "organization",
             "",
             false,
             false,
             false,
             true,
             std::vector<std::string_view>{
                 "recon_api_key_view",
                 "recon_accounts_view",
                 "recon_audit_view",
                 "operations_view",
                 "connectors_view",
                 "workflows_view",
                 "analytics_view",
                 "users_view",
                 "account_view",
                 "recon_sources_view",
                 "recon_exceptions_view",
                 "recon_transactions_view",
                 "recon_rules_view",
                 "alerts_view",
                 "monitoring_view"
             }},
            {"internal_operator",
             "internal_operator",
             "merchant",
             "organization",
             "",
             false,
             false,
             false,
             true,
             std::vector<std::string_view>{
                 "recon_api_key_view",
                 "recon_accounts_view",
                 "recon_audit_view",
                 "operations_view",
                 "connectors_view",
                 "workflows_view",
                 "analytics_view",
                 "users_view",
                 "account_view",
                 "recon_sources_view",
                 "recon_exceptions_view",
                 "recon_transactions_view",
                 "recon_rules_view",
                 "alerts_view",
                 "alerts_manage",
                 "monitoring_view",
                 "monitoring_manage"
             }},
            {"tenant_admin",
             "tenant_admin",
             "tenant",
             "organization",
             "",
             false,
             false,
             false,
             false,
             std::vector<std::string_view>{
                 "recon_api_key_view",
                 "recon_api_key_manage",
                 "recon_accounts_view",
                 "recon_accounts_manage",
                 "recon_audit_view",
                 "operations_view",
                 "operations_manage",
                 "connectors_view",
                 "connectors_manage",
                 "workflows_view",
                 "workflows_manage",
                 "analytics_view",
                 "users_view",
                 "users_manage",
                 "account_view",
                 "account_manage",
                 "recon_sources_view",
                 "recon_sources_manage",
                 "clone_connector_manage",
                 "recon_exceptions_view",
                 "recon_exceptions_manage",
                 "recon_transactions_view",
                 "recon_transactions_manage",
                 "recon_rules_view",
                 "recon_rules_manage"
             }},
            {"org_admin",
             "organization_admin",
             "organization",
             "organization",
             "",
             true,
             true,
             true,
             false,
             std::vector<std::string_view>{
                 "recon_api_key_view",
                 "recon_api_key_manage",
                 "recon_accounts_view",
                 "recon_accounts_manage",
                 "recon_audit_view",
                 "operations_view",
                 "operations_manage",
                 "connectors_view",
                 "connectors_manage",
                 "workflows_view",
                 "workflows_manage",
                 "analytics_view",
                 "users_view",
                 "users_manage",
                 "account_view",
                 "account_manage",
                 "clone_connector_manage",
                 "theme_view",
                 "theme_manage",
                 "configurations_view",
                 "configurations_manage",
                 "offers_view",
                 "offers_manage",
                 "recon_sources_view",
                 "recon_sources_manage",
                 "recon_exceptions_view",
                 "recon_exceptions_manage",
                 "recon_transactions_view",
                 "recon_transactions_manage",
                 "recon_rules_view",
                 "recon_rules_manage"
             }},
            {"merchant_admin",
             "merchant_admin",
             "merchant",
             "organization",
             "orchestration",
             true,
             true,
             true,
             false,
             std::vector<std::string_view>{
                 "operations_view",
                 "operations_manage",
                 "connectors_view",
                 "connectors_manage",
                 "workflows_view",
                 "workflows_manage",
                 "analytics_view",
                 "users_view",
                 "users_manage",
                 "account_view",
                 "account_manage",
                 "configurations_view",
                 "configurations_manage",
                 "offers_view",
                 "offers_manage",
                 "clone_connector_manage"
             }},
            {"merchant_view_only",
             "merchant_view_only",
             "merchant",
             "organization",
             "orchestration",
             true,
             true,
             true,
             false,
             std::vector<std::string_view>{
                 "operations_view",
                 "connectors_view",
                 "workflows_view",
                 "analytics_view",
                 "users_view",
                 "account_view"
             }},
            {"merchant_iam_admin",
             "merchant_iam",
             "merchant",
             "organization",
             "orchestration",
             true,
             true,
             true,
             false,
             std::vector<std::string_view>{
                 "operations_view",
                 "analytics_view",
                 "users_view",
                 "users_manage",
                 "account_view"
             }},
            {"merchant_developer",
             "merchant_developer",
             "merchant",
             "organization",
             "orchestration",
             true,
             true,
             true,
             false,
             std::vector<std::string_view>{
                 "operations_view",
                 "connectors_view",
                 "analytics_view",
                 "users_view",
                 "account_view",
                 "account_manage"
             }},
            {"merchant_operator",
             "merchant_operator",
             "merchant",
             "organization",
             "orchestration",
             true,
             true,
             true,
             false,
             std::vector<std::string_view>{
                 "operations_view",
                 "operations_manage",
                 "connectors_view",
                 "workflows_view",
                 "analytics_view",
                 "users_view",
                 "account_view"
             }},
            {"merchant_customer_support",
             "customer_support",
             "merchant",
             "organization",
             "orchestration",
             true,
             true,
             true,
             false,
             std::vector<std::string_view>{
                 "operations_view",
                 "analytics_view",
                 "users_view",
                 "account_view"
             }},
            {"profile_admin",
             "profile_admin",
             "profile",
             "organization",
             "orchestration",
             true,
             true,
             true,
             false,
             std::vector<std::string_view>{
                 "operations_view",
                 "operations_manage",
                 "connectors_view",
                 "connectors_manage",
                 "workflows_view",
                 "workflows_manage",
                 "analytics_view",
                 "users_view",
                 "users_manage",
                 "account_view",
                 "account_manage",
                 "configurations_view",
                 "configurations_manage",
                 "offers_view",
                 "offers_manage"
             }},
            {"profile_view_only",
             "profile_view_only",
             "profile",
             "organization",
             "orchestration",
             true,
             true,
             true,
             false,
             std::vector<std::string_view>{
                 "operations_view",
                 "connectors_view",
                 "workflows_view",
                 "analytics_view",
                 "users_view",
                 "account_view"
             }},
            {"profile_iam_admin",
             "profile_iam",
             "profile",
             "organization",
             "orchestration",
             true,
             true,
             true,
             false,
             std::vector<std::string_view>{
                 "operations_view",
                 "analytics_view",
                 "users_view",
                 "users_manage",
                 "account_view"
             }},
            {"profile_developer",
             "profile_developer",
             "profile",
             "organization",
             "orchestration",
             true,
             true,
             true,
             false,
             std::vector<std::string_view>{
                 "operations_view",
                 "connectors_view",
                 "analytics_view",
                 "users_view",
                 "account_view",
                 "account_manage"
             }},
            {"profile_operator",
             "profile_operator",
             "profile",
             "organization",
             "orchestration",
             true,
             true,
             true,
             false,
             std::vector<std::string_view>{
                 "operations_view",
                 "operations_manage",
                 "connectors_view",
                 "workflows_view",
                 "analytics_view",
                 "users_view",
                 "account_view"
             }},
            {"profile_customer_support",
             "profile_customer_support",
             "profile",
             "organization",
             "orchestration",
             true,
             true,
             true,
             false,
             std::vector<std::string_view>{
                 "operations_view",
                 "analytics_view",
                 "users_view",
                 "account_view"
             }},
            {"recon_merchant_admin",
             "recon_merchant_admin",
             "merchant",
             "organization",
             "recon",
             true,
             true,
             true,
             false,
             std::vector<std::string_view>{
                 "recon_accounts_view",
                 "recon_accounts_manage",
                 "recon_audit_view",
                 "recon_sources_view",
                 "recon_sources_manage",
                 "recon_exceptions_view",
                 "recon_exceptions_manage",
                 "recon_transactions_view",
                 "recon_transactions_manage",
                 "users_view",
                 "users_manage",
                 "recon_rules_view",
                 "recon_rules_manage",
                 "recon_api_key_view",
                 "recon_api_key_manage"
             }},
            {"recon_merchant_data_engineer",
             "recon_merchant_data_engineer",
             "merchant",
             "organization",
             "recon",
             true,
             true,
             true,
             false,
             std::vector<std::string_view>{
                 "recon_api_key_view",
                 "recon_accounts_view",
                 "recon_accounts_manage",
                 "recon_audit_view",
                 "recon_sources_view",
                 "recon_sources_manage",
                 "recon_transactions_view",
                 "users_view",
                 "recon_rules_view"
             }},
            {"recon_merchant_operator",
             "recon_merchant_operator",
             "merchant",
             "organization",
             "recon",
             true,
             true,
             true,
             false,
             std::vector<std::string_view>{
                 "recon_accounts_view",
                 "recon_audit_view",
                 "recon_exceptions_view",
                 "recon_exceptions_manage",
                 "recon_transactions_view",
                 "recon_transactions_manage",
                 "recon_sources_view",
                 "users_view",
                 "recon_rules_view"
             }},
            {"recon_merchant_view_only",
             "recon_merchant_view_only",
             "merchant",
             "organization",
             "recon",
             true,
             true,
             true,
             false,
             std::vector<std::string_view>{
                 "recon_accounts_view",
                 "recon_audit_view",
                 "recon_sources_view",
                 "recon_transactions_view",
                 "recon_exceptions_view",
                 "users_view",
                 "recon_rules_view",
                 "recon_api_key_view"
             }},
            {"recon_profile_admin",
             "recon_profile_admin",
             "profile",
             "organization",
             "recon",
             true,
             true,
             true,
             false,
             std::vector<std::string_view>{
                 "recon_accounts_view",
                 "recon_accounts_manage",
                 "recon_audit_view",
                 "recon_sources_view",
                 "recon_sources_manage",
                 "recon_exceptions_view",
                 "recon_exceptions_manage",
                 "recon_transactions_view",
                 "recon_transactions_manage",
                 "users_view",
                 "users_manage",
                 "recon_rules_view",
                 "recon_rules_manage"
             }},
            {"recon_profile_data_engineer",
             "recon_profile_data_engineer",
             "profile",
             "organization",
             "recon",
             true,
             true,
             true,
             false,
             std::vector<std::string_view>{
                 "recon_accounts_view",
                 "recon_accounts_manage",
                 "recon_audit_view",
                 "recon_sources_view",
                 "recon_sources_manage",
                 "recon_transactions_view",
                 "users_view",
                 "recon_rules_view"
             }},
            {"recon_profile_operator",
             "recon_profile_operator",
             "profile",
             "organization",
             "recon",
             true,
             true,
             true,
             false,
             std::vector<std::string_view>{
                 "recon_accounts_view",
                 "recon_audit_view",
                 "recon_exceptions_view",
                 "recon_exceptions_manage",
                 "recon_transactions_view",
                 "recon_transactions_manage",
                 "recon_sources_view",
                 "users_view",
                 "recon_rules_view"
             }},
            {"recon_profile_view_only",
             "recon_profile_view_only",
             "profile",
             "organization",
             "recon",
             true,
             true,
             true,
             false,
             std::vector<std::string_view>{
                 "recon_accounts_view",
                 "recon_audit_view",
                 "recon_sources_view",
                 "recon_transactions_view",
                 "recon_exceptions_view",
                 "users_view",
                 "recon_rules_view"
             }},
        };
        return seeds;
    }

    inline std::optional<RoleInfo> predefined_role(std::string_view role_id)
    {
        for (const auto& seed: predefined_role_seeds()) {
            if (role_id == seed.id) {
                RoleInfo info{};
                info.role_id = seed.id;
                info.role_name = seed.name;
                info.entity_type = seed.entity;
                info.scope = seed.scope;
                info.product_type_filter = seed.product_type;
                info.is_invitable = seed.invitable;
                info.is_deletable = seed.deletable;
                info.is_updatable = seed.updatable;
                info.is_internal = seed.internal;
                for (const auto group: seed.groups) {
                    info.groups.emplace_back(std::string{group});
                }
                return info;
            }
        }
        return std::nullopt;
    }

    inline std::vector<RoleInfo> all_predefined_roles()
    {
        std::vector<RoleInfo> out{};
        for (const auto& seed: predefined_role_seeds()) {
            out.push_back(*predefined_role(seed.id));
        }
        return out;
    }

    // Upstream `RoleName::new` (types/domain/user.rs:1364-1374).
    inline bool valid_role_name(const std::string& name)
    {
        if (name.empty()) {
            return false;
        }
        if (std::all_of(
                name.begin(),
                name.end(),
                [](unsigned char c)
                {
                    return std::isspace(c);
                }
            )) {
            return false;
        }
        if (name.size() > 64) {
            return false;
        }
        if (name.find(' ') != std::string::npos) {
            return false;
        }
        return true;
    }

    inline std::string lowercased(std::string value)
    {
        std::transform(
            value.begin(),
            value.end(),
            value.begin(),
            [](unsigned char c)
            {
                return static_cast<char>(std::tolower(c));
            }
        );
        return value;
    }

    inline std::string join(const std::vector<std::string>& values, std::string_view separator)
    {
        std::string out{};
        for (std::size_t i = 0; i < values.size(); ++i) {
            if (i != 0) {
                out += separator;
            }
            out += values[i];
        }
        return out;
    }

    inline std::vector<std::string> split(std::string_view value, char separator)
    {
        std::vector<std::string> out{};
        std::size_t start{0};
        while (start <= value.size()) {
            const auto pos = value.find(separator, start);
            if (pos == std::string_view::npos) {
                out.emplace_back(value.substr(start));
                break;
            }
            out.emplace_back(value.substr(start, pos - start));
            start = pos + 1;
        }
        return out;
    }

    // Upstream `validate_role_groups` (utils/user_role.rs:41-76).
    inline std::optional<core::error::AppError>
    validate_role_groups(const std::vector<std::string>& groups, std::string_view product_type)
    {
        if (groups.empty()) {
            return core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "Role groups cannot be empty"
            );
        }
        if (!product_type.empty()) {
            const auto categories = accessible_product_categories(product_type);
            for (const auto& group: groups) {
                const auto* def = perm_group(group);
                if (def == nullptr ||
                    std::find(categories.begin(), categories.end(), std::string{def->category}) ==
                        categories.end()) {
                    return core::error::AppErrorBuilder{}(
                        core::error::AppErrorKind::INVALID_REQUEST,
                        "Permission groups of different product types found"
                    );
                }
            }
        }
        std::set<std::string> unique{groups.begin(), groups.end()};
        for (const auto& group: unique) {
            if (is_internal_perm_group(group)) {
                return core::error::AppErrorBuilder{}(
                    core::error::AppErrorKind::INVALID_REQUEST,
                    "Invalid groups present in the custom role"
                );
            }
        }
        if (unique.size() != groups.size()) {
            return core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::INVALID_REQUEST,
                "Duplicate permission group found"
            );
        }
        return std::nullopt;
    }

} // namespace role_tables

// Shared business helpers for every user_role sub-runtime.
class UserRoleRuntimeSupport
{
public:
    explicit UserRoleRuntimeSupport(app::context::AppContext& context) noexcept :
        m_context{context},
        m_input{context}
    {
    }

    [[nodiscard]] core::error::AppError
    error(core::error::AppErrorKind kind, std::string message) const
    {
        return core::error::AppErrorBuilder{}.operator()(kind, std::move(message));
    }

    [[nodiscard]] std::string normalize_entity(std::string_view value) const
    {
        return m_input.normalize_entity(value);
    }

    void track_flow(std::string_view flow, std::string_view counter) const
    {
        m_context.metrics().record_flow(std::string{flow});
        m_context.metrics().counter(std::string{counter}).increment();
    }

    [[nodiscard]] std::string now_string() const
    {
        const auto seconds = std::chrono::duration_cast<std::chrono::seconds>(
                                 std::chrono::system_clock::now().time_since_epoch()
        )
                                 .count();
        return std::to_string(seconds);
    }

    [[nodiscard]] std::string entity_from_scope(std::string_view scope) const
    {
        if (scope == "organization" || scope == "merchant" || scope == "profile") {
            return std::string{scope};
        }
        return {};
    }

    // Upstream `utils::user_role::get_min_entity` (utils/user_role.rs:521-536).
    [[nodiscard]] std::optional<core::error::AppError>
    get_min_entity(std::string_view requestor, std::string_view filter) const
    {
        if (filter.empty()) {
            return std::nullopt;
        }
        if (role_tables::entity_rank(requestor) < role_tables::entity_rank(filter)) {
            return error(core::error::AppErrorKind::PRECONDITION_FAILED, "InvalidRoleOperation");
        }
        return std::nullopt;
    }

    [[nodiscard]] bool assignment_in_scope(
        const app::store::user_roles::UserRoleRecord& assignment,
        const RequestIdentity& identity
    ) const noexcept
    {
        if (!identity.merchant_id.empty() && !assignment.get_merchant_id().empty() &&
            assignment.get_merchant_id() != identity.merchant_id) {
            return false;
        }
        if (!identity.org_id.empty() && !assignment.get_organization_id().empty() &&
            assignment.get_organization_id() != identity.org_id) {
            return false;
        }
        return true;
    }

    [[nodiscard]] std::string merchant_product_type(
        const app::store::merchant_accounts::MerchantAccountRecord& merchant
    ) const
    {
        // Upstream defaults to Orchestration when product_type is absent.
        (void)merchant;
        return "orchestration";
    }

    // ── Role metadata side-store (local substitute for missing RoleRecord columns) ──
    void store_role_metadata(
        std::string_view role_id,
        const std::vector<std::string>& groups,
        std::string_view scope,
        std::string_view product_type
    ) const
    {
        m_context.cache().put(
            "user_role.groups." + std::string{role_id},
            core::cache::to_cache_value(role_tables::join(groups, ",")),
            core::cache::Ttl{}.seconds(86'400)
        );
        m_context.cache().put(
            "user_role.scope." + std::string{role_id},
            core::cache::to_cache_value(std::string{scope}),
            core::cache::Ttl{}.seconds(86'400)
        );
        m_context.cache().put(
            "user_role.product_type." + std::string{role_id},
            core::cache::to_cache_value(std::string{product_type}),
            core::cache::Ttl{}.seconds(86'400)
        );
    }

    [[nodiscard]] std::string cached_string(std::string_view key) const
    {
        const auto entry = m_context.cache().get(key);
        if (!entry.hit) {
            return {};
        }
        return core::cache::value_as<std::string>(entry.value, std::string{});
    }

    // Upstream `RoleInfo::from_role_id_*` (roles.rs:96-137) + `From<Role>` (roles.rs:140-155).
    [[nodiscard]] std::optional<role_tables::RoleInfo>
    role_info_from_id(const RequestIdentity& identity, std::string_view role_id) const
    {
        if (role_id.empty()) {
            return std::nullopt;
        }
        if (auto predefined = role_tables::predefined_role(role_id)) {
            return predefined;
        }
        const auto record = m_context.repository().find_role_by_string(role_id);
        if (!record) {
            return std::nullopt;
        }
        if (!identity.merchant_id.empty() && !record->get_merchant_id().empty() &&
            record->get_merchant_id() != identity.merchant_id) {
            return std::nullopt;
        }
        role_tables::RoleInfo info{};
        info.role_id = record->get_role_id();
        info.role_name = record->get_role_name();
        info.entity_type = record->get_entity_type();
        info.scope = cached_string("user_role.scope." + std::string{role_id});
        if (info.scope.empty()) {
            info.scope = "organization";
        }
        const auto groups = cached_string("user_role.groups." + std::string{role_id});
        if (!groups.empty()) {
            info.groups = role_tables::split(groups, ',');
        }
        info.product_type_filter = cached_string("user_role.product_type." + std::string{role_id});
        info.is_internal = record->get_is_internal() != 0;
        info.is_invitable = !info.is_internal;
        info.is_deletable = !info.is_internal;
        info.is_updatable = !info.is_internal && record->get_status() != "deleted";
        return info;
    }

    [[nodiscard]] std::vector<role_tables::RoleInfo> roles_for_entity(
        const RequestIdentity& identity,
        std::string_view requestor_entity,
        std::string_view entity_filter,
        bool include_internal
    ) const
    {
        (void)requestor_entity;
        std::vector<role_tables::RoleInfo> out{};
        for (const auto& role: role_tables::all_predefined_roles()) {
            if (!include_internal && role.is_internal) {
                continue;
            }
            if (!entity_filter.empty() && role.entity_type != entity_filter) {
                continue;
            }
            out.push_back(role);
        }
        for (const auto& record: m_context.repository().list_roles()) {
            auto info = role_info_from_id(identity, record.get_role_id());
            if (!info) {
                continue;
            }
            if (info->is_internal && !include_internal) {
                continue;
            }
            if (!entity_filter.empty() && info->entity_type != entity_filter) {
                continue;
            }
            out.push_back(*info);
        }
        return out;
    }

    // Upstream `validate_role_name` (utils/user_role.rs:78-128).
    [[nodiscard]] std::optional<core::error::AppError> validate_role_name(
        const std::string& name,
        const RequestIdentity& identity,
        std::string_view entity_type,
        std::string_view exclude_role_id = {}
    ) const
    {
        (void)identity;
        for (const auto& role: role_tables::all_predefined_roles()) {
            if (role.role_name == name) {
                return error(core::error::AppErrorKind::ALREADY_EXISTS, "RoleNameAlreadyExists");
            }
        }
        for (const auto& record: m_context.repository().list_roles()) {
            if (record.get_role_id() == exclude_role_id) {
                continue;
            }
            if (!entity_type.empty() && record.get_entity_type() != entity_type) {
                continue;
            }
            if (record.get_role_name() == name) {
                return error(core::error::AppErrorKind::ALREADY_EXISTS, "RoleNameAlreadyExists");
            }
        }
        return std::nullopt;
    }

    // Upstream hierarchy checks in role::create_role / create_role_v2 (role.rs:88-115,216-243).
    [[nodiscard]] std::optional<core::error::AppError> validate_create_role(
        const CreateRoleInputWire& input,
        const RequestIdentity& identity,
        const role_tables::RoleInfo& requestor
    ) const
    {
        (void)identity;
        const auto role_entity_input = normalize_entity(input.get_entity_type());
        const auto role_entity =
            role_entity_input.empty() ? std::string{"merchant"} : role_entity_input;
        if (role_entity == "organization") {
            return error(
                core::error::AppErrorKind::PRECONDITION_FAILED,
                "User trying to create org level custom role"
            );
        }
        const auto scope_entity = entity_from_scope(input.get_role_scope());
        if (role_tables::entity_rank(scope_entity) < role_tables::entity_rank(role_entity)) {
            return error(core::error::AppErrorKind::PRECONDITION_FAILED, "InvalidRoleOperation");
        }
        const auto max_entity =
            std::max(role_tables::entity_rank(scope_entity), role_tables::entity_rank(role_entity));
        if (role_tables::entity_rank(requestor.entity_type) < max_entity) {
            return error(core::error::AppErrorKind::PRECONDITION_FAILED, "InvalidRoleOperation");
        }
        if (!role_tables::valid_role_name(input.get_role_name())) {
            return error(core::error::AppErrorKind::VALIDATION_FAILED, "RoleNameParsingError");
        }
        return std::nullopt;
    }

    // Upstream `parent_group_info_request_to_permission_groups` (utils/user_role.rs:538-565).
    [[nodiscard]] std::expected<std::vector<std::string>, core::error::AppError>
    parent_group_info_request_to_permission_groups(
        const std::vector<ParentGroupInfoRequestWire>& parents
    ) const
    {
        std::vector<std::string> out{};
        for (const auto& parent: parents) {
            if (parent.get_scopes().empty()) {
                return std::unexpected{
                    error(core::error::AppErrorKind::INVALID_REQUEST, "InvalidRoleOperation")
                };
            }
            for (const auto& def: role_tables::perm_groups()) {
                if (def.parent != parent.get_name()) {
                    continue;
                }
                const std::string scope{def.scope};
                if (std::find(parent.get_scopes().begin(), parent.get_scopes().end(), scope) !=
                    parent.get_scopes().end()) {
                    out.emplace_back(def.name);
                }
            }
        }
        return out;
    }

    // Upstream `permission_groups_to_parent_group_info` + `resources_to_description`
    // (utils/user_role.rs:567-619), used by create_role_v2.
    [[nodiscard]] std::vector<ParentGroupDescriptionWire> parent_group_descriptions_from_groups(
        const std::vector<std::string>& groups,
        std::string_view entity
    ) const
    {
        return role_tables::descriptions_for_groups(entity, groups);
    }

    [[nodiscard]] std::vector<GroupInfoWire> group_authorization_info() const
    {
        std::vector<GroupInfoWire> out{};
        for (const auto& def: role_tables::perm_groups()) {
            if (def.description.empty()) {
                continue;
            }
            GroupInfoWire item{};
            item.set_group(std::string{def.name});
            item.set_description(std::string{def.description});
            out.push_back(std::move(item));
        }
        return out;
    }

    [[nodiscard]] std::vector<ParentInfoWire> parent_authorization_info() const
    {
        std::vector<ParentInfoWire> out{};
        for (const auto& def: role_tables::perm_groups()) {
            if (def.description.empty()) {
                continue;
            }
            const std::string parent{def.parent};
            auto it = std::find_if(
                out.begin(),
                out.end(),
                [&](const ParentInfoWire& item)
                {
                    return item.get_name() == parent;
                }
            );
            if (it != out.end()) {
                auto groups = it->get_groups();
                groups.push_back(std::string{def.name});
                it->set_groups(std::move(groups));
            } else {
                ParentInfoWire item{};
                item.set_name(parent);
                item.set_description(parent_description(parent));
                item.set_groups({std::string{def.name}});
                out.push_back(std::move(item));
            }
        }
        return out;
    }

    [[nodiscard]] std::string parent_description(std::string_view parent) const
    {
        if (parent == "operations") {
            return "Payments, Refunds, Payouts, Mandates, Disputes and Customers";
        }
        if (parent == "connectors") {
            return "Create, modify and delete connectors like Payment Processors, Payout "
                   "Processors and Fraud & Risk Manager";
        }
        if (parent == "workflows") {
            return "Create, modify and delete Routing, 3DS Decision Manager, Surcharge Decision "
                   "Manager";
        }
        if (parent == "analytics") {
            return "View Analytics";
        }
        if (parent == "users") {
            return "Manage and invite Users to the Team";
        }
        if (parent == "account") {
            return "Create, modify and delete Merchant Details like api keys, webhooks, etc";
        }
        if (parent == "theme") {
            return "Manage and view themes for the organization";
        }
        if (parent == "configurations") {
            return "Manage and view configurations";
        }
        if (parent == "clone_connector") {
            return "Clone a connector and its configuration";
        }
        if (parent == "recon_sources") {
            return "Recon ingestion and transformation pipelines";
        }
        if (parent == "recon_exceptions") {
            return "Recon exception investigation and resolution";
        }
        if (parent == "recon_transactions") {
            return "Recon staging entries and transactions";
        }
        if (parent == "recon_rules") {
            return "Reconciliation rules";
        }
        if (parent == "recon_api_key") {
            return "Recon API keys";
        }
        if (parent == "recon_accounts") {
            return "Recon ledger accounts and balances";
        }
        if (parent == "recon_audit") {
            return "Recon audit trail";
        }
        if (parent == "offers") {
            return "Manage and view offers";
        }
        return {};
    }

    [[nodiscard]] std::optional<app::store::users::UserRecord>
    active_user_by_email(std::string_view email) const
    {
        for (const auto& user: m_context.database().all<app::store::users::UserRecord>()) {
            if (user.get_email() == email && user.get_status() == "active") {
                return user;
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] std::optional<app::store::users::UserRecord>
    active_user_by_id(std::string_view user_id) const
    {
        for (const auto& user: m_context.database().all<app::store::users::UserRecord>()) {
            if (user.get_user_id() == user_id && user.get_status() == "active") {
                return user;
            }
        }
        return std::nullopt;
    }

    // Upstream `get_lineage_for_user_id_and_entity_for_accepting_invite`
    // (utils/user_role.rs:327-489): resolve each requested entity to a lineage triple.
    [[nodiscard]] std::vector<Lineage> lineages_for_accept(
        const RequestIdentity& identity,
        const std::vector<AcceptEntityWire>& entities
    ) const
    {
        std::vector<Lineage> out{};
        for (const auto& entity: entities) {
            const auto entity_type = normalize_entity(entity.get_entity_type());
            if (entity_type.empty()) {
                continue;
            }
            for (const auto& assignment: m_context.repository().list_user_roles()) {
                if (assignment.get_user_id() != identity.user_id ||
                    assignment.get_status() != "invitation_sent") {
                    continue;
                }
                Lineage lineage{
                    assignment.get_organization_id(),
                    assignment.get_merchant_id(),
                    assignment.get_profile_id()
                };
                if (entity_type == "organization" && lineage.org_id == entity.get_entity_id()) {
                    out.push_back(lineage);
                } else if (
                    entity_type == "merchant" && lineage.merchant_id == entity.get_entity_id()
                ) {
                    out.push_back(lineage);
                } else if (
                    entity_type == "profile" && lineage.profile_id == entity.get_entity_id()
                ) {
                    out.push_back(lineage);
                }
            }
        }
        return out;
    }

    [[nodiscard]] bool lineage_matches(
        const app::store::user_roles::UserRoleRecord& assignment,
        const std::vector<Lineage>& lineages
    ) const noexcept
    {
        for (const auto& lineage: lineages) {
            if ((lineage.org_id.empty() || lineage.org_id == assignment.get_organization_id()) &&
                (lineage.merchant_id.empty() ||
                 lineage.merchant_id == assignment.get_merchant_id()) &&
                (lineage.profile_id.empty() || lineage.profile_id == assignment.get_profile_id())) {
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] bool lineage_matches_entity(
        const app::store::user_roles::UserRoleRecord& assignment,
        std::string_view entity
    ) const noexcept
    {
        if (entity.empty()) {
            return true;
        }
        if (entity == "organization") {
            return !assignment.get_organization_id().empty();
        }
        if (entity == "merchant") {
            return !assignment.get_merchant_id().empty();
        }
        if (entity == "profile") {
            return !assignment.get_profile_id().empty();
        }
        return true;
    }

    [[nodiscard]] std::pair<std::string, std::string>
    entity_id_and_type(const app::store::user_roles::UserRoleRecord& assignment) const noexcept
    {
        if (!assignment.get_profile_id().empty()) {
            return {assignment.get_profile_id(), "profile"};
        }
        if (!assignment.get_merchant_id().empty()) {
            return {assignment.get_merchant_id(), "merchant"};
        }
        if (!assignment.get_organization_id().empty()) {
            return {assignment.get_organization_id(), "organization"};
        }
        return {};
    }

    [[nodiscard]] std::optional<std::string> entity_name_for(
        const app::store::user_roles::UserRoleRecord& assignment,
        std::string_view entity_type
    ) const
    {
        if (entity_type == "merchant" && !assignment.get_merchant_id().empty()) {
            const auto merchant = m_context.repository().find_merchant_account_by_string(
                assignment.get_merchant_id()
            );
            if (merchant) {
                return merchant->get_merchant_name();
            }
        }
        if (entity_type == "organization" && !assignment.get_organization_id().empty()) {
            const auto org = m_context.repository().find_organization_by_string(
                assignment.get_organization_id()
            );
            if (org) {
                return org->get_organization_name();
            }
        }
        if (entity_type == "profile" && !assignment.get_profile_id().empty()) {
            const auto profile =
                m_context.repository().find_profile_by_string(assignment.get_profile_id());
            if (profile) {
                return profile->get_profile_name();
            }
        }
        return std::nullopt;
    }

    // Upstream `CurrentFlow::next`/`get_token` stand-in: signed single-purpose token.
    [[nodiscard]] std::string issue_flow_token(
        std::string_view user_id,
        std::string_view next_flow,
        const std::vector<std::string>& roles
    ) const
    {
        const auto payload = std::format(
            R"({{"sub":"{}","user_id":"{}","purpose":"AcceptInvite","flow":"{}","roles":{}}})",
            user_id,
            user_id,
            next_flow,
            serde::Ser::encode_json(roles)
        );
        return app::crypto::JwtSigner{invite_secret()}.sign(payload);
    }

    [[nodiscard]] std::string invite_secret() const
    {
        const auto stored = m_context.key_store().get("jwt", "accept_invite");
        return stored ? stored->value() : std::string{"user-role-invite-secret"};
    }

    [[nodiscard]] std::string
    blacklist_key(std::string_view kind, std::string_view fingerprint) const
    {
        return std::string{"user_role.blacklist."} + std::string{kind} + ":" +
               app::crypto::KeyedHasher{"user-role-blacklist-pepper"}.hash(fingerprint);
    }

    void blacklist(std::string_view kind, std::string_view fingerprint) const
    {
        m_context.cache().put(
            blacklist_key(kind, fingerprint),
            core::cache::to_cache_value(std::string{"1"}),
            core::cache::Ttl{}.seconds(3'600)
        );
    }

private:
    app::context::AppContext& m_context;
    UserRoleInputSupport m_input;
};

} // namespace app::handlers::user_role
