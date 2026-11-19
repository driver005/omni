export module blog_handlers:model_newsletter;

import std;
import core_serde;
import core_database;
import :model_member;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::model {

/** @brief Ghost newsletters entity — full field set from data/schema/schema.js. */
class Newsletter
{
public:
    Newsletter() = default;

    void set_id(std::int64_t value) { m_id = std::move(value); }
    [[nodiscard]] const std::int64_t& get_id() const noexcept { return m_id; }
    void set_uuid(std::string value) { m_uuid = std::move(value); }
    [[nodiscard]] const std::string& get_uuid() const noexcept { return m_uuid; }
    void set_name(std::string value) { m_name = std::move(value); }
    [[nodiscard]] const std::string& get_name() const noexcept { return m_name; }
    void set_description(std::string value) { m_description = std::move(value); }
    [[nodiscard]] const std::string& get_description() const noexcept { return m_description; }
    void set_feedback_enabled(bool value) { m_feedback_enabled = std::move(value); }
    [[nodiscard]] const bool& get_feedback_enabled() const noexcept { return m_feedback_enabled; }
    void set_slug(std::string value) { m_slug = std::move(value); }
    [[nodiscard]] const std::string& get_slug() const noexcept { return m_slug; }
    void set_sender_name(std::string value) { m_sender_name = std::move(value); }
    [[nodiscard]] const std::string& get_sender_name() const noexcept { return m_sender_name; }
    void set_sender_email(std::string value) { m_sender_email = std::move(value); }
    [[nodiscard]] const std::string& get_sender_email() const noexcept { return m_sender_email; }
    void set_sender_reply_to(std::string value) { m_sender_reply_to = std::move(value); }
    [[nodiscard]] const std::string& get_sender_reply_to() const noexcept { return m_sender_reply_to; }
    void set_status(std::string value) { m_status = std::move(value); }
    [[nodiscard]] const std::string& get_status() const noexcept { return m_status; }
    void set_visibility(std::string value) { m_visibility = std::move(value); }
    [[nodiscard]] const std::string& get_visibility() const noexcept { return m_visibility; }
    void set_subscribe_on_signup(bool value) { m_subscribe_on_signup = std::move(value); }
    [[nodiscard]] const bool& get_subscribe_on_signup() const noexcept { return m_subscribe_on_signup; }
    void set_sort_order(std::int64_t value) { m_sort_order = std::move(value); }
    [[nodiscard]] const std::int64_t& get_sort_order() const noexcept { return m_sort_order; }
    void set_header_image(std::string value) { m_header_image = std::move(value); }
    [[nodiscard]] const std::string& get_header_image() const noexcept { return m_header_image; }
    void set_show_header_icon(bool value) { m_show_header_icon = std::move(value); }
    [[nodiscard]] const bool& get_show_header_icon() const noexcept { return m_show_header_icon; }
    void set_show_header_title(bool value) { m_show_header_title = std::move(value); }
    [[nodiscard]] const bool& get_show_header_title() const noexcept { return m_show_header_title; }
    void set_show_excerpt(bool value) { m_show_excerpt = std::move(value); }
    [[nodiscard]] const bool& get_show_excerpt() const noexcept { return m_show_excerpt; }
    void set_title_font_category(std::string value) { m_title_font_category = std::move(value); }
    [[nodiscard]] const std::string& get_title_font_category() const noexcept { return m_title_font_category; }
    void set_title_alignment(std::string value) { m_title_alignment = std::move(value); }
    [[nodiscard]] const std::string& get_title_alignment() const noexcept { return m_title_alignment; }
    void set_show_feature_image(bool value) { m_show_feature_image = std::move(value); }
    [[nodiscard]] const bool& get_show_feature_image() const noexcept { return m_show_feature_image; }
    void set_body_font_category(std::string value) { m_body_font_category = std::move(value); }
    [[nodiscard]] const std::string& get_body_font_category() const noexcept { return m_body_font_category; }
    void set_footer_content(std::string value) { m_footer_content = std::move(value); }
    [[nodiscard]] const std::string& get_footer_content() const noexcept { return m_footer_content; }
    void set_show_badge(bool value) { m_show_badge = std::move(value); }
    [[nodiscard]] const bool& get_show_badge() const noexcept { return m_show_badge; }
    void set_show_header_name(bool value) { m_show_header_name = std::move(value); }
    [[nodiscard]] const bool& get_show_header_name() const noexcept { return m_show_header_name; }
    void set_show_post_title_section(bool value) { m_show_post_title_section = std::move(value); }
    [[nodiscard]] const bool& get_show_post_title_section() const noexcept { return m_show_post_title_section; }
    void set_show_comment_cta(bool value) { m_show_comment_cta = std::move(value); }
    [[nodiscard]] const bool& get_show_comment_cta() const noexcept { return m_show_comment_cta; }
    void set_show_subscription_details(bool value) { m_show_subscription_details = std::move(value); }
    [[nodiscard]] const bool& get_show_subscription_details() const noexcept { return m_show_subscription_details; }
    void set_show_latest_posts(bool value) { m_show_latest_posts = std::move(value); }
    [[nodiscard]] const bool& get_show_latest_posts() const noexcept { return m_show_latest_posts; }
    void set_show_share_button(bool value) { m_show_share_button = std::move(value); }
    [[nodiscard]] const bool& get_show_share_button() const noexcept { return m_show_share_button; }
    void set_background_color(std::string value) { m_background_color = std::move(value); }
    [[nodiscard]] const std::string& get_background_color() const noexcept { return m_background_color; }
    void set_post_title_color(std::string value) { m_post_title_color = std::move(value); }
    [[nodiscard]] const std::string& get_post_title_color() const noexcept { return m_post_title_color; }
    void set_created_at(std::chrono::system_clock::time_point value) { m_created_at = std::move(value); }
    [[nodiscard]] const std::chrono::system_clock::time_point& get_created_at() const noexcept { return m_created_at; }
    void set_updated_at(std::chrono::system_clock::time_point value) { m_updated_at = std::move(value); }
    [[nodiscard]] const std::chrono::system_clock::time_point& get_updated_at() const noexcept { return m_updated_at; }
    void set_button_corners(std::string value) { m_button_corners = std::move(value); }
    [[nodiscard]] const std::string& get_button_corners() const noexcept { return m_button_corners; }
    void set_button_style(std::string value) { m_button_style = std::move(value); }
    [[nodiscard]] const std::string& get_button_style() const noexcept { return m_button_style; }
    void set_title_font_weight(std::string value) { m_title_font_weight = std::move(value); }
    [[nodiscard]] const std::string& get_title_font_weight() const noexcept { return m_title_font_weight; }
    void set_link_style(std::string value) { m_link_style = std::move(value); }
    [[nodiscard]] const std::string& get_link_style() const noexcept { return m_link_style; }
    void set_image_corners(std::string value) { m_image_corners = std::move(value); }
    [[nodiscard]] const std::string& get_image_corners() const noexcept { return m_image_corners; }
    void set_header_background_color(std::string value) { m_header_background_color = std::move(value); }
    [[nodiscard]] const std::string& get_header_background_color() const noexcept { return m_header_background_color; }
    void set_section_title_color(std::string value) { m_section_title_color = std::move(value); }
    [[nodiscard]] const std::string& get_section_title_color() const noexcept { return m_section_title_color; }

    [[nodiscard]] std::string wire_uuid() const { return m_uuid.empty() ? serde::uuid_for(m_id) : m_uuid; }
    [[nodiscard]] serde::RawJson wire_description() const { return m_description.empty() ? serde::RawJson{"null"} : serde::RawJson{"\"" + m_description + "\""}; }
    [[nodiscard]] std::string wire_sender_email() const { return m_sender_email.empty() ? std::string{"ghost@example.com"} : m_sender_email; }
    [[nodiscard]] std::string wire_sender_name() const { return m_sender_name.empty() ? std::string{"Ghost"} : m_sender_name; }
    [[nodiscard]] serde::RawJson wire_count() const { return {R"({"sent":0,"opened":0,"clicked":0})"}; }
    [[nodiscard]] bool wire_true() const { return true; }
    [[nodiscard]] std::string wire_visibility() const { return "public"; }
    [[nodiscard]] std::string wire_body_font_category() const { return "sans-serif"; }
    [[nodiscard]] std::string wire_button_color() const { return "#FF1A75"; }
    [[nodiscard]] std::string wire_button_corners() const { return "square"; }
    [[nodiscard]] std::string wire_button_style() const { return "solid"; }
    [[nodiscard]] std::string wire_header_background_color() const { return "#15171A"; }
    [[nodiscard]] std::string wire_image_corners() const { return "square"; }
    [[nodiscard]] std::string wire_link_color() const { return "#FF1A75"; }
    [[nodiscard]] std::string wire_link_style() const { return "underline"; }
    [[nodiscard]] std::string wire_title_alignment() const { return "left"; }
    [[nodiscard]] std::string wire_title_font_category() const { return "sans-serif"; }
    [[nodiscard]] std::string wire_title_font_weight() const { return "bold"; }
    [[nodiscard]] std::string wire_sender_reply_to() const { return "reply@example.com"; }
    [[nodiscard]] std::string wire_background_color() const { return "#FFFFFF"; }
    [[nodiscard]] serde::RawJson wire_timestamp() const { return {R"("2024-01-01T00:00:00.000Z")"}; }
    [[nodiscard]] serde::RawJson wire_null() const { return {"null"}; }
    void set_wire_noop(serde::RawJson) {}
    void set_wire_string_noop(std::string) {}

    std::int64_t m_id{};
    std::string m_uuid{};
    std::string m_name{};
    std::string m_description{};
    bool m_feedback_enabled{};
    std::string m_slug{};
    std::string m_sender_name{};
    std::string m_sender_email{};
    std::string m_sender_reply_to{};
    std::string m_status{};
    std::string m_visibility{};
    bool m_subscribe_on_signup{};
    std::int64_t m_sort_order{};
    std::string m_header_image{};
    bool m_show_header_icon{};
    bool m_show_header_title{};
    bool m_show_excerpt{};
    std::string m_title_font_category{};
    std::string m_title_alignment{};
    bool m_show_feature_image{};
    std::string m_body_font_category{};
    std::string m_footer_content{};
    bool m_show_badge{};
    bool m_show_header_name{};
    bool m_show_post_title_section{};
    bool m_show_comment_cta{};
    bool m_show_subscription_details{};
    bool m_show_latest_posts{};
    bool m_show_share_button{};
    std::string m_background_color{};
    std::string m_post_title_color{};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
    std::string m_button_corners{};
    std::string m_button_style{};
    std::string m_title_font_weight{};
    std::string m_link_style{};
    std::string m_image_corners{};
    std::string m_header_background_color{};
    std::string m_section_title_color{};
};




} // namespace blog::model

