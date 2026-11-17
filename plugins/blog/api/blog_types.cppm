module;
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <chrono>
#include <optional>
#include <expected>

export module blog_api:types;

import std;

export namespace ghost::blog {

using json = nlohmann::json;

/**
 * Blog Post entity
 */
struct Post {
    // Member initializers
    std::string m_id{};
    std::string m_uuid{};
    std::string m_title{};
    std::string m_slug{};
    std::string m_content{};
    std::string m_plaintext{};
    std::string m_author_id{};
    std::string m_status{};
    std::string m_visibility{};
    std::string m_created_at{};
    std::string m_updated_at{};
    std::string m_published_at{};
    std::string m_scheduled_at{};
    std::string m_feature_image{};
    std::string m_feature_image_alt{};
    std::string m_feature_image_caption{};
    std::string m_excerpt{};
    std::string m_codeinjection_head{};
    std::string m_codeinjection_foot{};
    std::string m_custom_excerpt{};
    std::string m_canonical_url{};
    std::string m_meta_title{};
    std::string m_meta_description{};
    std::string m_og_image{};
    std::string m_og_title{};
    std::string m_og_description{};
    std::string m_twitter_image{};
    std::string m_twitter_title{};
    std::string m_twitter_description{};
    std::string m_custom_template{};
    std::string m_email_only{};
    std::string m_comment_id{};
    json m_meta{};
    std::vector<std::string> m_tags{};
    std::vector<std::string> m_authors{};

    // 1. Default constructor
    Post() noexcept = default;

    // 2. Constructor with only plain types (no optional/vector)
    Post(std::string&& id, std::string&& uuid, std::string&& title, std::string&& slug,
         std::string&& content, std::string&& plaintext, std::string&& author_id,
         std::string&& status, std::string&& visibility, std::string&& created_at,
         std::string&& updated_at, std::string&& published_at, std::string&& scheduled_at,
         std::string&& feature_image, std::string&& feature_image_alt,
         std::string&& feature_image_caption, std::string&& excerpt,
         std::string&& codeinjection_head, std::string&& codeinjection_foot,
         std::string&& custom_excerpt, std::string&& canonical_url,
         std::string&& meta_title, std::string&& meta_description,
         std::string&& og_image, std::string&& og_title, std::string&& og_description,
         std::string&& twitter_image, std::string&& twitter_title,
         std::string&& twitter_description, std::string&& custom_template,
         std::string&& email_only, std::string&& comment_id) noexcept
        : m_id{std::move(id)}, m_uuid{std::move(uuid)}, m_title{std::move(title)},
          m_slug{std::move(slug)}, m_content{std::move(content)},
          m_plaintext{std::move(plaintext)}, m_author_id{std::move(author_id)},
          m_status{std::move(status)}, m_visibility{std::move(visibility)},
          m_created_at{std::move(created_at)}, m_updated_at{std::move(updated_at)},
          m_published_at{std::move(published_at)}, m_scheduled_at{std::move(scheduled_at)},
          m_feature_image{std::move(feature_image)}, m_feature_image_alt{std::move(feature_image_alt)},
          m_feature_image_caption{std::move(feature_image_caption)}, m_excerpt{std::move(excerpt)},
          m_codeinjection_head{std::move(codeinjection_head)}, m_codeinjection_foot{std::move(codeinjection_foot)},
          m_custom_excerpt{std::move(custom_excerpt)}, m_canonical_url{std::move(canonical_url)},
          m_meta_title{std::move(meta_title)}, m_meta_description{std::move(meta_description)},
          m_og_image{std::move(og_image)}, m_og_title{std::move(og_title)},
          m_og_description{std::move(og_description)}, m_twitter_image{std::move(twitter_image)},
          m_twitter_title{std::move(twitter_title)}, m_twitter_description{std::move(twitter_description)},
          m_custom_template{std::move(custom_template)}, m_email_only{std::move(email_only)},
          m_comment_id{std::move(comment_id)} {}

    // 3. Constructor with all fields (rvalues moved in)
    Post(std::string&& id, std::string&& uuid, std::string&& title, std::string&& slug,
         std::string&& content, std::string&& plaintext, std::string&& author_id,
         std::string&& status, std::string&& visibility, std::string&& created_at,
         std::string&& updated_at, std::string&& published_at, std::string&& scheduled_at,
         std::string&& feature_image, std::string&& feature_image_alt,
         std::string&& feature_image_caption, std::string&& excerpt,
         std::string&& codeinjection_head, std::string&& codeinjection_foot,
         std::string&& custom_excerpt, std::string&& canonical_url,
         std::string&& meta_title, std::string&& meta_description,
         std::string&& og_image, std::string&& og_title, std::string&& og_description,
         std::string&& twitter_image, std::string&& twitter_title,
         std::string&& twitter_description, std::string&& custom_template,
         std::string&& email_only, std::string&& comment_id,
         json&& meta, std::vector<std::string>&& tags, std::vector<std::string>&& authors) noexcept
        : m_id{std::move(id)}, m_uuid{std::move(uuid)}, m_title{std::move(title)},
          m_slug{std::move(slug)}, m_content{std::move(content)},
          m_plaintext{std::move(plaintext)}, m_author_id{std::move(author_id)},
          m_status{std::move(status)}, m_visibility{std::move(visibility)},
          m_created_at{std::move(created_at)}, m_updated_at{std::move(updated_at)},
          m_published_at{std::move(published_at)}, m_scheduled_at{std::move(scheduled_at)},
          m_feature_image{std::move(feature_image)}, m_feature_image_alt{std::move(feature_image_alt)},
          m_feature_image_caption{std::move(feature_image_caption)}, m_excerpt{std::move(excerpt)},
          m_codeinjection_head{std::move(codeinjection_head)}, m_codeinjection_foot{std::move(codeinjection_foot)},
          m_custom_excerpt{std::move(custom_excerpt)}, m_canonical_url{std::move(canonical_url)},
          m_meta_title{std::move(meta_title)}, m_meta_description{std::move(meta_description)},
          m_og_image{std::move(og_image)}, m_og_title{std::move(og_title)},
          m_og_description{std::move(og_description)}, m_twitter_image{std::move(twitter_image)},
          m_twitter_title{std::move(twitter_title)}, m_twitter_description{std::move(twitter_description)},
          m_custom_template{std::move(custom_template)}, m_email_only{std::move(email_only)},
          m_comment_id{std::move(comment_id)}, m_meta{std::move(meta)},
          m_tags{std::move(tags)}, m_authors{std::move(authors)} {}

