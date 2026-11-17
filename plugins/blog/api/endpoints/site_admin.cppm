module;
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <expected>

export module blog_api:endpoints:site_admin;

import std;
import blog_api:types;
import blog_database:database;
import core_router;
import interfaces;

export namespace ghost::blog::endpoints {

using json = nlohmann::json;

/**
 * Site Admin Controller - Matches Ghost's site.js controller
 */
class SiteAdminController {
public:
    explicit SiteAdminController(std::shared_ptr<IDatabase> database) noexcept
        : m_database{std::move(database)} {}

    struct Site {
        std::string m_id{};
        std::string m_uuid{};
        std::string m_title{};
        std::string m_description{};
        std::string m_logo{};
        std::string m_icon{};
        std::string m_cover_image{};
        std::string m_facebook{};
        std::string m_twitter{};
        std::string m_url{};
        std::string m_version{};
        std::string m_timezone{};
        std::string m_locale{};
        std::string m_language{};
        std::string m_ghost_version{};
        std::string m_environment{};
        json m_meta{};
        
        Site() noexcept = default;
        
        Site& add_title(std::string&& title) noexcept { m_title = std::move(title); return *this; }
        Site& add_description(std::string&& desc) noexcept { m_description = std::move(desc); return *this; }
        Site& add_url(std::string&& url) noexcept { m_url = std::move(url); return *this; }
        
        void set_id(std::string&& id) noexcept { m_id = std::move(id); }
        void set_uuid(std::string&& uuid) noexcept { m_uuid = std::move(uuid); }
        void set_title(std::string&& title) noexcept { m_title = std::move(title); }
        void set_description(std::string&& desc) noexcept { m_description = std::move(desc); }
        void set_logo(std::string&& logo) noexcept { m_logo = std::move(logo); }
        void set_icon(std::string&& icon) noexcept { m_icon = std::move(icon); }
        void set_cover_image(std::string&& cover) noexcept { m_cover_image = std::move(cover); }
        void set_facebook(std::string&& fb) noexcept { m_facebook = std::move(fb); }
        void set_twitter(std::string&& twitter) noexcept { m_twitter = std::move(twitter); }
        void set_url(std::string&& url) noexcept { m_url = std::move(url); }
        void set_version(std::string&& version) noexcept { m_version = std::move(version); }
        void set_timezone(std::string&& tz) noexcept { m_timezone = std::move(tz); }
        void set_locale(std::string&& locale) noexcept { m_locale = std::move(locale); }
        void set_language(std::string&& lang) noexcept { m_language = std::move(lang); }
        void set_ghost_version(std::string&& version) noexcept { m_ghost_version = std::move(version); }
        void set_environment(std::string&& env) noexcept { m_environment = std::move(env); }
        void set_meta(json&& meta) noexcept { m_meta = std::move(meta); }
        
        std::string& get_id() noexcept { return m_id; }
        std::string& get_uuid() noexcept { return m_uuid; }
        std::string& get_title() noexcept { return m_title; }
        std::string& get_description() noexcept { return m_description; }
        std::string& get_logo() noexcept { return m_logo; }
        std::string& get_icon() noexcept { return m_icon; }
        std::string& get_cover_image() noexcept { return m_cover_image; }
        std::string& get_facebook() noexcept { return m_facebook; }
        std::string& get_twitter() noexcept { return m_twitter; }
        std::string& get_url() noexcept { return m_url; }
        std::string& get_version() noexcept { return m_version; }
        std::string& get_timezone() noexcept { return m_timezone; }
        std::string& get_locale() noexcept { return m_locale; }
        std::string& get_language() noexcept { return m_language; }
        std::string& get_ghost_version() noexcept { return m_ghost_version; }
        std::string& get_environment() noexcept { return m_environment; }
        json& get_meta() noexcept { return m_meta; }
        
        const std::string& get_id() const noexcept { return m_id; }
        const std::string& get_uuid() const noexcept { return m_uuid; }
        const std::string& get_title() const noexcept { return m_title; }
        const std::string& get_description() const noexcept { return m_description; }
        const std::string& get_logo() const noexcept { return m_logo; }
        const std::string& get_icon() const noexcept { return m_icon; }
        const std::string& get_cover_image() const noexcept { return m_cover_image; }
        const std::string& get_facebook() const noexcept { return m_facebook; }
        const std::string& get_twitter() const noexcept { return m_twitter; }
        const std::string& get_url() const noexcept { return m_url; }
        const std::string& get_version() const noexcept { return m_version; }
        const std::string& get_timezone() const noexcept { return m_timezone; }
        const std::string& get_locale() const noexcept { return m_locale; }
        const std::string& get_language() const noexcept { return m_language; }
        const std::string& get_ghost_version() const noexcept { return m_ghost_version; }
        const std::string& get_environment() const noexcept { return m_environment; }
        const json& get_meta() const noexcept { return m_meta; }
        
        json to_json() const noexcept {
            json j;
            j["id"] = m_id;
            j["uuid"] = m_uuid;
            j["title"] = m_title;
            j["description"] = m_description;
            j["logo"] = m_logo;
            j["icon"] = m_icon;
            j["cover_image"] = m_cover_image;
            j["facebook"] = m_facebook;
            j["twitter"] = m_twitter;
            j["url"] = m_url;
            j["version"] = m_version;
            j["timezone"] = m_timezone;
            j["locale"] = m_locale;
            j["language"] = m_language;
            j["ghost_version"] = m_ghost_version;
            j["environment"] = m_environment;
            j["meta"] = m_meta;
            return j;
        }
    };

    // Read site
    std::expected<ApiResponse<Site>, std::string> read() noexcept {
        Site site;
        site.set_id("1");
        site.set_uuid("site-uuid");
        site.set_title("Ghost Blog");
        site.set_description("A Ghost Blog");
        site.set_url("https://example.com");
        site.set_version("1.0.0");
        site.set_timezone("UTC");
        site.set_locale("en_US");
        site.set_language("en");
        site.set_ghost_version("5.0.0");
        site.set_environment("production");
        return ApiResponse<Site>::ok(std::move(site));
    }

private:
    std::shared_ptr<IDatabase> m_database;
};

/**
 * Register site admin routes
 */
void register_site_admin_routes(core::router::RouterContext<256, 64, 10>& router_ctx,
                                std::shared_ptr<IDatabase> database) {
    SiteAdminController controller{std::move(database)};
    
    router_ctx.add_route(core::router::Route<8, 10>{"/admin/site/"}
        .get([controller = std::move(controller)](
            interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) mutable {
            send();
        }))
        .set_router_number(1);
}

} // namespace ghost::blog::endpoints