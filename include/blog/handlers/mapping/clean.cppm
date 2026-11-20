// clean.cppm — Ghost output "clean" layer for blog_handlers:mapping.
//
// Port of Ghost's
//   ghost/core/server/api/endpoints/utils/serializers/output/utils/clean.js
// plus the field sets emitted by the output mappers / serializers documented in
// docs/parity/behavior/{posts,users,members,tags,authors,comments,actions,tiers,offers,webhooks}.md.
//
// Strategy: keep only fields Ghost exposes for the entity (allow-list), apply
// Ghost's wire aliases, and perform the nested clean-ups clean.action() and the
// comment mapper perform.  Internal / restricted columns never survive because
// they are absent from every allow-list.
export module blog_handlers:clean;

import std;

export namespace blog::mapping {

/** @brief Apply Ghost output field allow-list + aliases to one JSON object.
 *
 *  @param json_object a single JSON object, or an array of objects (each is cleaned).
 *  @param entity singular or plural Ghost entity name: post/page, user/staff,
 *         member, tag, author, comment, action, tier/product, offer, webhook.
 *  @return JSON object string containing only Ghost-exposed fields.
 */
[[nodiscard]] std::string clean(std::string_view json_object, std::string_view entity);

} // namespace blog::mapping

namespace blog::mapping::detail {

struct Member {
    std::string key;
    std::string value; // raw JSON value
};
using Members = std::vector<Member>;

[[nodiscard]] inline std::string_view trim(std::string_view s)
{
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.front()))) s.remove_prefix(1);
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back()))) s.remove_suffix(1);
    return s;
}

/** @brief Index just past the closing quote of a JSON string that starts at @p start. */
[[nodiscard]] inline std::size_t quoted_end(std::string_view s, std::size_t start)
{
    bool escaped = false;
    for (std::size_t i = start; i < s.size(); ++i) {
        if (escaped) { escaped = false; continue; }
        if (s[i] == '\\') { escaped = true; continue; }
        if (s[i] == '"') return i + 1;
    }
    return std::string_view::npos;
}

/** @brief Index one past the end of a JSON value beginning at @p start. */
[[nodiscard]] inline std::size_t value_end(std::string_view s, std::size_t start)
{
    if (start >= s.size()) return start;
    const char first = s[start];
    if (first == '"') return quoted_end(s, start + 1);
    if (first == '{' || first == '[') {
        const char open = first;
        const char close = (first == '{') ? '}' : ']';
        int depth = 0;
        bool in_string = false;
        bool escaped = false;
        for (std::size_t i = start; i < s.size(); ++i) {
            const char c = s[i];
            if (in_string) {
                if (escaped) escaped = false;
                else if (c == '\\') escaped = true;
                else if (c == '"') in_string = false;
                continue;
            }
            if (c == '"') in_string = true;
            else if (c == open) ++depth;
            else if (c == close && --depth == 0) return i + 1;
        }
        return s.size();
    }
    std::size_t i = start;
    while (i < s.size() && s[i] != ',' && s[i] != '}' && s[i] != ']'
           && !std::isspace(static_cast<unsigned char>(s[i]))) {
        ++i;
    }
    return i;
}

/** @brief Minimal order-preserving JSON object parser (values stay raw JSON text). */
[[nodiscard]] inline Members parse_object(std::string_view text)
{
    Members out;
    const auto opening = text.find('{');
    if (opening == std::string_view::npos) return out;
    const std::string_view s = text.substr(opening);
    std::size_t i = 1;
    while (i < s.size()) {
        while (i < s.size() && (std::isspace(static_cast<unsigned char>(s[i])) || s[i] == ',')) ++i;
        if (i >= s.size() || s[i] == '}') break;
        if (s[i] != '"') { ++i; continue; }
        const auto close_quote = quoted_end(s, i + 1);
        if (close_quote == std::string_view::npos) break;
        std::string key{s.substr(i + 1, close_quote - i - 2)};
        i = close_quote;
        while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i]))) ++i;
        if (i < s.size() && s[i] == ':') ++i;
        while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i]))) ++i;
        const auto end = value_end(s, i);
        if (end <= i) break;
        out.push_back({std::move(key), std::string{s.substr(i, end - i)}});
        i = end;
    }
    return out;
}

