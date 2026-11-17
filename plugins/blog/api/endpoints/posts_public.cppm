module;
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <expected>

export module blog_api:endpoints:posts_public;

import std;
import blog_api:types;
import blog_database:database;
import blog_database:post_repository;
import core_router;
import interfaces;

export namespace ghost::blog::endpoints {

using json = nlohmann::json;

/**
 * Posts Public Controller - Matches Ghost's posts-public.js controller (Content API)
 */
class PostsPublicController {
public:
    explicit PostsPublicController(std::shared_ptr<IDatabase> database) noexcept
        : m_database{std::move(database)}, m_repo{m_database} {}

    // Allowed includes for Content API
    static constexpr std::array<std::string_view, 4> ALLOWED_INCLUDES = {"tags", "authors", "tiers", "sentiment"};
    static constexpr std::array<std::string_view, 3> ALLOWED_READ_FIELDS = {"id", "slug", "uuid"};

    // Browse options
    struct BrowseOptions {
        std::string m_include{};
        std::string m_filter{};
        std::string m_fields{};
        std::string m_formats{};
        int m_limit = 15;
        std::string m_order{};
        int m_page = 1;
        bool m_debug = false;
        bool m_absolute_urls = false;
        std::string m_collection{};
        bool m_skip_pagination = false;
        
        BrowseOptions() noexcept = default;
        
        BrowseOptions& add_include(std::string&& include) noexcept { m_include = std::move(include); return *this; }
        BrowseOptions& add_filter(std::string&& filter) noexcept { m_filter = std::move(filter); return *this; }
        BrowseOptions& add_fields(std::string&& fields) noexcept { m_fields = std::move(fields); return *this; }
        BrowseOptions& add_formats(std::string&& formats) noexcept { m_formats = std::move(formats); return *this; }
        BrowseOptions& add_limit(int limit) noexcept { m_limit = limit; return *this; }
        BrowseOptions& add_order(std::string&& order) noexcept { m_order = std::move(order); return *this; }
        BrowseOptions& add_page(int page) noexcept { m_page = page; return *this; }
        BrowseOptions& add_debug(bool debug) noexcept { m_debug = debug; return *this; }
        BrowseOptions& add_absolute_urls(bool absolute_urls) noexcept { m_absolute_urls = absolute_urls; return *this; }
        BrowseOptions& add_collection(std::string&& collection) noexcept { m_collection = std::move(collection); return *this; }
        BrowseOptions& add_skip_pagination(bool skip) noexcept { m_skip_pagination = skip; return *this; }
        
        void set_include(std::string&& include) noexcept { m_include = std::move(include); }
        void set_filter(std::string&& filter) noexcept { m_filter = std::move(filter); }
        void set_fields(std::string&& fields) noexcept { m_fields = std::move(fields); }
        void set_formats(std::string&& formats) noexcept { m_formats = std::move(formats); }
        void set_limit(int limit) noexcept { m_limit = limit; }
        void set_order(std::string&& order) noexcept { m_order = std::move(order); }
        void set_page(int page) noexcept { m_page = page; }
        void set_debug(bool debug) noexcept { m_debug = debug; }
        void set_absolute_urls(bool absolute_urls) noexcept { m_absolute_urls = absolute_urls; }
        void set_collection(std::string&& collection) noexcept { m_collection = std::move(collection); }
        void set_skip_pagination(bool skip) noexcept { m_skip_pagination = skip; }
        
        std::string& get_include() noexcept { return m_include; }
        std::string& get_filter() noexcept { return m_filter; }
        std::string& get_fields() noexcept { return m_fields; }
        std::string& get_formats() noexcept { return m_formats; }
        int& get_limit() noexcept { return m_limit; }
        std::string& get_order() noexcept { return m_order; }
        int& get_page() noexcept { return m_page; }
        bool& get_debug() noexcept { return m_debug; }
        bool& get_absolute_urls() noexcept { return m_absolute_urls; }
        std::string& get_collection() noexcept { return m_collection; }
        bool& get_skip_pagination() noexcept { return m_skip_pagination; }
        
        const std::string& get_include() const noexcept { return m_include; }
        const std::string& get_filter() const noexcept { return m_filter; }
        const std::string& get_fields() const noexcept { return m_fields; }
        const std::string& get_formats() const noexcept { return m_formats; }
        int get_limit() const noexcept { return m_limit; }
        const std::string& get_order() const noexcept { return m_order; }
        int get_page() const noexcept { return m_page; }
        bool get_debug() const noexcept { return m_debug; }
        bool get_absolute_urls() const noexcept { return m_absolute_urls; }
        const std::string& get_collection() const noexcept { return m_collection; }
        bool get_skip_pagination() const noexcept { return m_skip_pagination; }
    };