    // 4. Adder functions
    Post& add_tag(std::string&& tag) noexcept {
        m_tags.push_back(std::move(tag));
        return *this;
    }

    Post& add_author(std::string&& author) noexcept {
        m_authors.push_back(std::move(author));
        return *this;
    }

    // 5. Setters (rvalue)
    void set_id(std::string&& id) noexcept { m_id = std::move(id); }
    void set_uuid(std::string&& uuid) noexcept { m_uuid = std::move(uuid); }
    void set_title(std::string&& title) noexcept { m_title = std::move(title); }
    void set_slug(std::string&& slug) noexcept { m_slug = std::move(slug); }
    void set_content(std::string&& content) noexcept { m_content = std::move(content); }
    void set_plaintext(std::string&& plaintext) noexcept { m_plaintext = std::move(plaintext); }
    void set_author_id(std::string&& author_id) noexcept { m_author_id = std::move(author_id); }
    void set_status(std::string&& status) noexcept { m_status = std::move(status); }
    void set_visibility(std::string&& visibility) noexcept { m_visibility = std::move(visibility); }
    void set_created_at(std::string&& created_at) noexcept { m_created_at = std::move(created_at); }
    void set_updated_at(std::string&& updated_at) noexcept { m_updated_at = std::move(updated_at); }
    void set_published_at(std::string&& published_at) noexcept { m_published_at = std::move(published_at); }
    void set_scheduled_at(std::string&& scheduled_at) noexcept { m_scheduled_at = std::move(scheduled_at); }
    void set_feature_image(std::string&& feature_image) noexcept { m_feature_image = std::move(feature_image); }
    void set_feature_image_alt(std::string&& alt) noexcept { m_feature_image_alt = std::move(alt); }
    void set_feature_image_caption(std::string&& caption) noexcept { m_feature_image_caption = std::move(caption); }
    void set_excerpt(std::string&& excerpt) noexcept { m_excerpt = std::move(excerpt); }
    void set_codeinjection_head(std::string&& head) noexcept { m_codeinjection_head = std::move(head); }
    void set_codeinjection_foot(std::string&& foot) noexcept { m_codeinjection_foot = std::move(foot); }
    void set_custom_excerpt(std::string&& excerpt) noexcept { m_custom_excerpt = std::move(excerpt); }
    void set_canonical_url(std::string&& url) noexcept { m_canonical_url = std::move(url); }
    void set_meta_title(std::string&& title) noexcept { m_meta_title = std::move(title); }
    void set_meta_description(std::string&& desc) noexcept { m_meta_description = std::move(desc); }
    void set_og_image(std::string&& image) noexcept { m_og_image = std::move(image); }
    void set_og_title(std::string&& title) noexcept { m_og_title = std::move(title); }
    void set_og_description(std::string&& desc) noexcept { m_og_description = std::move(desc); }
    void set_twitter_image(std::string&& image) noexcept { m_twitter_image = std::move(image); }
    void set_twitter_title(std::string&& title) noexcept { m_twitter_title = std::move(title); }
    void set_twitter_description(std::string&& desc) noexcept { m_twitter_description = std::move(desc); }
    void set_custom_template(std::string&& tmpl) noexcept { m_custom_template = std::move(tmpl); }
    void set_email_only(std::string&& email) noexcept { m_email_only = std::move(email); }
    void set_comment_id(std::string&& id) noexcept { m_comment_id = std::move(id); }
    void set_meta(json&& meta) noexcept { m_meta = std::move(meta); }
    void set_tags(std::vector<std::string>&& tags) noexcept { m_tags = std::move(tags); }
    void set_authors(std::vector<std::string>&& authors) noexcept { m_authors = std::move(authors); }

    // 6. Getters (non-const)
    std::string& get_id() noexcept { return m_id; }
    std::string& get_uuid() noexcept { return m_uuid; }
    std::string& get_title() noexcept { return m_title; }
    std::string& get_slug() noexcept { return m_slug; }
    std::string& get_content() noexcept { return m_content; }
    std::string& get_plaintext() noexcept { return m_plaintext; }
    std::string& get_author_id() noexcept { return m_author_id; }
    std::string& get_status() noexcept { return m_status; }
    std::string& get_visibility() noexcept { return m_visibility; }
    std::string& get_created_at() noexcept { return m_created_at; }
    std::string& get_updated_at() noexcept { return m_updated_at; }
    std::string& get_published_at() noexcept { return m_published_at; }
    std::string& get_scheduled_at() noexcept { return m_scheduled_at; }
    std::string& get_feature_image() noexcept { return m_feature_image; }
    std::string& get_feature_image_alt() noexcept { return m_feature_image_alt; }
    std::string& get_feature_image_caption() noexcept { return m_feature_image_caption; }
    std::string& get_excerpt() noexcept { return m_excerpt; }
    std::string& get_codeinjection_head() noexcept { return m_codeinjection_head; }
    std::string& get_codeinjection_foot() noexcept { return m_codeinjection_foot; }
    std::string& get_custom_excerpt() noexcept { return m_custom_excerpt; }
    std::string& get_canonical_url() noexcept { return m_canonical_url; }
    std::string& get_meta_title() noexcept { return m_meta_title; }
    std::string& get_meta_description() noexcept { return m_meta_description; }
    std::string& get_og_image() noexcept { return m_og_image; }
    std::string& get_og_title() noexcept { return m_og_title; }
    std::string& get_og_description() noexcept { return m_og_description; }
    std::string& get_twitter_image() noexcept { return m_twitter_image; }
    std::string& get_twitter_title() noexcept { return m_twitter_title; }
    std::string& get_twitter_description() noexcept { return m_twitter_description; }
    std::string& get_custom_template() noexcept { return m_custom_template; }
    std::string& get_email_only() noexcept { return m_email_only; }
    std::string& get_comment_id() noexcept { return m_comment_id; }
    json& get_meta() noexcept { return m_meta; }
    std::vector<std::string>& get_tags() noexcept { return m_tags; }
    std::vector<std::string>& get_authors() noexcept { return m_authors; }

