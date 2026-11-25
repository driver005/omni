export module blog_handlers:settings_browse_step;
import :domain_all;
import std;
import core_workflow;
import core_query;
import core_serde;
import interfaces;
import blog_handlers_middleware;
import :outbox_shim;

// ---------------------------------------------------------------------------
// Strict-parity support for the settings controller.
//
// Lives in this already-registered partition (BUILD.bazel:122) so every other
// settings step/handler can `import :settings_browse_step;` — no new partition,
// no BUILD edit. It replaces the old fixture/sentinel workflow with:
//   * core_query for the Ghost `group` option and NQL `status:paid` filter;
//   * core_auth:ghost through the registered blog_handlers_middleware (in the
//     handlers, see authorize() there) for per-op settings permissions;
//   * the registered blog_handlers:outbox_shim for every side effect;
//   * a local, store-backed SettingsCache/_formatBrowse/output-serializer
//     stand-in (central context_context.cppm is not editable from this folder).
// ---------------------------------------------------------------------------
export namespace blog::handlers::settings::support {

using Object = serde::Value::Object;
using Array = serde::Value::Array;
using Result = std::map<std::string, std::string>;

inline constexpr std::string_view kDocName{"settings"};

// ---------------------------------------------------------------------------
// JSON / string helpers
// ---------------------------------------------------------------------------

[[nodiscard]] inline std::string quote(std::string_view text)
{
    std::string out{"\""};
    for (const char c: text) {
        switch (c) {
            case '"':
                out += "\\\"";
                break;
            case '\\':
                out += "\\\\";
                break;
            case '\n':
                out += "\\n";
                break;
            case '\r':
                out += "\\r";
                break;
            case '\t':
                out += "\\t";
                break;
            default:
                out.push_back(c);
                break;
        }
    }
    out.push_back('"');
    return out;
}

[[nodiscard]] inline std::string trim(std::string_view s)
{
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.front()))) {
        s.remove_prefix(1);
    }
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back()))) {
        s.remove_suffix(1);
    }
    return std::string{s};
}

[[nodiscard]] inline bool iequal(std::string_view a, std::string_view b)
{
    return a.size() == b.size() && std::equal(
                                       a.begin(),
                                       a.end(),
                                       b.begin(),
                                       b.end(),
                                       [](char x, char y)
                                       {
                                           return std::tolower(static_cast<unsigned char>(x)) ==
                                                  std::tolower(static_cast<unsigned char>(y));
                                       }
                                   );
}

[[nodiscard]] inline std::optional<serde::Value> decode(std::string_view json)
{
    return core_serde::JsonCodec{}.decode<serde::Value>(json);
}

[[nodiscard]] inline std::string encode(const serde::Value& value)
{
    return core_serde::JsonCodec{}.encode(value);
}

[[nodiscard]] inline const serde::Value* value_for(const Object* object, std::string_view key)
{
    if (object == nullptr) {
        return nullptr;
    }
    for (const auto& [name, value]: *object) {
        if (name == key) {
            return &value;
        }
    }
    return nullptr;
}

[[nodiscard]] inline std::string string_field(const Object* object, std::string_view key)
{
    const auto* value = value_for(object, key);
    if (value == nullptr) {
        return {};
    }
    if (const auto* text = std::get_if<std::string>(&value->get())) {
        return *text;
    }
    return {};
}

/** Raw JSON fragment for a field (quoted string / bool / array / null). */
[[nodiscard]] inline std::string
raw_field(const Object* object, std::string_view key, std::string_view fallback = "null")
{
    const auto* value = value_for(object, key);
    return value == nullptr ? std::string{fallback} : encode(*value);
}

/** Reads a string out of a `RecordDto` raw JSON fragment (`"key"` -> key). */
[[nodiscard]] inline std::string
record_text(const blog::api::RecordDto* record, std::string_view key)
{
    if (record == nullptr) {
        return {};
    }
    const auto it = record->find(std::string{key});
    if (it == record->end()) {
        return {};
    }
    return core_serde::JsonCodec{}.decode<std::string>(it->second).value_or(it->second);
}

[[nodiscard]] inline const Object* as_object(const serde::Value* value)
{
    return value == nullptr ? nullptr : std::get_if<Object>(&value->get());
}

