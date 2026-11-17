module;
#include <memory>

export module blog_api:endpoints:members_index;

import std;
import blog_api:types;
import blog_database:database;
import blog_api:endpoints:members_public;
import core_router;
import interfaces;

export namespace ghost::blog::endpoints {

/**
 * Members API Endpoints Registry - Registers all Members API endpoints
 */
class MembersEndpoints {
public:
    explicit MembersEndpoints(std::shared_ptr<IDatabase> database) noexcept
        : m_database{std::move(database)} {}

    void register_all(core::router::RouterContext<256, 64, 10>& router_ctx) noexcept {
        // Members (public)
        register_members_public_routes(router_ctx, m_database);
    }

private:
    std::shared_ptr<IDatabase> m_database;
};

} // namespace ghost::blog::endpoints