    // 7. Const getters
    const std::string& get_id() const noexcept { return m_id; }
    const std::string& get_uuid() const noexcept { return m_uuid; }
    const std::string& get_title() const noexcept { return m_title; }
    const std::string& get_slug() const noexcept { return m_slug; }
    const std::string& get_content() const noexcept { return m_content; }
    const std::string& get_plaintext() const noexcept { return m_plaintext; }
    const std::string& get_author_id() const noexcept { return m_author_id; }
    const std::string& get_status() const noexcept { return m_status; }
    const std::string& get_visibility() const noexcept { return m_visibility; }
    const std::string& get_created_at() const noexcept { return m_created_at; }
    const std::string& get_updated_at() const noexcept { return m_updated_at; }
    const std::string& get_published_at() const noexcept { return m_published_at; }
    const std::string& get_scheduled_at() const noexcept { return m_scheduled_at; }
    const std::string& get_feature_image() const noexcept { return m_feature_image; }
    const std::string& get_feature_image_alt() const noexcept { return m_feature_image_alt; }
    const std::string& get_feature_image_caption() const noexcept { return m_feature_image_caption; }
    const std::string& get_excerpt() const noexcept { return m_excerpt; }
    const std::string& get_codeinjection_head() const noexcept { return m_codeinjection_head; }
    const std::string& get_codeinjection_foot() const noexcept { return m_codeinjection_foot; }
    const std::string& get_custom_excerpt() const noexcept { return m_custom_excerpt; }
    const std::string& get_canonical_url() const noexcept { return m_canonical_url; }
    const std::string& get_meta_title() const noexcept { return m_meta_title; }
    const std::string& get_meta_description() const noexcept { return m_meta_description; }
    const std::string& get_og_image() const noexcept { return m_og_image; }
    const std::string& get_og_title() const noexcept { return m_og_title; }
    const std::string& get_og_description() const noexcept { return m_og_description; }
    const std::string& get_twitter_image() const noexcept { return m_twitter_image; }
    const std::string& get_twitter_title() const noexcept { return m_twitter_title; }
    const std::string& get_twitter_description() const noexcept { return m_twitter_description; }
    const std::string& get_custom_template() const noexcept { return m_custom_template; }
    const std::string& get_email_only() const noexcept { return m_email_only; }
    const std::string& get_comment_id() const noexcept { return m_comment_id; }
    const json& get_meta() const noexcept { return m_meta; }
    const std::vector<std::string>& get_tags() const noexcept { return m_tags; }
    const std::vector<std::string>& get_authors() const noexcept { return m_authors; }

    // JSON serialization (using std::expected for error handling)
    std::expected<json, std::string> to_json() const noexcept {
        try {
            json j;
            j["id"] = m_id;
            j["uuid"] = m_uuid;
            j["title"] = m_title;
            j["slug"] = m_slug;
            j["content"] = m_content;
            j["plaintext"] = m_plaintext;
            j["author_id"] = m_author_id;
            j["status"] = m_status;
            j["visibility"] = m_visibility;
            j["created_at"] = m_created_at;
            j["updated_at"] = m_updated_at;
            j["published_at"] = m_published_at;
            j["scheduled_at"] = m_scheduled_at;
            j["feature_image"] = m_feature_image;
            j["feature_image_alt"] = m_feature_image_alt;
            j["feature_image_caption"] = m_feature_image_caption;
            j["excerpt"] = m_excerpt;
            j["codeinjection_head"] = m_codeinjection_head;
            j["codeinjection_foot"] = m_codeinjection_foot;
            j["custom_excerpt"] = m_custom_excerpt;
            j["canonical_url"] = m_canonical_url;
            j["meta_title"] = m_meta_title;
            j["meta_description"] = m_meta_description;
            j["og_image"] = m_og_image;
            j["og_title"] = m_og_title;
            j["og_description"] = m_og_description;
            j["twitter_image"] = m_twitter_image;
            j["twitter_title"] = m_twitter_title;
            j["twitter_description"] = m_twitter_description;
            j["custom_template"] = m_custom_template;
            j["email_only"] = m_email_only;
            j["comment_id"] = m_comment_id;
            j["meta"] = m_meta;
            j["tags"] = m_tags;
            j["authors"] = m_authors;
            return j;
        } catch (const std::exception& e) {
            return std::unexpected(e.what());
        }
    }

