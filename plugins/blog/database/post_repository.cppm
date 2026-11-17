module;
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <expected>

export module blog_database:post_repository;

import std;
import blog_api:types;
import blog_database:database;
import connector;

export namespace ghost::blog {

using json = nlohmann::json;

/**
 * Post Repository - Handles all post-related database operations
 */
class PostRepository {
public:
    explicit PostRepository(std::shared_ptr<IDatabase> database) noexcept
        : m_database{std::move(database)} {}

    // CRUD Operations
    std::expected<ApiResponse<Post>, std::string> create(const Post& post) noexcept {
        if (!m_database || !m_database->is_connected()) {
            return std::unexpected("Database not connected");
        }
        try {
            // Insert post
            auto result = m_database->execute_update(
                "INSERT INTO posts (id, uuid, title, slug, content, plaintext, author_id, "
                "status, visibility, created_at, updated_at, published_at, scheduled_at, "
                "feature_image, feature_image_alt, feature_image_caption, excerpt, "
                "codeinjection_head, codeinjection_foot, custom_excerpt, canonical_url, "
                "meta_title, meta_description, og_image, og_title, og_description, "
                "twitter_image, twitter_title, twitter_description, custom_template, "
                "email_only, comment_id, meta) "
                "VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11, $12, $13, "
                "$14, $15, $16, $17, $18, $19, $20, $21, $22, $23, $24, $25, $26, "
                "$27, $28, $29, $30, $31, $32, $33) "
                "RETURNING *",
                json{
                    post.get_id(), post.get_uuid(), post.get_title(), post.get_slug(),
                    post.get_content(), post.get_plaintext(), post.get_author_id(),
                    post.get_status(), post.get_visibility(), post.get_created_at(),
                    post.get_updated_at(), post.get_published_at(), post.get_scheduled_at(),
                    post.get_feature_image(), post.get_feature_image_alt(),
                    post.get_feature_image_caption(), post.get_excerpt(),
                    post.get_codeinjection_head(), post.get_codeinjection_foot(),
                    post.get_custom_excerpt(), post.get_canonical_url(),
                    post.get_meta_title(), post.get_meta_description(),
                    post.get_og_image(), post.get_og_title(), post.get_og_description(),
                    post.get_twitter_image(), post.get_twitter_title(),
                    post.get_twitter_description(), post.get_custom_template(),
                    post.get_email_only(), post.get_comment_id(), post.get_meta()
                }
            );
            return ApiResponse<Post>::created(post);
        } catch (const std::exception& e) {
            return std::unexpected(e.what());
        }
    }

    std::expected<ApiResponse<std::optional<Post>>, std::string> find_by_id(const std::string& id) noexcept {
        if (!m_database || !m_database->is_connected()) {
            return std::unexpected("Database not connected");
        }
        try {
            auto result = m_database->execute_query(
                "SELECT * FROM posts WHERE id = $1", json{id}
            );
            if (result.empty()) {
                return ApiResponse<std::optional<Post>>::not_found("Post");
            }
            auto post = Post::from_json(result[0]);
            if (!post) {
                return std::unexpected(post.error());
            }
            return ApiResponse<std::optional<Post>>::ok(std::move(*post));
        } catch (const std::exception& e) {
            return std::unexpected(e.what());
        }
    }

    std::expected<ApiResponse<std::optional<Post>>, std::string> find_by_uuid(const std::string& uuid) noexcept {
        if (!m_database || !m_database->is_connected()) {
            return std::unexpected("Database not connected");
        }
        try {
            auto result = m_database->execute_query(
                "SELECT * FROM posts WHERE uuid = $1", json{uuid}
            );
            if (result.empty()) {
                return ApiResponse<std::optional<Post>>::not_found("Post");
            }
            auto post = Post::from_json(result[0]);
            if (!post) {
                return std::unexpected(post.error());
            }
            return ApiResponse<std::optional<Post>>::ok(std::move(*post));
        } catch (const std::exception& e) {
            return std::unexpected(e.what());
        }
    }

    std::expected<ApiResponse<std::optional<Post>>, std::string> find_by_slug(const std::string& slug) noexcept {
        if (!m_database || !m_database->is_connected()) {
            return std::unexpected("Database not connected");
        }
        try {
            auto result = m_database->execute_query(
                "SELECT * FROM posts WHERE slug = $1", json{slug}
            );
            if (result.empty()) {
                return ApiResponse<std::optional<Post>>::not_found("Post");
            }
            auto post = Post::from_json(result[0]);
            if (!post) {
                return std::unexpected(post.error());
            }
            return ApiResponse<std::optional<Post>>::ok(std::move(*post));
        } catch (const std::exception& e) {
            return std::unexpected(e.what());
        }
    }

