module;
#include <memory>

export module blog_api:endpoints:content_index;

import std;
import blog_api:types;
import blog_database:database;
import blog_api:endpoints:posts_public;
import blog_api:endpoints:tags_public;
import blog_api:endpoints:authors_public;
import core_router;
import interfaces;

export namespace ghost::blog::endpoints {

/**
 * Content API Endpoints Registry - Registers all Content API endpoints
 */
class ContentEndpoints {
public:
    explicit ContentEndpoints(std::shared_ptr<IDatabase> database) noexcept
        : m_database{std::move(database)} {}

    void register_all(core::router::RouterContext<256, 64, 10>& router_ctx) noexcept {
        // Posts (public)
        register_posts_public_routes(router_ctx, m_database);
        
        // Tags (public)
        register_tags_public_routes(router_ctx, m_database);
        
        // Authors (public)
        register_authors_public_routes(router_ctx, m_database);
    }

private:
    std::shared_ptr<IDatabase> m_database;
};

} // namespace ghost::blog::endpoints