    static std::expected<Post, std::string> from_json(const json& j) noexcept {
        try {
            Post post;
            post.set_id(j.value("id", std::string{}));
            post.set_uuid(j.value("uuid", std::string{}));
            post.set_title(j.value("title", std::string{}));
            post.set_slug(j.value("slug", std::string{}));
            post.set_content(j.value("content", std::string{}));
            post.set_plaintext(j.value("plaintext", std::string{}));
            post.set_author_id(j.value("author_id", std::string{}));
            post.set_status(j.value("status", std::string{}));
            post.set_visibility(j.value("visibility", std::string{}));
            post.set_created_at(j.value("created_at", std::string{}));
            post.set_updated_at(j.value("updated_at", std::string{}));
            post.set_published_at(j.value("published_at", std::string{}));
            post.set_scheduled_at(j.value("scheduled_at", std::string{}));
            post.set_feature_image(j.value("feature_image", std::string{}));
            post.set_feature_image_alt(j.value("feature_image_alt", std::string{}));
            post.set_feature_image_caption(j.value("feature_image_caption", std::string{}));
            post.set_excerpt(j.value("excerpt", std::string{}));
            post.set_codeinjection_head(j.value("codeinjection_head", std::string{}));
            post.set_codeinjection_foot(j.value("codeinjection_foot", std::string{}));
            post.set_custom_excerpt(j.value("custom_excerpt", std::string{}));
            post.set_canonical_url(j.value("canonical_url", std::string{}));
            post.set_meta_title(j.value("meta_title", std::string{}));
            post.set_meta_description(j.value("meta_description", std::string{}));
            post.set_og_image(j.value("og_image", std::string{}));
            post.set_og_title(j.value("og_title", std::string{}));
            post.set_og_description(j.value("og_description", std::string{}));
            post.set_twitter_image(j.value("twitter_image", std::string{}));
            post.set_twitter_title(j.value("twitter_title", std::string{}));
            post.set_twitter_description(j.value("twitter_description", std::string{}));
            post.set_custom_template(j.value("custom_template", std::string{}));
            post.set_email_only(j.value("email_only", std::string{}));
            post.set_comment_id(j.value("comment_id", std::string{}));
            if (j.contains("meta")) post.set_meta(j["meta"]);
            if (j.contains("tags")) post.set_tags(j["tags"].get<std::vector<std::string>>());
            if (j.contains("authors")) post.set_authors(j["authors"].get<std::vector<std::string>>());
            return post;
        } catch (const std::exception& e) {
            return std::unexpected(e.what());
        }
    }

    static Post create_new(std::string&& title, std::string&& author_id) noexcept {
        Post post;
        post.set_id(generate_uuid());
        post.set_uuid(generate_uuid());
        post.set_title(std::move(title));
        post.set_slug(generate_slug(post.get_title()));
        post.set_author_id(std::move(author_id));
        post.set_status("draft");
        post.set_visibility("public");
        post.set_created_at(current_iso8601());
        post.set_updated_at(post.get_created_at());
        return post;
    }

private:
    static std::string generate_uuid() noexcept {
        // Simple UUID v4 generation
        std::string uuid(36, '0');
        static const char* hex = "0123456789abcdef";
        for (int i = 0; i < 36; ++i) {
            if (i == 8 || i == 13 || i == 18 || i == 23) {
                uuid[i] = '-';
            } else if (i == 14) {
                uuid[i] = '4';
            } else {
                uuid[i] = hex[std::rand() % 16];
            }
        }
        uuid[19] = hex[(std::rand() % 4) + 8];
        return uuid;
    }

    static std::string generate_slug(const std::string& title) noexcept {
        std::string slug;
        slug.reserve(title.size());
        for (char c : title) {
            if (std::isalnum(c)) {
                slug += std::tolower(c);
            } else if (c == ' ') {
                slug += '-';
            }
        }
        // Remove consecutive dashes
        std::string result;
        bool last_was_dash = false;
        for (char c : slug) {
            if (c == '-') {
                if (!last_was_dash) {
                    result += c;
                    last_was_dash = true;
                }
            } else {
                result += c;
                last_was_dash = false;
            }
        }
        // Trim dashes
        if (!result.empty() && result.front() == '-') result.erase(0, 1);
        if (!result.empty() && result.back() == '-') result.pop_back();
        return result.empty() ? "untitled" : result;
    }

    static std::string current_iso8601() noexcept {
        auto now = std::chrono::system_clock::now();
        auto tt = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::gmtime(&tt), "%Y-%m-%dT%H:%M:%SZ");
        return ss.str();
    }
};

/**
 * Pagination parameters
 */
struct PaginationParams {
    std::string m_page{};
    std::string m_limit{};
    std::string m_order{};
    std::string m_direction{};

    PaginationParams() noexcept = default;

    // Plain types constructor
    PaginationParams(std::string&& page, std::string&& limit,
                     std::string&& order, std::string&& direction) noexcept
        : m_page{std::move(page)}, m_limit{std::move(limit)},
          m_order{std::move(order)}, m_direction{std::move(direction)} {}

    // All fields constructor
    PaginationParams(std::string&& page, std::string&& limit,
                     std::string&& order, std::string&& direction,
                     int page_val, int limit_val) noexcept
        : m_page{std::move(page)}, m_limit{std::move(limit)},
          m_order{std::move(order)}, m_direction{std::move(direction)} {}

    PaginationParams& add_page(std::string&& page) noexcept {
        m_page = std::move(page);
        return *this;
    }

    PaginationParams& add_limit(std::string&& limit) noexcept {
        m_limit = std::move(limit);
        return *this;
    }

    PaginationParams& add_order(std::string&& order) noexcept {
        m_order = std::move(order);
        return *this;
    }

    PaginationParams& add_direction(std::string&& direction) noexcept {
        m_direction = std::move(direction);
        return *this;
    }

