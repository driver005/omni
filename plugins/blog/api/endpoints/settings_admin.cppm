module;
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <expected>

export module blog_api:endpoints:settings_admin;

import std;
import blog_api:types;
import blog_database:database;
import core_router;
import interfaces;

export namespace ghost::blog::endpoints {

using json = nlohmann::json;

/**
 * Settings Admin Controller - Matches Ghost's settings.js controller
 */
class SettingsAdminController {
public:
    explicit SettingsAdminController(std::shared_ptr<IDatabase> database) noexcept
        : m_database{std::move(database)} {}

    struct Setting {
        std::string m_key{};
        std::string m_value{};
        std::string m_type{};  // "string", "boolean", "number", "json"
        std::string m_group{};
        std::string m_description{};
        bool m_public = false;
        
        Setting() noexcept = default;
        
        Setting& add_key(std::string&& key) noexcept { m_key = std::move(key); return *this; }
        Setting& add_value(std::string&& value) noexcept { m_value = std::move(value); return *this; }
        Setting& add_type(std::string&& type) noexcept { m_type = std::move(type); return *this; }
        
        void set_key(std::string&& key) noexcept { m_key = std::move(key); }
        void set_value(std::string&& value) noexcept { m_value = std::move(value); }
        void set_type(std::string&& type) noexcept { m_type = std::move(type); }
        void set_group(std::string&& group) noexcept { m_group = std::move(group); }
        void set_description(std::string&& desc) noexcept { m_description = std::move(desc); }
        void set_public(bool public_) noexcept { m_public = public_; }
        
        std::string& get_key() noexcept { return m_key; }
        std::string& get_value() noexcept { return m_value; }
        std::string& get_type() noexcept { return m_type; }
        std::string& get_group() noexcept { return m_group; }
        std::string& get_description() noexcept { return m_description; }
        bool& get_public() noexcept { return m_public; }
        
        const std::string& get_key() const noexcept { return m_key; }
        const std::string& get_value() const noexcept { return m_value; }
        const std::string& get_type() const noexcept { return m_type; }
        const std::string& get_group() const noexcept { return m_group; }
        const std::string& get_description() const noexcept { return m_description; }
        bool get_public() const noexcept { return m_public; }
        
        json to_json() const noexcept {
            json j;
            j["key"] = m_key;
            j["value"] = m_value;
            j["type"] = m_type;
            j["group"] = m_group;
            j["description"] = m_description;
            j["public"] = m_public;
            return j;
        }
    };

    struct BrowseOptions {
        std::string m_include{};
        std::string m_filter{};
        std::string m_fields{};
        int m_limit = 15;
        std::string m_order{};
        int m_page = 1;
        bool m_debug = false;
        
        BrowseOptions() noexcept = default;
        BrowseOptions& add_include(std::string&& include) noexcept { m_include = std::move(include); return *this; }
        BrowseOptions& add_filter(std::string&& filter) noexcept { m_filter = std::move(filter); return *this; }
        BrowseOptions& add_fields(std::string&& fields) noexcept { m_fields = std::move(fields); return *this; }
        BrowseOptions& add_limit(int limit) noexcept { m_limit = limit; return *this; }
        BrowseOptions& add_order(std::string&& order) noexcept { m_order = std::move(order); return *this; }
        BrowseOptions& add_page(int page) noexcept { m_page = page; return *this; }
        BrowseOptions& add_debug(bool debug) noexcept { m_debug = debug; return *this; }
        
        void set_include(std::string&& include) noexcept { m_include = std::move(include); }
        void set_filter(std::string&& filter) noexcept { m_filter = std::move(filter); }
        void set_fields(std::string&& fields) noexcept { m_fields = std::move(fields); }
        void set_limit(int limit) noexcept { m_limit = limit; }
        void set_order(std::string&& order) noexcept { m_order = std::move(order); }
        void set_page(int page) noexcept { m_page = page; }
        void set_debug(bool debug) noexcept { m_debug = debug; }
        