    std::expected<ApiResponse<std::vector<Post>>, std::string> find_all(const PaginationParams& params, const PostFilter& filter) noexcept {
        if (!m_database || !m_database->is_connected()) {
            return std::unexpected("Database not connected");
        }
        try {
            std::string sql = "SELECT * FROM posts";
            json query_params;
            std::string where_clause = build_where_clause(filter, query_params);
            if (!where_clause.empty()) {
                sql += " WHERE " + where_clause;
            }
            sql += " ORDER BY " + build_order_clause(params);
            sql += " " + build_pagination_clause(params);

            auto result = m_database->execute_query(sql, query_params);
            std::vector<Post> posts;
            posts.reserve(result.size());
            for (const auto& row : result) {
                auto post = Post::from_json(row);
                if (post) {
                    posts.push_back(std::move(*post));
                }
            }
            return ApiResponse<std::vector<Post>>::ok(std::move(posts));
        } catch (const std::exception& e) {
            return std::unexpected(e.what());
        }
    }

    std::expected<ApiResponse<Post>, std::string> update(const std::string& id, const Post& post) noexcept {
        if (!m_database || !m_database->is_connected()) {
            return std::unexpected("Database not connected");
        }
        try {
            auto result = m_database->execute_update(
                "UPDATE posts SET title = $2, slug = $3, content = $4, plaintext = $5, "
                "author_id = $6, status = $7, visibility = $8, updated_at = $9, "
                "published_at = $10, scheduled_at = $11, feature_image = $12, "
                "feature_image_alt = $13, feature_image_caption = $14, excerpt = $15, "
                "codeinjection_head = $16, codeinjection_foot = $17, custom_excerpt = $18, "
                "canonical_url = $19, meta_title = $20, meta_description = $21, "
                "og_image = $22, og_title = $23, og_description = $24, "
                "twitter_image = $25, twitter_title = $26, twitter_description = $27, "
                "custom_template = $28, email_only = $29, comment_id = $30, meta = $31 "
                "WHERE id = $1 RETURNING *",
                json{
                    id, post.get_title(), post.get_slug(), post.get_content(),
                    post.get_plaintext(), post.get_author_id(), post.get_status(),
                    post.get_visibility(), post.get_updated_at(), post.get_published_at(),
                    post.get_scheduled_at(), post.get_feature_image(),
                    post.get_feature_image_alt(), post.get_feature_image_caption(),
                    post.get_excerpt(), post.get_codeinjection_head(),
                    post.get_codeinjection_foot(), post.get_custom_excerpt(),
                    post.get_canonical_url(), post.get_meta_title(),
                    post.get_meta_description(), post.get_og_image(), post.get_og_title(),
                    post.get_og_description(), post.get_twitter_image(),
                    post.get_twitter_title(), post.get_twitter_description(),
                    post.get_custom_template(), post.get_email_only(),
                    post.get_comment_id(), post.get_meta()
                }
            );
            return find_by_id(id);
        } catch (const std::exception& e) {
            return std::unexpected(e.what());
        }
    }

    std::expected<ApiResponse<bool>, std::string> delete_by_id(const std::string& id) noexcept {
        if (!m_database || !m_database->is_connected()) {
            return std::unexpected("Database not connected");
        }
        try {
            m_database->execute_update("DELETE FROM posts WHERE id = $1", json{id});
            return ApiResponse<bool>::ok(true);
        } catch (const std::exception& e) {
            return std::unexpected(e.what());
        }
    }

    std::expected<ApiResponse<bool>, std::string> delete_by_uuid(const std::string& uuid) noexcept {
        if (!m_database || !m_database->is_connected()) {
            return std::unexpected("Database not connected");
        }
        try {
            m_database->execute_update("DELETE FROM posts WHERE uuid = $1", json{uuid});
            return ApiResponse<bool>::ok(true);
        } catch (const std::exception& e) {
            return std::unexpected(e.what());
        }
    }

    // Status Operations
    std::expected<ApiResponse<Post>, std::string> publish(const std::string& id) noexcept {
        if (!m_database || !m_database->is_connected()) {
            return std::unexpected("Database not connected");
        }
        try {
            auto now = Post::current_iso8601();
            m_database->execute_update(
                "UPDATE posts SET status = 'published', published_at = $2, updated_at = $2 WHERE id = $1",
                json{id, now}
            );
            return find_by_id(id);
        } catch (const std::exception& e) {
            return std::unexpected(e.what());
        }
    }