/** @brief Every top-level object inside a JSON array, in source order. */
[[nodiscard]] inline std::vector<Members> parse_array_objects(std::string_view text)
{
    std::vector<Members> out;
    std::size_t i = 0;
    while ((i = text.find('{', i)) != std::string_view::npos) {
        const auto end = value_end(text, i);
        if (end <= i) break;
        out.push_back(parse_object(text.substr(i, end - i)));
        i = end;
    }
    return out;
}

[[nodiscard]] inline std::string quote(std::string_view s)
{
    std::string out = "\"";
    for (const char c : s) {
        switch (c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\b': out += "\\b"; break;
            case '\f': out += "\\f"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    static constexpr char hex[] = "0123456789abcdef";
                    out += "\\u00";
                    out += hex[(static_cast<unsigned char>(c) >> 4) & 0xF];
                    out += hex[static_cast<unsigned char>(c) & 0xF];
                } else {
                    out += c;
                }
        }
    }
    out += '"';
    return out;
}

[[nodiscard]] inline std::string to_json(const Members& members)
{
    std::string out = "{";
    bool first = true;
    for (const auto& m : members) {
        if (!first) out += ',';
        out += quote(m.key) + ':' + m.value;
        first = false;
    }
    out += '}';
    return out;
}

[[nodiscard]] inline std::string to_json_array(const std::vector<Members>& objects)
{
    std::string out = "[";
    bool first = true;
    for (const auto& o : objects) {
        if (!first) out += ',';
        out += to_json(o);
        first = false;
    }
    out += ']';
    return out;
}

[[nodiscard]] inline Members::iterator find(Members& members, std::string_view key)
{
    return std::ranges::find_if(members, [key](const Member& m) { return m.key == key; });
}

[[nodiscard]] inline bool contains(const Members& members, std::string_view key)
{
    return std::ranges::any_of(members, [key](const Member& m) { return m.key == key; });
}

inline void erase_key(Members& members, std::string_view key)
{
    const auto it = find(members, key);
    if (it != members.end()) members.erase(it);
}

inline void set_key(Members& members, std::string_view key, std::string value)
{
    const auto it = find(members, key);
    if (it != members.end()) it->value = std::move(value);
    else members.push_back({std::string{key}, std::move(value)});
}

[[nodiscard]] inline bool in_fields(std::string_view key, std::span<const std::string_view> fields)
{
    return std::ranges::find(fields, key) != fields.end();
}

inline void keep_fields(Members& members, std::span<const std::string_view> fields)
{
    std::erase_if(members, [fields](const Member& m) { return !in_fields(m.key, fields); });
}

struct Alias { std::string_view from; std::string_view to; };

inline void apply_aliases(Members& members, std::span<const Alias> aliases)
{
    for (const auto& alias : aliases) {
        const auto it = find(members, alias.from);
        if (it == members.end()) continue;
        std::string value = std::move(it->value);
        members.erase(it);
        set_key(members, alias.to, std::move(value));
    }
}

/** @brief clean.action() / clean.js nested alias: <source field> -> `image`. */
inline void alias_to_image(Members& members, std::string_view source)
{
    std::string value;
    if (const auto it = find(members, source); it != members.end()) {
        value = std::move(it->value);
        members.erase(it);
    } else {
        return;
    }
    set_key(members, "image", std::move(value));
}

// ---- Ghost wire field sets -------------------------------------------------