namespace serde {
template<>
struct Serializable<blog::model::Newsletter>
{
    using M = blog::model::Newsletter;
    static constexpr auto fields()
    {
        return std::tuple{
            FieldDesc<"id", &M::get_id, &M::set_id, FieldOptions::init().with_db(FieldOptionsDb::init().pk()), wire_id_normalizer{}>{},
            FieldDesc<"uuid", &M::wire_uuid, &M::set_uuid>{},
            FieldDesc<"name", &M::get_name, &M::set_name>{},
            FieldDesc<"slug", &M::get_slug, &M::set_slug>{},
            FieldDesc<"description", &M::wire_description, &M::set_wire_noop>{},
            FieldDesc<"status", &M::get_status, &M::set_status>{},
            FieldDesc<"visibility", &M::wire_visibility, &M::set_wire_string_noop>{},
            FieldDesc<"body_font_category", &M::wire_body_font_category, &M::set_wire_string_noop>{},
            FieldDesc<"button_color", &M::wire_button_color, &M::set_wire_string_noop>{},
            FieldDesc<"button_corners", &M::wire_button_corners, &M::set_wire_string_noop>{},
            FieldDesc<"button_style", &M::wire_button_style, &M::set_wire_string_noop>{},
            FieldDesc<"header_background_color", &M::wire_header_background_color, &M::set_wire_string_noop>{},
            FieldDesc<"image_corners", &M::wire_image_corners, &M::set_wire_string_noop>{},
            FieldDesc<"link_color", &M::wire_link_color, &M::set_wire_string_noop>{},
            FieldDesc<"link_style", &M::wire_link_style, &M::set_wire_string_noop>{},
            FieldDesc<"title_alignment", &M::wire_title_alignment, &M::set_wire_string_noop>{},
            FieldDesc<"title_font_category", &M::wire_title_font_category, &M::set_wire_string_noop>{},
            FieldDesc<"title_font_weight", &M::wire_title_font_weight, &M::set_wire_string_noop>{},
            FieldDesc<"sender_reply_to", &M::wire_sender_reply_to, &M::set_wire_string_noop>{},
            FieldDesc<"background_color", &M::wire_background_color, &M::set_wire_string_noop>{},
            FieldDesc<"sort_order", &M::get_id, &M::set_id, FieldOptions{}, zero_normalizer{}>{},
            FieldDesc<"divider_color", &M::wire_null, &M::set_wire_noop>{},
            FieldDesc<"footer_content", &M::wire_null, &M::set_wire_noop>{},
            FieldDesc<"header_image", &M::wire_null, &M::set_wire_noop>{},
            FieldDesc<"post_title_color", &M::wire_null, &M::set_wire_noop>{},
            FieldDesc<"section_title_color", &M::wire_null, &M::set_wire_noop>{},
            FieldDesc<"sender_email", &M::wire_sender_email, &M::set_wire_string_noop>{},
            FieldDesc<"sender_name", &M::wire_sender_name, &M::set_wire_string_noop>{},
            FieldDesc<"feedback_enabled", &M::get_feedback_enabled, &M::set_feedback_enabled>{},
            FieldDesc<"show_badge", &M::wire_true, &M::set_wire_noop>{},
            FieldDesc<"show_comment_cta", &M::wire_true, &M::set_wire_noop>{},
            FieldDesc<"show_excerpt", &M::wire_true, &M::set_wire_noop>{},
            FieldDesc<"show_feature_image", &M::wire_true, &M::set_wire_noop>{},
            FieldDesc<"show_header_icon", &M::wire_true, &M::set_wire_noop>{},
            FieldDesc<"show_header_name", &M::wire_true, &M::set_wire_noop>{},
            FieldDesc<"show_header_title", &M::wire_true, &M::set_wire_noop>{},
            FieldDesc<"show_latest_posts", &M::wire_true, &M::set_wire_noop>{},
            FieldDesc<"show_post_title_section", &M::wire_true, &M::set_wire_noop>{},
            FieldDesc<"show_share_button", &M::wire_true, &M::set_wire_noop>{},
            FieldDesc<"show_subscription_details", &M::wire_true, &M::set_wire_noop>{},
            FieldDesc<"subscribe_on_signup", &M::get_subscribe_on_signup, &M::set_subscribe_on_signup>{},
            FieldDesc<"count", &M::wire_count, &M::set_wire_noop>{},
            FieldDesc<"created_at", &M::wire_timestamp, &M::set_wire_noop>{},
            FieldDesc<"updated_at", &M::wire_timestamp, &M::set_wire_noop>{}
        };
    }
    static constexpr auto db_fields()
    {
        return std::tuple{
            serde::FieldDesc<"id", &blog::model::Newsletter::get_id, &blog::model::Newsletter::set_id, serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<"uuid", &blog::model::Newsletter::get_uuid, &blog::model::Newsletter::set_uuid, serde::FieldOptions{}>{},
            serde::FieldDesc<"name", &blog::model::Newsletter::get_name, &blog::model::Newsletter::set_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"description", &blog::model::Newsletter::get_description, &blog::model::Newsletter::set_description, serde::FieldOptions{}>{},
            serde::FieldDesc<"feedback_enabled", &blog::model::Newsletter::get_feedback_enabled, &blog::model::Newsletter::set_feedback_enabled, serde::FieldOptions{}>{},
            serde::FieldDesc<"slug", &blog::model::Newsletter::get_slug, &blog::model::Newsletter::set_slug, serde::FieldOptions{}>{},
            serde::FieldDesc<"sender_name", &blog::model::Newsletter::get_sender_name, &blog::model::Newsletter::set_sender_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"sender_email", &blog::model::Newsletter::get_sender_email, &blog::model::Newsletter::set_sender_email, serde::FieldOptions{}>{},
            serde::FieldDesc<"sender_reply_to", &blog::model::Newsletter::get_sender_reply_to, &blog::model::Newsletter::set_sender_reply_to, serde::FieldOptions{}>{},
            serde::FieldDesc<"status", &blog::model::Newsletter::get_status, &blog::model::Newsletter::set_status, serde::FieldOptions{}>{},
            serde::FieldDesc<"visibility", &blog::model::Newsletter::get_visibility, &blog::model::Newsletter::set_visibility, serde::FieldOptions{}>{},
            serde::FieldDesc<"subscribe_on_signup", &blog::model::Newsletter::get_subscribe_on_signup, &blog::model::Newsletter::set_subscribe_on_signup, serde::FieldOptions{}>{},
            serde::FieldDesc<"sort_order", &blog::model::Newsletter::get_sort_order, &blog::model::Newsletter::set_sort_order, serde::FieldOptions{}>{},
            serde::FieldDesc<"header_image", &blog::model::Newsletter::get_header_image, &blog::model::Newsletter::set_header_image, serde::FieldOptions{}>{},
            serde::FieldDesc<"show_header_icon", &blog::model::Newsletter::get_show_header_icon, &blog::model::Newsletter::set_show_header_icon, serde::FieldOptions{}>{},
            serde::FieldDesc<"show_header_title", &blog::model::Newsletter::get_show_header_title, &blog::model::Newsletter::set_show_header_title, serde::FieldOptions{}>{},
            serde::FieldDesc<"show_excerpt", &blog::model::Newsletter::get_show_excerpt, &blog::model::Newsletter::set_show_excerpt, serde::FieldOptions{}>{},
            serde::FieldDesc<"title_font_category", &blog::model::Newsletter::get_title_font_category, &blog::model::Newsletter::set_title_font_category, serde::FieldOptions{}>{},
            serde::FieldDesc<"title_alignment", &blog::model::Newsletter::get_title_alignment, &blog::model::Newsletter::set_title_alignment, serde::FieldOptions{}>{},
            serde::FieldDesc<"show_feature_image", &blog::model::Newsletter::get_show_feature_image, &blog::model::Newsletter::set_show_feature_image, serde::FieldOptions{}>{},
            serde::FieldDesc<"body_font_category", &blog::model::Newsletter::get_body_font_category, &blog::model::Newsletter::set_body_font_category, serde::FieldOptions{}>{},
            serde::FieldDesc<"footer_content", &blog::model::Newsletter::get_footer_content, &blog::model::Newsletter::set_footer_content, serde::FieldOptions{}>{},
            serde::FieldDesc<"show_badge", &blog::model::Newsletter::get_show_badge, &blog::model::Newsletter::set_show_badge, serde::FieldOptions{}>{},
            serde::FieldDesc<"show_header_name", &blog::model::Newsletter::get_show_header_name, &blog::model::Newsletter::set_show_header_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"show_post_title_section", &blog::model::Newsletter::get_show_post_title_section, &blog::model::Newsletter::set_show_post_title_section, serde::FieldOptions{}>{},
            serde::FieldDesc<"show_comment_cta", &blog::model::Newsletter::get_show_comment_cta, &blog::model::Newsletter::set_show_comment_cta, serde::FieldOptions{}>{},
            serde::FieldDesc<"show_subscription_details", &blog::model::Newsletter::get_show_subscription_details, &blog::model::Newsletter::set_show_subscription_details, serde::FieldOptions{}>{},
            serde::FieldDesc<"show_latest_posts", &blog::model::Newsletter::get_show_latest_posts, &blog::model::Newsletter::set_show_latest_posts, serde::FieldOptions{}>{},
            serde::FieldDesc<"show_share_button", &blog::model::Newsletter::get_show_share_button, &blog::model::Newsletter::set_show_share_button, serde::FieldOptions{}>{},
            serde::FieldDesc<"background_color", &blog::model::Newsletter::get_background_color, &blog::model::Newsletter::set_background_color, serde::FieldOptions{}>{},
            serde::FieldDesc<"post_title_color", &blog::model::Newsletter::get_post_title_color, &blog::model::Newsletter::set_post_title_color, serde::FieldOptions{}>{},
            serde::FieldDesc<"created_at", &blog::model::Newsletter::get_created_at, &blog::model::Newsletter::set_created_at, serde::FieldOptions{}>{},
            serde::FieldDesc<"updated_at", &blog::model::Newsletter::get_updated_at, &blog::model::Newsletter::set_updated_at, serde::FieldOptions{}>{},
            serde::FieldDesc<"button_corners", &blog::model::Newsletter::get_button_corners, &blog::model::Newsletter::set_button_corners, serde::FieldOptions{}>{},
            serde::FieldDesc<"button_style", &blog::model::Newsletter::get_button_style, &blog::model::Newsletter::set_button_style, serde::FieldOptions{}>{},
            serde::FieldDesc<"title_font_weight", &blog::model::Newsletter::get_title_font_weight, &blog::model::Newsletter::set_title_font_weight, serde::FieldOptions{}>{},
            serde::FieldDesc<"link_style", &blog::model::Newsletter::get_link_style, &blog::model::Newsletter::set_link_style, serde::FieldOptions{}>{},
            serde::FieldDesc<"image_corners", &blog::model::Newsletter::get_image_corners, &blog::model::Newsletter::set_image_corners, serde::FieldOptions{}>{},
            serde::FieldDesc<"header_background_color", &blog::model::Newsletter::get_header_background_color, &blog::model::Newsletter::set_header_background_color, serde::FieldOptions{}>{},
            serde::FieldDesc<"section_title_color", &blog::model::Newsletter::get_section_title_color, &blog::model::Newsletter::set_section_title_color, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "newsletters"; }
};
} // namespace serde


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
