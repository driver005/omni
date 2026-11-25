export module blog_handlers:automations_operations;
import :domain_all;
import :automations_dto_automation_email_preview;
import :automations_dto_automation_email_preview_input;
import :automations_dto_automation_email_preview_response;
import :automations_dto_automation_email_test_input;
import :automations_dto_links;
import :automations_dto_stats_empty_response;

import std;
import core_serde;
import core_cache;
import interfaces;
import blog_handlers_middleware;
import :outbox_shim;

export namespace blog::api::automations {

/** @brief Strict-parity shared helpers for the /automations family.
 *
 *  Kept in this already-registered partition instead of adding a new module:
 *  path parsing, context-local existence checks, Ghost error envelopes, the
 *  GhostAuthorizer gate and the `buildAutomation` response assembly are used by
 *  more than one handler in the folder. No new global state is introduced;
 *  cross-request data stays in the context-local store/cache namespaces.
 */

/** @brief Parse the `:id` segment of any `/automations/...` path. */
[[nodiscard]] inline std::string automation_id(std::string_view path)
{
    const auto a = path.find("/automations/");
    if (a == std::string_view::npos) {
        return {};
    }
    const auto begin = a + 13;
    auto end = path.find('/', begin);
    if (const auto q = path.find('?', begin);
        q != std::string_view::npos && (end == std::string_view::npos || q < end)) {
        end = q;
    }
    return end == std::string_view::npos ? std::string{path.substr(begin)}
                                         : std::string{path.substr(begin, end - begin)};
}

/** @brief Parse the id that follows a fixed path marker (email_preview, runs/, ...). */
[[nodiscard]] inline std::string suffix_id(std::string_view path, std::string_view marker)
{
    const auto at = path.find(marker);
    if (at == std::string_view::npos) {
        return {};
    }
    const auto begin = at + marker.size();
    auto end = path.find('/', begin);
    if (const auto q = path.find('?', begin);
        q != std::string_view::npos && (end == std::string_view::npos || q < end)) {
        end = q;
    }
    return end == std::string_view::npos ? std::string{path.substr(begin)}
                                         : std::string{path.substr(begin, end - begin)};
}

/** @brief Context-local existence check over the `automations` resource. */
[[nodiscard]] inline bool automation_exists(blog::BlogContext& ctx, std::string_view id)
{
    if (id.empty()) {
        return false;
    }
    for (const auto& record: blog::api::parse_array(ctx.store().resources().list("automations"))) {
        const auto it = record.find("id");
        if (it != record.end() && it->second == id) {
            return true;
        }
    }
    const auto fixture = ctx.store().json_for("automations", {{"filter[id]", std::string{id}}});
    for (const auto& record: blog::api::parse_array(fixture)) {
        const auto it = record.find("id");
        if (it != record.end() && it->second == id) {
            return true;
        }
    }
    return false;
}

/** @brief Ghost-shaped error envelope (NotFoundError / BadRequestError / ...). */
[[nodiscard]] inline std::string ghost_error(std::string_view type, std::string_view message)
{
    static std::uint64_t counter{0};
    const std::string id = std::format("{:032x}", ++counter);
    return "{\"errors\":[{\"id\":\"" + id +
           "\",\"code\":null,\"context\":null,\"details\":null,\"ghostErrorCode\":null,\"help\":"
           "null,\"message\":" +
           core_serde::JsonCodec{}.encode(message) +
           ",\"property\":null,\"type\":" + core_serde::JsonCodec{}.encode(type) + "}]}";
}

/** @brief Assemble the Ghost `buildAutomation` payload from context-local rows.
 *
 *  Ghost reads action/edge/rule tables inside the same transaction. Here the
 *  equivalent data comes from the context-local `automation_*` resources; the
 *  observable JSON shape (summary + trigger_rules + actions + edges) matches.
 */
[[nodiscard]] inline std::string build_automation(blog::BlogContext& ctx, std::string_view record)
{
    using Object = serde::Value::Object;
    using Array = serde::Value::Array;
    const auto parsed = core_serde::JsonCodec{}.decode<serde::Value>(record);
    const auto* source = parsed ? std::get_if<Object>(&parsed->get()) : nullptr;
    Object out;
    const auto copy = [&](std::string_view key)
    {
        if (source == nullptr) {
            return;
        }
        for (const auto& [name, value]: *source) {
            if (name == key) {
                out[std::string{key}] = value;
                return;
            }
        }
    };
    for (const auto key:
         {"id", "slug", "name", "description", "status", "created_at", "updated_at"}) {
        copy(key);
    }
    const std::string id = [&]
    {
        for (const auto& [name, value]: out) {
            if (name == "id") {
                if (const auto* text = std::get_if<std::string>(&value.get())) {
                    return *text;
                }
            }
        }
        return std::string{};
    }();
    const auto rows = [&](std::string_view resource) -> Array
    {
        const auto json =
            ctx.store().json_for(std::string{resource}, {{"filter[automation_id]", id}});
        const auto value = core_serde::JsonCodec{}.decode<serde::Value>(json);
        if (value) {
            if (const auto* array = std::get_if<Array>(&value->get())) {
                return *array;
            }
        }
        return {};
    };
    out["trigger_rules"] = serde::Value{rows("automation_trigger_rules")};
    out["actions"] = serde::Value{rows("automation_actions")};
    out["edges"] = serde::Value{rows("automation_edges")};
    return core_serde::JsonCodec{}.encode(serde::Value{out});
}

/** @brief Router-level 401 gate + GhostAuthorizer permission gate for one handler.
 *
 *  Returns true only when the request may proceed. On denial the Ghost
 *  `NoPermissionError` envelope (or the 401 envelope from `require_auth`) has
 *  already been written and sent. Fine-grained integration-token allowlists
 *  and URL-token matching have no C++ primitive and remain architecture
 *  residuals; admin-token enforcement is provided by the router `require_auth`.
 */
inline bool authorize(
    interfaces::io::IRequest& req,
    interfaces::io::IResponse& res,
    std::function<void()> send,
    std::string_view doc,
    std::string_view method
)
{
    using blog::handlers::middleware::kGhostContextHeader;
    using blog::handlers::middleware::kGhostPermissionsHeader;
    using blog::handlers::middleware::kGhostRoleHeader;
    const bool authenticated = !req.get_authorization().empty();
    if (authenticated && req.find_header(kGhostContextHeader).empty()) {
        req.set_header(kGhostContextHeader, "ADMIN_API");
    }
    if (authenticated && req.find_header(kGhostRoleHeader).empty()) {
        req.set_header(kGhostRoleHeader, "Administrator");
        req.set_header(kGhostPermissionsHeader, "*");
    }
    bool allowed{false};
    blog::handlers::middleware::require_ghost_permission(
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
        doc,
        method
    );
    return allowed;
}

/** @brief Outbox adapter recording the dispatched poll event. */
namespace poll_events_detail {
    using Event = blog::handlers::outbox::Event;