// mappers/posts.js + clean.post + posts_meta flattening.
inline constexpr auto POST_FIELDS = std::to_array<std::string_view>({
    "id", "uuid", "title", "slug", "mobiledoc", "lexical", "html", "plaintext",
    "feature_image", "featured", "status", "visibility", "created_at", "updated_at",
    "published_at", "published_by", "custom_excerpt", "excerpt", "reading_time",
    "codeinjection_head", "codeinjection_foot", "custom_template", "canonical_url",
    "email_segment", "email_only", "og_image", "og_title", "og_description",
    "twitter_image", "twitter_title", "twitter_description", "meta_title", "meta_description",
    "email_subject", "frontmatter", "feature_image_alt", "feature_image_caption",
    "primary_tag", "primary_author", "tags", "authors", "tiers", "newsletter", "email",
    "url", "comment_id", "show_title_and_feature_image", "access", "comments",
    "sentiment", "count", "newsletters"
});

// Admin users serializer + clean.author (visibility / locale / password always removed).
inline constexpr auto USER_FIELDS = std::to_array<std::string_view>({
    "id", "name", "slug", "email", "profile_image", "cover_image", "bio", "website",
    "location", "facebook", "twitter", "threads", "bluesky", "mastodon", "tiktok",
    "youtube", "instagram", "linkedin", "accessibility", "status", "meta_title",
    "meta_description", "tour", "last_seen", "comment_notifications",
    "free_member_signup_notification", "paid_subscription_started_notification",
    "paid_subscription_canceled_notification", "mention_notifications",
    "recommendation_notifications", "milestone_notifications", "donation_notifications",
    "gift_subscription_notifications", "created_at", "updated_at", "url", "roles",
    "permissions", "count"
});

// clean.author removes these for the Content API; authors are the public shape.
inline constexpr auto AUTHOR_FIELDS = std::to_array<std::string_view>({
    "id", "name", "slug", "profile_image", "cover_image", "bio", "website",
    "location", "facebook", "twitter", "threads", "bluesky", "mastodon", "tiktok",
    "youtube", "instagram", "linkedin", "meta_title", "meta_description", "url", "count"
});

// output/members.js#serializeMember (products -> tiers alias applied first).
inline constexpr auto MEMBER_FIELDS = std::to_array<std::string_view>({
    "id", "uuid", "email", "name", "note", "geolocation", "subscribed",
    "created_at", "updated_at", "labels", "subscriptions", "avatar_image", "comped",
    "email_count", "email_opened_count", "email_open_rate", "email_recipients", "status",
    "last_seen_at", "attribution", "enable_comment_notifications",
    "enable_updates_and_announcements", "unsubscribe_url", "can_comment", "commenting",
    "tiers", "metafields", "current_subscription", "email_suppression", "newsletters"
});

// mappers/tags.js + clean.tag (parent_id / parent always removed).
inline constexpr auto TAG_FIELDS = std::to_array<std::string_view>({
    "id", "name", "slug", "description", "feature_image", "visibility", "og_image",
    "og_title", "og_description", "twitter_image", "twitter_title", "twitter_description",
    "meta_title", "meta_description", "codeinjection_head", "codeinjection_foot",
    "canonical_url", "accent_color", "created_at", "updated_at", "url", "count"
});

// mappers/comments.js commentFields + computed relations.
inline constexpr auto COMMENT_FIELDS = std::to_array<std::string_view>({
    "id", "parent_id", "in_reply_to_id", "in_reply_to_snippet", "status", "html",
    "created_at", "edited_at", "pinned", "member", "post", "parent", "replies",
    "liked", "disliked", "count"
});

inline constexpr auto COMMENT_MEMBER_FIELDS = std::to_array<std::string_view>({
    "id", "uuid", "name", "email", "expertise", "avatar_image", "can_comment", "commenting"
});
inline constexpr auto COMMENT_POST_FIELDS = std::to_array<std::string_view>({
    "id", "uuid", "title", "url", "feature_image", "excerpt"
});
inline constexpr auto COMMENT_COUNT_FIELDS = std::to_array<std::string_view>({
    "replies", "direct_replies", "likes", "dislikes", "reports"
});

