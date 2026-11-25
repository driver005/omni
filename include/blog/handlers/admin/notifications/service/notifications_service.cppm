// Ghost notification domain service (strict parity).
//
// Port of Ghost's
//   core/server/services/notifications/notifications.js
//   core/server/api/endpoints/notifications.js
//   core/server/api/endpoints/utils/serializers/output/notifications.js
//
// Uses only the shared abstractions contracted in docs/parity/behavior/CONTRACTS.md:
//   core_query (input pipeline), core_outbox + core_adapters (settings-edit side effects),
//   local_mapping::wrap (Ghost output envelope) and the existing context store.
module;
#include <time.h>
export module blog_handlers:notifications_service;

import :domain_all;
import :outbox_shim;
import std;
import interfaces;
import core_serde;

// shared local_query/local_outbox now provided by :outbox_shim

export namespace blog::handlers::notifications::service {

/** @brief Ghost's blog version (package.json / @tryghost/version). */
inline constexpr std::string_view kBlogVersion{"6.78.0-rc.0"};
inline constexpr std::string_view kSettingsKey{"notifications"};
/** @brief Existing-store namespace used as Ghost's `settings` key/value row. */
inline constexpr std::string_view kSettingsResource{"notification_settings"};

class NotificationService
{
public:
    /** @brief Ghost `notifications.browse({user:{id}})` + serializer `all()`. */
    [[nodiscard]] static std::string
    browse(blog::BlogContext& ctx, std::string_view path, std::string_view user_id);

    /** @brief Ghost `notifications.add()` + settings BREAD edit. */
    [[nodiscard]] static std::pair<int, std::string>
    add(blog::BlogContext& ctx, std::string_view body);

    /** @brief Ghost `notifications.destroy()` + settings BREAD edit. */
    [[nodiscard]] static std::pair<int, std::string>
    destroy(blog::BlogContext& ctx, std::string_view id, std::string_view user_id);
};

namespace detail {

    using Record = blog::api::RecordDto; // std::map<std::string, std::string> of raw JSON values

