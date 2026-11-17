module;
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <memory>
#include <optional>

export module blog_database:author_repository;

import std;
import blog_api:types;
import blog_database:database;

export namespace ghost::blog {

using json = nlohmann::json;

/**
 * Author Repository - Handles all author-related database operations
 */
class AuthorRepository {
public:
    explicit AuthorRepository(std::shared_ptr<IDatabase> database);
    
    // CRUD Operations
    ApiResponse<Author> create(const Author& author);
    ApiResponse<std::optional<Author>> find_by_id(const std::string& id);
    ApiResponse<std::optional<Author>> find_by_slug(const std::string& slug);
    ApiResponse<std::optional<Author>> find_by_email(const std::string& email);
    ApiResponse<std::vector<Author>> find_all(const PaginationParams& params);
    ApiResponse<Author> update(const std::string& id, const Author& author);
    ApiResponse<bool> delete_by_id(const std::string& id);
    
    // Post-Author Relationships
    ApiResponse<std::vector<Post>> get_posts(const std::string& author_id, const PaginationParams& params);
    ApiResponse<size_t> get_post_count(const std::string& author_id);
    ApiResponse<std::vector<Post>> get_published_posts(const std::string& author_id, const PaginationParams& params);
    ApiResponse<std::vector<Post>> get_drafts(const std::string& author_id, const PaginationParams& params);
    
    // Status Operations
    ApiResponse<Author> activate(const std::string& id);
    ApiResponse<Author> deactivate(const std::string& id);
    ApiResponse<Author> update_last_login(const std::string& id);
    
    // Search & Filter
    ApiResponse<std::vector<Author>> search(const std::string& query, const PaginationParams& params);
    ApiResponse<std::vector<Author>> find_by_status(const std::string& status, const PaginationParams& params);
    
    // Count
    ApiResponse<size_t> count();
    ApiResponse<size_t> count_by_status(const std::string& status);
    
    // Stats
    ApiResponse<json> get_stats();
    ApiResponse<json> get_author_stats(const std::string& author_id);
    ApiResponse<std::vector<Author>> get_popular_authors(int limit = 10);
    ApiResponse<std::vector<Author>> get_recent_authors(int limit = 10);
    
    // Profile
    ApiResponse<Author> update_profile(const std::string& id, const json& profile_data);
    ApiResponse<Author> update_avatar(const std::string& id, const std::string& image_url);
    ApiResponse<Author> update_bio(const std::string& id, const std::string& bio);

private:
    std::shared_ptr<IDatabase> database_;
    
    Author map_row_to_author(const json& row);
};

} // namespace ghost::blog