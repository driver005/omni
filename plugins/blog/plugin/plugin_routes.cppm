module;
#include <memory>
#include <string>

export module blog_plugin:plugin_routes;

import std;
import blog_api:types;
import blog_api:post_controller;
import core_router;

export namespace ghost::blog {

class IBlogApi;
class PostController;
class TagController;
class AuthorController;

/**
 * Plugin Routes - Registers all API routes with the core router
 */
class PluginRoutes {
public:
    explicit PluginRoutes(std::shared_ptr<IBlogApi> api);
    
    // Register all routes on the given router context
    void register_routes(core::router::RouterContext<256, 64, 10>& router_ctx);
    
    // Register individual route groups
    void register_post_routes(core::router::RouterContext<256, 64, 10>& router_ctx, 
                              core::router::RouteBuilder& builder);
    void register_tag_routes(core::router::RouterContext<256, 64, 10>& router_ctx,
                             core::router::RouteBuilder& builder);
    void register_author_routes(core::router::RouterContext<256, 64, 10>& router_ctx,
                                core::router::RouteBuilder& builder);
    void register_search_routes(core::router::RouterContext<256, 64, 10>& router_ctx,
                                core::router::RouteBuilder& builder);
    void register_health_routes(core::router::RouterContext<256, 64, 10>& router_ctx,
                                core::router::RouteBuilder& builder);
    
    // Register middleware
    void register_middleware(core::router::RouterContext<256, 64, 10>& router_ctx);

private:
    std::shared_ptr<IBlogApi> api_;
    std::unique_ptr<PostController> post_controller_;
    std::unique_ptr<TagController> tag_controller_;
    std::unique_ptr<AuthorController> author_controller_;
    
    // Helper to create route builder with base path
    core::router::RouteBuilder create_builder(core::router::RouterContext<256, 64, 10>& router_ctx,
                                              const std::string& base_path);
};

} // namespace ghost::blog