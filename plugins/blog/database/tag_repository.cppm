module;
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <memory>
#include <optional>

export module blog_database:tag_repository;

import std;
import blog_api:types;
import blog_database:database;

export namespace ghost::blog {

using json = nlohmann::json;

/**
 * Tag Repository - Handles all tag-related database operations
 */
class TagRepository {
public:
    explicit TagRepository(std::shared_ptr<IDatabase> database);
    
    // CRUD Operations
    ApiResponse<Tag> create(const Tag& tag);
    ApiResponse<std::optional<Tag>> find_by_id(const std::string& id);
    ApiResponse<std::optional<Tag>> find_by_slug(const std::string& slug);
    ApiResponse<std::optional<Tag>> find_by_name(const std::string& name);
    ApiResponse<std::vector<Tag>> find_all(const PaginationParams& params);
    ApiResponse<Tag> update(const std::string& id, const Tag& tag);
    ApiResponse<bool> delete_by_id(const std::string& id);
    
    // Post-Tag Relationships
    ApiResponse<std::vector<Post>> get_posts(const std::string& tag_id, const PaginationParams& params);
    ApiResponse<size_t> get_post_count(const std::string& tag_id);
    ApiResponse<bool> add_post(const std::string& tag_id, const std::string& post_id);
    ApiResponse<bool> remove_post(const std::string& tag_id, const std::string& post_id);
    ApiResponse<std::vector<Tag>> get_post_tags(const std::string& post_id);
    
    // Bulk Operations
    ApiResponse<std::vector<Tag>> bulk_create(const std::vector<Tag>& tags);
    ApiResponse<int> bulk_delete(const std::vector<std::string>& ids);
    
    // Search & Filter
    ApiResponse<std::vector<Tag>> search(const std::string& query, const PaginationParams& params);
    ApiResponse<std::vector<Tag>> find_by_visibility(const std::string& visibility, const PaginationParams& params);
    
    // Count
    ApiResponse<size_t> count();
    
    // Stats
    ApiResponse<json> get_stats();
    ApiResponse<std::vector<Tag>> get_popular_tags(int limit = 10);
    ApiResponse<std::vector<Tag>> get_recent_tags(int limit = 10);

private:
    std::shared_ptr<IDatabase> database_;
    
    Tag map_row_to_tag(const json& row);
};

} // namespace ghost::blog