    void set_page(std::string&& page) noexcept { m_page = std::move(page); }
    void set_limit(std::string&& limit) noexcept { m_limit = std::move(limit); }
    void set_order(std::string&& order) noexcept { m_order = std::move(order); }
    void set_direction(std::string&& direction) noexcept { m_direction = std::move(direction); }

    std::string& get_page() noexcept { return m_page; }
    std::string& get_limit() noexcept { return m_limit; }
    std::string& get_order() noexcept { return m_order; }
    std::string& get_direction() noexcept { return m_direction; }

    const std::string& get_page() const noexcept { return m_page; }
    const std::string& get_limit() const noexcept { return m_limit; }
    const std::string& get_order() const noexcept { return m_order; }
    const std::string& get_direction() const noexcept { return m_direction; }

    std::expected<bool, std::string> validate(std::string& error) const noexcept {
        try {
            int page = m_page.empty() ? 1 : std::stoi(m_page);
            int limit = m_limit.empty() ? 15 : std::stoi(m_limit);
            if (page < 1) {
                error = "Page must be >= 1";
                return false;
            }
            if (limit < 1 || limit > 100) {
                error = "Limit must be between 1 and 100";
                return false;
            }
            return true;
        } catch (const std::exception& e) {
            error = e.what();
            return false;
        }
    }
};

/**
 * Filter parameters for posts
 */
struct PostFilter {
    std::string m_status{};
    std::string m_author_id{};
    std::string m_tag{};
    std::string m_search{};
    std::string m_visibility{};

    PostFilter() noexcept = default;

    PostFilter(std::string&& status, std::string&& author_id,
               std::string&& tag, std::string&& search,
               std::string&& visibility) noexcept
        : m_status{std::move(status)}, m_author_id{std::move(author_id)},
          m_tag{std::move(tag)}, m_search{std::move(search)},
          m_visibility{std::move(visibility)} {}

    PostFilter& add_status(std::string&& status) noexcept {
        m_status = std::move(status);
        return *this;
    }

    PostFilter& add_author_id(std::string&& author_id) noexcept {
        m_author_id = std::move(author_id);
        return *this;
    }

    PostFilter& add_tag(std::string&& tag) noexcept {
        m_tag = std::move(tag);
        return *this;
    }

    PostFilter& add_search(std::string&& search) noexcept {
        m_search = std::move(search);
        return *this;
    }

    PostFilter& add_visibility(std::string&& visibility) noexcept {
        m_visibility = std::move(visibility);
        return *this;
    }

    void set_status(std::string&& status) noexcept { m_status = std::move(status); }
    void set_author_id(std::string&& author_id) noexcept { m_author_id = std::move(author_id); }
    void set_tag(std::string&& tag) noexcept { m_tag = std::move(tag); }
    void set_search(std::string&& search) noexcept { m_search = std::move(search); }
    void set_visibility(std::string&& visibility) noexcept { m_visibility = std::move(visibility); }

    std::string& get_status() noexcept { return m_status; }
    std::string& get_author_id() noexcept { return m_author_id; }
    std::string& get_tag() noexcept { return m_tag; }
    std::string& get_search() noexcept { return m_search; }
    std::string& get_visibility() noexcept { return m_visibility; }

    const std::string& get_status() const noexcept { return m_status; }
    const std::string& get_author_id() const noexcept { return m_author_id; }
    const std::string& get_tag() const noexcept { return m_tag; }
    const std::string& get_search() const noexcept { return m_search; }
    const std::string& get_visibility() const noexcept { return m_visibility; }

    bool is_empty() const noexcept {
        return m_status.empty() && m_author_id.empty() && m_tag.empty() &&
               m_search.empty() && m_visibility.empty();
    }
};

/**
 * Tag entity
 */
struct Tag {
    std::string m_id{};
    std::string m_name{};
    std::string m_slug{};
    std::string m_description{};
    std::string m_feature_image{};
    std::string m_visibility{};
    std::string m_meta_title{};
    std::string m_meta_description{};
    std::string m_og_image{};
    std::string m_og_title{};
    std::string m_og_description{};
    std::string m_twitter_image{};
    std::string m_twitter_title{};
    std::string m_twitter_description{};
    std::string m_created_at{};
    std::string m_updated_at{};
    int m_post_count = 0;

    Tag() noexcept = default;

    static std::string generate_uuid() noexcept {
        std::string uuid(36, '0');
        static const char* hex = "0123456789abcdef";
        for (int i = 0; i < 36; ++i) {
            if (i == 8 || i == 13 || i == 18 || i == 23) {
                uuid[i] = '-';
            } else if (i == 14) {
                uuid[i] = '4';
            } else {
                uuid[i] = hex[std::rand() % 16];
            }
        }
        uuid[19] = hex[(std::rand() % 4) + 8];
        return uuid;
    }

    Tag(std::string&& id, std::string&& name, std::string&& slug,
        std::string&& description, std::string&& feature_image,
        std::string&& visibility, std::string&& meta_title,
        std::string&& meta_description, std::string&& og_image,
        std::string&& og_title, std::string&& og_description,
        std::string&& twitter_image, std::string&& twitter_title,
        std::string&& twitter_description, std::string&& created_at,
        std::string&& updated_at, int post_count) noexcept
        : m_id{std::move(id)}, m_name{std::move(name)}, m_slug{std::move(slug)},
          m_description{std::move(description)}, m_feature_image{std::move(feature_image)},
          m_visibility{std::move(visibility)}, m_meta_title{std::move(meta_title)},
          m_meta_description{std::move(meta_description)}, m_og_image{std::move(og_image)},
          m_og_title{std::move(og_title)}, m_og_description{std::move(og_description)},
          m_twitter_image{std::move(twitter_image)}, m_twitter_title{std::move(twitter_title)},
          m_twitter_description{std::move(twitter_description)}, m_created_at{std::move(created_at)},
          m_updated_at{std::move(updated_at)}, m_post_count{post_count} {}

