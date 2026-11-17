module;
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <expected>

export module blog_api:endpoints:pages_admin;

import std;
import blog_api:types;
import blog_database:database;
import core_router;
import interfaces;

export namespace ghost::blog::endpoints {

using json = nlohmann::json;

/**
 * Pages Admin Controller - Matches Ghost's pages.js controller
 */
class PagesAdminController {
public:
    explicit PagesAdminController(std::shared_ptr<IDatabase> database) noexcept
        : m_database{std::move(database)} {}

    struct Page {
        std::string m_id{};
        std::string m_uuid{};
        std::string m_title{};
        std::string m_slug{};
        std::string m_content{};
        std::string m_plaintext{};
        std::string m_author_id{};
        std::string m_status{};  // "draft", "published"
        std::string m_visibility{};  // "public", "members", "paid", "tiers"
        std::string m_feature_image{};
        std::string m_feature_image_alt{};
        std::string m_feature_image_caption{};
        std::string m_excerpt{};
        std::string m_codeinjection_head{};
        std::string m_codeinjection_foot{};
        std::string m_custom_excerpt{};
        std::string m_canonical_url{};
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
        std::string m_created_at{};
        std::string m_updated_at{};
        std::string m_published_at{};
        json m_meta{};
        std::vector<std::string> m_tags{};
        std::vector<std::string> m_authors{};
        
        Page() noexcept = default;
        
        Page& add_title(std::string&& title) noexcept { m_title = std::move(title); return *this; }
        Page& add_slug(std::string&& slug) noexcept { m_slug = std::move(slug); return *this; }
        Page& add_status(std::string&& status) noexcept { m_status = std::move(status); return *this; }
        
        void set_id(std::string&& id) noexcept { m_id = std::move(id); }
        void set_uuid(std::string&& uuid) noexcept { m_uuid = std::move(uuid); }
        void set_title(std::string&& title) noexcept { m_title = std::move(title); }
        void set_slug(std::string&& slug) noexcept { m_slug = std::move(slug); }
        void set_content(std::string&& content) noexcept { m_content = std::move(content); }
        void set_plaintext(std::string&& plaintext) noexcept { m_plaintext = std::move(plaintext); }
        void set_author_id(std::string&& author_id) noexcept { m_author_id = std::move(author_id); }
        void set_status(std::string&& status) noexcept { m_status = std::move(status); }
        void set_visibility(std::string&& visibility) noexcept { m_visibility = std::move(visibility); }
        void set_feature_image(std::string&& img) noexcept { m_feature_image = std::move(img); }
        void set_feature_image_alt(std::string&& alt) noexcept { m_feature_image_alt = std::move(alt); }
        void set_feature_image_caption(std::string&& caption) noexcept { m_feature_image_caption = std::move(caption); }
        void set_excerpt(std::string&& excerpt) noexcept { m_excerpt = std::move(excerpt); }
        void set_codeinjection_head(std::string&& head) noexcept { m_codeinjection_head = std::move(head); }
        void set_codeinjection_foot(std::string&& foot) noexcept { m_codeinjection_foot = std::move(foot); }
        void set_custom_excerpt(std::string&& excerpt) noexcept { m_custom_excerpt = std::move(excerpt); }
        void set_canonical_url(std::string&& url) noexcept { m_canonical_url = std::move(url); }
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
        void set_created_at(std::string&& created) noexcept { m_created_at = std::move(created); }
        void set_updated_at(std::string&& updated) noexcept { m_updated_at = std::move(updated); }
        void set_published_at(std::string&& published) noexcept { m_published_at = std::move(published); }
        void set_meta(json&& meta) noexcept { m_meta = std::move(meta); }
        void set_tags(std::vector<std::string>&& tags) noexcept { m_tags = std::move(tags); }
        void set_authors(std::vector<std::string>&& authors) noexcept { m_authors = std::move(authors); }
        
        std::string& get_id() noexcept { return m_id; }
        std::string& get_uuid() noexcept { return m_uuid; }
        std::string& get_title() noexcept { return m_title; }
        std::string& get_slug() noexcept { return m_slug; }
        std::string& get_content() noexcept { return m_content; }
        std::string& get_plaintext() noexcept { return m_plaintext; }
        std::string& get_author_id() noexcept { return m_author_id; }
        std::string& get_status() noexcept { return m_status; }
        std::string& get_visibility() noexcept { return m_visibility; }
        std::string& get_feature_image() noexcept { return m_feature_image; }
        std::string& get_excerpt() noexcept { return m_excerpt; }
        std::string& get_created_at() noexcept { return m_created_at; }
        std::string& get_updated_at() noexcept { return m_updated_at; }
        std::string& get_published_at() noexcept { return m_published_at; }
        json& get_meta() noexcept { return m_meta; }
        std::vector<std::string>& get_tags() noexcept { return m_tags; }
        std::vector<std::string>& get_authors() noexcept { return m_authors; }
        