    std::expected<ApiResponse<Post>, std::string> unpublish(const std::string& id) noexcept {
        if (!m_database || !m_database->is_connected()) {
            return std::unexpected("Database not connected");
        }
        try {
            auto now = Post::current_iso8601();
            m_database->execute_update(
                "UPDATE posts SET status = 'draft', published_at = NULL, updated_at = $2 WHERE id = $1",
                json{id, now}
            );
            return find_by_id(id);
        } catch (const std::exception& e) {
            return std::unexpected(e.what());
        }
    }

    std::expected<ApiResponse<Post>, std::string> schedule(const std::string& id, const std::string& scheduled_at) noexcept {
        if (!m_database || !m_database->is_connected()) {
            return std::unexpected("Database not connected");
        }
        try {
            auto now = Post::current_iso8601();
            m_database->execute_update(
                "UPDATE posts SET status = 'scheduled', scheduled_at = $2, updated_at = $3 WHERE id = $1",
                json{id, scheduled_at, now}
            );
            return find_by_id(id);
        } catch (const std::exception& e) {
            return std::unexpected(e.what());
        }
    }

    std::expected<ApiResponse<Post>, std::string> archive(const std::string& id) noexcept {
        if (!m_database || !m_database->is_connected()) {
            return std::unexpected("Database not connected");
        }
        try {
            auto now = Post::current_iso8601();
            m_database->execute_update(
                "UPDATE posts SET status = 'archived', updated_at = $2 WHERE id = $1",
                json{id, now}
            );
            return find_by_id(id);
        } catch (const std::exception& e) {
            return std::unexpected(e.what());
        }
    }

    std::expected<ApiResponse<Post>, std::string> unarchive(const std::string& id) noexcept {
        if (!m_database || !m_database->is_connected()) {
            return std::unexpected("Database not connected");
        }
        try {
            auto now = Post::current_iso8601();
            m_database->execute_update(
                "UPDATE posts SET status = 'draft', updated_at = $2 WHERE id = $1",
                json{id, now}
            );
            return find_by_id(id);
        } catch (const std::exception& e) {
            return std::unexpected(e.what());
        }
    }

    // Tag Operations
    std::expected<ApiResponse<bool>, std::string> add_tag(const std::string& post_id, const std::string& tag_id) noexcept {
        if (!m_database || !m_database->is_connected()) {
            return std::unexpected("Database not connected");
        }
        try {
            m_database->execute_update(
                "INSERT INTO post_tags (post_id, tag_id) VALUES ($1, $2) ON CONFLICT DO NOTHING",
                json{post_id, tag_id}
            );
            return ApiResponse<bool>::ok(true);
        } catch (const std::exception& e) {
            return std::unexpected(e.what());
        }
    }

    std::expected<ApiResponse<bool>, std::string> remove_tag(const std::string& post_id, const std::string& tag_id) noexcept {
        if (!m_database || !m_database->is_connected()) {
            return std::unexpected("Database not connected");
        }
        try {
            m_database->execute_update(
                "DELETE FROM post_tags WHERE post_id = $1 AND tag_id = $2",
                json{post_id, tag_id}
            );
            return ApiResponse<bool>::ok(true);
        } catch (const std::exception& e) {
            return std::unexpected(e.what());
        }
    }

    std::expected<ApiResponse<std::vector<Tag>>, std::string> get_tags(const std::string& post_id) noexcept {
        if (!m_database || !m_database->is_connected()) {
            return std::unexpected("Database not connected");
        }
        try {
            auto result = m_database->execute_query(
                "SELECT t.* FROM tags t "
                "JOIN post_tags pt ON t.id = pt.tag_id "
                "WHERE pt.post_id = $1",
                json{post_id}
            );
            std::vector<Tag> tags;
            for (const auto& row : result) {
                auto tag = Tag::from_json(row);
                if (tag) tags.push_back(std::move(*tag));
            }
            return ApiResponse<std::vector<Tag>>::ok(std::move(tags));
        } catch (const std::exception& e) {
            return std::unexpected(e.what());
        }
    }

    std::expected<ApiResponse<bool>, std::string> set_tags(const std::string& post_id, const std::vector<std::string>& tag_ids) noexcept {
        if (!m_database || !m_database->is_connected()) {
            return std::unexpected("Database not connected");
        }
        try {
            m_database->execute_update("DELETE FROM post_tags WHERE post_id = $1", json{post_id});
            for (const auto& tag_id : tag_ids) {
                m_database->execute_update(
                    "INSERT INTO post_tags (post_id, tag_id) VALUES ($1, $2) ON CONFLICT DO NOTHING",
                    json{post_id, tag_id}
                );
            }
            return ApiResponse<bool>::ok(true);
        } catch (const std::exception& e) {
            return std::unexpected(e.what());
        }
    }