    [[nodiscard]] inline std::string_view trim(std::string_view s)
    {
        while (!s.empty() && std::isspace(static_cast<unsigned char>(s.front()))) {
            s.remove_prefix(1);
        }
        while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back()))) {
            s.remove_suffix(1);
        }
        return s;
    }

    [[nodiscard]] inline std::string lower(std::string_view s)
    {
        std::string out;
        out.reserve(s.size());
        for (const unsigned char c: s) {
            out.push_back(static_cast<char>(std::tolower(c)));
        }
        return out;
    }

    [[nodiscard]] inline std::string quote(std::string_view text)
    {
        return blog::api::quote_fragment(text);
    }

    [[nodiscard]] inline std::string unquote(std::string_view value)
    {
        const auto text = trim(value);
        if (text.size() >= 2 && text.front() == '"' && text.back() == '"') {
            std::string out;
            out.reserve(text.size() - 2);
            for (std::size_t i = 1; i + 1 < text.size(); ++i) {
                if (text[i] == '\\' && i + 2 < text.size()) {
                    out.push_back(text[i + 1]);
                    ++i;
                } else {
                    out.push_back(text[i]);
                }
            }
            return out;
        }
        return std::string{text};
    }

    [[nodiscard]] inline bool has(const Record& record, std::string_view key)
    {
        return record.find(std::string{key}) != record.end();
    }

    [[nodiscard]] inline std::string raw_field(const Record& record, std::string_view key)
    {
        const auto it = record.find(std::string{key});
        return it == record.end() ? std::string{} : it->second;
    }

    [[nodiscard]] inline std::string str_field(const Record& record, std::string_view key)
    {
        return unquote(raw_field(record, key));
    }

    [[nodiscard]] inline bool bool_field(const Record& record, std::string_view key)
    {
        const auto text = lower(str_field(record, key));
        return text == "true" || text == "1";
    }

    [[nodiscard]] inline bool contains_ci(std::string_view haystack, std::string_view needle)
    {
        return lower(haystack).contains(lower(needle));
    }

    [[nodiscard]] inline std::vector<std::string> string_array(std::string_view raw)
    {
        std::vector<std::string> out;
        for (auto part: std::views::split(std::string_view{raw}, ',')) {
            const std::string text{std::ranges::begin(part), std::ranges::end(part)};
            if (!trim(text).empty()) {
                out.push_back(unquote(text));
            }
        }
        return out;
    }

    [[nodiscard]] inline std::string serialize_string_array(const std::vector<std::string>& values)
    {
        std::string out{"["};
        for (std::size_t i = 0; i < values.size(); ++i) {
            if (i) {
                out += ',';
            }
            out += quote(values[i]);
        }
        return out + ']';
    }

    [[nodiscard]] inline std::string serialize_array(const std::vector<Record>& records)
    {
        std::string out{"["};
        for (std::size_t i = 0; i < records.size(); ++i) {
            if (i) {
                out += ',';
            }
            out += blog::api::record_to_json(records[i]);
        }
        return out + ']';
    }

    /** @brief Ghost `sanitizeNotificationHtml`: semantic tags only, no scripts/styles/inline CSS.
     */
    [[nodiscard]] inline bool allowed_tag(std::string_view name)
    {
        static constexpr std::string_view kTags[]{
            "p",
            "br",
            "hr",
            "strong",
            "b",
            "em",
            "i",
            "u",
            "code",
            "a",
            "ul",
            "ol",
            "li",
            "blockquote",
            "h1",
            "h2",
            "h3",
            "h4"
        };
        return std::ranges::any_of(
            kTags,
            [&](std::string_view tag)
            {
                return name == tag;
            }
        );
    }

    [[nodiscard]] inline std::string attribute(std::string_view tag, std::string_view name)
    {
        const auto pos = lower(tag).find(std::string{name} + "=");
        if (pos == std::string_view::npos) {
            return {};
        }
        auto rest = trim(tag.substr(pos + name.size() + 1));
        if (rest.empty()) {
            return {};
        }
        if (rest.front() == '"' || rest.front() == '\'') {
            const char q = rest.front();
            rest.remove_prefix(1);
            const auto end = rest.find(q);
            if (end == std::string_view::npos) {
                return {};
            }
            return std::string{rest.substr(0, end)};
        }
        const auto end = rest.find_first_of(" \t/>");
        return std::string{rest.substr(0, end)};
    }

    [[nodiscard]] inline bool safe_href(std::string_view href)
    {
        const auto text = lower(trim(href));
        return text.starts_with("http://") || text.starts_with("https://") ||
               text.starts_with("mailto:");
    }

    [[nodiscard]] inline std::string sanitize_html(std::string_view html)
    {
        std::string out;
        out.reserve(html.size());
        std::size_t i = 0;
        while (i < html.size()) {
            if (html[i] != '<') {
                out.push_back(html[i++]);
                continue;
            }
            const auto end = html.find('>', i);
            if (end == std::string_view::npos) {
                out += "&lt;";
                ++i;
                continue;
            }
            const auto tag = trim(html.substr(i + 1, end - i - 1));
            bool closing = tag.starts_with('/');
            auto name_text = tag;
            if (closing) {
                name_text.remove_prefix(1);
            }
            std::size_t length = 0;
            while (length < name_text.size() &&
                   (std::isalnum(static_cast<unsigned char>(name_text[length])) ||
                    name_text[length] == '1')) {
                ++length;
            }
            const auto name = lower(name_text.substr(0, length));
            if (!allowed_tag(name)) {
                // sanitize-html drops the content of non-text tags (script/style/...).
                if (!closing && (name == "script" || name == "style" || name == "textarea" ||
                                 name == "option")) {
                    const auto close = lower(html).find("</" + std::string{name}, end + 1);
                    if (close != std::string_view::npos) {
                        const auto close_end = html.find('>', close);
                        i = close_end == std::string_view::npos ? html.size() : close_end + 1;
                        continue;
                    }
                }
                // Other disallowed tags are removed but their text content survives.
                i = end + 1;
                continue;
            }
            if (closing) {
                out += "</" + std::string{name} + ">";
            } else if (name == "a") {
                const auto href = attribute(tag, "href");
                const auto title = attribute(tag, "title");
                out += "<a";
                if (safe_href(href)) {
                    out += " href=\"" + href + "\"";
                }
                if (!title.empty()) {
                    out += " title=\"" + title + "\"";
                }
                out += " target=\"_blank\" rel=\"noopener noreferrer\">";
            } else {
                out += "<" + std::string{name} + ">";
            }
            i = end + 1;
        }
        return out;
    }

    struct SemVer
    {
        int major{0};
        int minor{0};
        int patch{0};
        std::string pre{};
    };

    [[nodiscard]] inline SemVer parse_semver(std::string_view text)
    {
        SemVer result;
        auto value = trim(text);
        if (value.starts_with('v') || value.starts_with('V')) {
            value.remove_prefix(1);
        }
        const auto take_number = [&](int& target)
        {
            std::size_t n = 0;
            while (n < value.size() && std::isdigit(static_cast<unsigned char>(value[n]))) {
                ++n;
            }
            if (n == 0) {
                return;
            }
            target = std::stoi(std::string{value.substr(0, n)});
            value.remove_prefix(n);
        };
        take_number(result.major);
        if (value.starts_with('.')) {
            value.remove_prefix(1);
            take_number(result.minor);
        }
        if (value.starts_with('.')) {
            value.remove_prefix(1);
            take_number(result.patch);
        }
        if (value.starts_with('-')) {
            result.pre = std::string{value.substr(1)};
        }
        return result;
    }

    [[nodiscard]] inline int compare_semver(const SemVer& a, const SemVer& b)
    {
        if (a.major != b.major) {
            return a.major < b.major ? -1 : 1;
        }
        if (a.minor != b.minor) {
            return a.minor < b.minor ? -1 : 1;
        }
        if (a.patch != b.patch) {
            return a.patch < b.patch ? -1 : 1;
        }
        if (a.pre == b.pre) {
            return 0;
        }
        if (a.pre.empty()) {
            return 1; // release > prerelease
        }
        if (b.pre.empty()) {
            return -1;
        }
        return a.pre < b.pre ? -1 : 1;
    }

    [[nodiscard]] inline bool semver_gte(std::string_view a, std::string_view b)
    {
        return compare_semver(parse_semver(a), parse_semver(b)) >= 0;
    }

    [[nodiscard]] inline bool semver_gt(std::string_view a, std::string_view b)
    {
        return compare_semver(parse_semver(a), parse_semver(b)) > 0;
    }

    /** @brief Extract the first x.y.z lexeme from free text (Ghost's version regex). */
    [[nodiscard]] inline std::string version_in_text(std::string_view text)
    {
        for (std::size_t i = 0; i + 4 < text.size(); ++i) {
            if (!std::isdigit(static_cast<unsigned char>(text[i]))) {
                continue;
            }
            std::size_t n = i;
            int groups = 0;
            while (n < text.size() && groups < 3) {
                std::size_t digits = 0;
                while (n < text.size() && std::isdigit(static_cast<unsigned char>(text[n]))) {
                    ++n;
                    ++digits;
                }
                if (digits == 0) {
                    break;
                }
                ++groups;
                if (n < text.size() && text[n] == '.' && groups < 3) {
                    ++n;
                } else {
                    break;
                }
            }
            if (groups == 3) {
                return std::string{text.substr(i, n - i)};
            }
        }
        return {};
    }

    [[nodiscard]] inline std::string ghost_major_in_text(std::string_view text)
    {
        const auto lowered = lower(text);
        const auto pos = lowered.find("ghost ");
        if (pos == std::string_view::npos || pos + 6 >= lowered.size()) {
            return {};
        }
        if (!std::isdigit(static_cast<unsigned char>(lowered[pos + 6]))) {
            return {};
        }
        if (!contains_ci(text.substr(pos), ".0 is now available")) {
            return {};
        }
        return std::string{lowered.substr(pos + 6, 1)};
    }

    [[nodiscard]] inline bool was_seen(const Record& notification, std::string_view user_id)
    {
        if (has(notification, "seenBy")) {
            const auto ids = string_array(raw_field(notification, "seenBy"));
            return std::ranges::find(ids, std::string{user_id}) != ids.end();
        }
        return bool_field(notification, "seen");
    }

    [[nodiscard]] inline bool visible(const Record& notification, std::string_view user_id)
    {
        const bool seen = was_seen(notification, user_id);
        if (has(notification, "createdAtVersion") && !seen) {
            return semver_gte(str_field(notification, "createdAtVersion"), kBlogVersion);
        }

        const auto message = str_field(notification, "message");
        const bool major = !ghost_major_in_text(message).empty();
        const bool sec43 = contains_ci(message, "GHSA-9fgx-q25h-jxrg");
        if (!message.empty() && (!bool_field(notification, "custom") || major || sec43)) {
            auto version = version_in_text(message);
            if (version.empty() && sec43) {
                version = "4.3.3";
            }
            const auto ghost_major = ghost_major_in_text(message);
            if (!ghost_major.empty()) {
                version = ghost_major + ".0.0";
            }
            if (!version.empty() && semver_gt(version, kBlogVersion)) {
                return true;
            }
            return false;
        }
        return !seen;
    }

    /** @brief Ghost `fetchAllNotifications()` — reads settings, sanitizes on read. */
    [[nodiscard]] inline std::vector<Record> fetch_all(blog::BlogContext& ctx)
    {
        std::vector<Record> notifications;
        const auto rows = blog::api::parse_array(ctx.store().resources().list(kSettingsResource));
        if (!rows.empty()) {
            const auto it = rows.back().find("value");
            if (it != rows.back().end()) {
                notifications = blog::api::parse_array(it->second);
            }
        }
        for (auto& notification: notifications) {
            if (has(notification, "message")) {
                notification["message"] = quote(sanitize_html(str_field(notification, "message")));
            }
        }
        return notifications;
    }

    /** @brief Ghost `settingsBREADService.edit(key:'notifications', ...)` via the existing store.
     */
    inline void persist(blog::BlogContext& ctx, const std::vector<Record>& notifications)
    {
        auto& resources = ctx.store().resources();
        for (const auto& row: blog::api::parse_array(resources.list(kSettingsResource))) {
            const auto it = row.find("id");
            if (it != row.end()) {
                resources.destroy(kSettingsResource, unquote(it->second));
            }
        }
        const std::string value = serialize_array(notifications);
        resources.create(
            kSettingsResource,
            R"({"key":")" + std::string{kSettingsKey} + R"(","value":)" + value + "}"
        );
    }

    /** @brief Observable side effect: record the settings call (adapters) and drain the outbox. */
    inline void settings_edit_side_effect(const std::vector<Record>& notifications)
    {
        const std::string value = serialize_array(notifications);
        const std::string payload =
            R"({"key":")" + std::string{kSettingsKey} + R"(","value":)" + value + "}";
        local_outbox::global_outbox().emit(
            local_outbox::Event{
                .type = "settings.edit",
                .payload = payload,
                .aggregate_id = "notifications"
            }
        );
        static_cast<void>(local_outbox::global_outbox().drain());
    }

    [[nodiscard]] inline std::string make_object_id()
    {
        static thread_local std::mt19937 rng{std::random_device{}()};
        std::uniform_int_distribution<int> digit{0, 15};
        std::string id;
        id.reserve(24);
        for (int i = 0; i < 24; ++i) {
            id.push_back("0123456789abcdef"[digit(rng)]);
        }
        return id;
    }

    [[nodiscard]] inline std::string now_iso()
    {
        const auto time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        std::tm parts{};
        gmtime_r(&time, &parts);
        char buffer[32]{};
        std::strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S.000Z", &parts);
        return std::string{buffer};
    }

    /** @brief Ghost output serializer: drop seen/seenBy/addedAt/createdAtVersion. */
    [[nodiscard]] inline std::vector<Record> serialize_clean(const std::vector<Record>& records)
    {
        auto out = records;
        for (auto& record: out) {
            record.erase("seen");
            record.erase("seenBy");
            record.erase("addedAt");
            record.erase("createdAtVersion");
        }
        return out;
    }

} // namespace detail