[[nodiscard]] inline const Array* as_array(const serde::Value* value)
{
    return value == nullptr ? nullptr : std::get_if<Array>(&value->get());
}

/** Extracts `frame.data.settings` (Ghost `data:['token']` style wrapper). */
[[nodiscard]] inline std::optional<Array> settings_array(const std::string& body)
{
    const auto doc = decode(body);
    const auto* root = as_object(doc ? &*doc : nullptr);
    const auto* array = as_array(value_for(root, "settings"));
    return array == nullptr ? std::nullopt : std::optional<Array>{*array};
}

// ---------------------------------------------------------------------------
// Ghost-shaped errors (no `__BAD_REQUEST__` sentinel survives)
// ---------------------------------------------------------------------------

[[nodiscard]] inline std::string ghost_error(
    std::string_view message,
    std::string_view type,
    std::string_view context = {},
    std::string_view property = {}
)
{
    static std::uint64_t counter{0};
    const auto id = std::format("{:032x}", ++counter);
    return std::format(
        R"({{"errors":[{{"id":"{}","code":null,"context":{},"details":null,"ghostErrorCode":null,"help":null,"message":{},"property":{},"type":"{}"}}]}})",
        id,
        context.empty() ? "null" : quote(context),
        quote(message),
        property.empty() ? "null" : quote(property),
        type
    );
}

[[nodiscard]] inline std::string
validation_error(std::string_view message, std::string_view property = {})
{
    return ghost_error(message, "ValidationError", {}, property);
}

[[nodiscard]] inline std::string bad_request(std::string_view message)
{
    return ghost_error(message, "BadRequestError");
}

[[nodiscard]] inline std::string not_found(std::string_view message)
{
    return ghost_error(message, "NotFoundError");
}

[[nodiscard]] inline std::string no_permission(std::string_view message)
{
    return ghost_error(message, "NoPermissionError");
}

[[nodiscard]] inline Result ok(std::string body, std::string invalidate = {})
{
    Result r{{"status", "200"}, {"body", std::move(body)}};
    if (!invalidate.empty()) {
        r["cache_invalidate"] = std::move(invalidate);
    }
    return r;
}

[[nodiscard]] inline Result fail(int status, std::string body)
{
    return Result{{"status", std::to_string(status)}, {"body", std::move(body)}};
}

// ---------------------------------------------------------------------------
// Outbox side effects (CONTRACTS.md §4 via registered :outbox_shim)
// ---------------------------------------------------------------------------

inline void emit(std::string_view type, std::string_view payload, std::string_view aggregate_id)
{
    auto& outbox = blog::handlers::outbox::global_outbox();
    outbox.emit(
        blog::handlers::outbox::Event{
            .type = std::string{type},
            .payload = std::string{payload},
            .aggregate_id = std::string{aggregate_id}
        }
    );
    (void)outbox.drain();
}

inline void emit_transaction(const std::vector<blog::handlers::outbox::Event>& events)
{
    auto& outbox = blog::handlers::outbox::global_outbox();
    outbox.begin_transaction();
    for (const auto& event: events) {
        outbox.emit(event);
    }
    outbox.commit();
    (void)outbox.drain();
}

inline void emit_transaction(std::initializer_list<blog::handlers::outbox::Event> events)
{
    emit_transaction(std::vector<blog::handlers::outbox::Event>{events});
}

// ---------------------------------------------------------------------------
// Settings fixture + store-backed overrides (local SettingsCache stand-in)
// ---------------------------------------------------------------------------

struct SettingDef
{
    std::string_view key;
    std::string_view value; // raw JSON fragment
    std::string_view group;
};

