module;
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <expected>

export module blog_api:endpoints:authors_public;

import std;
import blog_api:types;
import blog_database:database;
import blog_database:author_repository;
import core_router;
import interfaces;

export namespace ghost::blog::endpoints {

using json = nlohmann::json;

/**
 * Authors Public Controller - Matches Ghost's authors-public.js controller (Content API)
 */
class AuthorsPublicController {
public:
    explicit AuthorsPublicController(std::shared_ptr<IDatabase> database) noexcept
        : m_database{std::move(database)}, m_repo{m_database} {}

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

    struct CacheKeyData {
        json m_options{};
        json m_auth{};
        std::string m_method{};
        
        CacheKeyData() noexcept = default;
        CacheKeyData(json&& options, json&& auth, std::string&& method) noexcept
            : m_options{std::move(options)}, m_auth{std::move(auth)}, m_method{std::move(method)} {}
        
        CacheKeyData& add_options(json&& options) noexcept { m_options = std::move(options); return *this; }
        CacheKeyData& add_auth(json&& auth) noexcept { m_auth = std::move(auth); return *this; }
        CacheKeyData& add_method(std::string&& method) noexcept { m_method = std::move(method); return *this; }
        
        void set_options(json&& options) noexcept { m_options = std::move(options); }
        void set_auth(json&& auth) noexcept { m_auth = std::move(auth); }
        void set_method(std::string&& method) noexcept { m_method = std::move(method); }
        
        json& get_options() noexcept { return m_options; }
        json& get_auth() noexcept { return m_auth; }
        std::string& get_method() noexcept { return m_method; }
        
        const json& get_options() const noexcept { return m_options; }
        const json& get_auth() const noexcept { return m_auth; }
        const std::string& get_method() const noexcept { return m_method; }
    };

    // Browse authors
    std::expected<ApiResponse<std::vector<Author>>, std::string> browse(const BrowseOptions& options) noexcept {
        PaginationParams params;
        params.set_page(std::to_string(options.get_page()));
        params.set_limit(std::to_string(options.get_limit()));
        params.set_order(options.get_order().empty() ? "name ASC" : options.get_order());
        
        auto result = m_repo.find_all(params);
        if (result) {
            auto& authors = result->get_data();
            // Filter for active authors only in public API
            std::vector<Author> filtered;
            for (const auto& author : authors) {
                if (author.get_status() == "active") {
                    filtered.push_back(author);
                }
            }
            result->set_data(std::move(filtered));
        }
        
        return result;
    }

    // Read author
    std::expected<ApiResponse<Author>, std::string> read(const std::string& identifier) noexcept {
        auto result = m_repo.find_by_id(identifier);
        if (result && result->get_data().has_value()) {
            auto author = *result->get_data();
            if (author.get_status() != "active") {
                return ApiResponse<Author>::not_found("Author");
            }
            return result;
        }
        
        result = m_repo.find_by_slug(identifier);
        if (result && result->get_data().has_value()) {
            auto author = *result->get_data();
            if (author.get_status() != "active") {
                return ApiResponse<Author>::not_found("Author");
            }
            return result;
        }
        
        return ApiResponse<Author>::not_found("Author");
    }

    // Get posts by author
    std::expected<ApiResponse<std::vector<Post>>, std::string> get_posts(const std::string& author_id, const BrowseOptions& options) noexcept {
        PaginationParams params;
        params.set_page(std::to_string(options.get_page()));
        params.set_limit(std::to_string(options.get_limit()));
        params.set_order(options.get_order().empty() ? "published_at DESC" : options.get_order());
        
        return m_repo.get_published_posts(author_id, params);
    }

    // Generate cache key
    CacheKeyData generate_cache_key(const BrowseOptions& options) noexcept {
        json cache_options;
        cache_options["include"] = options.get_include();
        cache_options["filter"] = options.get_filter();
        cache_options["fields"] = options.get_fields();
        cache_options["limit"] = options.get_limit();
        cache_options["order"] = options.get_order();
        cache_options["page"] = options.get_page();
        
        json auth = json{};
        
        return CacheKeyData{std::move(cache_options), std::move(auth), "browse"};
    }

private:
    std::shared_ptr<IDatabase> m_database;
    AuthorRepository m_repo;
};

/**
 * Register authors public routes (Content API)
 */
void register_authors_public_routes(core::router::RouterContext<256, 64, 10>& router_ctx,
                                    std::shared_ptr<IDatabase> database) {
    AuthorsPublicController controller{std::move(database)};
    
    router_ctx.add_route(core::router::Route<8, 10>{"/content/authors/"}
        .get([controller = std::move(controller)](
            interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) mutable {
            send();
        }))
        .set_router_number(1);
    
    router_ctx.add_route(core::router::Route<8, 10>{"/content/authors/:id/"}
        .get([controller = std::move(controller)](
            interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) mutable {
            send();
        }))
        .set_router_number(1);
    
    router_ctx.add_route(core::router::Route<8, 10>{"/content/authors/slug/:slug/"}
        .get([controller = std::move(controller)](
            interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) mutable {
            send();
        }))
        .set_router_number(1);
    
    router_ctx.add_route(core::router::Route<8, 10>{"/content/authors/:id/posts/"}
        .get([controller = std::move(controller)](
            interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) mutable {
            send();
        }))
        .set_router_number(1);
}

} // namespace ghost::blog::endpoints