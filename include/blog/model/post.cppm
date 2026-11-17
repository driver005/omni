module;
#include "entity_macros.h"

export module blog_model:post;

import std;
import core_serde;
import core_database;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::model {

/** @brief Ghost posts entity — full field set from data/schema/schema.js. */
class Post
{
public:
    Post() = default;

    void set_id(std::int64_t value) { m_id = std::move(value); }
    [[nodiscard]] const std::int64_t& get_id() const noexcept { return m_id; }
    void set_uuid(std::string value) { m_uuid = std::move(value); }
    [[nodiscard]] const std::string& get_uuid() const noexcept { return m_uuid; }
    void set_title(std::string value) { m_title = std::move(value); }
    [[nodiscard]] const std::string& get_title() const noexcept { return m_title; }
    void set_slug(std::string value) { m_slug = std::move(value); }
    [[nodiscard]] const std::string& get_slug() const noexcept { return m_slug; }
    void set_mobiledoc(std::string value) { m_mobiledoc = std::move(value); }
    [[nodiscard]] const std::string& get_mobiledoc() const noexcept { return m_mobiledoc; }
    void set_lexical(std::string value) { m_lexical = std::move(value); }
    [[nodiscard]] const std::string& get_lexical() const noexcept { return m_lexical; }
    void set_html(std::string value) { m_html = std::move(value); }
    [[nodiscard]] const std::string& get_html() const noexcept { return m_html; }
    void set_comment_id(std::string value) { m_comment_id = std::move(value); }
    [[nodiscard]] const std::string& get_comment_id() const noexcept { return m_comment_id; }
    void set_plaintext(std::string value) { m_plaintext = std::move(value); }
    [[nodiscard]] const std::string& get_plaintext() const noexcept { return m_plaintext; }
    void set_feature_image(std::string value) { m_feature_image = std::move(value); }
    [[nodiscard]] const std::string& get_feature_image() const noexcept { return m_feature_image; }
    void set_featured(bool value) { m_featured = std::move(value); }
    [[nodiscard]] const bool& get_featured() const noexcept { return m_featured; }
    void set_type(std::string value) { m_type = std::move(value); }
    [[nodiscard]] const std::string& get_type() const noexcept { return m_type; }
    void set_status(std::string value) { m_status = std::move(value); }
    [[nodiscard]] const std::string& get_status() const noexcept { return m_status; }
    void set_locale(std::string value) { m_locale = std::move(value); }
    [[nodiscard]] const std::string& get_locale() const noexcept { return m_locale; }
    void set_visibility(std::string value) { m_visibility = std::move(value); }
    [[nodiscard]] const std::string& get_visibility() const noexcept { return m_visibility; }
    void set_email_recipient_filter(std::string value) { m_email_recipient_filter = std::move(value); }
    [[nodiscard]] const std::string& get_email_recipient_filter() const noexcept { return m_email_recipient_filter; }
    void set_created_at(std::chrono::system_clock::time_point value) { m_created_at = std::move(value); }
    [[nodiscard]] const std::chrono::system_clock::time_point& get_created_at() const noexcept { return m_created_at; }
    void set_updated_at(std::chrono::system_clock::time_point value) { m_updated_at = std::move(value); }
    [[nodiscard]] const std::chrono::system_clock::time_point& get_updated_at() const noexcept { return m_updated_at; }
    void set_published_at(std::chrono::system_clock::time_point value) { m_published_at = std::move(value); }
    [[nodiscard]] const std::chrono::system_clock::time_point& get_published_at() const noexcept { return m_published_at; }
    void set_published_by(std::string value) { m_published_by = std::move(value); }
    [[nodiscard]] const std::string& get_published_by() const noexcept { return m_published_by; }
    void set_custom_excerpt(std::string value) { m_custom_excerpt = std::move(value); }
    [[nodiscard]] const std::string& get_custom_excerpt() const noexcept { return m_custom_excerpt; }
    void set_auto_excerpt(std::string value) { m_auto_excerpt = std::move(value); }
    [[nodiscard]] const std::string& get_auto_excerpt() const noexcept { return m_auto_excerpt; }
    void set_reading_time(std::int64_t value) { m_reading_time = std::move(value); }
    [[nodiscard]] const std::int64_t& get_reading_time() const noexcept { return m_reading_time; }
    void set_codeinjection_head(std::string value) { m_codeinjection_head = std::move(value); }
    [[nodiscard]] const std::string& get_codeinjection_head() const noexcept { return m_codeinjection_head; }
    void set_codeinjection_foot(std::string value) { m_codeinjection_foot = std::move(value); }
    [[nodiscard]] const std::string& get_codeinjection_foot() const noexcept { return m_codeinjection_foot; }
    void set_custom_template(std::string value) { m_custom_template = std::move(value); }
    [[nodiscard]] const std::string& get_custom_template() const noexcept { return m_custom_template; }
    void set_canonical_url(std::string value) { m_canonical_url = std::move(value); }
    [[nodiscard]] const std::string& get_canonical_url() const noexcept { return m_canonical_url; }
    void set_newsletter_id(std::string value) { m_newsletter_id = std::move(value); }
    [[nodiscard]] const std::string& get_newsletter_id() const noexcept { return m_newsletter_id; }
    void set_show_title_and_feature_image(bool value) { m_show_title_and_feature_image = std::move(value); }
    [[nodiscard]] const bool& get_show_title_and_feature_image() const noexcept { return m_show_title_and_feature_image; }

    std::int64_t m_id{};
    std::string m_uuid{};
    std::string m_title{};
    std::string m_slug{};
    std::string m_mobiledoc{};
    std::string m_lexical{};
    std::string m_html{};
    std::string m_comment_id{};
    std::string m_plaintext{};
    std::string m_feature_image{};
    bool m_featured{};
    std::string m_type{};
    std::string m_status{};
    std::string m_locale{};
    std::string m_visibility{};
    std::string m_email_recipient_filter{};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
    std::chrono::system_clock::time_point m_published_at{};
    std::string m_published_by{};
    std::string m_custom_excerpt{};
    std::string m_auto_excerpt{};
    std::int64_t m_reading_time{};
    std::string m_codeinjection_head{};
    std::string m_codeinjection_foot{};
    std::string m_custom_template{};
    std::string m_canonical_url{};
    std::string m_newsletter_id{};
    bool m_show_title_and_feature_image{};
};




} // namespace blog::model