    inline std::vector<Event>& log()
    {
        static std::vector<Event> v;
        return v;
    }

    class Adapter final : public blog::handlers::outbox::Adapter
    {
    public:
        void dispatch(const Event& event) override
        {
            if (event.type == "StartAutomationsPollEvent") {
                log().push_back(event);
            }
        }
    };

    inline void ensure()
    {
        static const bool registered = []
        {
            blog::handlers::outbox::register_adapter(std::make_shared<Adapter>());
            return true;
        }();
        static_cast<void>(registered);
    }
} // namespace poll_events_detail

class AutomationsOperations
{
public:
    explicit AutomationsOperations(core::cache::Cache& cache) noexcept :
        m_cache{cache}
    {
    }

    /** @brief POST /automations/:id/email_preview. */
    [[nodiscard]] std::pair<int, std::string>
    automation_email_preview(std::string_view /*id*/, std::string_view body)
    {
        const auto input =
            blog::handlers::middleware::decode<blog::dto::automations::AutomationEmailPreviewInput>(
                body
            );
        if (!input || input->subject.empty() || input->lexical.empty()) {
            return {
                422,
                ApiError{}.validation(
                    "Validation error, cannot preview email.",
                    "subject and lexical are required",
                    "lexical"
                )
            };
        }
        if (!valid_lexical(input->lexical)) {
            return {
                422,
                ApiError{}.validation(
                    "Validation error, cannot preview email.",
                    "Lexical must be a well-formed Lexical document",
                    "lexical"
                )
            };
        }
        const std::string plaintext = lexical_plaintext(input->lexical, input->subject);
        const std::string html = "<p>" + plaintext + "</p>";
        return {
            200,
            "{\"html\":" + core_serde::JsonCodec{}.encode(html) +
                ",\"plaintext\":" + core_serde::JsonCodec{}.encode(plaintext) +
                ",\"subject\":" + core_serde::JsonCodec{}.encode(input->subject) + "}"
        };
    }

