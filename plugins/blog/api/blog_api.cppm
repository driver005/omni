module;
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <chrono>
#include <mutex>
#include <optional>

export module blog_api:api;

import std;
import :types;
import :post_controller;
import :json_serializer;
import core_router;
import interfaces;

export namespace ghost::blog {

/**
 * Blog API Implementation - uses database repositories
 */
class BlogApi : public IBlogApi {
public:
    explicit BlogApi(std::shared_ptr<IDatabase> database);
    
    ApiResponse<std::vector<Post>> get_posts(const PaginationParams& params, const PostFilter& filter) override;
    ApiResponse<Post> get_post(const std::string& id) override;
    ApiResponse<Post> get_post_by_uuid(const std::string& uuid) override;
    ApiResponse<Post> get_post_by_slug(const std::string& slug) override;
    ApiResponse<Post> create_post(const Post& post) override;
    ApiResponse<Post> update_post(const std::string& id, const Post& post) override;
    ApiResponse<bool> delete_post(const std::string& id) override;
    ApiResponse<Post> publish_post(const std::string& id) override;
    ApiResponse<Post> unpublish_post(const std::string& id) override;
    ApiResponse<Post> schedule_post(const std::string& id, const std::string& scheduled_at) override;
    
    ApiResponse<std::vector<Tag>> get_tags(const PaginationParams& params) override;
    ApiResponse<Tag> get_tag(const std::string& id) override;
    ApiResponse<Tag> get_tag_by_slug(const std::string& slug) override;
    ApiResponse<Tag> create_tag(const Tag& tag) override;
    ApiResponse<Tag> update_tag(const std::string& id, const Tag& tag) override;
    ApiResponse<bool> delete_tag(const std::string& id) override;
    ApiResponse<std::vector<Post>> get_posts_by_tag(const std::string& tag_id, const PaginationParams& params) override;
    
    ApiResponse<std::vector<Author>> get_authors(const PaginationParams& params) override;
    ApiResponse<Author> get_author(const std::string& id) override;
    ApiResponse<Author> get_author_by_slug(const std::string& slug) override;
    ApiResponse<std::vector<Post>> get_posts_by_author(const std::string& author_id, const PaginationParams& params) override;
    
    ApiResponse<std::vector<Post>> search_posts(const std::string& query, const PaginationParams& params) override;
    
    ApiResponse<json> get_stats() override;

private:
    std::shared_ptr<IDatabase> database_;
};

/**
 * Factory function
 */
std::unique_ptr<IBlogApi> create_blog_api(std::shared_ptr<IDatabase> database);

} // namespace ghost::blog