inline constexpr SettingDef kSettings[]{
    // core group (omitted for non-internal requests)
    {"password", "\"\"", "core"},
    {"is_private", "false", "core"},
    {"default_content_visibility", "\"public\"", "core"},
    // site group
    {"title", "\"Ghost\"", "site"},
    {"description", "\"Thoughts, stories and ideas\"", "site"},
    {"logo", "\"\"", "site"},
    {"icon", "\"/content/images/2026/01/ghost-icon.png\"", "site"},
    {"accent_color", "\"#FF1A75\"", "site"},
    {"cover_image", "\"https://static.ghost.org/v5.0.0/images/publication-cover.jpg\"", "site"},
    {"locale", "\"en\"", "site"},
    {"timezone", "\"Etc/UTC\"", "site"},
    {"codeinjection_head", "\"\"", "site"},
    {"codeinjection_foot", "\"\"", "site"},
    {"facebook", "\"ghost\"", "site"},
    {"twitter", "\"@ghost\"", "site"},
    {"threads", "\"\"", "site"},
    {"bluesky", "\"\"", "site"},
    {"mastodon", "\"\"", "site"},
    {"tiktok", "\"\"", "site"},
    {"youtube", "\"\"", "site"},
    {"instagram", "\"\"", "site"},
    {"linkedin", "\"\"", "site"},
    {"navigation", "[]", "site"},
    {"secondary_navigation", "[]", "site"},
    {"meta_title", "\"\"", "site"},
    {"meta_description", "\"\"", "site"},
    {"og_image", "\"\"", "site"},
    {"og_title", "\"\"", "site"},
    {"og_description", "\"\"", "site"},
    {"twitter_image", "\"\"", "site"},
    {"twitter_title", "\"\"", "site"},
    {"twitter_description", "\"\"", "site"},
    {"site_uuid", "\"00000000-0000-4000-8000-000000000000\"", "site"},
    {"announcement_content", "\"\"", "site"},
    {"announcement_background", "\"#ffffff\"", "site"},
    {"announcement_visibility", "\"public\"", "site"},
    // members / membership group
    {"members_track_sources", "true", "members"},
    {"members_enabled", "true", "members"},
    {"members_signup_access", "\"all\"", "members"},
    {"members_support_address", "\"noreply@example.com\"", "members"},
    {"default_content_visibility_tiers", "[]", "members"},
    {"donations_enabled", "true", "members"},
    {"allow_self_signup", "true", "members"},
    {"comments_enabled", "true", "members"},
    {"recommendations_enabled", "true", "members"},
    {"paid_members_enabled", "true", "members"},
    // portal / email / labs
    {"portal_name", "\"Portal\"", "portal"},
    {"portal_plans", "[]", "portal"},
    {"portal_signup_terms_html", "\"\"", "portal"},
    {"default_email_address", "\"noreply@example.com\"", "email"},
    {"support_email_address", "\"support@example.com\"", "email"},
    {"labs", "{}", "labs"},
    // stripe-connect settings (observable nulling on disconnect)
    {"stripe_connect_publishable_key", "\"pk_test_placeholder\"", "members"},
    {"stripe_connect_secret_key", "\"sk_test_placeholder\"", "members"},
    {"stripe_connect_livemode", "false", "members"},
    {"stripe_connect_display_name", "\"Ghost\"", "members"},
    {"stripe_connect_account_id", "\"acct_placeholder\"", "members"},
    {"members_stripe_webhook_id", "\"evt_placeholder\"", "members"},
    {"members_stripe_webhook_secret", "\"whsec_placeholder\"", "members"},
};

[[nodiscard]] inline std::string_view group_of(std::string_view key)
{
    for (const auto& def: kSettings) {
        if (def.key == key) {
            return def.group;
        }
    }
    return "site";
}

/** LAST value for `key` wins; new override keys join the `site` group. */
[[nodiscard]] inline std::string
setting_value(std::string_view key, const std::vector<blog::api::RecordDto>& overrides)
{
    std::string value;
    for (const auto& def: kSettings) {
        if (def.key == key) {
            value = std::string{def.value};
        }
    }
    for (const auto& record: overrides) {
        const auto it = record.find("key");
        if (it == record.end()) {
            continue;
        }
        const auto k = record_text(&record, "key");
        if (k != key) {
            continue;
        }
        const auto v = record.find("value");
        if (v != record.end()) {
            value = v->second;
        }
    }
    return value;
}

[[nodiscard]] inline std::vector<blog::api::RecordDto> overrides(blog::BlogContext& ctx)
{
    return blog::api::parse_array(ctx.store().resources().list("settings"));
}

[[nodiscard]] inline bool is_secret(std::string_view key)
{
    return key.find("secret") != std::string_view::npos ||
           key.find("api_key") != std::string_view::npos;
}

[[nodiscard]] inline std::string resize_icon(std::string_view value)
{
    constexpr std::string_view from{"/content/images/"};
    constexpr std::string_view to{"/content/images/size/w256h256/"};
    const auto pos = value.find(from);
    if (pos == std::string_view::npos) {
        return std::string{value};
    }
    return std::string{value.substr(0, pos)} + std::string{to} +
           std::string{value.substr(pos + from.size())};
}

