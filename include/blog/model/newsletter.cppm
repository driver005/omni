module;
#include "entity_macros.h"

export module blog_model:newsletter;

import std;
import core_database;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::model {

/** @brief Ghost newsletters entity — full field set from data/schema/schema.js. */
struct Newsletter
{
    std::int64_t id{};
    std::string uuid{};
    std::string name{};
    std::string description{};
    bool feedback_enabled{};
    std::string slug{};
    std::string sender_name{};
    std::string sender_email{};
    std::string sender_reply_to{};
    std::string status{};
    std::string visibility{};
    bool subscribe_on_signup{};
    std::int64_t sort_order{};
    std::string header_image{};
    bool show_header_icon{};
    bool show_header_title{};
    bool show_excerpt{};
    std::string title_font_category{};
    std::string title_alignment{};
    bool show_feature_image{};
    std::string body_font_category{};
    std::string footer_content{};
    bool show_badge{};
    bool show_header_name{};
    bool show_post_title_section{};
    bool show_comment_cta{};
    bool show_subscription_details{};
    bool show_latest_posts{};
    bool show_share_button{};
    std::string background_color{};
    std::string post_title_color{};
    std::chrono::system_clock::time_point created_at{};
    std::chrono::system_clock::time_point updated_at{};
    std::string button_corners{};
    std::string button_style{};
    std::string title_font_weight{};
    std::string link_style{};
    std::string image_corners{};
    std::string header_background_color{};
    std::string section_title_color{};
};

GHOST_ENTITY(Newsletter, "newsletters",
    GHOST_FIELD("id", &Newsletter::id, core::db::ColumnType::BIGINT, core::db::FieldFlags::PRIMARY_KEY),
    GHOST_FIELD("uuid", &Newsletter::uuid, core::db::ColumnType::TEXT),
    GHOST_FIELD("name", &Newsletter::name, core::db::ColumnType::TEXT),
    GHOST_FIELD("description", &Newsletter::description, core::db::ColumnType::TEXT),
    GHOST_FIELD("feedback_enabled", &Newsletter::feedback_enabled, core::db::ColumnType::BOOLEAN),
    GHOST_FIELD("slug", &Newsletter::slug, core::db::ColumnType::TEXT),
    GHOST_FIELD("sender_name", &Newsletter::sender_name, core::db::ColumnType::TEXT),
    GHOST_FIELD("sender_email", &Newsletter::sender_email, core::db::ColumnType::TEXT),
    GHOST_FIELD("sender_reply_to", &Newsletter::sender_reply_to, core::db::ColumnType::TEXT),
    GHOST_FIELD("status", &Newsletter::status, core::db::ColumnType::TEXT),
    GHOST_FIELD("visibility", &Newsletter::visibility, core::db::ColumnType::TEXT),
    GHOST_FIELD("subscribe_on_signup", &Newsletter::subscribe_on_signup, core::db::ColumnType::BOOLEAN),
    GHOST_FIELD("sort_order", &Newsletter::sort_order, core::db::ColumnType::INTEGER),
    GHOST_FIELD("header_image", &Newsletter::header_image, core::db::ColumnType::TEXT),
    GHOST_FIELD("show_header_icon", &Newsletter::show_header_icon, core::db::ColumnType::BOOLEAN),
    GHOST_FIELD("show_header_title", &Newsletter::show_header_title, core::db::ColumnType::BOOLEAN),
    GHOST_FIELD("show_excerpt", &Newsletter::show_excerpt, core::db::ColumnType::BOOLEAN),
    GHOST_FIELD("title_font_category", &Newsletter::title_font_category, core::db::ColumnType::TEXT),
    GHOST_FIELD("title_alignment", &Newsletter::title_alignment, core::db::ColumnType::TEXT),
    GHOST_FIELD("show_feature_image", &Newsletter::show_feature_image, core::db::ColumnType::BOOLEAN),
    GHOST_FIELD("body_font_category", &Newsletter::body_font_category, core::db::ColumnType::TEXT),
    GHOST_FIELD("footer_content", &Newsletter::footer_content, core::db::ColumnType::TEXT),
    GHOST_FIELD("show_badge", &Newsletter::show_badge, core::db::ColumnType::BOOLEAN),
    GHOST_FIELD("show_header_name", &Newsletter::show_header_name, core::db::ColumnType::BOOLEAN),
    GHOST_FIELD("show_post_title_section", &Newsletter::show_post_title_section, core::db::ColumnType::BOOLEAN),
    GHOST_FIELD("show_comment_cta", &Newsletter::show_comment_cta, core::db::ColumnType::BOOLEAN),
    GHOST_FIELD("show_subscription_details", &Newsletter::show_subscription_details, core::db::ColumnType::BOOLEAN),
    GHOST_FIELD("show_latest_posts", &Newsletter::show_latest_posts, core::db::ColumnType::BOOLEAN),
    GHOST_FIELD("show_share_button", &Newsletter::show_share_button, core::db::ColumnType::BOOLEAN),
    GHOST_FIELD("background_color", &Newsletter::background_color, core::db::ColumnType::TEXT),
    GHOST_FIELD("post_title_color", &Newsletter::post_title_color, core::db::ColumnType::TEXT),
    GHOST_FIELD("created_at", &Newsletter::created_at, core::db::ColumnType::TIMESTAMP),
    GHOST_FIELD("updated_at", &Newsletter::updated_at, core::db::ColumnType::TIMESTAMP),
    GHOST_FIELD("button_corners", &Newsletter::button_corners, core::db::ColumnType::TEXT),
    GHOST_FIELD("button_style", &Newsletter::button_style, core::db::ColumnType::TEXT),
    GHOST_FIELD("title_font_weight", &Newsletter::title_font_weight, core::db::ColumnType::TEXT),
    GHOST_FIELD("link_style", &Newsletter::link_style, core::db::ColumnType::TEXT),
    GHOST_FIELD("image_corners", &Newsletter::image_corners, core::db::ColumnType::TEXT),
    GHOST_FIELD("header_background_color", &Newsletter::header_background_color, core::db::ColumnType::TEXT),
    GHOST_FIELD("section_title_color", &Newsletter::section_title_color, core::db::ColumnType::TEXT));

} // namespace blog::model

#ifdef CONGELADO_TEST
namespace blog::model::tests {
using namespace boost::ut;
suite<"blog_model_newsletter"> blog_model_newsletter_suite = []
{
    "reflects full Ghost newsletters schema"_test = []
    {
        static_assert(core::db::reflect::ReflectedEntity<Newsletter>);
        expect(core::db::reflect::field_count_v<Newsletter> == 40);
        expect(core::db::reflect::EntityMeta<Newsletter>::table == "newsletters");
    };
};
} // namespace blog::model::tests
#endif
