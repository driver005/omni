export module blog_handlers:announcements_announcement_get_step;
import :domain_all;
import std;
import core_workflow;
import core_serde;
import interfaces;

export namespace blog::handlers::announcements::announcement_get::workflow {

namespace announcement_detail {

// Ghost ANNOUNCEMENT_VISIBILITY values (announcement.values.ts:4-7).
inline constexpr std::string_view kVisitors{"visitors"};
inline constexpr std::string_view kFreeMembers{"free_members"};
inline constexpr std::string_view kPaidMembers{"paid_members"};

// Decode a store value (a JSON fragment) to plain text. "null"/empty -> empty.
[[nodiscard]] inline std::string text(std::string_view raw)
{
    if (raw.empty() || raw == "null") {
        return {};
    }
    if (raw.front() == '"') {
        if (const auto decoded = core_serde::JsonCodec{}.decode<std::string>(raw)) {
            return *decoded;
        }
        return std::string{raw.substr(1, raw.size() - 2)};
    }
    return std::string{raw};
}

// Parse announcement_visibility as a typed JSON array of Ghost tokens and test
// exact membership - never a bare substring match (announcements.js:22-27).
[[nodiscard]] inline bool visibility_contains(std::string_view raw, std::string_view wanted)
{
    const auto arr = text(raw); // e.g. ["visitors","free_members"]
    std::size_t i = 0;
    while (i < arr.size()) {
        const auto q1 = arr.find('"', i);
        if (q1 == std::string_view::npos) {
            break;
        }
        std::string token;
        bool closed = false;
        for (std::size_t j = q1 + 1; j < arr.size(); ++j) {
            if (arr[j] == '\\' && j + 1 < arr.size()) {
                token += arr[++j];
            } else if (arr[j] == '"') {
                i = j + 1;
                closed = true;
                break;
            } else {
                token += arr[j];
            }
        }
        if (!closed) {
            break;
        }
        if (token == wanted) {
            return true;
        }
    }
    return false;
}

} // namespace announcement_detail

class ExecuteStep {
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"announcements_announcement_get_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output> invoke(
        const Input& in,
        core::workflow::WorkflowContext&
    ) const {
        auto& ctx = *s_ctx;

        // Ghost reads frame.options.context?.member (announcements.js:47). C++
        // has no member-session primitive; status is carried by X-Member-Status
        // (codebase convention). Missing -> visitor, "free" -> free, any other
        // non-empty value (incl. comped) -> paid (announcements.js:8,24-27).
        const std::string status{in.find_header("X-Member-Status")};
        const bool has_member = !status.empty();
        const bool is_free = status == "free";
        const bool is_paid = has_member && !is_free;

        // Ghost settingsCache.get('announcement_*') (announcements.js:14-16).
        // The context-local settings store is the shared equivalent. Read-only:
        // no seeding writes - absent keys take Ghost's DB defaults. An injected
        // "announcement_settings" record set is accepted as a read-only fixture
        // source for contexts that cannot seed json_for("settings").
        auto rows = blog::api::parse_array(ctx.store().json_for("settings", {}));
        const auto injected = blog::api::parse_array(
            ctx.store().resources().list("announcement_settings")
        );
        rows.insert(rows.end(), injected.begin(), injected.end());

        auto setting = [&rows](std::string_view key) -> std::string {
            for (const auto& rec : rows) {
                const auto kit = rec.find("key");
                if (kit != rec.end() &&
                    announcement_detail::text(kit->second) == key) {
                    const auto vit = rec.find("value");
                    return vit == rec.end() ? std::string{} : vit->second;
                }
                if (const auto direct = rec.find(std::string{key}); direct != rec.end()) {
                    return direct->second;
                }
            }
            return {};
        };

        // Ghost DB defaults: content null (hidden), background "dark",
        // visibility "[]" (default-settings.json:634-647).
        const auto content = announcement_detail::text(setting("announcement_content"));
        const auto visibility = setting("announcement_visibility");
        const auto background_decoded = announcement_detail::text(setting("announcement_background"));
        const auto background = background_decoded.empty()
            ? std::string{"dark"}
            : background_decoded;

        // Visibility: visitors -> everyone; free_members -> visitor+member;
        // paid_members -> paid member (announcements.js:20-27).
        const bool visible = !content.empty() && (
            (!has_member && announcement_detail::visibility_contains(
                                visibility, announcement_detail::kVisitors)) ||
            (is_free && announcement_detail::visibility_contains(
                            visibility, announcement_detail::kFreeMembers)) ||
            (is_paid && announcement_detail::visibility_contains(
                            visibility, announcement_detail::kPaidMembers))
        );

        std::string body{"{}"};
        if (visible) {
            body = std::format(
                R"({{"announcement":{},"announcement_background":{}}})",
                core_serde::JsonCodec{}.encode(content),
                core_serde::JsonCodec{}.encode(background)
            );
        }

        return core::workflow::StepResponse<Output>{{{"status", "200"}, {"body", body}}};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};

} // namespace blog::handlers::announcements::announcement_get::workflow