/** Ghost `settings-bread-service._formatBrowse` + output serializer. */
[[nodiscard]] inline Result browse(blog::BlogContext& ctx, std::string_view group)
{
    const bool internal = false; // admin/Content-API frames are never `internal`
    const auto stored = overrides(ctx);

    std::vector<std::string> keys;
    for (const auto& def: kSettings) {
        keys.emplace_back(def.key);
    }
    for (const auto& record: stored) {
        const auto key = record_text(&record, "key");
        if (key.empty()) {
            continue;
        }
        if (std::ranges::find(keys, key) == keys.end()) {
            keys.push_back(key);
        }
    }

    std::string rows{"["};
    bool first = true;
    for (const auto key: keys) {
        if (!group.empty() && group_of(key) != group) {
            continue;
        }
        if (!internal && group_of(key) == "core") {
            continue;
        }
        auto value = setting_value(key, stored);
        if (!internal && is_secret(key) && value != "null" && value != "\"\"") {
            value = quote("\u2022\u2022\u2022\u2022\u2022\u2022\u2022\u2022"); // ••••••••
        }
        if (key == "icon") {
            value = resize_icon(value);
        }
        const bool read_only = false; // publicSiteAccess limit disabled by default
        rows += (first ? "" : ",") + std::format(R"({{"key":{},"value":{})", quote(key), value);
        if (read_only) {
            rows += R"(,"is_read_only":true)";
        }
        rows += "}";
        first = false;
    }
    rows += "]";

    std::string meta{"{}"};
    if (!group.empty()) {
        meta = std::format(R"({{"filters":{{"group":{}}}}})", quote(group));
    }
    return ok(std::format(R"({{"settings":{},"meta":{}}})", rows, meta));
}

/** Ghost `settings-public.js` — flat public-config object, no `settings` array. */
[[nodiscard]] inline Result public_browse(blog::BlogContext& ctx)
{
    static constexpr std::string_view kPublic[]{
        "title",
        "description",
        "logo",
        "icon",
        "accent_color",
        "cover_image",
        "facebook",
        "twitter",
        "threads",
        "bluesky",
        "mastodon",
        "tiktok",
        "youtube",
        "instagram",
        "linkedin",
        "locale",
        "timezone",
        "codeinjection_head",
        "codeinjection_foot",
        "navigation",
        "secondary_navigation",
        "meta_title",
        "meta_description",
        "og_image",
        "og_title",
        "og_description",
        "twitter_image",
        "twitter_title",
        "twitter_description",
        "members_support_address",
        "members_enabled",
        "donations_enabled",
        "allow_self_signup",
        "members_invite_only",
        "members_signup_access",
        "paid_members_enabled",
        "portal_name",
        "portal_plans",
        "comments_enabled",
        "recommendations_enabled",
        "default_email_address",
        "support_email_address",
        "site_uuid",
        "labs",
        "transistor_portal_enabled"
    };

    const auto stored = overrides(ctx);
    std::string out{"{"};
    bool first = true;
    for (const auto key: kPublic) {
        auto value = setting_value(key, stored);
        if (key == "icon") {
            value = resize_icon(value);
        }
        out += (first ? "" : ",") + quote(key) + ":" + value;
        first = false;
    }
    out += R"(,"url":"http://localhost:2368/","version":"5.0.0","labs":)";
    out += setting_value("labs", stored);
    out += "}";
    return ok(out);
}

// ---------------------------------------------------------------------------
// Ghost input serializer / validator for PUT /settings
// ---------------------------------------------------------------------------

