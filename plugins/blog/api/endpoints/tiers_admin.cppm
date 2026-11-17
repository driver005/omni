module;
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <expected>

export module blog_api:endpoints:tiers_admin;

import std;
import blog_api:types;
import blog_database:database;
import core_router;
import interfaces;

export namespace ghost::blog::endpoints {

using json = nlohmann::json;

/**
 * Tiers Admin Controller - Matches Ghost's tiers.js controller
 */
class TiersAdminController {
public:
    explicit TiersAdminController(std::shared_ptr<IDatabase> database) noexcept
        : m_database{std::move(database)} {}

    struct Tier {
        std::string m_id{};
        std::string m_uuid{};
        std::string m_name{};
        std::string m_slug{};
        std::string m_description{};
        std::string m_type{};  // "free", "paid"
        std::string m_active{};  // "active", "inactive"
        std::string m_visibility{};  // "public", "none"
        int m_monthly_price = 0;
        int m_yearly_price = 0;
        int m_trial_days = 0;
        std::string m_currency{"USD"};
        std::string m_welcome_page_url{};
        std::string m_created_at{};
        std::string m_updated_at{};
        json m_meta{};
        std::vector<json> m_benefits{};
        
        Tier() noexcept = default;
        
        Tier& add_name(std::string&& name) noexcept { m_name = std::move(name); return *this; }
        Tier& add_slug(std::string&& slug) noexcept { m_slug = std::move(slug); return *this; }
        Tier& add_type(std::string&& type) noexcept { m_type = std::move(type); return *this; }
        
        void set_id(std::string&& id) noexcept { m_id = std::move(id); }
        void set_uuid(std::string&& uuid) noexcept { m_uuid = std::move(uuid); }
        void set_name(std::string&& name) noexcept { m_name = std::move(name); }
        void set_slug(std::string&& slug) noexcept { m_slug = std::move(slug); }
        void set_description(std::string&& desc) noexcept { m_description = std::move(desc); }
        void set_type(std::string&& type) noexcept { m_type = std::move(type); }
        void set_active(std::string&& active) noexcept { m_active = std::move(active); }
        void set_visibility(std::string&& visibility) noexcept { m_visibility = std::move(visibility); }
        void set_monthly_price(int price) noexcept { m_monthly_price = price; }
        void set_yearly_price(int price) noexcept { m_yearly_price = price; }
        void set_trial_days(int days) noexcept { m_trial_days = days; }
        void set_currency(std::string&& currency) noexcept { m_currency = std::move(currency); }
        void set_welcome_page_url(std::string&& url) noexcept { m_welcome_page_url = std::move(url); }
        void set_created_at(std::string&& created) noexcept { m_created_at = std::move(created); }
        void set_updated_at(std::string&& updated) noexcept { m_updated_at = std::move(updated); }
        void set_meta(json&& meta) noexcept { m_meta = std::move(meta); }
        void set_benefits(std::vector<json>&& benefits) noexcept { m_benefits = std::move(benefits); }
        
        std::string& get_id() noexcept { return m_id; }
        std::string& get_uuid() noexcept { return m_uuid; }
        std::string& get_name() noexcept { return m_name; }
        std::string& get_slug() noexcept { return m_slug; }
        std::string& get_description() noexcept { return m_description; }
        std::string& get_type() noexcept { return m_type; }
        std::string& get_active() noexcept { return m_active; }
        std::string& get_visibility() noexcept { return m_visibility; }
        int& get_monthly_price() noexcept { return m_monthly_price; }
        int& get_yearly_price() noexcept { return m_yearly_price; }
        int& get_trial_days() noexcept { return m_trial_days; }
        std::string& get_currency() noexcept { return m_currency; }
        std::string& get_welcome_page_url() noexcept { return m_welcome_page_url; }
        std::string& get_created_at() noexcept { return m_created_at; }
        std::string& get_updated_at() noexcept { return m_updated_at; }
        json& get_meta() noexcept { return m_meta; }
        std::vector<json>& get_benefits() noexcept { return m_benefits; }
        
        const std::string& get_id() const noexcept { return m_id; }
        const std::string& get_uuid() const noexcept { return m_uuid; }
        const std::string& get_name() const noexcept { return m_name; }
        const std::string& get_slug() const noexcept { return m_slug; }
        const std::string& get_description() const noexcept { return m_description; }
        const std::string& get_type() const noexcept { return m_type; }
        const std::string& get_active() const noexcept { return m_active; }
        const std::string& get_visibility() const noexcept { return m_visibility; }
        int get_monthly_price() const noexcept { return m_monthly_price; }
        int get_yearly_price() const noexcept { return m_yearly_price; }
        int get_trial_days() const noexcept { return m_trial_days; }
        const std::string& get_currency() const noexcept { return m_currency; }
        const std::string& get_welcome_page_url() const noexcept { return m_welcome_page_url; }
        const std::string& get_created_at() const noexcept { return m_created_at; }
        const std::string& get_updated_at() const noexcept { return m_updated_at; }
        const json& get_meta() const noexcept { return m_meta; }
        const std::vector<json>& get_benefits() const noexcept { return m_benefits; }
        
        json to_json() const noexcept {
            json j;
            j["id"] = m_id;
            j["uuid"] = m_uuid;
            j["name"] = m_name;
            j["slug"] = m_slug;
            j["description"] = m_description;
            j["type"] = m_type;
            j["active"] = m_active;
            j["visibility"] = m_visibility;
            j["monthly_price"] = m_monthly_price;
            j["yearly_price"] = m_yearly_price;
            j["trial_days"] = m_trial_days;
            j["currency"] = m_currency;
            j["welcome_page_url"] = m_welcome_page_url;
            j["created_at"] = m_created_at;
            j["updated_at"] = m_updated_at;
            j["meta"] = m_meta;
            j["benefits"] = m_benefits;
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

    // Browse tiers
    std::expected<ApiResponse<std::vector<Tier>>, std::string> browse(const BrowseOptions& options) noexcept {
        std::vector<Tier> tiers;
        return ApiResponse<std::vector<Tier>>::ok(std::move(tiers));
    }

    // Read tier
    std::expected<ApiResponse<Tier>, std::string> read(const std::string& identifier) noexcept {
        Tier tier;
        tier.set_id(identifier);
        return ApiResponse<Tier>::ok(std::move(tier));
    }

    // Add tier
    std::expected<ApiResponse<Tier>, std::string> add(const Tier& tier) noexcept {
        return ApiResponse<Tier>::created(tier);
    }

    // Edit tier
    std::expected<ApiResponse<Tier>, std::string> edit(const std::string& id, const Tier& tier) noexcept {
        Tier updated = tier;
        updated.set_id(id);
        return ApiResponse<Tier>::ok(std::move(updated));
    }

private:
    std::shared_ptr<IDatabase> m_database;
};

/**
 * Register tiers admin routes
 */
void register_tiers_admin_routes(core::router::RouterContext<256, 64, 10>& router_ctx,
                                 std::shared_ptr<IDatabase> database) {
    TiersAdminController controller{std::move(database)};
    
    router_ctx.add_route(core::router::Route<8, 10>{"/admin/tiers/"}
        .get([controller = std::move(controller)](
            interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) mutable {
            send();
        }))
        .set_router_number(1);
    
    router_ctx.add_route(core::router::Route<8, 10>{"/admin/tiers/:id/"}
        .get([controller = std::move(controller)](
            interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) mutable {
            send();
        }))
        .put([controller = std::move(controller)](
            interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) mutable {
            send();
        })
        .set_router_number(1);
}

} // namespace ghost::blog::endpoints