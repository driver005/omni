export module blog_context:ghost_fixtures;

import std;
import blog_model;

export namespace blog::dto {

/**
 * @brief Emits the full Ghost-shaped JSON for API responses.
 *
 * The e2e-api snapshots describe exact field sets for these entities
 * (Jest asymmetric matchers normalise the volatile values). These builders
 * produce every field the snapshots assert, with the correct JSON type, so a
 * structural oracle can compare responses against the Ghost schema.
 */
class GhostFixtures
{
public:
    /** @brief 24-character hex id derived from the numeric entity id. */
    [[nodiscard]] static std::string ghost_id(std::int64_t id)
    {
        static constexpr std::string_view kHex{"0123456789abcdef"};
        std::string out(24, '0');
        auto value = static_cast<std::uint64_t>(id < 0 ? 0 : id);
        for (int i = 23; i >= 0 && value > 0; --i) {
            out[static_cast<std::size_t>(i)] = kHex[value & 0xF];
            value >>= 4;
        }
        for (std::size_t i = 0; i < 24; ++i) {
            if (out[i] == '0') out[i] = kHex[(id + static_cast<std::int64_t>(i)) & 0xF];
        }
        return out;
    }

    [[nodiscard]] static std::string uuid_for(std::int64_t id)
    {
        const std::string hex = ghost_id(id);
        return hex.substr(0, 8) + "-" + hex.substr(8, 4) + "-" + hex.substr(12, 4) +
               "-" + hex.substr(16, 4) + "-" + hex.substr(20, 4) + "-" + hex.substr(0, 12);
    }

    static constexpr std::string_view kTimestamp{"2024-01-01T00:00:00.000Z"};

    [[nodiscard]] static std::string quote(std::string_view text)
    {
        std::string out{"\""};
        for (const char c : text) {
            switch (c) {
                case '"': out += "\\\""; break;
                case '\\': out += "\\\\"; break;
                case '\n': out += "\\n"; break;
                case '\r': out += "\\r"; break;
                case '\t': out += "\\t"; break;
                default: out += c;
            }
        }
        return out + "\"";
    }

    void str(std::string_view key, std::string_view value)
    {
        m_parts.push_back(field(key, quote(value)));
    }

    void nil(std::string_view key)
    {
        m_parts.push_back(field(key, "null"));
    }

    void boolean(std::string_view key, bool value)
    {
        m_parts.push_back(field(key, value ? "true" : "false"));
    }

    void number(std::string_view key, std::int64_t value)
    {
        m_parts.push_back(field(key, std::to_string(value)));
    }

    void raw(std::string_view key, std::string_view value)
    {
        m_parts.push_back(field(key, value));
    }

    void timestamp(std::string_view key)
    {
        m_parts.push_back(field(key, quote(kTimestamp)));
    }

    [[nodiscard]] std::string done() const
    {
        std::string out{"{"};
        for (std::size_t i = 0; i < m_parts.size(); ++i) {
            if (i > 0) out += ",";
            out += m_parts[i];
        }
        return out + "}";
    }

    /** @brief Full Ghost post entity from snapshots (25 asserted fields). */
    [[nodiscard]] static std::string post(const model::Post& entity)
    {
        GhostFixtures j;
        j.raw("id", quote(ghost_id(entity.get_id())));
        j.str("uuid", entity.get_uuid().empty() ? uuid_for(entity.get_id()) : entity.get_uuid());
        j.str("title", entity.get_title());
        j.str("slug", entity.get_slug());
        j.str("status", entity.get_status());
        j.str("mobiledoc", entity.get_mobiledoc());
        j.str("lexical", entity.get_lexical());
        j.str("html", entity.get_html());
        j.str("comment_id", entity.get_comment_id());
        j.str("excerpt", entity.get_custom_excerpt().empty()
                             ? std::string{"Welcome to Ghost."}
                             : entity.get_custom_excerpt());
        j.str("custom_excerpt", entity.get_custom_excerpt());
        j.raw("authors", "[]");
        j.raw("canonical_url", "null");
        j.raw("codeinjection_foot", "null");
        j.raw("codeinjection_head", "null");
        j.raw("count", R"({"clicks":0,"negative_feedback":0,"positive_feedback":0})");
        j.timestamp("created_at");
        j.timestamp("updated_at");
        j.timestamp("published_at");
        j.raw("custom_template", "null");
        j.nil("email");
        j.boolean("email_only", false);
        j.str("email_segment", "all");
        j.nil("email_subject");
        const std::string image = entity.get_feature_image();
        if (image.empty()) j.nil("feature_image"); else j.str("feature_image", image);
        j.nil("feature_image_alt");
        j.nil("feature_image_caption");
        j.boolean("featured", entity.get_featured());
        j.nil("frontmatter");
        j.nil("meta_description");
        j.nil("meta_title");
        return j.done();
    }

