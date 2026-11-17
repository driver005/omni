module;
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <expected>

export module blog_api:endpoints:newsletters_admin;

import std;
import blog_api:types;
import blog_database:database;
import core_router;
import interfaces;

export namespace ghost::blog::endpoints {

using json = nlohmann::json;

/**
 * Newsletters Admin Controller - Matches Ghost's newsletters.js controller
 */
class NewslettersAdminController {
public:
    explicit NewslettersAdminController(std::shared_ptr<IDatabase> database) noexcept
        : m_database{std::move(database)} {}

    struct Newsletter {
        std::string m_id{};
        std::string m_uuid{};
        std::string m_name{};
        std::string m_slug{};
        std::string m_description{};
        std::string m_sender_name{};
        std::string m_sender_email{};
        std::string m_sender_reply_to{};
        std::string m_title{};
        std::string m_custom_excerpt{};
        std::string m_show_title_and_feature_image{};
        std::string m_locale{};
        std::string m_status{};  // "active", "inactive"
        std::string m_visibility{};  // "public", "members", "paid", "tiers"
        std::string m_feature_image{};
        std::string m_feature_image_alt{};
        std::string m_feature_image_caption{};
        std::string m_meta_title{};
        std::string m_meta_description{};
        std::string m_og_image{};
        std::string m_og_title{};
        std::string m_og_description{};
        std::string m_twitter_image{};
        std::string m_twitter_title{};
        std::string m_twitter_description{};
        std::string m_custom_template{};
        std::string m_email_only{};
        std::string m_email_recipient_filter{};
        std::string m_created_at{};
        std::string m_updated_at{};
        json m_meta{};
        std::vector<std::string> m_tiers{};
        std::vector<std::string> m_segments{};
        
        Newsletter() noexcept = default;
        
        Newsletter& add_name(std::string&& name) noexcept { m_name = std::move(name); return *this; }
        Newsletter& add_slug(std::string&& slug) noexcept { m_slug = std::move(slug); return *this; }
        Newsletter& add_status(std::string&& status) noexcept { m_status = std::move(status); return *this; }
        
        void set_id(std::string&& id) noexcept { m_id = std::move(id); }
        void set_uuid(std::string&& uuid) noexcept { m_uuid = std::move(uuid); }
        void set_name(std::string&& name) noexcept { m_name = std::move(name); }
        void set_slug(std::string&& slug) noexcept { m_slug = std::move(slug); }
        void set_description(std::string&& desc) noexcept { m_description = std::move(desc); }
        void set_sender_name(std::string&& name) noexcept { m_sender_name = std::move(name); }
        void set_sender_email(std::string&& email) noexcept { m_sender_email = std::move(email); }
        void set_sender_reply_to(std::string&& email) noexcept { m_sender_reply_to = std::move(email); }
        void set_title(std::string&& title) noexcept { m_title = std::move(title); }
        void set_custom_excerpt(std::string&& excerpt) noexcept { m_custom_excerpt = std::move(excerpt); }
        void set_show_title_and_feature_image(std::string&& show) noexcept { m_show_title_and_feature_image = std::move(show); }
        void set_locale(std::string&& locale) noexcept { m_locale = std::move(locale); }
        void set_status(std::string&& status) noexcept { m_status = std::move(status); }
        void set_visibility(std::string&& visibility) noexcept { m_visibility = std::move(visibility); }
        void set_feature_image(std::string&& img) noexcept { m_feature_image = std::move(img); }
        void set_feature_image_alt(std::string&& alt) noexcept { m_feature_image_alt = std::move(alt); }
        void set_feature_image_caption(std::string&& caption) noexcept { m_feature_image_caption = std::move(caption); }
        void set_meta_title(std::string&& title) noexcept { m_meta_title = std::move(title); }
        void set_meta_description(std::string&& desc) noexcept { m_meta_description = std::move(desc); }
        void set_og_image(std::string&& img) noexcept { m_og_image = std::move(img); }
        void set_og_title(std::string&& title) noexcept { m_og_title = std::move(title); }
        void set_og_description(std::string&& desc) noexcept { m_og_description = std::move(desc); }
        void set_twitter_image(std::string&& img) noexcept { m_twitter_image = std::move(img); }
        void set_twitter_title(std::string&& title) noexcept { m_twitter_title = std::move(title); }
        void set_twitter_description(std::string&& desc) noexcept { m_twitter_description = std::move(desc); }
        void set_custom_template(std::string&& tmpl) noexcept { m_custom_template = std::move(tmpl); }
        void set_email_only(std::string&& email) noexcept { m_email_only = std::move(email); }
        void set_email_recipient_filter(std::string&& filter) noexcept { m_email_recipient_filter = std::move(filter); }
        void set_created_at(std::string&& created) noexcept { m_created_at = std::move(created); }
        void set_updated_at(std::string&& updated) noexcept { m_updated_at = std::move(updated); }
        void set_meta(json&& meta) noexcept { m_meta = std::move(meta); }
        void set_tiers(std::vector<std::string>&& tiers) noexcept { m_tiers = std::move(tiers); }
        void set_segments(std::vector<std::string>&& segments) noexcept { m_segments = std::move(segments); }
        