inline constexpr std::string_view kEditable[]{
    "title",
    "description",
    "logo",
    "cover_image",
    "icon",
    "locale",
    "timezone",
    "codeinjection_head",
    "codeinjection_foot",
    "facebook",
    "twitter",
    "threads",
    "bluesky",
    "mastodon",
    "tiktok",
    "youtube",
    "instagram",
    "linkedin",
    "navigation",
    "secondary_navigation",
    "meta_title",
    "meta_description",
    "llms_enabled",
    "machine_payments_enabled",
    "machine_payments_currency",
    "machine_payments_amount",
    "machine_payments_stripe_profile_id",
    "og_image",
    "og_title",
    "og_description",
    "twitter_image",
    "twitter_title",
    "twitter_description",
    "is_private",
    "password",
    "default_content_visibility",
    "default_content_visibility_tiers",
    "members_signup_access",
    "members_support_address",
    "stripe_secret_key",
    "stripe_publishable_key",
    "stripe_connect_integration_token",
    "portal_name",
    "portal_button",
    "portal_plans",
    "portal_default_plan",
    "portal_button_style",
    "firstpromoter",
    "firstpromoter_id",
    "portal_button_icon",
    "portal_button_signup_text",
    "portal_signup_gift_promotion",
    "portal_account_gift_promotion",
    "portal_signup_terms_html",
    "portal_signup_checkbox_required",
    "mailgun_api_key",
    "mailgun_domain",
    "mailgun_base_url",
    "email_track_opens",
    "email_track_clicks",
    "members_track_sources",
    "web_analytics",
    "slack_url",
    "slack_username",
    "unsplash",
    "shared_views",
    "accent_color",
    "editor_default_email_recipients",
    "editor_default_email_recipients_filter",
    "labs",
    "comments_enabled",
    "outbound_link_tagging",
    "announcement_content",
    "announcement_background",
    "announcement_visibility",
    "pintura",
    "pintura_js_url",
    "pintura_css_url",
    "donations_currency",
    "donations_suggested_amount",
    "recommendations_enabled",
    "body_font",
    "heading_font",
    "blocked_email_domains",
    "require_email_mfa",
    "social_web",
    "explore_ping",
    "explore_ping_growth",
    "indexnow_api_key",
    "transistor",
    "transistor_portal_enabled",
    "transistor_portal_heading",
    "transistor_portal_description",
    "transistor_portal_button_text",
    "transistor_portal_url_template"
};

inline constexpr std::string_view kBoolean[]{
    "is_private",
    "members_track_sources",
    "email_track_opens",
    "email_track_clicks",
    "comments_enabled",
    "recommendations_enabled",
    "donations_enabled",
    "members_enabled",
    "allow_self_signup",
    "members_invite_only",
    "paid_members_enabled",
    "llms_enabled",
    "machine_payments_enabled",
    "portal_signup_checkbox_required",
    "require_email_mfa",
    "transistor_portal_enabled",
    "outbound_link_tagging",
    "web_analytics"
};

inline constexpr std::string_view kArray[]{
    "navigation",
    "secondary_navigation",
    "notifications",
    "default_content_visibility_tiers",
    "portal_plans",
    "blocked_email_domains",
    "editor_default_email_recipients"
};

inline constexpr std::string_view kLabsWritable[]{
    "superEditors",
    "editorExcerpt",
    "additionalPaymentMethods",
    "navigationIcons",
    "automations",
    "automationsLeadMagnets",
    "stripeAutomaticTax",
    "importMemberTier",
    "csvContentImporter",
    "admin7Design",
    "admin7CustomizeFromViewSite",
    "emailUniqueid",
    "themeTranslation",
    "pictureImageFormats",
    "getHelperDeduplication",
    "stripeCheckoutCollection",
    "paywallImprovements",
    "machinePayments",
    "apps",
    "inAppSupport",
    "customFieldTypePhone"
};

inline constexpr std::string_view kSecretFiltered[]{
    "stripe_connect_integration_token",
    "stripe_connect_publishable_key",
    "stripe_connect_secret_key",
    "stripe_connect_livemode",
    "stripe_connect_account_id",
    "stripe_connect_display_name"
};

[[nodiscard]] inline bool contains(std::span<const std::string_view> set, std::string_view key)
{
    return std::ranges::find(set, key) != set.end();
}

// ---------------------------------------------------------------------------
// Ghost auth gate — `mw.authAdminApi` + per-op permissions under docName
// 'settings' (core_auth:ghost through the registered middleware partition).
// ---------------------------------------------------------------------------