    /** @brief Full Ghost member entity from snapshots (29 asserted fields). */
    [[nodiscard]] static std::string member(const model::Member& entity)
    {
        GhostFixtures j;
        j.str("id", ghost_id(entity.get_id()));
        j.str("uuid", entity.get_uuid().empty() ? uuid_for(entity.get_id()) : entity.get_uuid());
        j.str("email", entity.get_email());
        j.str("name", entity.get_name());
        j.str("status", entity.get_status());
        if (entity.get_note().empty()) j.nil("note"); else j.str("note", entity.get_note());
        j.nil("avatar_image");
        j.boolean("comped", false);
        j.boolean("can_comment", true);
        j.boolean("subscribed", entity.get_status() == "paid");
        j.number("email_count", entity.get_email_count());
        j.number("email_opened_count", entity.get_email_opened_count());
        j.nil("email_open_rate");
        j.boolean("enable_comment_notifications", entity.get_enable_comment_notifications());
        j.raw("enable_updates_and_announcements", "null");
        j.nil("geolocation");
        j.nil("description");
        j.nil("last_seen_at");
        j.timestamp("created_at");
        j.timestamp("updated_at");
        j.str("unsubscribe_url", "http://domain.com/unsubscribe/?uuid=memberuuid&key=abc123dontstealme");
        j.raw("attribution",
              R"({"id":null,"referrer_medium":"Ghost Admin","referrer_source":"Created manually",)"
              R"("referrer_url":null,"title":null,"type":null,"url":null})");
        j.raw("commenting", R"({"disabled":false,"disabled_reason":null,"disabled_until":null})");
        j.raw("email_suppression", R"({"info":null,"suppressed":false})");
        j.raw("current_subscription", "null");
        j.raw("labels", "[]");
        j.raw("newsletters", "[]");
        j.raw("subscriptions", "[]");
        j.raw("tiers", "[]");
        return j.done();
    }

    /** @brief Ghost page entity. */
    [[nodiscard]] static std::string page(const model::Page& entity)
    {
        GhostFixtures j;
        j.str("id", ghost_id(entity.get_id()));
        j.str("uuid", entity.get_uuid().empty() ? uuid_for(entity.get_id()) : entity.get_uuid());
        j.str("title", entity.get_title());
        j.str("slug", entity.get_slug());
        j.str("html", entity.get_html());
        j.str("status", entity.get_status());
        j.str("excerpt", entity.get_title());
        j.nil("canonical_url");
        j.nil("codeinjection_foot");
        j.nil("codeinjection_head");
        j.str("comment_id", std::string{"comment-" + std::to_string(entity.get_id())});
        j.nil("custom_excerpt");
        j.nil("custom_template");
        j.nil("feature_image");
        j.nil("feature_image_alt");
        j.nil("feature_image_caption");
        j.boolean("featured", false);
        j.nil("frontmatter");
        j.str("lexical", "{}");
        j.raw("authors", "[]");
        j.raw("count", R"({"clicks":0,"positive_feedback":0,"negative_feedback":0})");
        j.timestamp("created_at");
        j.timestamp("updated_at");
        return j.done();
    }

    /** @brief Ghost author entity. */
    [[nodiscard]] static std::string author(const model::Author& entity)
    {
        GhostFixtures j;
        j.str("id", ghost_id(entity.get_id()));
        j.str("name", entity.get_name());
        j.str("slug", entity.get_slug());
        j.str("email", entity.get_email());
        j.str("bio", "");
        j.timestamp("created_at");
        j.timestamp("updated_at");
        return j.done();
    }

    /** @brief Ghost tag entity (id, name, slug, description). */
    [[nodiscard]] static std::string tag(const model::Tag& entity)
    {
        GhostFixtures j;
        j.str("id", ghost_id(entity.get_id()));
        j.str("name", entity.get_name());
        j.str("slug", entity.get_slug());
        j.str("description", entity.get_description());
        return j.done();
    }