    Tag& add_name(std::string&& name) noexcept { m_name = std::move(name); return *this; }
    Tag& add_slug(std::string&& slug) noexcept { m_slug = std::move(slug); return *this; }
    Tag& add_description(std::string&& desc) noexcept { m_description = std::move(desc); return *this; }

    void set_id(std::string&& id) noexcept { m_id = std::move(id); }
    void set_name(std::string&& name) noexcept { m_name = std::move(name); }
    void set_slug(std::string&& slug) noexcept { m_slug = std::move(slug); }
    void set_description(std::string&& desc) noexcept { m_description = std::move(desc); }
    void set_feature_image(std::string&& img) noexcept { m_feature_image = std::move(img); }
    void set_visibility(std::string&& vis) noexcept { m_visibility = std::move(vis); }
    void set_meta_title(std::string&& title) noexcept { m_meta_title = std::move(title); }
    void set_meta_description(std::string&& desc) noexcept { m_meta_description = std::move(desc); }
    void set_og_image(std::string&& img) noexcept { m_og_image = std::move(img); }
    void set_og_title(std::string&& title) noexcept { m_og_title = std::move(title); }
    void set_og_description(std::string&& desc) noexcept { m_og_description = std::move(desc); }
    void set_twitter_image(std::string&& img) noexcept { m_twitter_image = std::move(img); }
    void set_twitter_title(std::string&& title) noexcept { m_twitter_title = std::move(title); }
    void set_twitter_description(std::string&& desc) noexcept { m_twitter_description = std::move(desc); }
    void set_created_at(std::string&& created) noexcept { m_created_at = std::move(created); }
    void set_updated_at(std::string&& updated) noexcept { m_updated_at = std::move(updated); }
    void set_post_count(int count) noexcept { m_post_count = count; }

    std::string& get_id() noexcept { return m_id; }
    std::string& get_name() noexcept { return m_name; }
    std::string& get_slug() noexcept { return m_slug; }
    std::string& get_description() noexcept { return m_description; }
    std::string& get_feature_image() noexcept { return m_feature_image; }
    std::string& get_visibility() noexcept { return m_visibility; }
    std::string& get_meta_title() noexcept { return m_meta_title; }
    std::string& get_meta_description() noexcept { return m_meta_description; }
    std::string& get_og_image() noexcept { return m_og_image; }
    std::string& get_og_title() noexcept { return m_og_title; }
    std::string& get_og_description() noexcept { return m_og_description; }
    std::string& get_twitter_image() noexcept { return m_twitter_image; }
    std::string& get_twitter_title() noexcept { return m_twitter_title; }
    std::string& get_twitter_description() noexcept { return m_twitter_description; }
    std::string& get_created_at() noexcept { return m_created_at; }
    std::string& get_updated_at() noexcept { return m_updated_at; }
    int& get_post_count() noexcept { return m_post_count; }

    const std::string& get_id() const noexcept { return m_id; }
    const std::string& get_name() const noexcept { return m_name; }
    const std::string& get_slug() const noexcept { return m_slug; }
    const std::string& get_description() const noexcept { return m_description; }
    const std::string& get_feature_image() const noexcept { return m_feature_image; }
    const std::string& get_visibility() const noexcept { return m_visibility; }
    const std::string& get_meta_title() const noexcept { return m_meta_title; }
    const std::string& get_meta_description() const noexcept { return m_meta_description; }
    const std::string& get_og_image() const noexcept { return m_og_image; }
    const std::string& get_og_title() const noexcept { return m_og_title; }
    const std::string& get_og_description() const noexcept { return m_og_description; }
    const std::string& get_twitter_image() const noexcept { return m_twitter_image; }
    const std::string& get_twitter_title() const noexcept { return m_twitter_title; }
    const std::string& get_twitter_description() const noexcept { return m_twitter_description; }
    const std::string& get_created_at() const noexcept { return m_created_at; }
    const std::string& get_updated_at() const noexcept { return m_updated_at; }
    int get_post_count() const noexcept { return m_post_count; }

    std::expected<json, std::string> to_json() const noexcept {
        try {
            json j;
            j["id"] = m_id;
            j["name"] = m_name;
            j["slug"] = m_slug;
            j["description"] = m_description;
            j["feature_image"] = m_feature_image;
            j["visibility"] = m_visibility;
            j["meta_title"] = m_meta_title;
            j["meta_description"] = m_meta_description;
            j["og_image"] = m_og_image;
            j["og_title"] = m_og_title;
            j["og_description"] = m_og_description;
            j["twitter_image"] = m_twitter_image;
            j["twitter_title"] = m_twitter_title;
            j["twitter_description"] = m_twitter_description;
            j["created_at"] = m_created_at;
            j["updated_at"] = m_updated_at;
            j["post_count"] = m_post_count;
            return j;
        } catch (const std::exception& e) {
            return std::unexpected(e.what());
        }
    }