// models/action.js fields + clean.action() embedded actor / resource.
inline constexpr auto ACTION_FIELDS = std::to_array<std::string_view>({
    "id", "resource_id", "resource_type", "actor_id", "actor_type", "event",
    "context", "created_at", "actor", "resource"
});
inline constexpr auto ACTION_USER_ACTOR_FIELDS = std::to_array<std::string_view>({
    "id", "name", "slug", "image"
});
inline constexpr auto ACTION_OTHER_ACTOR_FIELDS = std::to_array<std::string_view>({
    "id", "name", "slug", "image"
});
inline constexpr auto ACTION_RESOURCE_FIELDS = std::to_array<std::string_view>({
    "id", "title", "slug", "image", "name"
});

// output/tiers.js#serializeTier.
inline constexpr auto TIER_FIELDS = std::to_array<std::string_view>({
    "id", "name", "description", "slug", "active", "type", "welcome_page_url",
    "created_at", "updated_at", "visibility", "benefits", "currency", "monthly_price",
    "yearly_price", "trial_days", "requirements"
});

// OfferMapper.toDTO (offers.md:344-348).
inline constexpr auto OFFER_FIELDS = std::to_array<std::string_view>({
    "id", "name", "code", "display_title", "display_description", "type", "cadence",
    "amount", "duration", "duration_in_months", "currency_restriction", "currency",
    "status", "redemption_count", "redemption_type", "tier", "created_at", "last_redeemed"
});

// webhooks.md:88-90 (default serializer, full model toJSON).
inline constexpr auto WEBHOOK_FIELDS = std::to_array<std::string_view>({
    "id", "name", "event", "target_url", "secret", "api_version", "integration_id",
    "status", "created_at", "updated_at", "last_triggered_at", "last_triggered_status",
    "last_triggered_error"
});

inline constexpr auto RESTRICTED_FIELDS = std::to_array<std::string_view>({
    "password", "token", "gift_link_token", "secret", "transient_id", "password_hash"
});

enum class Kind { Unknown, Post, User, Member, Tag, Author, Comment, Action, Tier, Offer, Webhook };

[[nodiscard]] inline Kind classify(std::string_view entity)
{
    std::string name{trim(entity)};
    std::ranges::transform(name, name.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    // Strip a plural / Ghost collection suffix.
    if (!name.empty() && name.back() == 's') {
        if (name == "products") return Kind::Tier;
        name.pop_back();
    }
    if (name == "post" || name == "page") return Kind::Post;
    if (name == "user" || name == "staff") return Kind::User;
    if (name == "member") return Kind::Member;
    if (name == "tag") return Kind::Tag;
    if (name == "author") return Kind::Author;
    if (name == "comment") return Kind::Comment;
    if (name == "action") return Kind::Action;
    if (name == "tier" || name == "product") return Kind::Tier;
    if (name == "offer") return Kind::Offer;
    if (name == "webhook") return Kind::Webhook;
    return Kind::Unknown;
}

[[nodiscard]] inline std::span<const std::string_view> fields_for(Kind kind)
{
    switch (kind) {
        case Kind::Post:    return POST_FIELDS;
        case Kind::User:    return USER_FIELDS;
        case Kind::Member:  return MEMBER_FIELDS;
        case Kind::Tag:     return TAG_FIELDS;
        case Kind::Author:  return AUTHOR_FIELDS;
        case Kind::Comment: return COMMENT_FIELDS;
        case Kind::Action:  return ACTION_FIELDS;
        case Kind::Tier:    return TIER_FIELDS;
        case Kind::Offer:   return OFFER_FIELDS;
        case Kind::Webhook: return WEBHOOK_FIELDS;
        case Kind::Unknown: break;
    }
    return {};
}

// ---- nested cleans ---------------------------------------------------------

inline void clean_nested(Member& member, std::span<const std::string_view> fields)
{
    auto nested = parse_object(member.value);
    keep_fields(nested, fields);
    member.value = to_json(nested);
}

inline void clean_comment_nested(Member& member, int depth)
{
    if (depth > 3) return;
    if (member.key == "member") {
        clean_nested(member, COMMENT_MEMBER_FIELDS);
    } else if (member.key == "post") {
        clean_nested(member, COMMENT_POST_FIELDS);
    } else if (member.key == "count") {
        clean_nested(member, COMMENT_COUNT_FIELDS);
    } else if (member.key == "parent") {
        auto nested = parse_object(member.value);
        for (auto& n : nested) clean_comment_nested(n, depth + 1);
        keep_fields(nested, COMMENT_FIELDS);
        member.value = to_json(nested);
    } else if (member.key == "replies") {
        auto objects = parse_array_objects(member.value);
        for (auto& object : objects) {
            for (auto& n : object) clean_comment_nested(n, depth + 1);
            keep_fields(object, COMMENT_FIELDS);
        }
        member.value = to_json_array(objects);
    }
}

inline void clean_action_embedded(Members& members)
{
    const std::string_view actor_type = [&] {
        const auto it = find(members, "actor_type");
        return it == members.end() ? std::string_view{} : std::string_view{it->value};
    }();

    for (auto& m : members) {
        if (m.key == "actor") {
            auto nested = parse_object(m.value);
            if (!nested.empty()) {
                if (actor_type == "\"user\"") {
                    alias_to_image(nested, "profile_image");
                    keep_fields(nested, ACTION_USER_ACTOR_FIELDS);
                } else {
                    alias_to_image(nested, "icon_image");
                    keep_fields(nested, ACTION_OTHER_ACTOR_FIELDS);
                }
                m.value = to_json(nested);
            }
        } else if (m.key == "resource") {
            auto nested = parse_object(m.value);
            if (!nested.empty()) {
                alias_to_image(nested, "feature_image");
                keep_fields(nested, ACTION_RESOURCE_FIELDS);
                m.value = to_json(nested);
            }
        }
    }
}

} // namespace blog::mapping::detail