    // Author Operations
    std::expected<ApiResponse<std::optional<Author>>, std::string> get_author(const std::string& post_id) noexcept {
        if (!m_database || !m_database->is_connected()) {
            return std::unexpected("Database not connected");
        }
        try {
            auto result = m_database->execute_query(
                "SELECT a.* FROM authors a "
                "JOIN posts p ON a.id = p.author_id "
                "WHERE p.id = $1",
                json{post_id}
            );
            if (result.empty()) {
                return ApiResponse<std::optional<Author>>::not_found("Author");
            }
            auto author = Author::from_json(result[0]);
            if (!author) {
                return std::unexpected(author.error());
            }
            return ApiResponse<std::optional<Author>>::ok(std::move(*author));
        } catch (const std::exception& e) {
            return std::unexpected(e.what());
        }
    }

    std::expected<ApiResponse<bool>, std::string> set_author(const std::string& post_id, const std::string& author_id) noexcept {
        if (!m_database || !m_database->is_connected()) {
            return std::unexpected("Database not connected");
        }
        try {
            m_database->execute_update(
                "UPDATE posts SET author_id = $2, updated_at = $3 WHERE id = $1",
                json{post_id, author_id, Post::current_iso8601()}
            );
            return ApiResponse<bool>::ok(true);
        } catch (const std::exception& e) {
            return std::unexpected(e.what());
        }
    }

    // Search & Filter
    std::expected<ApiResponse<std::vector<Post>>, std::string> search(const std::string& query, const PaginationParams& params) noexcept {
        if (!m_database || !m_database->is_connected()) {
            return std::unexpected("Database not connected");
        }
        try {
            std::string sql = "SELECT * FROM posts WHERE (title ILIKE $1 OR content ILIKE $1 OR plaintext ILIKE $1)";
            json query_params = {"%" + query + "%"};
            sql += " ORDER BY " + build_order_clause(params);
            sql += " " + build_pagination_clause(params);

            auto result = m_database->execute_query(sql, query_params);
            std::vector<Post> posts;
            for (const auto& row : result) {
                auto post = Post::from_json(row);
                if (post) posts.push_back(std::move(*post));
            }
            return ApiResponse<std::vector<Post>>::ok(std::move(posts));
        } catch (const std::exception& e) {
            return std::unexpected(e.what());
        }
    }

    std::expected<ApiResponse<std::vector<Post>>, std::string> find_by_status(const std::string& status, const PaginationParams& params) noexcept {
        PostFilter filter;
        filter.set_status(std::string{status});
        return find_all(params, filter);
    }

    std::expected<ApiResponse<std::vector<Post>>, std::string> find_by_author(const std::string& author_id, const PaginationParams& params) noexcept {
        PostFilter filter;
        filter.set_author_id(std::string{author_id});
        return find_all(params, filter);
    }

    std::expected<ApiResponse<std::vector<Post>>, std::string> find_by_tag(const std::string& tag_id, const PaginationParams& params) noexcept {
        if (!m_database || !m_database->is_connected()) {
            return std::unexpected("Database not connected");
        }
        try {
            std::string sql = "SELECT p.* FROM posts p "
                "JOIN post_tags pt ON p.id = pt.post_id "
                "WHERE pt.tag_id = $1";
            json query_params = {tag_id};
            sql += " ORDER BY " + build_order_clause(params);
            sql += " " + build_pagination_clause(params);

            auto result = m_database->execute_query(sql, query_params);
            std::vector<Post> posts;
            for (const auto& row : result) {
                auto post = Post::from_json(row);
                if (post) posts.push_back(std::move(*post));
            }
            return ApiResponse<std::vector<Post>>::ok(std::move(posts));
        } catch (const std::exception& e) {
            return std::unexpected(e.what());
        }
    }

    std::expected<ApiResponse<std::vector<Post>>, std::string> find_by_visibility(const std::string& visibility, const PaginationParams& params) noexcept {
        PostFilter filter;
        filter.set_visibility(std::string{visibility});
        return find_all(params, filter);
    }

