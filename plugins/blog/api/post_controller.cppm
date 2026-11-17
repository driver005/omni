module;
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <memory>

export module blog_api:post_controller;

import std;
import :types;
import :json_serializer;
import core_router;
import interfaces;

export namespace ghost::blog {

/**
 * Post Controller - handles all post-related HTTP routes
 * Uses the core router from include/core/router/
 */
class PostController {
public:
    explicit PostController(std::shared_ptr<IBlogApi> api);
    
    // Register all routes on the given router context
    void register_routes(core::router::RouterContext<256, 64, 10>& router_ctx, std::size_t base_router = 0);
    
    // Get the route builder for manual registration
    core::router::RouteBuilder get_route_builder() const;

private:
    std::shared_ptr<IBlogApi> api_;
    
    // Route handlers (static for use with core router)
    static void handle_list_posts(const std::shared_ptr<IBlogApi>& api, 
                                  interfaces::io::IRequest& req, 
                                  interfaces::io::IResponse& res, 
                                  std::function<void()> send);
    static void handle_get_post(const std::shared_ptr<IBlogApi>& api,
                                interfaces::io::IRequest& req,
                                interfaces::io::IResponse& res,
                                std::function<void()> send);
    static void handle_get_post_by_uuid(const std::shared_ptr<IBlogApi>& api,
                                        interfaces::io::IRequest& req,
                                        interfaces::io::IResponse& res,
                                        std::function<void()> send);
    static void handle_get_post_by_slug(const std::shared_ptr<IBlogApi>& api,
                                        interfaces::io::IRequest& req,
                                        interfaces::io::IResponse& res,
                                        std::function<void()> send);
    static void handle_create_post(const std::shared_ptr<IBlogApi>& api,
                                   interfaces::io::IRequest& req,
                                   interfaces::io::IResponse& res,
                                   std::function<void()> send);
    static void handle_update_post(const std::shared_ptr<IBlogApi>& api,
                                   interfaces::io::IRequest& req,
                                   interfaces::io::IResponse& res,
                                   std::function<void()> send);
    static void handle_delete_post(const std::shared_ptr<IBlogApi>& api,
                                   interfaces::io::IRequest& req,
                                   interfaces::io::IResponse& res,
                                   std::function<void()> send);
    static void handle_publish_post(const std::shared_ptr<IBlogApi>& api,
                                    interfaces::io::IRequest& req,
                                    interfaces::io::IResponse& res,
                                    std::function<void()> send);
    static void handle_unpublish_post(const std::shared_ptr<IBlogApi>& api,
                                      interfaces::io::IRequest& req,
                                      interfaces::io::IResponse& res,
                                      std::function<void()> send);
    static void handle_schedule_post(const std::shared_ptr<IBlogApi>& api,
                                     interfaces::io::IRequest& req,
                                     interfaces::io::IResponse& res,
                                     std::function<void()> send);
    
    // Tag handlers
    static void handle_list_tags(const std::shared_ptr<IBlogApi>& api,
                                 interfaces::io::IRequest& req,
                                 interfaces::io::IResponse& res,
                                 std::function<void()> send);
    static void handle_get_tag(const std::shared_ptr<IBlogApi>& api,
                               interfaces::io::IRequest& req,
                               interfaces::io::IResponse& res,
                               std::function<void()> send);
    static void handle_get_tag_by_slug(const std::shared_ptr<IBlogApi>& api,
                                       interfaces::io::IRequest& req,
                                       interfaces::io::IResponse& res,
                                       std::function<void()> send);
    static void handle_create_tag(const std::shared_ptr<IBlogApi>& api,
                                  interfaces::io::IRequest& req,
                                  interfaces::io::IResponse& res,
                                  std::function<void()> send);
    static void handle_update_tag(const std::shared_ptr<IBlogApi>& api,
                                  interfaces::io::IRequest& req,
                                  interfaces::io::IResponse& res,
                                  std::function<void()> send);
    static void handle_delete_tag(const std::shared_ptr<IBlogApi>& api,
                                  interfaces::io::IRequest& req,
                                  interfaces::io::IResponse& res,
                                  std::function<void()> send);
    static void handle_get_posts_by_tag(const std::shared_ptr<IBlogApi>& api,
                                        interfaces::io::IRequest& req,
                                        interfaces::io::IResponse& res,
                                        std::function<void()> send);
    
    // Author handlers
    static void handle_list_authors(const std::shared_ptr<IBlogApi>& api,
                                    interfaces::io::IRequest& req,
                                    interfaces::io::IResponse& res,
                                    std::function<void()> send);
    static void handle_get_author(const std::shared_ptr<IBlogApi>& api,
                                  interfaces::io::IRequest& req,
                                  interfaces::io::IResponse& res,
                                  std::function<void()> send);
    static void handle_get_author_by_slug(const std::shared_ptr<IBlogApi>& api,
                                          interfaces::io::IRequest& req,
                                          interfaces::io::IResponse& res,
                                          std::function<void()> send);
    static void handle_get_posts_by_author(const std::shared_ptr<IBlogApi>& api,
                                           interfaces::io::IRequest& req,
                                           interfaces::io::IResponse& res,
                                           std::function<void()> send);
    
    // Search handler
    static void handle_search_posts(const std::shared_ptr<IBlogApi>& api,
                                    interfaces::io::IRequest& req,
                                    interfaces::io::IResponse& res,
                                    std::function<void()> send);
    
    // Stats handler
    static void handle_get_stats(const std::shared_ptr<IBlogApi>& api,
                                 interfaces::io::IRequest& req,
                                 interfaces::io::IResponse& res,
                                 std::function<void()> send);
    
    // Helper methods
    static PaginationParams parse_pagination(const interfaces::io::IRequest& request);
    static PostFilter parse_filter(const interfaces::io::IRequest& request);
    static Post parse_post_body(const interfaces::io::IRequest& request);
    static Tag parse_tag_body(const interfaces::io::IRequest& request);
    static std::string get_path_param(const interfaces::io::IRequest& request, const std::string& name);
    static std::string get_query_param(const interfaces::io::IRequest& request, const std::string& name);
    
    // Response helpers
    static void send_json(interfaces::io::IResponse& res, 
                          interfaces::io::types::Status status, 
                          const json& data,
                          std::function<void()> send);
    static void send_error(interfaces::io::IResponse& res,
                           interfaces::io::types::Status status,
                           const std::string& message,
                           std::function<void()> send);
};

} // namespace ghost::blog