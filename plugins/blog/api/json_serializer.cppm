module;
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <chrono>

export module blog_api:json_serializer;

import std;
import :types;

export namespace ghost::blog {

/**
 * JSON Serialization utilities for Blog entities
 * Handles serialization/deserialization with proper field mapping
 */
class JsonSerializer {
public:
    // Post serialization
    static json serialize_post(const Post& post, bool include_meta = true);
    static json serialize_post_list(const std::vector<Post>& posts, bool include_meta = true);
    static Post deserialize_post(const json& json);
    static std::vector<Post> deserialize_post_list(const json& json);
    
    // Tag serialization
    static json serialize_tag(const Tag& tag);
    static json serialize_tag_list(const std::vector<Tag>& tags);
    static Tag deserialize_tag(const json& json);
    static std::vector<Tag> deserialize_tag_list(const json& json);
    
    // Author serialization
    static json serialize_author(const Author& author);
    static json serialize_author_list(const std::vector<Author>& authors);
    static Author deserialize_author(const json& json);
    static std::vector<Author> deserialize_author_list(const json& json);
    
    // API Response serialization
    template<typename T>
    static json serialize_response(const ApiResponse<T>& response);
    
    // Pagination serialization
    static json serialize_pagination(const PaginationParams& params, 
                                     size_t total_count,
                                     size_t total_pages);
    
    // Filter serialization
    static json serialize_filter(const PostFilter& filter);
    
    // Error serialization
    static json serialize_error(const std::string& message, int code = 400);
    static json serialize_validation_error(const std::vector<std::string>& errors);
    
    // Stats serialization
    static json serialize_stats(const json& stats);
    
private:
    // Helper methods
    static std::string current_iso8601();
    static std::string generate_slug(const std::string& title);
    static void sanitize_html(std::string& html);
};

} // namespace ghost::blog