    static std::expected<Tag, std::string> from_json(const json& j) noexcept {
        try {
            Tag tag;
            tag.set_id(j.value("id", std::string{}));
            tag.set_name(j.value("name", std::string{}));
            tag.set_slug(j.value("slug", std::string{}));
            tag.set_description(j.value("description", std::string{}));
            tag.set_feature_image(j.value("feature_image", std::string{}));
            tag.set_visibility(j.value("visibility", std::string{}));
            tag.set_meta_title(j.value("meta_title", std::string{}));
            tag.set_meta_description(j.value("meta_description", std::string{}));
            tag.set_og_image(j.value("og_image", std::string{}));
            tag.set_og_title(j.value("og_title", std::string{}));
            tag.set_og_description(j.value("og_description", std::string{}));
            tag.set_twitter_image(j.value("twitter_image", std::string{}));
            tag.set_twitter_title(j.value("twitter_title", std::string{}));
            tag.set_twitter_description(j.value("twitter_description", std::string{}));
            tag.set_created_at(j.value("created_at", std::string{}));
            tag.set_updated_at(j.value("updated_at", std::string{}));
            tag.set_post_count(j.value("post_count", 0));
            return tag;
        } catch (const std::exception& e) {
            return std::unexpected(e.what());
        }
    }
};

/**
 * Author entity
 */
struct Author {
    std::string m_id{};
    std::string m_name{};
    std::string m_slug{};
    std::string m_email{};
    std::string m_profile_image{};
    std::string m_cover_image{};
    std::string m_bio{};
    std::string m_website{};
    std::string m_location{};
    std::string m_accessibility{};
    std::string m_status{};
    std::string m_last_login{};
    std::string m_created_at{};
    std::string m_updated_at{};
    json m_meta{};
    int m_post_count = 0;

    Author() noexcept = default;

    static std::string generate_uuid() noexcept {
        std::string uuid(36, '0');
        static const char* hex = "0123456789abcdef";
        for (int i = 0; i < 36; ++i) {
            if (i == 8 || i == 13 || i == 18 || i == 23) {
                uuid[i] = '-';
            } else if (i == 14) {
                uuid[i] = '4';
            } else {
                uuid[i] = hex[std::rand() % 16];
            }
        }
        uuid[19] = hex[(std::rand() % 4) + 8];
        return uuid;
    }

    Author(std::string&& id, std::string&& name, std::string&& slug,
           std::string&& email, std::string&& profile_image,
           std::string&& cover_image, std::string&& bio,
           std::string&& website, std::string&& location,
           std::string&& accessibility, std::string&& status,
           std::string&& last_login, std::string&& created_at,
           std::string&& updated_at, json&& meta, int post_count) noexcept
        : m_id{std::move(id)}, m_name{std::move(name)}, m_slug{std::move(slug)},
          m_email{std::move(email)}, m_profile_image{std::move(profile_image)},
          m_cover_image{std::move(cover_image)}, m_bio{std::move(bio)},
          m_website{std::move(website)}, m_location{std::move(location)},
          m_accessibility{std::move(accessibility)}, m_status{std::move(status)},
          m_last_login{std::move(last_login)}, m_created_at{std::move(created_at)},
          m_updated_at{std::move(updated_at)}, m_meta{std::move(meta)},
          m_post_count{post_count} {}

    Author& add_name(std::string&& name) noexcept { m_name = std::move(name); return *this; }
    Author& add_email(std::string&& email) noexcept { m_email = std::move(email); return *this; }
    Author& add_slug(std::string&& slug) noexcept { m_slug = std::move(slug); return *this; }

    void set_id(std::string&& id) noexcept { m_id = std::move(id); }
    void set_name(std::string&& name) noexcept { m_name = std::move(name); }
    void set_slug(std::string&& slug) noexcept { m_slug = std::move(slug); }
    void set_email(std::string&& email) noexcept { m_email = std::move(email); }
    void set_profile_image(std::string&& img) noexcept { m_profile_image = std::move(img); }
    void set_cover_image(std::string&& img) noexcept { m_cover_image = std::move(img); }
    void set_bio(std::string&& bio) noexcept { m_bio = std::move(bio); }
    void set_website(std::string&& site) noexcept { m_website = std::move(site); }
    void set_location(std::string&& loc) noexcept { m_location = std::move(loc); }
    void set_accessibility(std::string&& acc) noexcept { m_accessibility = std::move(acc); }
    void set_status(std::string&& status) noexcept { m_status = std::move(status); }
    void set_last_login(std::string&& login) noexcept { m_last_login = std::move(login); }
    void set_created_at(std::string&& created) noexcept { m_created_at = std::move(created); }
    void set_updated_at(std::string&& updated) noexcept { m_updated_at = std::move(updated); }
    void set_meta(json&& meta) noexcept { m_meta = std::move(meta); }
    void set_post_count(int count) noexcept { m_post_count = count; }

    std::string& get_id() noexcept { return m_id; }
    std::string& get_name() noexcept { return m_name; }
    std::string& get_slug() noexcept { return m_slug; }
    std::string& get_email() noexcept { return m_email; }
    std::string& get_profile_image() noexcept { return m_profile_image; }
    std::string& get_cover_image() noexcept { return m_cover_image; }
    std::string& get_bio() noexcept { return m_bio; }
    std::string& get_website() noexcept { return m_website; }
    std::string& get_location() noexcept { return m_location; }
    std::string& get_accessibility() noexcept { return m_accessibility; }
    std::string& get_status() noexcept { return m_status; }
    std::string& get_last_login() noexcept { return m_last_login; }
    std::string& get_created_at() noexcept { return m_created_at; }
    std::string& get_updated_at() noexcept { return m_updated_at; }
    json& get_meta() noexcept { return m_meta; }
    int& get_post_count() noexcept { return m_post_count; }

    const std::string& get_id() const noexcept { return m_id; }
    const std::string& get_name() const noexcept { return m_name; }
    const std::string& get_slug() const noexcept { return m_slug; }
    const std::string& get_email() const noexcept { return m_email; }
    const std::string& get_profile_image() const noexcept { return m_profile_image; }
    const std::string& get_cover_image() const noexcept { return m_cover_image; }
    const std::string& get_bio() const noexcept { return m_bio; }
    const std::string& get_website() const noexcept { return m_website; }
    const std::string& get_location() const noexcept { return m_location; }
    const std::string& get_accessibility() const noexcept { return m_accessibility; }
    const std::string& get_status() const noexcept { return m_status; }
    const std::string& get_last_login() const noexcept { return m_last_login; }
    const std::string& get_created_at() const noexcept { return m_created_at; }
    const std::string& get_updated_at() const noexcept { return m_updated_at; }
    const json& get_meta() const noexcept { return m_meta; }
    int get_post_count() const noexcept { return m_post_count; }