        std::string& get_id() noexcept { return m_id; }
        std::string& get_uuid() noexcept { return m_uuid; }
        std::string& get_name() noexcept { return m_name; }
        std::string& get_slug() noexcept { return m_slug; }
        std::string& get_description() noexcept { return m_description; }
        std::string& get_sender_name() noexcept { return m_sender_name; }
        std::string& get_sender_email() noexcept { return m_sender_email; }
        std::string& get_sender_reply_to() noexcept { return m_sender_reply_to; }
        std::string& get_title() noexcept { return m_title; }
        std::string& get_custom_excerpt() noexcept { return m_custom_excerpt; }
        std::string& get_show_title_and_feature_image() noexcept { return m_show_title_and_feature_image; }
        std::string& get_locale() noexcept { return m_locale; }
        std::string& get_status() noexcept { return m_status; }
        std::string& get_visibility() noexcept { return m_visibility; }
        std::string& get_feature_image() noexcept { return m_feature_image; }
        std::string& get_created_at() noexcept { return m_created_at; }
        std::string& get_updated_at() noexcept { return m_updated_at; }
        json& get_meta() noexcept { return m_meta; }
        std::vector<std::string>& get_tiers() noexcept { return m_tiers; }
        std::vector<std::string>& get_segments() noexcept { return m_segments; }
        
        const std::string& get_id() const noexcept { return m_id; }
        const std::string& get_uuid() const noexcept { return m_uuid; }
        const std::string& get_name() const noexcept { return m_name; }
        const std::string& get_slug() const noexcept { return m_slug; }
        const std::string& get_description() const noexcept { return m_description; }
        const std::string& get_sender_name() const noexcept { return m_sender_name; }
        const std::string& get_sender_email() const noexcept { return m_sender_email; }
        const std::string& get_sender_reply_to() const noexcept { return m_sender_reply_to; }
        const std::string& get_title() const noexcept { return m_title; }
        const std::string& get_custom_excerpt() const noexcept { return m_custom_excerpt; }
        const std::string& get_show_title_and_feature_image() const noexcept { return m_show_title_and_feature_image; }
        const std::string& get_locale() const noexcept { return m_locale; }
        const std::string& get_status() const noexcept { return m_status; }
        const std::string& get_visibility() const noexcept { return m_visibility; }
        const std::string& get_feature_image() const noexcept { return m_feature_image; }
        const std::string& get_created_at() const noexcept { return m_created_at; }
        const std::string& get_updated_at() const noexcept { return m_updated_at; }
        const json& get_meta() const noexcept { return m_meta; }
        const std::vector<std::string>& get_tiers() const noexcept { return m_tiers; }
        const std::vector<std::string>& get_segments() const noexcept { return m_segments; }
        