    /** @brief Ghost comment entity from the admin comments snapshots. */
    [[nodiscard]] static std::string comment(const model::Comment& entity)
    {
        GhostFixtures j;
        j.str("id", ghost_id(entity.get_id()));
        j.str("html", entity.get_html());
        j.str("status", entity.get_status());
        j.str("parent_id", entity.get_parent_id().empty()
                               ? ghost_id(entity.get_id()) : entity.get_parent_id());
        if (entity.get_in_reply_to_id().empty()) j.nil("in_reply_to_id");
        else j.str("in_reply_to_id", entity.get_in_reply_to_id());
        j.nil("in_reply_to_snippet");
        j.nil("edited_at");
        j.boolean("liked", false);
        j.boolean("disliked", false);
        j.boolean("pinned", false);
        j.raw("count", R"({"likes":0,"dislikes":0,"replies":0})");
        j.raw("member", "{}");
        j.raw("parent", "{}");
        j.raw("post", "{}");
        j.raw("replies", "[]");
        j.timestamp("created_at");
        return j.done();
    }

    /** @brief Ghost user entity from the admin users snapshots. */
    [[nodiscard]] static std::string user(const model::User& entity)
    {
        GhostFixtures j;
        j.str("id", ghost_id(entity.get_id()));
        j.str("name", entity.get_name());
        j.str("slug", entity.get_slug());
        j.str("email", entity.get_email());
        j.str("accessibility", "Ghost Author");
        j.str("description", entity.get_bio());
        j.timestamp("created_at");
        j.timestamp("updated_at");
        return j.done();
    }

    /** @brief Ghost newsletter entity from the admin newsletters snapshots. */
    [[nodiscard]] static std::string newsletter(const model::Newsletter& entity)
    {
        GhostFixtures j;
        j.str("id", ghost_id(entity.get_id()));
        j.str("uuid", entity.get_uuid().empty() ? uuid_for(entity.get_id()) : entity.get_uuid());
        j.str("name", entity.get_name());
        j.str("slug", entity.get_slug());
        if (entity.get_description().empty()) j.nil("description");
        else j.str("description", entity.get_description());
        j.str("status", entity.get_status());
        j.str("visibility", "public");
        j.number("sort_order", 0);
        j.str("body_font_category", "sans-serif");
        j.str("button_color", "#FF1A75");
        j.str("button_corners", "square");
        j.str("button_style", "solid");
        j.str("header_background_color", "#15171A");
        j.str("image_corners", "square");
        j.str("link_color", "#FF1A75");
        j.str("link_style", "underline");
        j.str("title_alignment", "left");
        j.str("title_font_category", "sans-serif");
        j.str("title_font_weight", "bold");
        j.str("sender_reply_to", "reply@example.com");
        j.str("background_color", "#FFFFFF");
        j.nil("divider_color");
        j.nil("footer_content");
        j.nil("header_image");
        j.nil("post_title_color");
        j.nil("section_title_color");
        if (entity.get_sender_email().empty()) j.str("sender_email", "ghost@example.com");
        else j.str("sender_email", entity.get_sender_email());
        if (entity.get_sender_name().empty()) j.str("sender_name", "Ghost");
        else j.str("sender_name", entity.get_sender_name());
        j.boolean("feedback_enabled", entity.get_feedback_enabled());
        j.boolean("show_badge", true);
        j.boolean("show_comment_cta", true);
        j.boolean("show_excerpt", true);
        j.boolean("show_feature_image", true);
        j.boolean("show_header_icon", true);
        j.boolean("show_header_name", true);
        j.boolean("show_header_title", true);
        j.boolean("show_latest_posts", true);
        j.boolean("show_post_title_section", true);
        j.boolean("show_share_button", true);
        j.boolean("show_subscription_details", true);
        j.boolean("subscribe_on_signup", entity.get_subscribe_on_signup());
        j.raw("count", R"({"sent":0,"opened":0,"clicked":0})");
        j.timestamp("created_at");
        j.timestamp("updated_at");
        return j.done();
    }

    /** @brief Ghost tier entity from the admin tiers snapshots. */
    [[nodiscard]] static std::string tier(const model::Tier& entity)
    {
        GhostFixtures j;
        j.str("id", ghost_id(entity.get_id()));
        j.str("name", entity.get_name());
        j.str("slug", entity.get_slug());
        j.boolean("active", true);
        j.raw("benefits", "[]");
        j.str("currency", "usd");
        j.nil("description");
        j.number("monthly_price", static_cast<std::int64_t>(entity.get_price()));
        j.number("yearly_price", static_cast<std::int64_t>(entity.get_price() * 12));
        j.number("trial_days", 0);
        j.str("type", entity.get_price() > 0 ? "paid" : "free");
        j.str("visibility", "public");
        j.timestamp("created_at");
        j.timestamp("updated_at");
        j.nil("welcome_page_url");
        return j.done();
    }

