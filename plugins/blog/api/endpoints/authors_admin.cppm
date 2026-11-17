module;
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <expected>

export module blog_api:endpoints:authors_admin;

import std;
import blog_api:types;
import blog_database:database;
import blog_database:author_repository;
import core_router;
import interfaces;

export namespace ghost::blog::endpoints {

using json = nlohmann::json;

/**
 * Authors Admin Controller - Matches Ghost's users.js controller (authors in admin)
 */
class AuthorsAdminController {
public:
    explicit AuthorsAdminController(std::shared_ptr<IDatabase> database) noexcept
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

    // Browse authors
    std::expected<ApiResponse<std::vector<Author>>, std::string> browse(const BrowseOptions& options) noexcept {
        PaginationParams params;
        params.set_page(std::to_string(options.get_page()));
        params.set_limit(std::to_string(options.get_limit()));
        params.set_order(options.get_order().empty() ? "name ASC" : options.get_order());
        
        return m_repo.find_all(params);
    }

    // Read author
    std::expected<ApiResponse<Author>, std::string> read(const std::string& identifier) noexcept {
        auto result = m_repo.find_by_id(identifier);
        if (result && result->get_data().has_value()) {
            return result;
        }
        
        result = m_repo.find_by_slug(identifier);
        if (result && result->get_data().has_value()) {
            return result;
        }
        
        result = m_repo.find_by_email(identifier);
        return result ? std::move(*result) : ApiResponse<Author>::not_found("Author");
    }

    // Add author
    std::expected<ApiResponse<Author>, std::string> add(const Author& author) noexcept {
        return m_repo.create(author);
    }

    // Edit author
    std::expected<ApiResponse<Author>, std::string> edit(const std::string& id, const Author& author) noexcept {
        return m_repo.update(id, author);
    }

    // Delete author
    std::expected<ApiResponse<bool>, std::string> destroy(const std::string& id) noexcept {
        return m_repo.delete_by_id(id);
    }

    // Get posts by author
    std::expected<ApiResponse<std::vector<Post>>, std::string> get_posts(const std::string& author_id, const BrowseOptions& options) noexcept {
        PaginationParams params;
        params.set_page(std::to_string(options.get_page()));
        params.set_limit(std::to_string(options.get_limit()));
        params.set_order(options.get_order().empty() ? "published_at DESC" : options.get_order());
        
        return m_repo.get_posts(author_id, params);
    }

    // Change password
    std::expected<ApiResponse<Author>, std::string> change_password(const std::string& id, const std::string& new_password) noexcept {
        auto result = m_repo.find_by_id(id);
        if (!result || !result->get_data().has_value()) {
            return ApiResponse<Author>::not_found("Author");
        }
        // Password change would be handled by auth service
        return result ? std::move(*result) : ApiResponse<Author>::error("Password change not implemented");
    }

    // Transfer ownership
    std::expected<ApiResponse<Author>, std::string> transfer_ownership(const std::string& from_id, const std::string& to_id) noexcept {
        // Ownership transfer logic
        return ApiResponse<Author>::error("Ownership transfer not implemented");
    }

    // Regenerate staff token
    std::expected<ApiResponse<Author>, std::string> regenerate_staff_token(const std::string& id) noexcept {
        auto result = m_repo.find_by_id(id);
        return result ? std::move(*result) : ApiResponse<Author>::not_found("Author");
    }

    // Read staff token
    std::expected<ApiResponse<json>, std::string> read_staff_token(const std::string& id) noexcept {
        auto result = m_repo.find_by_id(id);
        if (!result || !result->get_data().has_value()) {
            return ApiResponse<json>::not_found("Author");
        }
        json token_data;
        token_data["token"] = "generated_token_here";
        return ApiResponse<json>::ok(std::move(token_data));
    }

private:
    std::shared_ptr<IDatabase> m_database;
    AuthorRepository m_repo;
};

/**
 * Register authors admin routes
 */
void register_authors_admin_routes(core::router::RouterContext<256, 64, 10>& router_ctx,
                                   std::shared_ptr<IDatabase> database) {
    AuthorsAdminController controller{std::move(database)};
    
    router_ctx.add_route(core::router::Route<8, 10>{"/admin/users/"}
        .get([controller = std::move(controller)](
            interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) mutable {
            send();
        }))
        .set_router_number(1);
    
    router_ctx.add_route(core::router::Route<8, 10>{"/admin/users/:id/"}
        .get([controller = std::move(controller)](
            interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) mutable {
            send();
        }))
        .set_router_number(1);
    
    router_ctx.add_route(core::router::Route<8, 10>{"/admin/users/slug/:slug/"}
        .get([controller = std::move(controller)](
            interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) mutable {
            send();
        }))
        .set_router_number(1);
    
    router_ctx.add_route(core::router::Route<8, 10>{"/admin/users/email/:email/"}
        .get([controller = std::move(controller)](
            interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) mutable {
            send();
        }))
        .set_router_number(1);
    
    router_ctx.add_route(core::router::Route<8, 10>{"/admin/users/:id/token/"}
        .get([controller = std::move(controller)](
            interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) mutable {
            send();
        }))
        .set_router_number(1);
    
    router_ctx.add_route(core::router::Route<8, 10>{"/admin/users/password/"}
        .put([controller = std::move(controller)](
            interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) mutable {
            send();
        }))
        .set_router_number(1);
    
    router_ctx.add_route(core::router::Route<8, 10>{"/admin/users/owner/"}
        .put([controller = std::move(controller)](
            interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) mutable {
            send();
        }))
        .set_router_number(1);
    
    router_ctx.add_route(core::router::Route<8, 10>{"/admin/users/:id/token/"}
        .put([controller = std::move(controller)](
            interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send) mutable {
            send();
        }))
        .set_router_number(1);
}

} // namespace ghost::blog::endpoints