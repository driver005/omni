module;
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <expected>

export module blog_api:endpoints:posts_admin;

import std;
import blog_api:types;
import blog_database:database;
import blog_database:post_repository;
import core_router;
import interfaces;

export namespace ghost::blog::endpoints {

using json = nlohmann::json;

/**
 * Posts Admin Controller - Matches Ghost's posts.js controller
 */
class PostsAdminController {
public:
    explicit PostsAdminController(std::shared_ptr<IDatabase> database) noexcept
        : m_database{std::move(database)}, m_repo{m_database} {}

    // Browse posts (admin)
    struct BrowseOptions {
        std::string m_include{};
        std::string m_filter{};
        std::string m_fields{};
        std::string m_collection{};
        std::string m_formats{};
        int m_limit = 15;
        std::string m_order{};
        int m_page = 1;
        bool m_debug = false;
        bool m_absolute_urls = false;
        
        BrowseOptions() noexcept = default;
        BrowseOptions(std::string&& include, std::string&& filter, std::string&& fields,
                      std::string&& collection, std::string&& formats, int limit,
                      std::string&& order, int page, bool debug, bool absolute_urls) noexcept
            : m_include{std::move(include)}, m_filter{std::move(filter)},
              m_fields{std::move(fields)}, m_collection{std::move(collection)},
              m_formats{std::move(formats)}, m_limit{limit}, m_order{std::move(order)},
              m_page{page}, m_debug{debug}, m_absolute_urls{absolute_urls} {}
        
        BrowseOptions& add_include(std::string&& include) noexcept { m_include = std::move(include); return *this; }
        BrowseOptions& add_filter(std::string&& filter) noexcept { m_filter = std::move(filter); return *this; }
        BrowseOptions& add_fields(std::string&& fields) noexcept { m_fields = std::move(fields); return *this; }
        BrowseOptions& add_collection(std::string&& collection) noexcept { m_collection = std::move(collection); return *this; }
        BrowseOptions& add_formats(std::string&& formats) noexcept { m_formats = std::move(formats); return *this; }
        BrowseOptions& add_limit(int limit) noexcept { m_limit = limit; return *this; }
        BrowseOptions& add_order(std::string&& order) noexcept { m_order = std::move(order); return *this; }
        BrowseOptions& add_page(int page) noexcept { m_page = page; return *this; }
        BrowseOptions& add_debug(bool debug) noexcept { m_debug = debug; return *this; }
        BrowseOptions& add_absolute_urls(bool absolute_urls) noexcept { m_absolute_urls = absolute_urls; return *this; }
        
        void set_include(std::string&& include) noexcept { m_include = std::move(include); }
        void set_filter(std::string&& filter) noexcept { m_filter = std::move(filter); }
        void set_fields(std::string&& fields) noexcept { m_fields = std::move(fields); }
        void set_collection(std::string&& collection) noexcept { m_collection = std::move(collection); }
        void set_formats(std::string&& formats) noexcept { m_formats = std::move(formats); }
        void set_limit(int limit) noexcept { m_limit = limit; }
        void set_order(std::string&& order) noexcept { m_order = std::move(order); }
        void set_page(int page) noexcept { m_page = page; }
        void set_debug(bool debug) noexcept { m_debug = debug; }
        void set_absolute_urls(bool absolute_urls) noexcept { m_absolute_urls = absolute_urls; }
        
        std::string& get_include() noexcept { return m_include; }
        std::string& get_filter() noexcept { return m_filter; }
        std::string& get_fields() noexcept { return m_fields; }
        std::string& get_collection() noexcept { return m_collection; }
        std::string& get_formats() noexcept { return m_formats; }
        int& get_limit() noexcept { return m_limit; }
        std::string& get_order() noexcept { return m_order; }
        int& get_page() noexcept { return m_page; }
        bool& get_debug() noexcept { return m_debug; }
        bool& get_absolute_urls() noexcept { return m_absolute_urls; }
        
        const std::string& get_include() const noexcept { return m_include; }
        const std::string& get_filter() const noexcept { return m_filter; }
        const std::string& get_fields() const noexcept { return m_fields; }
        const std::string& get_collection() const noexcept { return m_collection; }
        const std::string& get_formats() const noexcept { return m_formats; }
        int get_limit() const noexcept { return m_limit; }
        const std::string& get_order() const noexcept { return m_order; }
        int get_page() const noexcept { return m_page; }
        bool get_debug() const noexcept { return m_debug; }
        bool get_absolute_urls() const noexcept { return m_absolute_urls; }
    };

    // Validation result
    struct ValidationResult {
        bool m_valid = true;
        std::vector<std::string> m_errors{};
        
        ValidationResult() noexcept = default;
        ValidationResult(std::vector<std::string>&& errors) noexcept
            : m_valid{false}, m_errors{std::move(errors)} {}
        
        ValidationResult& add_error(std::string&& error) noexcept {
            m_valid = false;
            m_errors.push_back(std::move(error));
            return *this;
        }
        
        void set_valid(bool valid) noexcept { m_valid = valid; }
        void add_error_ref(const std::string& error) { m_valid = false; m_errors.push_back(error); }
        
        bool& get_valid() noexcept { return m_valid; }
        std::vector<std::string>& get_errors() noexcept { return m_errors; }
        const bool& get_valid() const noexcept { return m_valid; }
        const std::vector<std::string>& get_errors() const noexcept { return m_errors; }
    };

    // Browse posts
    std::expected<ApiResponse<std::vector<Post>>, std::string> browse(const BrowseOptions& options) noexcept {
        PostFilter filter;
        // Parse filter string if provided
        if (!options.get_filter().empty()) {
            parse_filter(options.get_filter(), filter);
        }
        
        PaginationParams params;
        params.set_page(std::to_string(options.get_page()));
        params.set_limit(std::to_string(options.get_limit()));
        params.set_order(options.get_order().empty() ? "published_at DESC" : options.get_order());
        
        return m_repo.find_all(params, filter);
    }