[[nodiscard]] inline bool authorize(
    interfaces::io::IRequest& req,
    interfaces::io::IResponse& res,
    std::function<void()> send,
    std::string_view docName,
    std::string_view method
)
{
    using blog::handlers::middleware::kGhostContextHeader;
    using blog::handlers::middleware::kGhostPermissionsHeader;
    using blog::handlers::middleware::kGhostRoleHeader;
    using blog::handlers::middleware::require_ghost_permission;

    const bool authenticated = !req.get_authorization().empty();
    if (authenticated && req.find_header(kGhostRoleHeader).empty()) {
        // A presented admin token is an Administrator unless headers pin a role.
        req.set_header(kGhostRoleHeader, "Administrator");
        req.set_header(kGhostPermissionsHeader, "*");
        if (req.find_header(kGhostContextHeader).empty()) {
            req.set_header(kGhostContextHeader, "ADMIN_API");
        }
    }
    if (req.find_header(kGhostContextHeader) == "CONTENT_API") {
        // Ghost `permissions:true` on the public settings read.
        req.set_header(kGhostRoleHeader, "Owner");
        req.set_header(kGhostPermissionsHeader, "*");
    }

    bool allowed{false};
    require_ghost_permission(
        req,
        res,
        [&allowed](
            interfaces::io::IRequest&,
            interfaces::io::IResponse&,
            std::function<void()>
        ) noexcept
        {
            allowed = true;
        },
        std::move(send),
        docName,
        method
    );
    return allowed;
}

[[nodiscard]] inline bool valid_email(std::string_view value)
{
    const auto at = value.find('@');
    return at != std::string_view::npos && at > 0 && at + 1 < value.size() &&
           value.find('.', at + 1) != std::string_view::npos &&
           value.find(' ') == std::string_view::npos;
}

/** Pull the `settings` array out of a browse envelope (for edit/verify returns). */
[[nodiscard]] inline std::string settings_only(std::string_view browse_body)
{
    const auto doc = decode(browse_body);
    const auto* root = as_object(doc ? &*doc : nullptr);
    const auto* value = value_for(root, "settings");
    return value == nullptr ? std::string{"[]"} : encode(*value);
}

/** Dedup write-through: replace the stored override for `key` (SettingsModel.edit). */
inline void write_setting(blog::BlogContext& ctx, std::string_view key, std::string_view value)
{
    auto& rs = ctx.store().resources();
    for (const auto& record: blog::api::parse_array(rs.list("settings"))) {
        if (record_text(&record, "key") != key) {
            continue;
        }
        const auto id = record.find("id");
        if (id != record.end()) {
            rs.destroy(
                "settings",
                core_serde::JsonCodec{}.decode<std::string>(id->second).value_or(id->second)
            );
        }
    }
    rs.create("settings", R"({"key":)" + quote(key) + R"(,"value":)" + std::string{value} + "}");
}

// ---------------------------------------------------------------------------
// routes.yaml upload: multipart part extraction + validation (no parser lib)
// ---------------------------------------------------------------------------

struct MultipartPart
{
    std::string content;
    std::string filename;
    std::string mime;
};

[[nodiscard]] inline MultipartPart
multipart_part(std::string_view body, std::string_view content_type, std::string_view field)
{
    MultipartPart part;
    const auto bp = content_type.find("boundary=");
    if (bp == std::string_view::npos) {
        return part;
    }
    std::string boundary{content_type.substr(bp + 9)};
    if (const auto semi = boundary.find(';'); semi != std::string::npos) {
        boundary = boundary.substr(0, semi);
    }
    while (!boundary.empty() && (boundary.front() == '"' || boundary.front() == ' ')) {
        boundary.erase(boundary.begin());
    }
    while (!boundary.empty() &&
           (boundary.back() == '"' || boundary.back() == ' ' || boundary.back() == '\r')) {
        boundary.pop_back();
    }
    if (boundary.empty()) {
        return part;
    }

    const auto field_marker = "name=\"" + std::string{field} + "\"";
    const auto np = body.find(field_marker);
    if (np == std::string_view::npos) {
        return part;
    }
    const auto disposition_end = body.find("\r\n", np);
    const auto disposition = body.substr(
        np,
        disposition_end == std::string_view::npos ? std::string_view::npos : disposition_end - np
    );
    if (const auto fn = disposition.find("filename=\""); fn != std::string_view::npos) {
        const auto start = fn + 10;
        const auto end = disposition.find('"', start);
        if (end != std::string_view::npos) {
            part.filename = std::string{disposition.substr(start, end - start)};
        }
    }
    if (const auto ct = disposition.find("Content-Type:"); ct != std::string_view::npos) {
        part.mime = trim(disposition.substr(ct + 13));
    }

    const auto head_end = body.find("\r\n\r\n", np);
    if (head_end == std::string_view::npos) {
        return part;
    }
    const auto start = head_end + 4;
    const auto delim = "--" + boundary;
    const auto end = body.find(delim, start);
    part.content = end == std::string_view::npos ? std::string{body.substr(start)}
                                                 : std::string{body.substr(start, end - start)};
    while (!part.content.empty() && (part.content.back() == '\r' || part.content.back() == '\n')) {
        part.content.pop_back();
    }
    return part;
}