    // Count Operations
    std::expected<ApiResponse<size_t>, std::string> count(const PostFilter& filter = {}) noexcept {
        if (!m_database || !m_database->is_connected()) {
            return std::unexpected("Database not connected");
        }
        try {
            std::string sql = "SELECT COUNT(*) FROM posts";
            json query_params;
            std::string where_clause = build_where_clause(filter, query_params);
            if (!where_clause.empty()) {
                sql += " WHERE " + where_clause;
            }
            auto result = m_database->execute_query(sql, query_params);
            size_t count = result.empty() ? 0 : result[0].value("count", 0);
            return ApiResponse<size_t>::ok(count);
        } catch (const std::exception& e) {
            return std::unexpected(e.what());
        }
    }

    std::expected<ApiResponse<size_t>, std::string> count_by_status(const std::string& status) noexcept {
        PostFilter filter;
        filter.set_status(std::string{status});
        return count(filter);
    }

    std::expected<ApiResponse<size_t>, std::string> count_by_author(const std::string& author_id) noexcept {
        PostFilter filter;
        filter.set_author_id(std::string{author_id});
        return count(filter);
    }

    std::expected<ApiResponse<size_t>, std::string> count_by_tag(const std::string& tag_id) noexcept {
        if (!m_database || !m_database->is_connected()) {
            return std::unexpected("Database not connected");
        }
        try {
            auto result = m_database->execute_query(
                "SELECT COUNT(*) FROM post_tags WHERE tag_id = $1",
                json{tag_id}
            );
            size_t count = result.empty() ? 0 : result[0].value("count", 0);
            return ApiResponse<size_t>::ok(count);
        } catch (const std::exception& e) {
            return std::unexpected(e.what());
        }
    }

    // Stats
    std::expected<ApiResponse<json>, std::string> get_stats() noexcept {
        if (!m_database || !m_database->is_connected()) {
            return std::unexpected("Database not connected");
        }
        try {
            json stats;
            auto total = count();
            if (total) stats["total_posts"] = total->get_data();
            auto published = count_by_status("published");
            if (published) stats["published_posts"] = published->get_data();
            auto drafts = count_by_status("draft");
            if (drafts) stats["draft_posts"] = drafts->get_data();
            auto scheduled = count_by_status("scheduled");
            if (scheduled) stats["scheduled_posts"] = scheduled->get_data();
            return ApiResponse<json>::ok(std::move(stats));
        } catch (const std::exception& e) {
            return std::unexpected(e.what());
        }
    }

private:
    std::shared_ptr<IDatabase> m_database;

    Post map_row_to_post(const json& row) {
        auto post = Post::from_json(row);
        if (!post) throw std::runtime_error(post.error());
        return std::move(*post);
    }

    std::string build_where_clause(const PostFilter& filter, json& params) {
        std::vector<std::string> conditions;
        if (!filter.get_status().empty()) {
            conditions.push_back("status = $" + std::to_string(params.size() + 1));
            params.push_back(filter.get_status());
        }
        if (!filter.get_author_id().empty()) {
            conditions.push_back("author_id = $" + std::to_string(params.size() + 1));
            params.push_back(filter.get_author_id());
        }
        if (!filter.get_tag().empty()) {
            conditions.push_back("id IN (SELECT post_id FROM post_tags WHERE tag_id = $" + std::to_string(params.size() + 1) + ")");
            params.push_back(filter.get_tag());
        }
        if (!filter.get_search().empty()) {
            conditions.push_back("(title ILIKE $" + std::to_string(params.size() + 1) + " OR content ILIKE $" + std::to_string(params.size() + 1) + " OR plaintext ILIKE $" + std::to_string(params.size() + 1) + ")");
            params.push_back("%" + filter.get_search() + "%");
        }
        if (!filter.get_visibility().empty()) {
            conditions.push_back("visibility = $" + std::to_string(params.size() + 1));
            params.push_back(filter.get_visibility());
        }
        if (conditions.empty()) return "";
        return std::accumulate(conditions.begin() + 1, conditions.end(), conditions[0],
            [](const std::string& a, const std::string& b) { return a + " AND " + b; });
    }

    std::string build_order_clause(const PaginationParams& params) {
        std::string order = params.get_order().empty() ? "created_at" : params.get_order();
        std::string direction = params.get_direction().empty() ? "DESC" : params.get_direction();
        return order + " " + direction;
    }

    std::string build_pagination_clause(const PaginationParams& params) {
        int page = params.get_page().empty() ? 1 : std::stoi(params.get_page());
        int limit = params.get_limit().empty() ? 15 : std::stoi(params.get_limit());
        int offset = (page - 1) * limit;
        return "LIMIT " + std::to_string(limit) + " OFFSET " + std::to_string(offset);
    }
};

} // namespace ghost::blog