    // Cache key data
    struct CacheKeyData {
        json m_options{};
        bool m_skip_pagination = false;
        json m_auth{};
        std::string m_method{};
        
        CacheKeyData() noexcept = default;
        CacheKeyData(json&& options, bool skip_pagination, json&& auth, std::string&& method) noexcept
            : m_options{std::move(options)}, m_skip_pagination{skip_pagination},
              m_auth{std::move(auth)}, m_method{std::move(method)} {}
        
        CacheKeyData& add_options(json&& options) noexcept { m_options = std::move(options); return *this; }
        CacheKeyData& add_skip_pagination(bool skip) noexcept { m_skip_pagination = skip; return *this; }
        CacheKeyData& add_auth(json&& auth) noexcept { m_auth = std::move(auth); return *this; }
        CacheKeyData& add_method(std::string&& method) noexcept { m_method = std::move(method); return *this; }
        
        void set_options(json&& options) noexcept { m_options = std::move(options); }
        void set_skip_pagination(bool skip) noexcept { m_skip_pagination = skip; }
        void set_auth(json&& auth) noexcept { m_auth = std::move(auth); }
        void set_method(std::string&& method) noexcept { m_method = std::move(method); }
        
        json& get_options() noexcept { return m_options; }
        bool& get_skip_pagination() noexcept { return m_skip_pagination; }
        json& get_auth() noexcept { return m_auth; }
        std::string& get_method() noexcept { return m_method; }
        
        const json& get_options() const noexcept { return m_options; }
        bool get_skip_pagination() const noexcept { return m_skip_pagination; }
        const json& get_auth() const noexcept { return m_auth; }
        const std::string& get_method() const noexcept { return m_method; }
    };

    // Read cache key data (includes gift and identifier)
    struct ReadCacheKeyData : CacheKeyData {
        json m_gift{};
        json m_identifier{};
        
        ReadCacheKeyData() noexcept = default;
        ReadCacheKeyData(json&& options, json&& auth, json&& gift, std::string&& method, json&& identifier) noexcept
            : CacheKeyData{std::move(options), false, std::move(auth), std::move(method)},
              m_gift{std::move(gift)}, m_identifier{std::move(identifier)} {}
        
        ReadCacheKeyData& add_gift(json&& gift) noexcept { m_gift = std::move(gift); return *this; }
        ReadCacheKeyData& add_identifier(json&& identifier) noexcept { m_identifier = std::move(identifier); return *this; }
        
        void set_gift(json&& gift) noexcept { m_gift = std::move(gift); }
        void set_identifier(json&& identifier) noexcept { m_identifier = std::move(identifier); }
        
        json& get_gift() noexcept { return m_gift; }
        json& get_identifier() noexcept { return m_identifier; }
        
        const json& get_gift() const noexcept { return m_gift; }
        const json& get_identifier() const noexcept { return m_identifier; }
    };

    // Browse posts (public)
    std::expected<ApiResponse<std::vector<Post>>, std::string> browse(const BrowseOptions& options) noexcept {
        PostFilter filter;
        parse_filter(options.get_filter(), filter);
        
        // Force published status for public API
        filter.set_status("published");
        
        PaginationParams params;
        params.set_page(std::to_string(options.get_page()));
        params.set_limit(std::to_string(options.get_limit()));
        params.set_order(options.get_order().empty() ? "published_at DESC" : options.get_order());
        
        // Apply restricted fields transformer
        auto result = m_repo.find_all(params, filter);
        if (result) {
            // Filter out restricted fields for public API
            auto& posts = result->get_data();
            for (auto& post : posts) {
                restrict_public_fields(post);
            }
        }
        
        return result;
    }