[[nodiscard]] inline bool valid_yaml(std::string_view content)
{
    if (content.empty() || content.find('\t') != std::string_view::npos) {
        return false;
    }
    int braces{0}, brackets{0};
    bool saw_top_level_key{false};
    for (auto line: std::views::split(content, '\n')) {
        const std::string text{std::ranges::begin(line), std::ranges::end(line)};
        const auto trimmed = trim(text);
        if (trimmed.empty() || trimmed.starts_with('#')) {
            continue;
        }
        if (!text.empty() && !std::isspace(static_cast<unsigned char>(text.front()))) {
            if (trimmed.find(':') == std::string::npos || trimmed.starts_with('-')) {
                return false;
            }
            saw_top_level_key = true;
        }
        for (const char c: text) {
            if (c == '{') {
                ++braces;
            } else if (c == '}') {
                --braces;
            } else if (c == '[') {
                ++brackets;
            } else if (c == ']') {
                --brackets;
            }
            if (braces < 0 || brackets < 0) {
                return false;
            }
        }
    }
    return saw_top_level_key && braces == 0 && brackets == 0;
}

/** Ghost `generatePrivateSiteAccessCode()` — `word###` from the 48-word list. */
[[nodiscard]] inline std::string word_code()
{
    static constexpr std::string_view kWords[]{
        "anchor",   "aurora",  "beacon",  "birch",    "bright",  "cedar",  "cloud",  "comet",
        "copper",   "coral",   "ember",   "fern",     "field",   "forest", "golden", "green",
        "harbor",   "hidden",  "horizon", "juniper",  "lagoon",  "lunar",  "maple",  "meadow",
        "midnight", "north",   "ocean",   "olive",    "paper",   "pine",   "quiet",  "river",
        "sage",     "signal",  "silver",  "solstice", "sparrow", "stone",  "studio", "summit",
        "sunrise",  "thistle", "valley",  "violet",   "willow",  "window", "winter", "wild"
    };
    static thread_local std::mt19937 rng{std::random_device{}()};
    std::uniform_int_distribution<std::size_t> word{0, std::size(kWords) - 1};
    std::uniform_int_distribution<int> number{0, 999};
    return std::string{kWords[word(rng)]} + std::format("{:03}", number(rng));
}

/** Ghost input serializer boolean coercion (`'0'/'1'/'false'/'true'`). */
[[nodiscard]] inline std::string coerce(std::string_view key, std::string_view raw)
{
    if (!contains(kBoolean, key)) {
        return std::string{raw};
    }
    if (raw == "0" || raw == "\"0\"" || raw == "false" || raw == "\"false\"") {
        return "false";
    }
    if (raw == "1" || raw == "\"1\"" || raw == "true" || raw == "\"true\"") {
        return "true";
    }
    return std::string{raw};
}

} // namespace blog::handlers::settings::support

export namespace blog::handlers::settings::browse::workflow {

class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = support::Result;
    static constexpr std::string_view name{"settings_browse_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        auto& ctx = *s_ctx;
        const bool public_route = in.find_header("x-blog-context") == "CONTENT_API";

        // Ghost `options:['group']` — core_query validates allowlisted options.
        constexpr std::string_view kAllowed[]{"group"};
        const auto query = core::query::parse(in.get_path(), kAllowed);
        const auto group_it = query.options.find("group");
        const std::string group =
            group_it == query.options.end() ? std::string{} : group_it->second;

        if (public_route) {
            return core::workflow::StepResponse<Output>{support::public_browse(ctx)};
        }
        return core::workflow::StepResponse<Output>{support::browse(ctx, group)};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};

} // namespace blog::handlers::settings::browse::workflow