        const std::string& get_id() const noexcept { return m_id; }
        const std::string& get_uuid() const noexcept { return m_uuid; }
        const std::string& get_title() const noexcept { return m_title; }
        const std::string& get_slug() const noexcept { return m_slug; }
        const std::string& get_content() const noexcept { return m_content; }
        const std::string& get_plaintext() const noexcept { return m_plaintext; }
        const std::string& get_author_id() const noexcept { return m_author_id; }
        const std::string& get_status() const noexcept { return m_status; }
        const std::string& get_visibility() const noexcept { return m_visibility; }
        const std::string& get_feature_image() const noexcept { return m_feature_image; }
        const std::string& get_excerpt() const noexcept { return m_excerpt; }
        const std::string& get_created_at() const noexcept { return m_created_at; }
        const std::string& get_updated_at() const noexcept { return m_updated_at; }
        const std::string& get_published_at() const noexcept { return m_published_at; }
        const json& get_meta() const noexcept { return m_meta; }
        const std::vector<std::string>& get_tags() const noexcept { return m_tags; }
        const std::vector<std::string>& get_authors() const noexcept { return m_authors; }
        
        json to_json() const noexcept {
            json j;
            j["id"] = m_id;
            j["uuid"] = m_uuid;
            j["title"] = m_title;
            j["slug"] = m_slug;
            j["content"] = m_content;
            j["plaintext"] = m_plaintext;
            j["author_id"] = m_author_id;
            j["status"] = m_status;
            j["visibility"] = m_visibility;
            j["feature_image"] = m_feature_image;
            j["feature_image_alt"] = m_feature_image_alt;
            j["feature_image_caption"] = m_feature_image_caption;
            j["excerpt"] = m_excerpt;
            j["codeinjection_head"] = m_codeinjection_head;
            j["codeinjection_foot"] = m_codeinjection_foot;
            j["custom_excerpt"] = m_custom_excerpt;
            j["canonical_url"] = m_canonical_url;
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
            j["created_at"] = m_created_at;
            j["updated_at"] = m_updated_at;
            j["published_at"] = m_published_at;
            j["meta"] = m_meta;
            j["tags"] = m_tags;
            j["authors"] = m_authors;
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

    // Browse pages
    std::expected<ApiResponse<std::vector<Page>>, std::string> browse(const BrowseOptions& options) noexcept {
        std::vector<Page> pages;
        return ApiResponse<std::vector<Page>>::ok(std::move(pages));
    }

    // Read page
    std::expected<ApiResponse<Page>, std::string> read(const std::string& identifier) noexcept {
        Page page;
        page.set_id(identifier);
        return ApiResponse<Page>::ok(std::move(page));
    }

    // Add page
    std::expected<ApiResponse<Page>, std::string> add(const Page& page) noexcept {
        return ApiResponse<Page>::created(page);
    }

    // Edit page
    std::expected<ApiResponse<Page>, std::string> edit(const std::string& id, const Page& page) noexcept {
        Page updated = page;
        updated.set_id(id);
        return ApiResponse<Page>::ok(std::move(updated));
    }

    // Destroy page
    std::expected<ApiResponse<bool>, std::string> destroy(const std::string& id) noexcept {
        return ApiResponse<bool>::ok(true);
    }

    // Bulk operations
    std::expected<ApiResponse<std::vector<Page>>, std::string> bulk_edit(const json& bulk_data) noexcept {
        return ApiResponse<std::vector<Page>>::ok({});
    }

    std::expected<ApiResponse<std::vector<Page>>, std::string> bulk_destroy(const std::vector<std::string>& ids) noexcept {
        return ApiResponse<std::vector<Page>>::ok({});
    }

    // Copy page
    std::expected<ApiResponse<Page>, std::string> copy(const std::string& id) noexcept {
        Page page;
        page.set_id(id);
        return ApiResponse<Page>::created(page);
    }

private:
    std::shared_ptr<IDatabase> m_database;
};

/**
 * Register pages admin routes
 */
void register_pages_admin_routes(core::router::RouterContext<256, 64, 10>& router_ctx,
                                 std::shared_ptr<IDatabase> database) {
    PagesAdminController controller{std::move(database)};
    
    router_ctx.add_route(core::router::Route<8, 10>{"/admin/pages/"}
        .get([controller = std::move(controller)](
            interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) mutable {
            send();
        }))
        .set_router_number(1);
    
    router_ctx.add_route(core::router::Route<8, 10>{"/admin/pages/:id/"}
        .get([controller = std::move(controller)](
            interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) mutable {
            send();
        }))
        .set_router_number(1);
    
    router_ctx.add_route(core::router::Route<8, 10>{"/admin/pages/slug/:slug/"}
        .get([controller = std::move(controller)](
            interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) mutable {
            send();
        }))
        .set_router_number(1);
}

} // namespace ghost::blog::endpoints