namespace blog::mapping {

[[nodiscard]] std::string clean(std::string_view json_object, std::string_view entity)
{
    using namespace detail;

    // Accept a rows array as a convenience: clean every element in place.
    const auto trimmed = trim(json_object);
    if (!trimmed.empty() && trimmed.front() == '[') {
        auto objects = parse_array_objects(trimmed);
        std::string out = "[";
        bool first = true;
        for (const auto& object : objects) {
            if (!first) out += ',';
            out += clean(to_json(object), entity);
            first = false;
        }
        out += ']';
        return out;
    }

    auto members = parse_object(json_object);
    if (members.empty()) return "{}";

    const Kind kind = classify(entity);

    if (kind == Kind::Unknown) {
        // No Ghost allow-list known: never leak restricted columns.
        std::erase_if(members, [](const Member& m) { return in_fields(m.key, RESTRICTED_FIELDS); });
        return to_json(members);
    }

    // Wire aliases from the Ghost mappers.
    if (kind == Kind::Post) {
        constexpr std::array<Alias, 1> aliases{{{"email_recipient_filter", "email_segment"}}};
        apply_aliases(members, aliases);
    } else if (kind == Kind::Member) {
        constexpr std::array<Alias, 1> aliases{{{"products", "tiers"}}};
        apply_aliases(members, aliases);
    } else if (kind == Kind::Tier) {
        // serializeTier computes `active` from the domain `status` field.
        const auto status = find(members, "status");
        if (status != members.end() && !contains(members, "active")) {
            set_key(members, "active", status->value == "\"active\"" ? "true" : "false");
        }
        erase_key(members, "status");
    }

    // clean.post conditional drops (run before the allow-list filter).
    if (kind == Kind::Post) {
        if (!contains(members, "tags")) erase_key(members, "primary_tag");
        if (!contains(members, "authors")) erase_key(members, "primary_author");
        const auto type = find(members, "type");
        if (type == members.end() || type->value != "\"page\"") {
            erase_key(members, "show_title_and_feature_image");
        }
    }

    // Nested relation cleaning.
    if (kind == Kind::Action) {
        clean_action_embedded(members);
    } else if (kind == Kind::Comment) {
        for (auto& m : members) clean_comment_nested(m, 0);
    }

    keep_fields(members, fields_for(kind));
    return to_json(members);
}

} // namespace blog::mapping