        std::string& get_include() noexcept { return m_include; }
        std::string& get_filter() noexcept { return m_filter; }
        std::string& get_fields() noexcept { return m_fields; }
        int& get_limit() noexcept { return m_limit; }
        std::string& get_order() noexcept { return m_order; }
        int& get_page() noexcept { return m_page; }
        bool& get_debug() noexcept { return m_debug; }
        
        const std::string& get_include() const noexcept { return m_include; }
        const std::string& get_filter() const noexcept { return m_filter; }
        const std::string& get_fields() const noexcept { return m_fields; }
        int get_limit() const noexcept { return m_limit; }
        const std::string& get_order() const noexcept { return m_order; }
        int get_page() const noexcept { return m_page; }
        bool get_debug() const noexcept { return m_debug; }
    };

    // Browse settings
    std::expected<ApiResponse<std::vector<Setting>>, std::string> browse(const BrowseOptions& options) noexcept {
        std::vector<Setting> settings;
        return ApiResponse<std::vector<Setting>>::ok(std::move(settings));
    }

    // Edit settings (bulk)
    std::expected<ApiResponse<std::vector<Setting>>, std::string> edit(const std::vector<Setting>& settings) noexcept {
        return ApiResponse<std::vector<Setting>>::ok(settings);
    }

    // Download routes YAML
    std::expected<ApiResponse<json>, std::string> download_routes() noexcept {
        json routes;
        routes["routes"] = json::array();
        return ApiResponse<json>::ok(std::move(routes));
    }

    // Upload routes YAML
    std::expected<ApiResponse<json>, std::string> upload_routes(const std::string& file_path) noexcept {
        json result;
        result["imported"] = 0;
        return ApiResponse<json>::ok(std::move(result));
    }

    // Regenerate access code
    std::expected<ApiResponse<json>, std::string> regenerate_access_code() noexcept {
        return ApiResponse<json>::ok(json{{"access_code", "new_access_code_123"}});
    }

    // Verify key update
    std::expected<ApiResponse<json>, std::string> verify_key_update(const json& data) noexcept {
        return ApiResponse<json>::ok(json{{"verified", true}});
    }

    // Disconnect Stripe Connect
    std::expected<ApiResponse<bool>, std::string> disconnect_stripe_connect() noexcept {
        return ApiResponse<bool>::ok(true);
    }

private:
    std::shared_ptr<IDatabase> m_database;
};

/**
 * Register settings admin routes
 */
void register_settings_admin_routes(core::router::RouterContext<256, 64, 10>& router_ctx,
                                    std::shared_ptr<IDatabase> database) {
    SettingsAdminController controller{std::move(database)};
    
    router_ctx.add_route(core::router::Route<8, 10>{"/admin/settings/"}
        .get([controller = std::move(controller)](
            interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) mutable {
            send();
        }))
        .put([controller = std::move(controller)](
            interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) mutable {
            send();
        })
        .set_router_number(1);
    
    router_ctx.add_route(core::router::Route<8, 10>{"/admin/settings/routes/yaml/"}
        .get([controller = std::move(controller)](
            interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) mutable {
            send();
        }))
        .post([controller = std::move(controller)](
            interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) mutable {
            send();
        })
        .set_router_number(1);
    
    router_ctx.add_route(core::router::Route<8, 10>{"/admin/settings/access_code/regenerate/"}
        .post([controller = std::move(controller)](
            interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) mutable {
            send();
        }))
        .set_router_number(1);
    
    router_ctx.add_route(core::router::Route<8, 10>{"/admin/settings/verifications/"}
        .put([controller = std::move(controller)](
            interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) mutable {
            send();
        }))
        .set_router_number(1);
    
    router_ctx.add_route(core::router::Route<8, 10>{"/admin/settings/stripe/connect/"}
        .delete_([controller = std::move(controller)](
            interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) mutable {
            send();
        }))
        .set_router_number(1);
}

} // namespace ghost::blog::endpoints