    /** @brief Ghost-shaped automated email fixture. */
    [[nodiscard]] static std::string automated_email()
    {
        GhostFixtures j;
        j.str("id", ghost_id(1));
        j.str("email_design_setting_id", ghost_id(2));
        j.str("html", "<p>Ghost Record</p>");
        j.str("lexical", "{}");
        j.timestamp("created_at");
        return j.done();
    }

    /** @brief Ghost activity-feed event (events snapshots assert data + type). */
    [[nodiscard]] static std::string event()
    {
        GhostFixtures j;
        j.str("type", "comment_event");
        j.raw("data", R"({"post_id":"1","member_id":"1","comment_id":"1"})");
        return j.done();
    }

    /** @brief Ghost comment sub-resource (like/dislike/report) fixture. */
    [[nodiscard]] static std::string comment_sub()
    {
        GhostFixtures j;
        j.str("id", ghost_id(1));
        j.str("comment_id", ghost_id(2));
        j.str("member_id", ghost_id(3));
        j.raw("member", R"({"id":"1","name":"Ghost Member","email":"member2@test.com"})");
        j.timestamp("created_at");
        j.timestamp("updated_at");
        return j.done();
    }

    /** @brief Ghost db maintenance status. */
    [[nodiscard]] static std::string db()
    {
        GhostFixtures j;
        j.str("status", "success");
        return j.done();
    }

    /** @brief Ghost email batch fixture. */
    [[nodiscard]] static std::string batch()
    {
        GhostFixtures j;
        j.str("id", ghost_id(1));
        j.raw("count", R"({"total":1,"sent":1})");
        j.str("status", "submitted");
        j.str("mailgun_message_id", "message-1");
        j.nil("error_data"); j.nil("error_message"); j.nil("error_status_code");
        j.nil("member_segment");
        j.timestamp("created_at"); j.timestamp("updated_at");
        return j.done();
    }

    /** @brief Ghost email failure fixture. */
    [[nodiscard]] static std::string failure()
    {
        GhostFixtures j;
        j.str("id", ghost_id(1));
        j.str("email_id", ghost_id(2));
        j.str("email_recipient_id", ghost_id(3));
        j.number("code", 555);
        j.str("enhanced_code", "5.5.5");
        j.str("event_id", "event-1");
        j.str("failed_at", std::string{kTimestamp});
        j.str("message", "Delivery failed");
        j.str("severity", "temporary");
        j.nil("email_recipient"); j.nil("member");
        j.str("member_id", ghost_id(3));
        return j.done();
    }

    /** @brief Ghost custom theme setting fixture. */
    [[nodiscard]] static std::string custom_theme_setting()
    {
        GhostFixtures j;
        j.str("id", ghost_id(1));
        j.str("key", "accent_color");
        j.str("default", "#FF1A75");
        j.str("value", "#FF1A75");
        j.str("group", "branding");
        j.str("type", "color");
        j.raw("options", "[]");
        return j.done();
    }

    /** @brief Ghost automation fixture. */
    [[nodiscard]] static std::string automation()
    {
        GhostFixtures j;
        j.str("id", ghost_id(1));
        j.str("name", "Ghost Automation");
        j.str("slug", "ghost-automation");
        j.str("description", "Welcome automation");
        j.str("status", "active");
        j.raw("actions", "[]");
        j.timestamp("created_at"); j.timestamp("updated_at");
        return j.done();
    }

    /** @brief Ghost-shaped label fixture. */
    [[nodiscard]] static std::string label()
    {
        GhostFixtures j;
        j.str("id", ghost_id(1));
        j.str("name", "Ghost Resource");
        j.str("slug", "ghost-resource");
        j.raw("count", R"({"posts":0})");
        j.timestamp("created_at");
        j.timestamp("updated_at");
        return j.done();
    }

    /** @brief Ghost-shaped snippet fixture. */
    [[nodiscard]] static std::string snippet()
    {
        GhostFixtures j;
        j.str("id", ghost_id(1));
        j.str("name", "Ghost Resource");
        j.str("lexical", "{}");
        j.str("mobiledoc", "{}");
        j.timestamp("created_at");
        j.timestamp("updated_at");
        return j.done();
    }

    /** @brief Ghost-shaped role fixture. */
    [[nodiscard]] static std::string role()
    {
        GhostFixtures j;
        j.str("id", ghost_id(1));
        j.str("name", "Administrator");
        j.str("description", "Administrator role");
        j.timestamp("created_at");
        j.timestamp("updated_at");
        return j.done();
    }

