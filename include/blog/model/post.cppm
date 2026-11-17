module;
#include "entity_macros.h"

export module blog_model:post;

import std;
import core_database;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::model {

/** @brief Ghost posts entity — full field set from data/schema/schema.js. */
struct Post
{
    std::int64_t id{};
    std::string uuid{};
    std::string title{};
    std::string slug{};
    std::string mobiledoc{};
    std::string lexical{};
    std::string html{};
    std::string comment_id{};
    std::string plaintext{};
    std::string feature_image{};
    bool featured{};
    std::string type{};
    std::string status{};
    std::string locale{};
    std::string visibility{};
    std::string email_recipient_filter{};
    std::chrono::system_clock::time_point created_at{};
    std::chrono::system_clock::time_point updated_at{};
    std::chrono::system_clock::time_point published_at{};
    std::string published_by{};
    std::string custom_excerpt{};
    std::string auto_excerpt{};
    std::int64_t reading_time{};
    std::string codeinjection_head{};
    std::string codeinjection_foot{};
    std::string custom_template{};
    std::string canonical_url{};
    std::string newsletter_id{};
    bool show_title_and_feature_image{};
};

GHOST_ENTITY(Post, "posts",
    GHOST_FIELD("id", &Post::id, core::db::ColumnType::BIGINT, core::db::FieldFlags::PRIMARY_KEY),
    GHOST_FIELD("uuid", &Post::uuid, core::db::ColumnType::TEXT),
    GHOST_FIELD("title", &Post::title, core::db::ColumnType::TEXT),
    GHOST_FIELD("slug", &Post::slug, core::db::ColumnType::TEXT),
    GHOST_FIELD("mobiledoc", &Post::mobiledoc, core::db::ColumnType::TEXT),
    GHOST_FIELD("lexical", &Post::lexical, core::db::ColumnType::TEXT),
    GHOST_FIELD("html", &Post::html, core::db::ColumnType::TEXT),
    GHOST_FIELD("comment_id", &Post::comment_id, core::db::ColumnType::TEXT),
    GHOST_FIELD("plaintext", &Post::plaintext, core::db::ColumnType::TEXT),
    GHOST_FIELD("feature_image", &Post::feature_image, core::db::ColumnType::TEXT),
    GHOST_FIELD("featured", &Post::featured, core::db::ColumnType::BOOLEAN),
    GHOST_FIELD("type", &Post::type, core::db::ColumnType::TEXT),
    GHOST_FIELD("status", &Post::status, core::db::ColumnType::TEXT),
    GHOST_FIELD("locale", &Post::locale, core::db::ColumnType::TEXT),
    GHOST_FIELD("visibility", &Post::visibility, core::db::ColumnType::TEXT),
    GHOST_FIELD("email_recipient_filter", &Post::email_recipient_filter, core::db::ColumnType::TEXT),
    GHOST_FIELD("created_at", &Post::created_at, core::db::ColumnType::TIMESTAMP),
    GHOST_FIELD("updated_at", &Post::updated_at, core::db::ColumnType::TIMESTAMP),
    GHOST_FIELD("published_at", &Post::published_at, core::db::ColumnType::TIMESTAMP),
    GHOST_FIELD("published_by", &Post::published_by, core::db::ColumnType::TEXT),
    GHOST_FIELD("custom_excerpt", &Post::custom_excerpt, core::db::ColumnType::TEXT),
    GHOST_FIELD("auto_excerpt", &Post::auto_excerpt, core::db::ColumnType::TEXT),
    GHOST_FIELD("reading_time", &Post::reading_time, core::db::ColumnType::INTEGER),
    GHOST_FIELD("codeinjection_head", &Post::codeinjection_head, core::db::ColumnType::TEXT),
    GHOST_FIELD("codeinjection_foot", &Post::codeinjection_foot, core::db::ColumnType::TEXT),
    GHOST_FIELD("custom_template", &Post::custom_template, core::db::ColumnType::TEXT),
    GHOST_FIELD("canonical_url", &Post::canonical_url, core::db::ColumnType::TEXT),
    GHOST_FIELD("newsletter_id", &Post::newsletter_id, core::db::ColumnType::TEXT),
    GHOST_FIELD("show_title_and_feature_image", &Post::show_title_and_feature_image, core::db::ColumnType::BOOLEAN));

} // namespace blog::model

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
        expect(core::db::reflect::column_of<Post, &Post::feature_image>() == "feature_image");
    };
};

} // namespace blog::model::tests
#endif