inline std::string
NotificationService::browse(blog::BlogContext& ctx, std::string_view path, std::string_view user_id)
{
    auto notifications = detail::fetch_all(ctx);
    std::stable_sort(
        notifications.begin(),
        notifications.end(),
        [](const detail::Record& a, const detail::Record& b)
        {
            return detail::str_field(a, "addedAt") > detail::str_field(b, "addedAt");
        }
    );

    std::vector<detail::Record> visible;
    for (const auto& notification: notifications) {
        if (detail::visible(notification, user_id)) {
            visible.push_back(notification);
        }
    }

    std::string rows = detail::serialize_array(detail::serialize_clean(visible));
    static constexpr std::array<std::string_view, 6>
        kAllowed{"limit", "page", "offset", "order", "fields", "filter"};
    const auto query = local_query::parse(path, kAllowed);
    try {
        rows = local_query::apply(rows, query, rows.size() ? rows.size() : 15UL);
    } catch (...) {
        return blog::api::ApiError{}.validation("Invalid Ghost query", "Invalid Ghost query", "");
    }
    return local_mapping::wrap("notifications", rows);
}

inline std::pair<int, std::string>
NotificationService::add(blog::BlogContext& ctx, std::string_view body)
{
    const auto fields = blog::api::JsonBody{}.parse(body);
    const auto it = fields.find("notifications");
    const auto incoming =
        blog::api::parse_array(it == fields.end() ? std::string{"[]"} : it->second);
    if (incoming.empty()) {
        return {200, local_mapping::wrap("notifications", "[]")};
    }

    auto all = detail::fetch_all(ctx);
    std::vector<detail::Record> to_add;
    const auto object_id = detail::make_object_id();
    for (const auto& candidate: incoming) {
        const auto duplicate = std::ranges::any_of(
            all,
            [&](const detail::Record& existing)
            {
                return detail::str_field(existing, "id") == detail::str_field(candidate, "id");
            }
        );
        if (duplicate) {
            continue;
        }
        if (!detail::has(candidate, "message") ||
            detail::raw_field(candidate, "message") == "null") {
            return {
                422,
                blog::api::ApiError{}.validation(
                    "Notification message must be a string.",
                    "Notification message must be a string.",
                    ""
                )
            };
        }

        detail::Record record = candidate;
        if (!detail::has(record, "dismissible")) {
            record["dismissible"] = "true";
        }
        if (!detail::has(record, "location")) {
            record["location"] = "\"bottom\"";
        }
        if (!detail::has(record, "status")) {
            record["status"] = "\"alert\"";
        }
        if (!detail::has(record, "id")) {
            record["id"] = detail::quote(object_id);
        }
        if (!detail::has(record, "createdAtVersion")) {
            record["createdAtVersion"] = detail::quote(kBlogVersion);
        }
        record["seen"] = "false";
        record["addedAt"] = detail::quote(detail::now_iso());
        record["message"] =
            detail::quote(detail::sanitize_html(detail::str_field(candidate, "message")));
        to_add.push_back(std::move(record));
    }

    const bool has_release = std::ranges::any_of(
        incoming,
        [](const detail::Record& record)
        {
            return !detail::bool_field(record, "custom");
        }
    );
    if (has_release) {
        std::erase_if(
            all,
            [](const detail::Record& record)
            {
                return !detail::bool_field(record, "custom");
            }
        );
    }

    if (to_add.empty()) {
        detail::persist(ctx, all);
        detail::settings_edit_side_effect(all);
        return {200, local_mapping::wrap("notifications", "[]")};
    }

    std::vector<detail::Record> releases;
    for (const auto& record: to_add) {
        if (!detail::bool_field(record, "custom")) {
            releases.push_back(record);
        }
    }
    if (releases.size() > 1) {
        std::stable_sort(
            releases.begin(),
            releases.end(),
            [](const detail::Record& a, const detail::Record& b)
            {
                return detail::str_field(a, "created_at") > detail::str_field(b, "created_at");
            }
        );
        detail::Record newest = releases.front();
        std::erase_if(
            to_add,
            [](const detail::Record& record)
            {
                return !detail::bool_field(record, "custom");
            }
        );
        to_add.push_back(std::move(newest));
    }

    auto stored = all;
    stored.insert(stored.end(), to_add.begin(), to_add.end());
    detail::persist(ctx, stored);
    detail::settings_edit_side_effect(stored);

    return {
        201,
        local_mapping::wrap(
            "notifications",
            detail::serialize_array(detail::serialize_clean(to_add))
        )
    };
}

