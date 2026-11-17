module;
#include <memory>

export module blog_api:endpoints;

import std;
import blog_api:types;
import blog_database:database;
import core_router;
import interfaces;

export namespace ghost::blog::endpoints {

class EndpointsRegistry {
public:
    explicit EndpointsRegistry(std::shared_ptr<IDatabase> database) noexcept
        : m_database{std::move(database)} {}

    // Admin API Endpoints
    void register_admin_endpoints(core::router::RouterContext<256, 64, 10>& router_ctx);
    
    // Content API Endpoints
    void register_content_endpoints(core::router::RouterContext<256, 64, 10>& router_ctx);
    
    // Members API Endpoints
    void register_members_endpoints(core::router::RouterContext<256, 64, 10>& router_ctx);

private:
    std::shared_ptr<IDatabase> m_database;
    
    // Helper to get database
    std::shared_ptr<IDatabase> get_database() const noexcept { return m_database; }
};

} // namespace ghost::blog::endpoints