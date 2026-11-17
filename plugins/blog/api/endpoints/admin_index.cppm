module;
#include <memory>

export module blog_api:endpoints:admin_index;

import std;
import blog_api:types;
import blog_database:database;
import blog_api:endpoints:posts_admin;
import blog_api:endpoints:tags_admin;
import blog_api:endpoints:authors_admin;
import blog_api:endpoints:members_admin;
import blog_api:endpoints:pages_admin;
import blog_api:endpoints:newsletters_admin;
import blog_api:endpoints:tiers_admin;
import blog_api:endpoints:settings_admin;
import blog_api:endpoints:site_admin;
import core_router;
import interfaces;

export namespace ghost::blog::endpoints {

/**
 * Admin API Endpoints Registry - Registers all Admin API endpoints
 */
class AdminEndpoints {
public:
    explicit AdminEndpoints(std::shared_ptr<IDatabase> database) noexcept
        : m_database{std::move(database)} {}

    void register_all(core::router::RouterContext<256, 64, 10>& router_ctx) noexcept {
        // Posts
        register_posts_admin_routes(router_ctx, m_database);
        
        // Tags
        register_tags_admin_routes(router_ctx, m_database);
        
        // Authors (Users)
        register_authors_admin_routes(router_ctx, m_database);
        
        // Members
        register_members_admin_routes(router_ctx, m_database);
        
        // Pages
        register_pages_admin_routes(router_ctx, m_database);
        
        // Newsletters
        register_newsletters_admin_routes(router_ctx, m_database);
        
        // Tiers
        register_tiers_admin_routes(router_ctx, m_database);
        
        // Settings
        register_settings_admin_routes(router_ctx, m_database);
        
        // Site
        register_site_admin_routes(router_ctx, m_database);
    }

private:
    std::shared_ptr<IDatabase> m_database;
};

} // namespace ghost::blog::endpoints