module;

export module blog_handlers:comments_model_wire_fixtures;

import std;
import core_serde;
import :comments_model_member;

export namespace blog::model::comments {

[[nodiscard]] inline serde::Value parse_wire_json(std::string_view text)
{
    const auto value = core_serde::JsonCodec{}.decode<serde::Value>(text);
    return value ? *value : serde::Value{};
}

struct EmailWire
{
    std::string id{serde::ghost_id(1)};
    std::string uuid{serde::uuid_for(1)};
    std::string subject{"Ghost Newsletter"};
    std::string html{"<p>Hello</p>"};
    std::string plaintext{"Hello"};
    std::string from{"Ghost <ghost@example.com>"};
    serde::Value reply_to{blog::model::comments::parse_wire_json("null")};
    serde::Value source{blog::model::comments::parse_wire_json("null")};
    std::string source_type{"posts"};
    std::string status{"sent"};
    std::string post_id{serde::ghost_id(2)};
    serde::Value newsletter_id{blog::model::comments::parse_wire_json("null")};
    std::string recipient_filter{"all"};
    std::string submitted_at{"2024-01-01T00:00:00.000Z"};
    std::int64_t email_count{10};
    std::int64_t delivered_count{9};
    std::int64_t opened_count{4};
    std::int64_t failed_count{1};
    serde::Value csd_email_count{blog::model::comments::parse_wire_json("null")};
    bool feedback_enabled{true};
    bool track_clicks{true};
    bool track_opens{true};
    serde::Value error{blog::model::comments::parse_wire_json("null")};
    serde::Value error_data{blog::model::comments::parse_wire_json("null")};
    std::string created_at{"2024-01-01T00:00:00.000Z"};
    std::string updated_at{"2024-01-01T00:00:00.000Z"};
};

struct AutomatedEmailWire
{
    std::string id{serde::ghost_id(1)};
    std::string email_design_setting_id{serde::ghost_id(2)};
    std::string html{"<p>Ghost Record</p>"};
    std::string lexical{"{}"};
    std::string created_at{"2024-01-01T00:00:00.000Z"};
};

struct EventWire
{
    std::string type{"comment_event"};
    serde::Value data{blog::model::comments::parse_wire_json(
        R"({"post_id":"1","member_id":"1","comment_id":"1"})"
    )};
};

struct CommentSubWire
{
    std::string id{serde::ghost_id(1)};
    std::string comment_id{serde::ghost_id(2)};
    std::string member_id{serde::ghost_id(3)};
    serde::Value member{blog::model::comments::parse_wire_json(
        R"({"id":"1","name":"Ghost Member","email":"member2@test.com"})"
    )};
    std::string created_at{"2024-01-01T00:00:00.000Z"};
    std::string updated_at{"2024-01-01T00:00:00.000Z"};
};

struct DbWire
{
    std::string status{"success"};
};

struct BatchWire
{
    std::string id{serde::ghost_id(1)};
    serde::Value count{blog::model::comments::parse_wire_json(R"({"total":1,"sent":1})")};
    std::string status{"submitted"};
    std::string mailgun_message_id{"message-1"};
    serde::Value error_data{blog::model::comments::parse_wire_json("null")};
    serde::Value error_message{blog::model::comments::parse_wire_json("null")};
    serde::Value error_status_code{blog::model::comments::parse_wire_json("null")};
    serde::Value member_segment{blog::model::comments::parse_wire_json("null")};
    std::string created_at{"2024-01-01T00:00:00.000Z"};
    std::string updated_at{"2024-01-01T00:00:00.000Z"};
};

struct FailureWire
{
    std::string id{serde::ghost_id(1)};
    std::string email_id{serde::ghost_id(2)};
    std::string email_recipient_id{serde::ghost_id(3)};
    std::int64_t code{static_cast<std::int64_t>(555)};
    std::string enhanced_code{"5.5.5"};
    std::string event_id{"event-1"};
    std::string failed_at{"2024-01-01T00:00:00.000Z"};
    std::string message{"Delivery failed"};
    std::string severity{"temporary"};
    serde::Value email_recipient{blog::model::comments::parse_wire_json("null")};
    serde::Value member{blog::model::comments::parse_wire_json("null")};
    std::string member_id{serde::ghost_id(3)};
};

struct CustomThemeSettingWire
{
    std::string id{serde::ghost_id(1)};
    std::string key{"accent_color"};
    std::string default_value{"#FF1A75"};
    std::string value{"#FF1A75"};
    std::string group{"branding"};
    std::string type{"color"};
    serde::Value options{blog::model::comments::parse_wire_json("[]")};
};

struct AutomationWire
{
    std::string id{serde::ghost_id(1)};
    std::string name{"Ghost Automation"};
    std::string slug{"ghost-automation"};
    std::string description{"Welcome automation"};
    std::string status{"active"};
    serde::Value actions{blog::model::comments::parse_wire_json("[]")};
    std::string created_at{"2024-01-01T00:00:00.000Z"};
    std::string updated_at{"2024-01-01T00:00:00.000Z"};
};

struct LabelWire
{
    std::string id{serde::ghost_id(1)};
    std::string name{"Ghost Resource"};
    std::string slug{"ghost-resource"};
    serde::Value count{blog::model::comments::parse_wire_json(R"({"posts":0})")};
    std::string created_at{"2024-01-01T00:00:00.000Z"};
    std::string updated_at{"2024-01-01T00:00:00.000Z"};
};

struct SnippetWire
{
    std::string id{serde::ghost_id(1)};
    std::string name{"Ghost Resource"};
    std::string lexical{"{}"};
    std::string mobiledoc{"{}"};
    std::string created_at{"2024-01-01T00:00:00.000Z"};
    std::string updated_at{"2024-01-01T00:00:00.000Z"};
};

struct RoleWire
{
    std::string id{serde::ghost_id(1)};
    std::string name{"Administrator"};
    std::string description{"Administrator role"};
    std::string created_at{"2024-01-01T00:00:00.000Z"};
    std::string updated_at{"2024-01-01T00:00:00.000Z"};
};

struct RecommendationWire
{
    std::string id{serde::ghost_id(1)};
    std::string url{"https://ghost.local"};
    std::string title{"Ghost Recommendation"};
    serde::Value description{blog::model::comments::parse_wire_json("null")};
    serde::Value excerpt{blog::model::comments::parse_wire_json("null")};
    serde::Value favicon{blog::model::comments::parse_wire_json("null")};
    serde::Value featuredImage{blog::model::comments::parse_wire_json("null")};
    serde::Value featured_image{blog::model::comments::parse_wire_json("null")};
    serde::Value created_at{blog::model::comments::parse_wire_json("null")};
    std::string createdAt{"2024-01-01T00:00:00.000Z"};
    serde::Value updated_at{blog::model::comments::parse_wire_json("null")};
    serde::Value reason{blog::model::comments::parse_wire_json("null")};
    bool oneClickSubscribe{true};
    bool one_click_subscribe{true};
    serde::Value count{blog::model::comments::parse_wire_json(R"({"clicks":0,"subscribers":0})")};
};

struct LinkWire
{
    std::string post_id{serde::ghost_id(1)};
    serde::Value link{
        blog::model::comments::parse_wire_json(R"({"url":"https://ghost.local","title":"Ghost"})")
    };
    serde::Value count{blog::model::comments::parse_wire_json(R"({"clicks":0})")};
};

struct OfferWire
{
    std::string id{serde::ghost_id(1)};
    std::string name{"Black Friday"};
    std::string code{"black-friday"};
    std::int64_t amount{50};
    std::string cadence{"once"};
    std::string currency{"usd"};
    bool currency_restriction{false};
    std::string display_description{"Save 50%"};
    std::string display_title{"Black Friday Sale"};
    std::string duration{"once"};
    serde::Value duration_in_months{blog::model::comments::parse_wire_json("null")};
    serde::Value last_redeemed{blog::model::comments::parse_wire_json("null")};
    std::string redemption_type{"public"};
    std::int64_t redemption_count{0};
    std::string status{"active"};
    std::string type_value{"percent"};
    serde::Value tier{blog::model::comments::parse_wire_json("null")};
    std::string created_at{"2024-01-01T00:00:00.000Z"};
    std::string updated_at{"2024-01-01T00:00:00.000Z"};
};

struct NotificationWire
{
    std::string id{serde::ghost_id(1)};
    std::string message{"Ghost notification"};
    std::string type_value{"info"};
    std::string status{"unread"};
    std::string location{"admin"};
    bool custom{false};
    bool dismissible{true};
};

struct WebhookWire
{
    std::string id{serde::ghost_id(1)};
    std::string name{"Ghost Webhook"};
    std::string event{"post.published"};
    std::string target_url{"https://ghost.local/hook"};
    std::string api_version{"v5.0"};
    std::string integration_id{serde::ghost_id(2)};
    std::string secret{"secret"};
    serde::Value last_triggered_at{blog::model::comments::parse_wire_json("null")};
    serde::Value last_triggered_error{blog::model::comments::parse_wire_json("null")};
    serde::Value last_triggered_status{blog::model::comments::parse_wire_json("null")};
    std::string created_at{"2024-01-01T00:00:00.000Z"};
    std::string updated_at{"2024-01-01T00:00:00.000Z"};
};


} // namespace blog::model::comments

export namespace blog::model::comments {

[[nodiscard]] inline std::string
encode_wire_key(const std::string& json, std::string_view from, std::string_view to)
{
    std::string out = json;
    const std::string needle = std::string{"\""} + std::string{from} + "\":";
    const std::string replacement = std::string{"\""} + std::string{to} + "\":";
    for (std::size_t at = out.find(needle); at != std::string::npos;
         at = out.find(needle, at + replacement.size())) {
        out.replace(at, needle.size(), replacement);
    }
    return out;
}

[[nodiscard]] inline std::string encode_notification_wire()
{
    return encode_wire_key(
        core_serde::JsonCodec{}.encode(NotificationWire{}),
        "type_value",
        "type"
    );
}

[[nodiscard]] inline std::string encode_offer_wire()
{
    return encode_wire_key(core_serde::JsonCodec{}.encode(OfferWire{}), "type_value", "type");
}

[[nodiscard]] inline std::string encode_custom_theme_setting_wire()
{
    return encode_wire_key(
        core_serde::JsonCodec{}.encode(CustomThemeSettingWire{}),
        "default_value",
        "default"
    );
}

} // namespace blog::model::comments