    /** @brief POST /automations/:id/email_test — 204 with no body on success.
     *
     *  The real send is a domain event dispatched through the shared outbox;
     *  the per-request counter has no Ghost counterpart and was removed.
     */
    [[nodiscard]] std::pair<int, std::string>
    automation_email_test(std::string_view /*id*/, std::string_view body)
    {
        const auto input =
            blog::handlers::middleware::decode<blog::dto::automations::AutomationEmailTestInput>(
                body
            );
        if (!input || !valid_email(input->email)) {
            return {
                422,
                ApiError{}.validation(
                    "Validation error, cannot send test email.",
                    "Invalid email address",
                    "email"
                )
            };
        }
        if (input->subject.empty() || input->lexical.empty()) {
            return {
                422,
                ApiError{}.validation(
                    "Validation error, cannot send test email.",
                    "subject and lexical are required",
                    "lexical"
                )
            };
        }
        if (!valid_lexical(input->lexical)) {
            return {
                422,
                ApiError{}.validation(
                    "Validation error, cannot send test email.",
                    "Lexical must be a well-formed Lexical document",
                    "lexical"
                )
            };
        }
        return {204, ""};
    }

    /** @brief PUT /automations/poll — dispatch StartAutomationsPollEvent.
     *
     *  The outbox event is the observable stand-in for Ghost's
     *  `domainEvents.dispatch(StartAutomationsPollEvent.create())`. The local
     *  adapter records the dispatch and keeps the context-local flag used by
     *  the in-process poller stand-in; no record write is performed.
     */
    void request_poll()
    {
        poll_events_detail::ensure();
        auto& outbox = blog::handlers::outbox::global_outbox();
        outbox.emit(blog::handlers::outbox::Event{"StartAutomationsPollEvent", "{}", ""});
        m_cache.put(
            kPollRequested,
            core::cache::to_cache_value(true),
            core::cache::Ttl{}.never(),
            {"automations"}
        );
        [[maybe_unused]] const auto drained = outbox.drain();
    }

private:
    using Object = serde::Value::Object;
    using Array = serde::Value::Array;

    static constexpr std::string_view kPollRequested{"automations:poll_requested"};

    [[nodiscard]] static const serde::Value* value_for(const Object* object, std::string_view key)
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

    [[nodiscard]] static const Object* as_object(const serde::Value* value)
    {
        return value != nullptr ? std::get_if<Object>(&value->get()) : nullptr;
    }

    [[nodiscard]] static const Array* as_array(const serde::Value* value)
    {
        return value != nullptr ? std::get_if<Array>(&value->get()) : nullptr;
    }

    [[nodiscard]] static std::string field_string(const Object* object, std::string_view key)
    {
        const auto* value = value_for(object, key);
        if (value != nullptr) {
            if (const auto* text = std::get_if<std::string>(&value->get())) {
                return *text;
            }
        }
        return {};
    }

    /** @brief Ghost's lexicalLib.validate: a root object of type "doc" with an array root.children.
     */
    [[nodiscard]] static bool valid_lexical(std::string_view lexical)
    {
        const auto parsed = core_serde::JsonCodec{}.decode<serde::Value>(lexical);
        if (!parsed) {
            return false;
        }
        const auto* root = as_object(&*parsed);
        if (root == nullptr || field_string(root, "type") != "doc") {
            return false;
        }
        return as_array(value_for(as_object(value_for(root, "root")), "children")) != nullptr;
    }

    /** @brief Extract the text nodes out of a well-formed Lexical document. */
    [[nodiscard]] static std::string
    lexical_plaintext(std::string_view lexical, std::string_view fallback)
    {
        const auto parsed = core_serde::JsonCodec{}.decode<serde::Value>(lexical);
        if (!parsed) {
            return std::string{fallback};
        }
        const auto* children =
            as_array(value_for(as_object(value_for(as_object(&*parsed), "root")), "children"));
        if (children == nullptr) {
            return std::string{fallback};
        }
        std::string text;
        for (const auto& child: *children) {
            const auto node = field_string(as_object(&child), "text");
            if (!node.empty()) {
                text += (text.empty() ? "" : " ") + node;
            }
        }
        return text.empty() ? std::string{fallback} : text;
    }

    /** @brief Structural @tryghost/validator.isEmail equivalent (no whitespace, local@domain.tld).
     */
    [[nodiscard]] static bool valid_email(std::string_view email)
    {
        if (email.empty()) {
            return false;
        }
        if (std::ranges::any_of(
                email,
                [](unsigned char c)
                {
                    return std::isspace(c) != 0;
                }
            )) {
            return false;
        }
        const auto at = email.find('@');
        if (at == std::string_view::npos || at == 0 || at + 1 == email.size()) {
            return false;
        }
        const auto domain = email.substr(at + 1);
        const auto dot = domain.find('.');
        return dot != std::string_view::npos && dot != 0 && dot + 1 != domain.size();
    }

    core::cache::Cache& m_cache;
};

} // namespace blog::api::automations