namespace serde {
template<>
struct Serializable<blog::model::Post>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"id", &blog::model::Post::get_id, &blog::model::Post::set_id, serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<"uuid", &blog::model::Post::get_uuid, &blog::model::Post::set_uuid, serde::FieldOptions{}>{},
            serde::FieldDesc<"title", &blog::model::Post::get_title, &blog::model::Post::set_title, serde::FieldOptions{}>{},
            serde::FieldDesc<"slug", &blog::model::Post::get_slug, &blog::model::Post::set_slug, serde::FieldOptions{}>{},
            serde::FieldDesc<"mobiledoc", &blog::model::Post::get_mobiledoc, &blog::model::Post::set_mobiledoc, serde::FieldOptions{}>{},
            serde::FieldDesc<"lexical", &blog::model::Post::get_lexical, &blog::model::Post::set_lexical, serde::FieldOptions{}>{},
            serde::FieldDesc<"html", &blog::model::Post::get_html, &blog::model::Post::set_html, serde::FieldOptions{}>{},
            serde::FieldDesc<"comment_id", &blog::model::Post::get_comment_id, &blog::model::Post::set_comment_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"plaintext", &blog::model::Post::get_plaintext, &blog::model::Post::set_plaintext, serde::FieldOptions{}>{},
            serde::FieldDesc<"feature_image", &blog::model::Post::get_feature_image, &blog::model::Post::set_feature_image, serde::FieldOptions{}>{},
            serde::FieldDesc<"featured", &blog::model::Post::get_featured, &blog::model::Post::set_featured, serde::FieldOptions{}>{},
            serde::FieldDesc<"type", &blog::model::Post::get_type, &blog::model::Post::set_type, serde::FieldOptions{}>{},
            serde::FieldDesc<"status", &blog::model::Post::get_status, &blog::model::Post::set_status, serde::FieldOptions{}>{},
            serde::FieldDesc<"locale", &blog::model::Post::get_locale, &blog::model::Post::set_locale, serde::FieldOptions{}>{},
            serde::FieldDesc<"visibility", &blog::model::Post::get_visibility, &blog::model::Post::set_visibility, serde::FieldOptions{}>{},
            serde::FieldDesc<"email_recipient_filter", &blog::model::Post::get_email_recipient_filter, &blog::model::Post::set_email_recipient_filter, serde::FieldOptions{}>{},
            serde::FieldDesc<"created_at", &blog::model::Post::get_created_at, &blog::model::Post::set_created_at, serde::FieldOptions{}>{},
            serde::FieldDesc<"updated_at", &blog::model::Post::get_updated_at, &blog::model::Post::set_updated_at, serde::FieldOptions{}>{},
            serde::FieldDesc<"published_at", &blog::model::Post::get_published_at, &blog::model::Post::set_published_at, serde::FieldOptions{}>{},
            serde::FieldDesc<"published_by", &blog::model::Post::get_published_by, &blog::model::Post::set_published_by, serde::FieldOptions{}>{},
            serde::FieldDesc<"custom_excerpt", &blog::model::Post::get_custom_excerpt, &blog::model::Post::set_custom_excerpt, serde::FieldOptions{}>{},
            serde::FieldDesc<"auto_excerpt", &blog::model::Post::get_auto_excerpt, &blog::model::Post::set_auto_excerpt, serde::FieldOptions{}>{},
            serde::FieldDesc<"reading_time", &blog::model::Post::get_reading_time, &blog::model::Post::set_reading_time, serde::FieldOptions{}>{},
            serde::FieldDesc<"codeinjection_head", &blog::model::Post::get_codeinjection_head, &blog::model::Post::set_codeinjection_head, serde::FieldOptions{}>{},
            serde::FieldDesc<"codeinjection_foot", &blog::model::Post::get_codeinjection_foot, &blog::model::Post::set_codeinjection_foot, serde::FieldOptions{}>{},
            serde::FieldDesc<"custom_template", &blog::model::Post::get_custom_template, &blog::model::Post::set_custom_template, serde::FieldOptions{}>{},
            serde::FieldDesc<"canonical_url", &blog::model::Post::get_canonical_url, &blog::model::Post::set_canonical_url, serde::FieldOptions{}>{},
            serde::FieldDesc<"newsletter_id", &blog::model::Post::get_newsletter_id, &blog::model::Post::set_newsletter_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"show_title_and_feature_image", &blog::model::Post::get_show_title_and_feature_image, &blog::model::Post::set_show_title_and_feature_image, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "posts"; }
};
} // namespace serde


#ifdef CONGELADO_TEST
namespace blog::model::tests {
using namespace boost::ut;

suite<"blog_model_post"> blog_model_post_suite = []
{
    "reflects full Ghost post schema"_test = []
    {
        static_assert(core::db::reflect::ReflectedEntity<Post>);
        expect(core::db::reflect::field_count_v<Post> == 29);
        expect(core::db::reflect::EntityMeta<Post>::table == "posts");
    };
};

} // namespace blog::model::tests
#endif