        json to_json() const noexcept {
            json j;
            j["id"] = m_id;
            j["uuid"] = m_uuid;
            j["name"] = m_name;
            j["slug"] = m_slug;
            j["description"] = m_description;
            j["sender_name"] = m_sender_name;
            j["sender_email"] = m_sender_email;
            j["sender_reply_to"] = m_sender_reply_to;
            j["title"] = m_title;
            j["custom_excerpt"] = m_custom_excerpt;
            j["show_title_and_feature_image"] = m_show_title_and_feature_image;
            j["locale"] = m_locale;
            j["status"] = m_status;
            j["visibility"] = m_visibility;
            j["feature_image"] = m_feature_image;
            j["feature_image_alt"] = m_feature_image_alt;
            j["feature_image_caption"] = m_feature_image_caption;
            j["meta_title"] = m_meta_title;
            j["meta_description"] = m_meta_description;
            j["og_image"] = m_og_image;
            j["og_title"] = m_og_title;
            j["og_description"] = m_og_description;
            j["twitter_image"] = m_twitter_image;
            j["twitter_title"] = m_twitter_title;
            j["twitter_description"] = m_twitter_description;
            j["custom_template"] = m_custom_template;
            j["email_only"] = m_email_only;
            j["email_recipient_filter"] = m_email_recipient_filter;
            j["created_at"] = m_created_at;
            j["updated_at"] = m_updated_at;
            j["meta"] = m_meta;
            j["tiers"] = m_tiers;
            j["segments"] = m_segments;
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

    // Browse newsletters
    std::expected<ApiResponse<std::vector<Newsletter>>, std::string> browse(const BrowseOptions& options) noexcept {
        std::vector<Newsletter> newsletters;
        return ApiResponse<std::vector<Newsletter>>::ok(std::move(newsletters));
    }

    // Read newsletter
    std::expected<ApiResponse<Newsletter>, std::string> read(const std::string& identifier) noexcept {
        Newsletter newsletter;
        newsletter.set_id(identifier);
        return ApiResponse<Newsletter>::ok(std::move(newsletter));
    }

    // Add newsletter
    std::expected<ApiResponse<Newsletter>, std::string> add(const Newsletter& newsletter) noexcept {
        return ApiResponse<Newsletter>::created(newsletter);
    }

    // Edit newsletter
    std::expected<ApiResponse<Newsletter>, std::string> edit(const std::string& id, const Newsletter& newsletter) noexcept {
        Newsletter updated = newsletter;
        updated.set_id(id);
        return ApiResponse<Newsletter>::ok(std::move(updated));
    }

    // Destroy newsletter
    std::expected<ApiResponse<bool>, std::string> destroy(const std::string& id) noexcept {
        return ApiResponse<bool>::ok(true);
    }

    // Verify property update
    std::expected<ApiResponse<Newsletter>, std::string> verify_property_update(const std::string& id, const json& data) noexcept {
        Newsletter newsletter;
        newsletter.set_id(id);
        return ApiResponse<Newsletter>::ok(std::move(newsletter));
    }

private:
    std::shared_ptr<IDatabase> m_database;
};

/**
 * Register newsletters admin routes
 */
void register_newsletters_admin_routes(core::router::RouterContext<256, 64, 10>& router_ctx,
                                       std::shared_ptr<IDatabase> database) {
    NewslettersAdminController controller{std::move(database)};
    
    router_ctx.add_route(core::router::Route<8, 10>{"/admin/newsletters/"}
        .get([controller = std::move(controller)](
            interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) mutable {
            send();
        }))
        .set_router_number(1);
    
    router_ctx.add_route(core::router::Route<8, 10>{"/admin/newsletters/:id/"}
        .get([controller = std::move(controller)](
            interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) mutable {
            send();
        }))
        .put([controller = std::move(controller)](
            interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) mutable {
            send();
        })
        .set_router_number(1);
    
    router_ctx.add_route(core::router::Route<8, 10>{"/admin/newsletters/verifications/"}
        .put([controller = std::move(controller)](
            interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) mutable {
            send();
        }))
        .set_router_number(1);
}

} // namespace ghost::blog::endpoints