module;
#include <memory>

export module blog_api:endpoints:main_index;

import std;
import blog_api:types;
import blog_database:database;
import blog_api:endpoints:admin_index;
import blog_api:endpoints:content_index;
import blog_api:endpoints:members_index;
import core_router;
import interfaces;

export namespace ghost::blog::endpoints {

/**
 * Main Endpoints Registry - Registers all API endpoints (Admin, Content, Members)
 */
class EndpointsRegistry {
public:
    explicit EndpointsRegistry(std::shared_ptr<IDatabase> database) noexcept
        : m_database{std::move(database)},
          m_admin_endpoints{m_database},
          m_content_endpoints{m_database},
          m_members_endpoints{m_database} {}

    void register_all(core::router::RouterContext<256, 64, 10>& router_ctx) noexcept {
        // Register Admin API endpoints (mounted at /admin/)
        m_admin_endpoints.register_all(router_ctx);
        
        // Register Content API endpoints (mounted at /content/)
        m_content_endpoints.register_all(router_ctx);
        
        // Register Members API endpoints (mounted at /members/)
        m_members_endpoints.register_all(router_ctx);
    }

private:
    std::shared_ptr<IDatabase> m_database;
    AdminEndpoints m_admin_endpoints;
    ContentEndpoints m_content_endpoints;
    MembersEndpoints m_members_endpoints;
};

/**
 * Factory function to create and register all endpoints
 */
std::unique_ptr<EndpointsRegistry> create_endpoints_registry(std::shared_ptr<IDatabase> database) noexcept {
    return std::make_unique<EndpointsRegistry>(std::move(database));
}

} // namespace ghost::blog::endpoints