    // Read single post (public)
    std::expected<ApiResponse<Post>, std::string> read(const json& data, const BrowseOptions& options) noexcept {
        // Must have at least one identifier
        if (!data.contains("id") && !data.contains("slug") && !data.contains("uuid")) {
            return ApiResponse<Post>::error("A post id, slug or uuid is required.", 400);
        }
        
        std::expected<ApiResponse<std::optional<Post>>, std::string> result;
        
        if (data.contains("id")) {
            result = m_repo.find_by_id(data["id"].get<std::string>());
        } else if (data.contains("uuid")) {
            result = m_repo.find_by_uuid(data["uuid"].get<std::string>());
        } else if (data.contains("slug")) {
            result = m_repo.find_by_slug(data["slug"].get<std::string>());
        }
        
        if (!result || !result->get_data().has_value()) {
            return ApiResponse<Post>::not_found("Post");
        }
        
        auto post = std::move(*result->get_data());
        
        // Apply gift access if needed
        apply_gift_access(post);
        
        // Restrict public fields
        restrict_public_fields(post);
        
        return ApiResponse<Post>::ok(std::move(post));
    }

    // Generate cache key for browse
    CacheKeyData generate_browse_cache_key(const BrowseOptions& options) noexcept {
        json cache_options;
        cache_options["include"] = options.get_include();
        cache_options["filter"] = options.get_filter();
        cache_options["fields"] = options.get_fields();
        cache_options["formats"] = options.get_formats();
        cache_options["limit"] = options.get_limit();
        cache_options["order"] = options.get_order();
        cache_options["page"] = options.get_page();
        cache_options["absolute_urls"] = options.get_absolute_urls();
        cache_options["collection"] = options.get_collection();
        
        json auth = generate_auth_data();
        
        return CacheKeyData{std::move(cache_options), options.get_skip_pagination(), std::move(auth), "browse"};
    }

    // Generate cache key for read
    ReadCacheKeyData generate_read_cache_key(const json& data, const BrowseOptions& options) noexcept {
        json cache_options;
        cache_options["include"] = options.get_include();
        cache_options["fields"] = options.get_fields();
        cache_options["formats"] = options.get_formats();
        cache_options["absolute_urls"] = options.get_absolute_urls();
        
        json auth = generate_auth_data();
        json gift = generate_gift_key_data();
        json identifier;
        
        if (data.contains("id")) identifier["id"] = data["id"];
        if (data.contains("slug")) identifier["slug"] = data["slug"];
        if (data.contains("uuid")) identifier["uuid"] = data["uuid"];
        
        return ReadCacheKeyData{std::move(cache_options), std::move(auth), std::move(gift), "read", std::move(identifier)};
    }

private:
    std::shared_ptr<IDatabase> m_database;
    PostRepository m_repo;
    
    static void parse_filter(const std::string& filter_str, PostFilter& filter) noexcept {
        size_t pos = 0;
        while (pos < filter_str.size()) {
            size_t comma = filter_str.find(',', pos);
            if (comma == std::string::npos) comma = filter_str.size();
            
            size_t colon = filter_str.find(':', pos);
            if (colon != std::string::npos && colon < comma) {
                std::string key = filter_str.substr(pos, colon - pos);
                std::string value = filter_str.substr(colon + 1, comma - colon - 1);
                
                if (key == "tag") filter.set_tag(std::move(value));
                else if (key == "authors") filter.set_author_id(std::move(value));
                else if (key == "visibility") filter.set_visibility(std::move(value));
            }
            
            pos = comma + 1;
        }
    }
    
    static void restrict_public_fields(Post& post) noexcept {
        // Remove restricted fields for public API
        // In a real implementation, this would be done at serialization time
    }
    
    static void apply_gift_access(Post& post) noexcept {
        // Apply gift access logic
    }
    
    static json generate_auth_data() noexcept {
        return json{};
    }
    
    static json generate_gift_key_data() noexcept {
        return json{};
    }
};

/**
 * Register posts public routes (Content API)
 */
void register_posts_public_routes(core::router::RouterContext<256, 64, 10>& router_ctx,
                                  std::shared_ptr<IDatabase> database) {
    PostsPublicController controller{std::move(database)};
    
    // Browse posts
    router_ctx.add_route(core::router::Route<8, 10>{"/content/posts/"}
        .get([controller = std::move(controller)](
            interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) mutable {
            // Implementation would go here
            send();
        }))
        .set_router_number(1);
    
    // Read post
    router_ctx.add_route(core::router::Route<8, 10>{"/content/posts/:id/"}
        .get([controller = std::move(controller)](
            interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) mutable {
            send();
        }))
        .set_router_number(1);
    
    router_ctx.add_route(core::router::Route<8, 10>{"/content/posts/slug/:slug/"}
        .get([controller = std::move(controller)](
            interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) mutable {
            send();
        }))
        .set_router_number(1);
}

} // namespace ghost::blog::endpoints