inline std::pair<int, std::string>
NotificationService::destroy(blog::BlogContext& ctx, std::string_view id, std::string_view user_id)
{
    auto all = detail::fetch_all(ctx);
    const auto position = std::ranges::find_if(
        all,
        [&](const detail::Record& record)
        {
            return detail::str_field(record, "id") == id;
        }
    );
    if (position == all.end()) {
        return {
            404,
            blog::api::ApiError{}.validation(
                "Notification does not exist.",
                "Notification does not exist.",
                "",
                "NotFoundError"
            )
        };
    }
    if (!detail::bool_field(*position, "dismissible")) {
        return {
            403,
            blog::api::ApiError{}.validation(
                "You do not have permission to dismiss this notification.",
                "You do not have permission to dismiss this notification.",
                "",
                "NoPermissionError"
            )
        };
    }
    if (!detail::was_seen(*position, user_id)) {
        (*position)["seen"] = "true";
        auto seen_by = detail::string_array(detail::raw_field(*position, "seenBy"));
        seen_by.emplace_back(user_id);
        (*position)["seenBy"] = detail::serialize_string_array(seen_by);
    }

    detail::persist(ctx, all);
    detail::settings_edit_side_effect(all);
    return {204, ""};
}

} // namespace blog::handlers::notifications::service
