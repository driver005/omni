export module blog_testing:ghost_schema;

import std;

export namespace blog::testing {

/** @brief Minimal JSON tree used by the structural snapshot oracle. */
struct JsonNode
{
    enum class Kind { Null, Bool, Number, String, Array, Object };
    Kind kind{Kind::Null};
    std::string scalar{};
    std::vector<JsonNode> array{};
    std::map<std::string, JsonNode> object{};
    [[nodiscard]] bool is(Kind k) const noexcept { return kind == k; }
};

/** @brief Dependency-free recursive-descent JSON parser for response bodies. */
class JsonParser
{
public:
    [[nodiscard]] std::optional<JsonNode> parse(std::string_view text) const
    {
        m_text = text; m_pos = 0;
        skip();
        auto node = value();
        if (!node) return std::nullopt;
        return node;
    }
private:
    void skip() const
    {
        while (m_pos < m_text.size() && std::isspace(static_cast<unsigned char>(m_text[m_pos]))) ++m_pos;
    }
    [[nodiscard]] bool eat(char c) const
    {
        skip();
        if (m_pos < m_text.size() && m_text[m_pos] == c) { ++m_pos; return true; }
        return false;
    }
    [[nodiscard]] std::optional<JsonNode> value() const
    {
        skip();
        if (m_pos >= m_text.size()) return std::nullopt;
        const char c = m_text[m_pos];
        if (c == '{') return object_value();
        if (c == '[') return array_value();
        if (c == '"') {
            JsonNode n; n.kind = JsonNode::Kind::String;
            auto s = string_value();
            if (!s) return std::nullopt;
            n.scalar = *s;
            return n;
        }
        if (c == 't' || c == 'f') {
            JsonNode n; n.kind = JsonNode::Kind::Bool;
            if (m_text.substr(m_pos, 4) == "true") { n.scalar = "true"; m_pos += 4; }
            else if (m_text.substr(m_pos, 5) == "false") { n.scalar = "false"; m_pos += 5; }
            else return std::nullopt;
            return n;
        }
        if (c == 'n') {
            if (m_text.substr(m_pos, 4) != "null") return std::nullopt;
            m_pos += 4;
            return JsonNode{};
        }
        if (c == '-' || std::isdigit(static_cast<unsigned char>(c))) {
            JsonNode n; n.kind = JsonNode::Kind::Number;
            const auto start = m_pos;
            if (m_text[m_pos] == '-') ++m_pos;
            while (m_pos < m_text.size() && (std::isdigit(static_cast<unsigned char>(m_text[m_pos])) ||
                   m_text[m_pos] == '.' || m_text[m_pos] == 'e' || m_text[m_pos] == 'E' ||
                   m_text[m_pos] == '+' || m_text[m_pos] == '-')) ++m_pos;
            n.scalar = std::string{m_text.substr(start, m_pos - start)};
            return n;
        }
        return std::nullopt;
    }
    [[nodiscard]] std::optional<std::string> string_value() const
    {
        if (!eat('"')) return std::nullopt;
        std::string out;
        while (m_pos < m_text.size()) {
            const char c = m_text[m_pos++];
            if (c == '"') return out;
            if (c == '\\' && m_pos < m_text.size()) {
                const char e = m_text[m_pos++];
                switch (e) {
                    case '"': out += '"'; break;
                    case '\\': out += '\\'; break;
                    case 'n': out += '\n'; break;
                    case 'r': out += '\r'; break;
                    case 't': out += '\t'; break;
                    default: out += e;
                }
            } else out += c;
        }
        return std::nullopt;
    }
    [[nodiscard]] std::optional<JsonNode> object_value() const
    {
        if (!eat('{')) return std::nullopt;
        JsonNode n; n.kind = JsonNode::Kind::Object;
        if (eat('}')) return n;
        for (;;) {
            auto key = string_value();
            if (!key) return std::nullopt;
            if (!eat(':')) return std::nullopt;
            auto val = value();
            if (!val) return std::nullopt;
            n.object[*key] = *val;
            if (eat('}')) break;
            if (!eat(',')) return std::nullopt;
        }
        return n;
    }
    [[nodiscard]] std::optional<JsonNode> array_value() const
    {
        if (!eat('[')) return std::nullopt;
        JsonNode n; n.kind = JsonNode::Kind::Array;
        if (eat(']')) return n;
        for (;;) {
            auto val = value();
            if (!val) return std::nullopt;
            n.array.push_back(*val);
            if (eat(']')) break;
            if (!eat(',')) return std::nullopt;
        }
        return n;
    }
    mutable std::string_view m_text{};
    mutable std::size_t m_pos{0};
};

/** @brief One Ghost snapshot field and the JSON types its matchers allow. */
struct FieldSpec
{
    std::string_view name;
    std::array<std::string_view, 6> types;
    std::size_t type_count;
};

struct EntitySchema
{
    std::string_view collection;
    std::vector<FieldSpec> fields;
    bool root_object{false};
};

/** @brief Schemas extracted from the Ghost e2e-api snapshot suites. */
inline const EntitySchema kPostSchema{
    "posts",
    std::vector<FieldSpec>{
        FieldSpec{"authors", {"any"}, 1},
        FieldSpec{"canonical_url", {"null"}, 1},
        FieldSpec{"codeinjection_foot", {"null"}, 1},
        FieldSpec{"codeinjection_head", {"null"}, 1},
        FieldSpec{"comment_id", {"string"}, 1},
        FieldSpec{"count", {"object"}, 1},
        FieldSpec{"created_at", {"string"}, 1},
        FieldSpec{"custom_excerpt", {"null","string"}, 2},
        FieldSpec{"custom_template", {"null"}, 1},
        FieldSpec{"email", {"null"}, 1},
        FieldSpec{"email_only", {"boolean"}, 1},
        FieldSpec{"email_segment", {"string"}, 1},
        FieldSpec{"email_subject", {"null"}, 1},
        FieldSpec{"excerpt", {"null","string"}, 2},
        FieldSpec{"feature_image", {"null","string"}, 2},
        FieldSpec{"feature_image_alt", {"null"}, 1},
        FieldSpec{"feature_image_caption", {"null"}, 1},
        FieldSpec{"featured", {"boolean"}, 1},
        FieldSpec{"frontmatter", {"null"}, 1},
        FieldSpec{"html", {"string"}, 1},
        FieldSpec{"id", {"string"}, 1},
        FieldSpec{"lexical", {"null","string"}, 2},
        FieldSpec{"meta_description", {"null"}, 1},
        FieldSpec{"meta_title", {"null"}, 1},
        FieldSpec{"mobiledoc", {"string"}, 1},
    },
    false
};

inline const EntitySchema kMemberSchema{
    "members",
    std::vector<FieldSpec>{
        FieldSpec{"attribution", {"object"}, 1},
        FieldSpec{"avatar_image", {"null"}, 1},
        FieldSpec{"can_comment", {"boolean"}, 1},
        FieldSpec{"commenting", {"object"}, 1},
        FieldSpec{"comped", {"boolean"}, 1},
        FieldSpec{"created_at", {"string"}, 1},
        FieldSpec{"current_subscription", {"null"}, 1},
        FieldSpec{"description", {"null"}, 1},
        FieldSpec{"email", {"string"}, 1},
        FieldSpec{"email_count", {"number"}, 1},
        FieldSpec{"email_open_rate", {"null"}, 1},
        FieldSpec{"email_opened_count", {"number"}, 1},
        FieldSpec{"email_suppression", {"object"}, 1},
        FieldSpec{"enable_comment_notifications", {"boolean"}, 1},
        FieldSpec{"enable_updates_and_announcements", {"null"}, 1},
        FieldSpec{"geolocation", {"null"}, 1},
        FieldSpec{"id", {"string"}, 1},
        FieldSpec{"labels", {"array"}, 1},
        FieldSpec{"last_seen_at", {"null"}, 1},
        FieldSpec{"name", {"string"}, 1},
        FieldSpec{"newsletters", {"array"}, 1},
        FieldSpec{"note", {"null"}, 1},
        FieldSpec{"status", {"string"}, 1},
        FieldSpec{"subscribed", {"boolean"}, 1},
        FieldSpec{"subscriptions", {"array"}, 1},
        FieldSpec{"tiers", {"array"}, 1},
        FieldSpec{"unsubscribe_url", {"string"}, 1},
        FieldSpec{"updated_at", {"string"}, 1},
        FieldSpec{"uuid", {"string"}, 1},
    },
    false
};

inline const EntitySchema kUserSchema{
    "users",
    std::vector<FieldSpec>{
        FieldSpec{"accessibility", {"string"}, 1},
        FieldSpec{"created_at", {"string"}, 1},
        FieldSpec{"description", {"string"}, 1},
        FieldSpec{"id", {"string"}, 1},
        FieldSpec{"name", {"string"}, 1},
        FieldSpec{"updated_at", {"string"}, 1},
    },
    false
};

inline const EntitySchema kNewsletterSchema{
    "newsletters",
    std::vector<FieldSpec>{
        FieldSpec{"background_color", {"string"}, 1},
        FieldSpec{"body_font_category", {"string"}, 1},
        FieldSpec{"button_color", {"string"}, 1},
        FieldSpec{"button_corners", {"string"}, 1},
        FieldSpec{"button_style", {"string"}, 1},
        FieldSpec{"count", {"object"}, 1},
        FieldSpec{"created_at", {"string"}, 1},
        FieldSpec{"description", {"null","string"}, 2},
        FieldSpec{"divider_color", {"null"}, 1},
        FieldSpec{"feedback_enabled", {"boolean"}, 1},
        FieldSpec{"footer_content", {"null"}, 1},
        FieldSpec{"header_background_color", {"string"}, 1},
        FieldSpec{"header_image", {"null","string"}, 2},
        FieldSpec{"id", {"string"}, 1},
        FieldSpec{"image_corners", {"string"}, 1},
        FieldSpec{"link_color", {"string"}, 1},
        FieldSpec{"link_style", {"string"}, 1},
        FieldSpec{"name", {"string"}, 1},
        FieldSpec{"post_title_color", {"null"}, 1},
        FieldSpec{"section_title_color", {"null"}, 1},
        FieldSpec{"sender_email", {"null","string"}, 2},
        FieldSpec{"sender_name", {"null","string"}, 2},
        FieldSpec{"sender_reply_to", {"string"}, 1},
        FieldSpec{"show_badge", {"boolean"}, 1},
        FieldSpec{"show_comment_cta", {"boolean"}, 1},
        FieldSpec{"show_excerpt", {"boolean"}, 1},
        FieldSpec{"show_feature_image", {"boolean"}, 1},
        FieldSpec{"show_header_icon", {"boolean"}, 1},
        FieldSpec{"show_header_name", {"boolean"}, 1},
        FieldSpec{"show_header_title", {"boolean"}, 1},
        FieldSpec{"show_latest_posts", {"boolean"}, 1},
        FieldSpec{"show_post_title_section", {"boolean"}, 1},
        FieldSpec{"show_share_button", {"boolean"}, 1},
        FieldSpec{"show_subscription_details", {"boolean"}, 1},
        FieldSpec{"slug", {"string"}, 1},
        FieldSpec{"sort_order", {"number"}, 1},
        FieldSpec{"status", {"string"}, 1},
        FieldSpec{"subscribe_on_signup", {"boolean"}, 1},
        FieldSpec{"title_alignment", {"string"}, 1},
        FieldSpec{"title_font_category", {"string"}, 1},
        FieldSpec{"title_font_weight", {"string"}, 1},
        FieldSpec{"updated_at", {"string"}, 1},
        FieldSpec{"uuid", {"string"}, 1},
        FieldSpec{"visibility", {"string"}, 1},
    },
    false
};

inline const EntitySchema kTierSchema{
    "tiers",
    std::vector<FieldSpec>{
        FieldSpec{"active", {"boolean"}, 1},
        FieldSpec{"benefits", {"array"}, 1},
        FieldSpec{"created_at", {"string"}, 1},
        FieldSpec{"currency", {"string"}, 1},
        FieldSpec{"description", {"null","string"}, 2},
        FieldSpec{"id", {"string"}, 1},
        FieldSpec{"monthly_price", {"number"}, 1},
        FieldSpec{"name", {"string"}, 1},
        FieldSpec{"slug", {"string"}, 1},
        FieldSpec{"trial_days", {"number"}, 1},
        FieldSpec{"type", {"string"}, 1},
        FieldSpec{"updated_at", {"null","string"}, 2},
        FieldSpec{"visibility", {"string"}, 1},
        FieldSpec{"welcome_page_url", {"null","string"}, 2},
        FieldSpec{"yearly_price", {"number"}, 1},
    },
    false
};

inline const EntitySchema kCommentSchema{
    "comments",
    std::vector<FieldSpec>{
        FieldSpec{"count", {"object"}, 1},
        FieldSpec{"created_at", {"string"}, 1},
        FieldSpec{"disliked", {"boolean"}, 1},
        FieldSpec{"edited_at", {"null","string"}, 2},
        FieldSpec{"html", {"null","string"}, 2},
        FieldSpec{"id", {"string"}, 1},
        FieldSpec{"in_reply_to_id", {"null"}, 1},
        FieldSpec{"in_reply_to_snippet", {"null","string"}, 2},
        FieldSpec{"liked", {"any"}, 1},
        FieldSpec{"member", {"object"}, 1},
        FieldSpec{"parent", {"object"}, 1},
        FieldSpec{"parent_id", {"string"}, 1},
        FieldSpec{"pinned", {"any"}, 1},
        FieldSpec{"post", {"object"}, 1},
        FieldSpec{"replies", {"array"}, 1},
        FieldSpec{"status", {"string"}, 1},
    },
    false
};

inline const EntitySchema kOfferSchema{
    "offers",
    std::vector<FieldSpec>{
        FieldSpec{"amount", {"number"}, 1},
        FieldSpec{"cadence", {"string"}, 1},
        FieldSpec{"code", {"string"}, 1},
        FieldSpec{"created_at", {"string"}, 1},
        FieldSpec{"currency", {"null","string"}, 2},
        FieldSpec{"currency_restriction", {"boolean"}, 1},
        FieldSpec{"display_description", {"string"}, 1},
        FieldSpec{"display_title", {"string"}, 1},
        FieldSpec{"duration", {"string"}, 1},
        FieldSpec{"duration_in_months", {"null"}, 1},
        FieldSpec{"id", {"string"}, 1},
        FieldSpec{"last_redeemed", {"null"}, 1},
        FieldSpec{"name", {"string"}, 1},
        FieldSpec{"redemption_count", {"number"}, 1},
        FieldSpec{"redemption_type", {"string"}, 1},
        FieldSpec{"status", {"string"}, 1},
        FieldSpec{"tier", {"null","object"}, 2},
        FieldSpec{"type", {"string"}, 1},
    },
    false
};

inline const EntitySchema kNotificationSchema{
    "notifications",
    std::vector<FieldSpec>{
        FieldSpec{"custom", {"boolean"}, 1},
        FieldSpec{"dismissible", {"boolean"}, 1},
        FieldSpec{"id", {"string"}, 1},
        FieldSpec{"location", {"string"}, 1},
        FieldSpec{"message", {"string"}, 1},
        FieldSpec{"status", {"string"}, 1},
        FieldSpec{"type", {"string"}, 1},
    },
    false
};

inline const EntitySchema kWebhookSchema{
    "webhooks",
    std::vector<FieldSpec>{
        FieldSpec{"api_version", {"string"}, 1},
        FieldSpec{"created_at", {"string"}, 1},
        FieldSpec{"event", {"string"}, 1},
        FieldSpec{"id", {"string"}, 1},
        FieldSpec{"integration_id", {"string"}, 1},
        FieldSpec{"last_triggered_at", {"null"}, 1},
        FieldSpec{"last_triggered_error", {"null"}, 1},
        FieldSpec{"last_triggered_status", {"null"}, 1},
        FieldSpec{"name", {"string"}, 1},
        FieldSpec{"secret", {"string"}, 1},
        FieldSpec{"target_url", {"string"}, 1},
        FieldSpec{"updated_at", {"string"}, 1},
    },
    false
};

inline const EntitySchema kEmailSchema{
    "emails",
    std::vector<FieldSpec>{
        FieldSpec{"created_at", {"string"}, 1},
        FieldSpec{"csd_email_count", {"null"}, 1},
        FieldSpec{"delivered_count", {"number"}, 1},
        FieldSpec{"email_count", {"number"}, 1},
        FieldSpec{"error", {"null","string"}, 2},
        FieldSpec{"error_data", {"null"}, 1},
        FieldSpec{"failed_count", {"number"}, 1},
        FieldSpec{"feedback_enabled", {"boolean"}, 1},
        FieldSpec{"from", {"null","string"}, 2},
        FieldSpec{"html", {"string"}, 1},
        FieldSpec{"id", {"string"}, 1},
        FieldSpec{"newsletter_id", {"null"}, 1},
        FieldSpec{"opened_count", {"number"}, 1},
        FieldSpec{"plaintext", {"string"}, 1},
        FieldSpec{"post_id", {"string"}, 1},
        FieldSpec{"recipient_filter", {"string"}, 1},
        FieldSpec{"reply_to", {"null"}, 1},
        FieldSpec{"source", {"null"}, 1},
        FieldSpec{"source_type", {"string"}, 1},
        FieldSpec{"status", {"string"}, 1},
        FieldSpec{"subject", {"string"}, 1},
        FieldSpec{"submitted_at", {"string"}, 1},
        FieldSpec{"track_clicks", {"boolean"}, 1},
        FieldSpec{"track_opens", {"boolean"}, 1},
        FieldSpec{"updated_at", {"string"}, 1},
        FieldSpec{"uuid", {"string"}, 1},
    },
    false
};

inline const EntitySchema kLabelSchema{
    "labels",
    std::vector<FieldSpec>{
        FieldSpec{"count", {"object"}, 1},
        FieldSpec{"created_at", {"string"}, 1},
        FieldSpec{"id", {"string"}, 1},
        FieldSpec{"name", {"string"}, 1},
        FieldSpec{"slug", {"string"}, 1},
        FieldSpec{"updated_at", {"string"}, 1},
    },
    false
};

inline const EntitySchema kSnippetSchema{
    "snippets",
    std::vector<FieldSpec>{
        FieldSpec{"created_at", {"string"}, 1},
        FieldSpec{"id", {"string"}, 1},
        FieldSpec{"lexical", {"null","string"}, 2},
        FieldSpec{"mobiledoc", {"string"}, 1},
        FieldSpec{"name", {"string"}, 1},
        FieldSpec{"updated_at", {"string"}, 1},
    },
    false
};

inline const EntitySchema kRoleSchema{
    "roles",
    std::vector<FieldSpec>{
        FieldSpec{"created_at", {"string"}, 1},
        FieldSpec{"description", {"string"}, 1},
        FieldSpec{"id", {"string"}, 1},
        FieldSpec{"name", {"string"}, 1},
        FieldSpec{"updated_at", {"string"}, 1},
    },
    false
};

inline const EntitySchema kRecommendationSchema{
    "recommendations",
    std::vector<FieldSpec>{
        FieldSpec{"count", {"object"}, 1},
        FieldSpec{"createdAt", {"string"}, 1},
        FieldSpec{"created_at", {"null","string"}, 2},
        FieldSpec{"description", {"null","string"}, 2},
        FieldSpec{"excerpt", {"null","string"}, 2},
        FieldSpec{"favicon", {"null","string"}, 2},
        FieldSpec{"featuredImage", {"null"}, 1},
        FieldSpec{"featured_image", {"null","string"}, 2},
        FieldSpec{"id", {"null","string"}, 2},
        FieldSpec{"oneClickSubscribe", {"boolean"}, 1},
        FieldSpec{"one_click_subscribe", {"boolean"}, 1},
        FieldSpec{"reason", {"null","string"}, 2},
        FieldSpec{"title", {"null","string"}, 2},
        FieldSpec{"updated_at", {"null","string"}, 2},
        FieldSpec{"url", {"string"}, 1},
    },
    false
};

inline const EntitySchema kLinkSchema{
    "links",
    std::vector<FieldSpec>{
        FieldSpec{"count", {"object"}, 1},
        FieldSpec{"link", {"object"}, 1},
        FieldSpec{"post_id", {"string"}, 1},
    },
    false
};

inline const EntitySchema kAutomatedEmailSchema{
    "automated_emails",
    std::vector<FieldSpec>{
        FieldSpec{"created_at", {"string"}, 1},
        FieldSpec{"email_design_setting_id", {"string"}, 1},
        FieldSpec{"html", {"string"}, 1},
        FieldSpec{"id", {"string"}, 1},
        FieldSpec{"lexical", {"string"}, 1},
    },
    false
};

inline const EntitySchema kEventSchema{
    "events",
    std::vector<FieldSpec>{
        FieldSpec{"data", {"object"}, 1},
        FieldSpec{"type", {"string"}, 1},
    },
    false
};

inline const EntitySchema kDbSchema{
    "db",
    std::vector<FieldSpec>{
        FieldSpec{"status", {"string"}, 1},
    },
    false
};

inline const EntitySchema kBulkSchema{
    "bulk",
    std::vector<FieldSpec>{
        FieldSpec{"meta", {"object"}, 1},
    },
    false
};

inline const EntitySchema kPagesSchema{
    "pages",
    std::vector<FieldSpec>{
        FieldSpec{"authors", {"any"}, 1},
        FieldSpec{"canonical_url", {"null"}, 1},
        FieldSpec{"codeinjection_foot", {"null"}, 1},
        FieldSpec{"codeinjection_head", {"null"}, 1},
        FieldSpec{"comment_id", {"string"}, 1},
        FieldSpec{"count", {"object"}, 1},
        FieldSpec{"created_at", {"string"}, 1},
        FieldSpec{"custom_excerpt", {"null"}, 1},
        FieldSpec{"custom_template", {"null"}, 1},
        FieldSpec{"excerpt", {"null","string"}, 2},
        FieldSpec{"feature_image", {"null"}, 1},
        FieldSpec{"feature_image_alt", {"null"}, 1},
        FieldSpec{"feature_image_caption", {"null"}, 1},
        FieldSpec{"featured", {"boolean"}, 1},
        FieldSpec{"frontmatter", {"null"}, 1},
        FieldSpec{"html", {"null","string"}, 2},
        FieldSpec{"id", {"string"}, 1},
        FieldSpec{"lexical", {"string"}, 1},
    },
    false
};

inline const EntitySchema kMemberObjectSchema{
    "members",
    std::vector<FieldSpec>{
        FieldSpec{"attribution", {"object"}, 1},
        FieldSpec{"avatar_image", {"null"}, 1},
        FieldSpec{"can_comment", {"boolean"}, 1},
        FieldSpec{"commenting", {"object"}, 1},
        FieldSpec{"comped", {"boolean"}, 1},
        FieldSpec{"created_at", {"string"}, 1},
        FieldSpec{"current_subscription", {"null"}, 1},
        FieldSpec{"description", {"null"}, 1},
        FieldSpec{"email", {"string"}, 1},
        FieldSpec{"email_count", {"number"}, 1},
        FieldSpec{"email_open_rate", {"null"}, 1},
        FieldSpec{"email_opened_count", {"number"}, 1},
        FieldSpec{"email_suppression", {"object"}, 1},
        FieldSpec{"enable_comment_notifications", {"boolean"}, 1},
        FieldSpec{"enable_updates_and_announcements", {"null"}, 1},
        FieldSpec{"geolocation", {"null"}, 1},
        FieldSpec{"id", {"string"}, 1},
        FieldSpec{"labels", {"array"}, 1},
        FieldSpec{"last_seen_at", {"null"}, 1},
        FieldSpec{"name", {"string"}, 1},
        FieldSpec{"newsletters", {"array"}, 1},
        FieldSpec{"note", {"null"}, 1},
        FieldSpec{"status", {"string"}, 1},
        FieldSpec{"subscribed", {"boolean"}, 1},
        FieldSpec{"subscriptions", {"array"}, 1},
        FieldSpec{"tiers", {"array"}, 1},
        FieldSpec{"unsubscribe_url", {"string"}, 1},
        FieldSpec{"updated_at", {"string"}, 1},
        FieldSpec{"uuid", {"string"}, 1},
    },
    true
};

inline const EntitySchema kStats_mrrSchema{
    "stats",
    std::vector<FieldSpec>{
        FieldSpec{"currency", {"string"}, 1},
        FieldSpec{"date", {"string"}, 1},
        FieldSpec{"mrr", {"number"}, 1},
    },
    false
};

inline const EntitySchema kStats_membersSchema{
    "stats",
    std::vector<FieldSpec>{
        FieldSpec{"comped", {"number"}, 1},
        FieldSpec{"date", {"string"}, 1},
        FieldSpec{"free", {"number"}, 1},
        FieldSpec{"gift", {"number"}, 1},
        FieldSpec{"paid", {"number"}, 1},
        FieldSpec{"paid_canceled", {"number"}, 1},
        FieldSpec{"paid_subscribed", {"number"}, 1},
    },
    false
};

inline const EntitySchema kStats_subscriptionsSchema{
    "stats",
    std::vector<FieldSpec>{
        FieldSpec{"cadence", {"string"}, 1},
        FieldSpec{"cancellations", {"number"}, 1},
        FieldSpec{"count", {"number"}, 1},
        FieldSpec{"date", {"string"}, 1},
        FieldSpec{"negative_delta", {"number"}, 1},
        FieldSpec{"positive_delta", {"number"}, 1},
        FieldSpec{"signups", {"number"}, 1},
        FieldSpec{"tier", {"string"}, 1},
    },
    false
};

inline const EntitySchema kSiteSchema{
    "site",
    std::vector<FieldSpec>{
        FieldSpec{"accent_color", {"string"}, 1},
        FieldSpec{"allow_external_signup", {"boolean"}, 1},
        FieldSpec{"cover_image", {"string"}, 1},
        FieldSpec{"description", {"string"}, 1},
        FieldSpec{"icon", {"null"}, 1},
        FieldSpec{"locale", {"string"}, 1},
        FieldSpec{"logo", {"null"}, 1},
        FieldSpec{"site_uuid", {"string"}, 1},
        FieldSpec{"timezone", {"string"}, 1},
        FieldSpec{"title", {"string"}, 1},
        FieldSpec{"url", {"string"}, 1},
        FieldSpec{"version", {"string"}, 1},
    },
    false
};

inline const EntitySchema kConfigSchema{
    "config",
    std::vector<FieldSpec>{
        FieldSpec{"auth", {"object"}, 1},
        FieldSpec{"clientExtensions", {"object"}, 1},
        FieldSpec{"database", {"string"}, 1},
        FieldSpec{"emailAnalytics", {"boolean"}, 1},
        FieldSpec{"enableDeveloperExperiments", {"boolean"}, 1},
        FieldSpec{"environment", {"string"}, 1},
        FieldSpec{"klipy", {"object"}, 1},
        FieldSpec{"labs", {"any"}, 1},
        FieldSpec{"mail", {"string"}, 1},
        FieldSpec{"mailgunIsConfigured", {"boolean"}, 1},
        FieldSpec{"security", {"object"}, 1},
        FieldSpec{"signupForm", {"object"}, 1},
        FieldSpec{"stripeDirect", {"boolean"}, 1},
        FieldSpec{"useGravatar", {"boolean"}, 1},
        FieldSpec{"version", {"string"}, 1},
    },
    false
};

inline const EntitySchema kEmailPreviewSchema{
    "email_previews",
    std::vector<FieldSpec>{
        FieldSpec{"html", {"string"}, 1},
        FieldSpec{"plaintext", {"string"}, 1},
        FieldSpec{"subject", {"string"}, 1},
    },
    false
};

inline const EntitySchema kEmailStatusSchema{
    "email_statuses",
    std::vector<FieldSpec>{
        FieldSpec{"id", {"string"}, 1},
        FieldSpec{"sending", {"object"}, 1},
    },
    false
};

inline const EntitySchema kFailuresSchema{
    "failures",
    std::vector<FieldSpec>{
        FieldSpec{"code", {"number"}, 1},
        FieldSpec{"email_id", {"string"}, 1},
        FieldSpec{"email_recipient", {"null","object"}, 2},
        FieldSpec{"email_recipient_id", {"string"}, 1},
        FieldSpec{"enhanced_code", {"string"}, 1},
        FieldSpec{"event_id", {"string"}, 1},
        FieldSpec{"failed_at", {"string"}, 1},
        FieldSpec{"id", {"string"}, 1},
        FieldSpec{"member", {"null","object"}, 2},
        FieldSpec{"member_id", {"string"}, 1},
        FieldSpec{"message", {"string"}, 1},
        FieldSpec{"severity", {"string"}, 1},
    },
    false
};

inline const EntitySchema kBatchesSchema{
    "batches",
    std::vector<FieldSpec>{
        FieldSpec{"count", {"object"}, 1},
        FieldSpec{"created_at", {"string"}, 1},
        FieldSpec{"error_data", {"null"}, 1},
        FieldSpec{"error_message", {"null"}, 1},
        FieldSpec{"error_status_code", {"null"}, 1},
        FieldSpec{"id", {"string"}, 1},
        FieldSpec{"mailgun_message_id", {"string"}, 1},
        FieldSpec{"member_segment", {"null"}, 1},
        FieldSpec{"status", {"string"}, 1},
        FieldSpec{"updated_at", {"string"}, 1},
    },
    false
};

inline const EntitySchema kAutomationSchema{
    "automations",
    std::vector<FieldSpec>{
        FieldSpec{"actions", {"array"}, 1},
        FieldSpec{"created_at", {"string"}, 1},
        FieldSpec{"description", {"string"}, 1},
        FieldSpec{"id", {"string"}, 1},
        FieldSpec{"name", {"string"}, 1},
        FieldSpec{"slug", {"string"}, 1},
        FieldSpec{"status", {"string"}, 1},
        FieldSpec{"updated_at", {"string"}, 1},
    },
    false
};

inline const EntitySchema kSlugSchema{
    "slugs",
    std::vector<FieldSpec>{
        FieldSpec{"slug", {"string"}, 1},
    },
    false
};

inline const EntitySchema kCustomThemeSettingSchema{
    "custom_theme_settings",
    std::vector<FieldSpec>{
        FieldSpec{"default", {"string"}, 1},
        FieldSpec{"group", {"string"}, 1},
        FieldSpec{"id", {"string"}, 1},
        FieldSpec{"key", {"string"}, 1},
        FieldSpec{"options", {"array"}, 1},
        FieldSpec{"type", {"string"}, 1},
        FieldSpec{"value", {"string"}, 1},
    },
    false
};

inline const EntitySchema kLinks2Schema{
    "links",
    std::vector<FieldSpec>{
        FieldSpec{"count", {"object"}, 1},
        FieldSpec{"link", {"object"}, 1},
        FieldSpec{"post_id", {"string"}, 1},
    },
    false
};

inline const EntitySchema kMentionSchema{
    "mentions",
    std::vector<FieldSpec>{
        FieldSpec{"id", {"string"}, 1},
        FieldSpec{"payload", {"null"}, 1},
        FieldSpec{"resource", {"null"}, 1},
        FieldSpec{"source", {"string"}, 1},
        FieldSpec{"source_author", {"null"}, 1},
        FieldSpec{"source_excerpt", {"null"}, 1},
        FieldSpec{"source_favicon", {"null"}, 1},
        FieldSpec{"source_featured_image", {"null"}, 1},
        FieldSpec{"source_site_title", {"string"}, 1},
        FieldSpec{"source_title", {"string"}, 1},
        FieldSpec{"target", {"string"}, 1},
        FieldSpec{"timestamp", {"string"}, 1},
        FieldSpec{"verified", {"boolean"}, 1},
    },
    false
};

inline const EntitySchema kSettingSchema{
    "settings",
    std::vector<FieldSpec>{
        FieldSpec{"key", {"string"}, 1},
        FieldSpec{"value", {"null","string","boolean"}, 3},
    },
    false
};

inline const EntitySchema kCommentLikeSchema{
    "comment_likes",
    std::vector<FieldSpec>{
        FieldSpec{"comment_id", {"string"}, 1},
        FieldSpec{"created_at", {"string"}, 1},
        FieldSpec{"id", {"string"}, 1},
        FieldSpec{"member", {"object"}, 1},
        FieldSpec{"member_id", {"string"}, 1},
        FieldSpec{"updated_at", {"string"}, 1},
    },
    false
};

inline const EntitySchema kCommentDislikeSchema{
    "comment_dislikes",
    std::vector<FieldSpec>{
        FieldSpec{"comment_id", {"string"}, 1},
        FieldSpec{"created_at", {"string"}, 1},
        FieldSpec{"id", {"string"}, 1},
        FieldSpec{"member", {"object"}, 1},
        FieldSpec{"member_id", {"string"}, 1},
        FieldSpec{"updated_at", {"string"}, 1},
    },
    false
};

inline const EntitySchema kCommentReportSchema{
    "comment_reports",
    std::vector<FieldSpec>{
        FieldSpec{"comment_id", {"string"}, 1},
        FieldSpec{"created_at", {"string"}, 1},
        FieldSpec{"id", {"string"}, 1},
        FieldSpec{"member", {"object"}, 1},
        FieldSpec{"member_id", {"string"}, 1},
        FieldSpec{"updated_at", {"string"}, 1},
    },
    false
};

struct ValidationReport
{
    std::size_t checked{0};
    std::size_t passed{0};
    std::vector<std::string> failures{};
};

/** @brief Validate a response body against a Ghost entity schema. */
[[nodiscard]] inline ValidationReport validate(std::string_view body, const EntitySchema& schema)
{
    ValidationReport report;
    const JsonParser parser;
    const auto root = parser.parse(body);
    if (!root || !root->is(JsonNode::Kind::Object)) {
        report.failures.push_back("body is not a JSON object");
        return report;
    }
    const JsonNode* entity_node = nullptr;
    if (schema.root_object) {
        entity_node = &*root;
    } else {
        const auto top = root->object.find(std::string{schema.collection});
        if (top == root->object.end() ||
            (!top->second.is(JsonNode::Kind::Array) && !top->second.is(JsonNode::Kind::Object))) {
            report.failures.push_back("missing collection body: " + std::string{schema.collection});
            return report;
        }
        entity_node = &top->second;
    }
    if (entity_node->is(JsonNode::Kind::Object)) {
        const auto& object = entity_node->object;
        for (const auto& spec : schema.fields) {
            ++report.checked;
            const auto it = object.find(std::string{spec.name});
            if (it == object.end()) {
                report.failures.push_back("missing field: " + std::string{spec.name});
                continue;
            }
            const JsonNode& node = it->second;
            bool ok = false;
            for (std::size_t i = 0; i < spec.type_count; ++i) {
                const auto t = spec.types[i];
                if (t == "any") { ok = true; break; }
                if (t == "null" && node.is(JsonNode::Kind::Null)) { ok = true; break; }
                if (t == "string" && node.is(JsonNode::Kind::String)) { ok = true; break; }
                if (t == "number" && node.is(JsonNode::Kind::Number)) { ok = true; break; }
                if (t == "boolean" && node.is(JsonNode::Kind::Bool)) { ok = true; break; }
                if (t == "object" && node.is(JsonNode::Kind::Object)) { ok = true; break; }
                if (t == "array" && node.is(JsonNode::Kind::Array)) { ok = true; break; }
            }
            if (ok) ++report.passed;
            else report.failures.push_back("wrong type: " + std::string{spec.name});
        }
        return report;
    }
    if (entity_node->array.empty()) return report;
    if (!entity_node->array.front().is(JsonNode::Kind::Object)) {
        report.failures.push_back("collection element is not an object");
        return report;
    }
    const auto& entity = entity_node->array.front().object;
    for (const auto& spec : schema.fields) {
        ++report.checked;
        const auto it = entity.find(std::string{spec.name});
        if (it == entity.end()) {
            report.failures.push_back("missing field: " + std::string{spec.name});
            continue;
        }
        const JsonNode& node = it->second;
        bool ok = false;
        for (std::size_t i = 0; i < spec.type_count; ++i) {
            const auto t = spec.types[i];
            if (t == "any") { ok = true; break; }
            if (t == "null" && node.is(JsonNode::Kind::Null)) { ok = true; break; }
            if (t == "string" && node.is(JsonNode::Kind::String)) { ok = true; break; }
            if (t == "number" && node.is(JsonNode::Kind::Number)) { ok = true; break; }
            if (t == "boolean" && node.is(JsonNode::Kind::Bool)) { ok = true; break; }
            if (t == "object" && node.is(JsonNode::Kind::Object)) { ok = true; break; }
            if (t == "array" && node.is(JsonNode::Kind::Array)) { ok = true; break; }
        }
        if (ok) ++report.passed;
        else report.failures.push_back("wrong type: " + std::string{spec.name});
    }
    return report;
}

} // namespace blog::testing