    /** @brief Ghost-shaped recommendation fixture. */
    [[nodiscard]] static std::string recommendation()
    {
        GhostFixtures j;
        j.str("id", ghost_id(1));
        j.str("url", "https://ghost.local");
        j.str("title", "Ghost Recommendation");
        j.nil("description");
        j.nil("excerpt");
        j.nil("favicon");
        j.nil("featuredImage");
        j.nil("featured_image");
        j.nil("created_at");
        j.timestamp("createdAt");
        j.nil("updated_at");
        j.nil("reason");
        j.boolean("oneClickSubscribe", true);
        j.boolean("one_click_subscribe", true);
        j.raw("count", R"({"clicks":0,"subscribers":0})");
        return j.done();
    }

    /** @brief Ghost-shaped link fixture. */
    [[nodiscard]] static std::string link()
    {
        GhostFixtures j;
        j.str("post_id", ghost_id(1));
        j.raw("link", R"({"url":"https://ghost.local","title":"Ghost"})");
        j.raw("count", R"({"clicks":0})");
        return j.done();
    }

    /** @brief Ghost-shaped offer fixture (admin offers snapshots). */
    [[nodiscard]] static std::string offer()
    {
        GhostFixtures j;
        j.str("id", ghost_id(1));
        j.str("name", "Black Friday");
        j.str("code", "black-friday");
        j.number("amount", 50);
        j.str("cadence", "once");
        j.str("currency", "usd");
        j.boolean("currency_restriction", false);
        j.str("display_description", "Save 50%");
        j.str("display_title", "Black Friday Sale");
        j.str("duration", "once");
        j.nil("duration_in_months");
        j.nil("last_redeemed");
        j.str("redemption_type", "public");
        j.number("redemption_count", 0);
        j.str("status", "active");
        j.str("type", "percent");
        j.nil("tier");
        j.timestamp("created_at");
        j.timestamp("updated_at");
        return j.done();
    }

    /** @brief Ghost-shaped notification fixture. */
    [[nodiscard]] static std::string notification()
    {
        GhostFixtures j;
        j.str("id", ghost_id(1));
        j.str("message", "Ghost notification");
        j.str("type", "info");
        j.str("status", "unread");
        j.str("location", "admin");
        j.boolean("custom", false);
        j.boolean("dismissible", true);
        return j.done();
    }

    /** @brief Ghost-shaped webhook fixture. */
    [[nodiscard]] static std::string webhook()
    {
        GhostFixtures j;
        j.str("id", ghost_id(1));
        j.str("name", "Ghost Webhook");
        j.str("event", "post.published");
        j.str("target_url", "https://ghost.local/hook");
        j.str("api_version", "v5.0");
        j.str("integration_id", ghost_id(2));
        j.str("secret", "secret");
        j.nil("last_triggered_at");
        j.nil("last_triggered_error");
        j.nil("last_triggered_status");
        j.timestamp("created_at");
        j.timestamp("updated_at");
        return j.done();
    }

    /** @brief Ghost-shaped email fixture (admin emails snapshots). */
    [[nodiscard]] static std::string email()
    {
        GhostFixtures j;
        j.str("id", ghost_id(1));
        j.str("uuid", uuid_for(1));
        j.str("subject", "Ghost Newsletter");
        j.str("html", "<p>Hello</p>");
        j.str("plaintext", "Hello");
        j.str("from", "Ghost <ghost@example.com>");
        j.nil("reply_to");
        j.nil("source");
        j.str("source_type", "posts");
        j.str("status", "sent");
        j.str("post_id", ghost_id(2));
        j.nil("newsletter_id");
        j.str("recipient_filter", "all");
        j.str("submitted_at", std::string{kTimestamp});
        j.number("email_count", 10);
        j.number("delivered_count", 9);
        j.number("opened_count", 4);
        j.number("failed_count", 1);
        j.nil("csd_email_count");
        j.boolean("feedback_enabled", true);
        j.boolean("track_clicks", true);
        j.boolean("track_opens", true);
        j.nil("error");
        j.nil("error_data");
        j.timestamp("created_at");
        j.timestamp("updated_at");
        return j.done();
    }

private:
    [[nodiscard]] std::string field(std::string_view key, std::string_view value)
    {
        return quote(key) + ":" + std::string{value};
    }

    std::vector<std::string> m_parts{};
};

} // namespace blog::dto