    // Read single post
    std::expected<ApiResponse<Post>, std::string> read(const std::string& identifier) noexcept {
        // Try by ID first, then UUID, then slug
        auto result = m_repo.find_by_id(identifier);
        if (result && result->get_data().has_value()) {
            return result;
        }
        
        result = m_repo.find_by_uuid(identifier);
        if (result && result->get_data().has_value()) {
            return result;
        }
        
        return m_repo.find_by_slug(identifier);
    }

    // Add new post
    std::expected<ApiResponse<Post>, std::string> add(const Post& post) noexcept {
        return m_repo.create(post);
    }

    // Edit post
    std::expected<ApiResponse<Post>, std::string> edit(const std::string& id, const Post& post) noexcept {
        return m_repo.update(id, post);
    }

    // Delete post
    std::expected<ApiResponse<bool>, std::string> destroy(const std::string& id) noexcept {
        return m_repo.delete_by_id(id);
    }

    // Bulk operations
    std::expected<ApiResponse<std::vector<Post>>, std::string> bulk_edit(const json& bulk_data, const json& options) noexcept {
        return m_repo.bulk_update_status({}, "");
    }

    std::expected<ApiResponse<json>, std::string> bulk_destroy(const json& options) noexcept {
        return ApiResponse<json>::ok(json{});
    }

    // Copy post
    std::expected<ApiResponse<Post>, std::string> copy(const std::string& id) noexcept {
        auto result = m_repo.find_by_id(id);
        if (!result || !result->get_data().has_value()) {
            return std::unexpected("Post not found");
        }
        
        Post new_post = *result->get_data();
        new_post.set_id(Post::generate_uuid());
        new_post.set_uuid(Post::generate_uuid());
        new_post.set_slug(new_post.get_slug() + "-copy");
        new_post.set_status("draft");
        new_post.set_created_at(Post::current_iso8601());
        new_post.set_updated_at(new_post.get_created_at());
        new_post.set_published_at("");
        
        return m_repo.create(new_post);
    }

    // Export CSV
    std::expected<ApiResponse<json>, std::string> export_csv(const BrowseOptions& options) noexcept {
        auto result = browse(options);
        if (!result) {
            return std::unexpected(result.error());
        }
        
        json export_data;
        export_data["data"] = result->get_data();
        export_data["filename"] = "posts_export.csv";
        
        return ApiResponse<json>::ok(std::move(export_data));
    }

    // Import CSV
    struct ImportResult {
        std::string m_import_id{};
        int m_total = 0;
        
        ImportResult() noexcept = default;
        ImportResult(std::string&& import_id, int total) noexcept
            : m_import_id{std::move(import_id)}, m_total{total} {}
        
        ImportResult& add_import_id(std::string&& id) noexcept { m_import_id = std::move(id); return *this; }
        ImportResult& add_total(int total) noexcept { m_total = total; return *this; }
        
        void set_import_id(std::string&& id) noexcept { m_import_id = std::move(id); }
        void set_total(int total) noexcept { m_total = total; }
        
        std::string& get_import_id() noexcept { return m_import_id; }
        int& get_total() noexcept { return m_total; }
        const std::string& get_import_id() const noexcept { return m_import_id; }
        int get_total() const noexcept { return m_total; }
    };
    
    std::expected<ApiResponse<ImportResult>, std::string> import_csv(const std::string& file_path, const json& mapping) noexcept {
        // Placeholder for CSV import
        ImportResult import_result;
        import_result.set_import_id(Post::generate_uuid());
        import_result.set_total(0);
        
        return ApiResponse<ImportResult>::ok(std::move(import_result));
    }

private:
    std::shared_ptr<IDatabase> m_database;
    PostRepository m_repo;
    
    static void parse_filter(const std::string& filter_str, PostFilter& filter) noexcept {
        // Simple filter parsing: "key:value,key2:value2"
        size_t pos = 0;
        while (pos < filter_str.size()) {
            size_t comma = filter_str.find(',', pos);
            if (comma == std::string::npos) comma = filter_str.size();
            
            size_t colon = filter_str.find(':', pos);
            if (colon != std::string::npos && colon < comma) {
                std::string key = filter_str.substr(pos, colon - pos);
                std::string value = filter_str.substr(colon + 1, comma - colon - 1);
                
                if (key == "status") filter.set_status(std::move(value));
                else if (key == "author") filter.set_author_id(std::move(value));
                else if (key == "tag") filter.set_tag(std::move(value));
                else if (key == "visibility") filter.set_visibility(std::move(value));
            }
            
            pos = comma + 1;
        }
    }
};

/**
 * Register posts admin routes
 */
void register_posts_admin_routes(core::router::RouterContext<256, 64, 10>& router_ctx,
                                 std::shared_ptr<IDatabase> database) {
    PostsAdminController controller{std::move(database)};
    
    // Browse posts
    router_ctx.add_route(core::router::Route<8, 10>{"/admin/posts/"}
        .get([controller = std::move(controller)](
            interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) mutable {
            // Parse query params
            PostsAdminController::BrowseOptions options;
            // ... parse query params
            auto result = controller.browse(options);
            if (result) {
                auto json_result = result->get_data().to_json();
                if (json_result) {
                    res.set_status(interfaces::io::types::Status::OK);
                    res.add_header(interfaces::io::types::Token::CONTENT_TYPE, "application/json");
                    // Send response
                }
            }
            send();
        }))
        .set_router_number(1));
}

} // namespace ghost::blog::endpoints