    std::expected<json, std::string> to_json() const noexcept {
        try {
            json j;
            j["id"] = m_id;
            j["name"] = m_name;
            j["slug"] = m_slug;
            j["email"] = m_email;
            j["profile_image"] = m_profile_image;
            j["cover_image"] = m_cover_image;
            j["bio"] = m_bio;
            j["website"] = m_website;
            j["location"] = m_location;
            j["accessibility"] = m_accessibility;
            j["status"] = m_status;
            j["last_login"] = m_last_login;
            j["created_at"] = m_created_at;
            j["updated_at"] = m_updated_at;
            j["meta"] = m_meta;
            j["post_count"] = m_post_count;
            return j;
        } catch (const std::exception& e) {
            return std::unexpected(e.what());
        }
    }

    static std::expected<Author, std::string> from_json(const json& j) noexcept {
        try {
            Author author;
            author.set_id(j.value("id", std::string{}));
            author.set_name(j.value("name", std::string{}));
            author.set_slug(j.value("slug", std::string{}));
            author.set_email(j.value("email", std::string{}));
            author.set_profile_image(j.value("profile_image", std::string{}));
            author.set_cover_image(j.value("cover_image", std::string{}));
            author.set_bio(j.value("bio", std::string{}));
            author.set_website(j.value("website", std::string{}));
            author.set_location(j.value("location", std::string{}));
            author.set_accessibility(j.value("accessibility", std::string{}));
            author.set_status(j.value("status", std::string{}));
            author.set_last_login(j.value("last_login", std::string{}));
            author.set_created_at(j.value("created_at", std::string{}));
            author.set_updated_at(j.value("updated_at", std::string{}));
            if (j.contains("meta")) author.set_meta(j["meta"]);
            author.set_post_count(j.value("post_count", 0));
            return author;
        } catch (const std::exception& e) {
            return std::unexpected(e.what());
        }
    }
};

/**
 * API Response wrapper
 */
template<typename T>
struct ApiResponse {
    bool m_success = true;
    T m_data{};
    json m_meta{};
    std::string m_error{};
    int m_status_code = 200;

    ApiResponse() noexcept = default;

    ApiResponse(bool&& success, T&& data, json&& meta,
                std::string&& error, int status_code) noexcept
        : m_success{std::move(success)}, m_data{std::move(data)},
          m_meta{std::move(meta)}, m_error{std::move(error)},
          m_status_code{status_code} {}

    ApiResponse& add_success(bool&& success) noexcept {
        m_success = std::move(success);
        return *this;
    }

    ApiResponse& add_data(T&& data) noexcept {
        m_data = std::move(data);
        return *this;
    }

    ApiResponse& add_meta(json&& meta) noexcept {
        m_meta = std::move(meta);
        return *this;
    }

    ApiResponse& add_error(std::string&& error) noexcept {
        m_error = std::move(error);
        return *this;
    }

    void set_success(bool success) noexcept { m_success = success; }
    void set_data(T&& data) noexcept { m_data = std::move(data); }
    void set_meta(json&& meta) noexcept { m_meta = std::move(meta); }
    void set_error(std::string&& error) noexcept { m_error = std::move(error); }
    void set_status_code(int code) noexcept { m_status_code = code; }

    bool& get_success() noexcept { return m_success; }
    T& get_data() noexcept { return m_data; }
    json& get_meta() noexcept { return m_meta; }
    std::string& get_error() noexcept { return m_error; }
    int& get_status_code() noexcept { return m_status_code; }

    const bool& get_success() const noexcept { return m_success; }
    const T& get_data() const noexcept { return m_data; }
    const json& get_meta() const noexcept { return m_meta; }
    const std::string& get_error() const noexcept { return m_error; }
    int get_status_code() const noexcept { return m_status_code; }

    std::expected<json, std::string> to_json() const noexcept {
        try {
            json j;
            j["success"] = m_success;
            if constexpr (!std::is_same_v<T, void>) {
                j["data"] = m_data;
            }
            j["meta"] = m_meta;
            if (!m_error.empty()) {
                j["error"] = m_error;
            }
            j["status_code"] = m_status_code;
            return j;
        } catch (const std::exception& e) {
            return std::unexpected(e.what());
        }
    }

    static ApiResponse<T> ok(T&& data, json&& meta = {}) noexcept {
        ApiResponse<T> resp;
        resp.set_success(true);
        resp.set_data(std::move(data));
        resp.set_meta(std::move(meta));
        resp.set_status_code(200);
        return resp;
    }

    static ApiResponse<T> created(T&& data) noexcept {
        ApiResponse<T> resp;
        resp.set_success(true);
        resp.set_data(std::move(data));
        resp.set_status_code(201);
        return resp;
    }

    static ApiResponse<T> error(std::string&& message, int code = 400) noexcept {
        ApiResponse<T> resp;
        resp.set_success(false);
        resp.set_error(std::move(message));
        resp.set_status_code(code);
        return resp;
    }

    static ApiResponse<T> not_found(std::string&& resource = "Resource") noexcept {
        ApiResponse<T> resp;
        resp.set_success(false);
        resp.set_error(resource + " not found");
        resp.set_status_code(404);
        return resp;
    }
};

} // namespace ghost::blog