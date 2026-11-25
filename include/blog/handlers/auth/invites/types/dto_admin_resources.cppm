module;
#include <ctime>
#include <rfl/Generic.hpp>
export module blog_handlers:invites_dto_admin_resources;

import std;
import core_serde;
import :domain_all;
import core_query;
import core_auth;
import :outbox_shim;

export namespace blog::dto::invites {

namespace local_mapping {
    [[nodiscard]] inline std::string
    wrap(std::string_view resource, std::string_view rows, std::string_view meta = {})
    {
        return "{\"" + std::string{resource} + "\":" + std::string{rows} +
               (meta.empty() ? std::string{} : ",\"meta\":" + std::string{meta}) + "}";
    }

    [[nodiscard]] inline std::string clean(std::string_view json, std::string_view)
    {
        return std::string{json};
    }

    [[nodiscard]] inline std::string pagination(
        std::size_t total,
        std::size_t limit,
        std::size_t page,
        std::size_t pages,
        std::string_view prev,
        std::string_view next
    )
    {
        return std::format(
            R"("total":{},"limit":{},"page":{},"pages":{},"prev":{},"next":{})",
            total,
            limit,
            page,
            pages,
            prev.empty() ? std::string{"null"} : std::string{prev},
            next.empty() ? std::string{"null"} : std::string{next}
        );
    }
} // namespace local_mapping

class RoleDto
{
public:
    void set_id(std::string value)
    {
        id = std::move(value);
    }

    [[nodiscard]] const std::string& get_id() const noexcept
    {
        return id;
    }

    void set_name(std::string value)
    {
        name = std::move(value);
    }

    [[nodiscard]] const std::string& get_name() const noexcept
    {
        return name;
    }

    void set_description(std::string value)
    {
        description = std::move(value);
    }

    [[nodiscard]] const std::string& get_description() const noexcept
    {
        return description;
    }

    std::string id{"1"};
    std::string name{"Administrator"};
    std::string description{"Full access"};
};

class RolesResponse
{
public:
    void set_roles(std::vector<RoleDto> value)
    {
        roles = std::move(value);
    }

    [[nodiscard]] const std::vector<RoleDto>& get_roles() const noexcept
    {
        return roles;
    }

    std::vector<RoleDto> roles{};
};

class InviteDto
{
public:
    void set_id(std::string value)
    {
        id = std::move(value);
    }

    [[nodiscard]] const std::string& get_id() const noexcept
    {
        return id;
    }

    void set_email(std::string value)
    {
        email = std::move(value);
    }

    [[nodiscard]] const std::string& get_email() const noexcept
    {
        return email;
    }

    void set_status(std::string value)
    {
        status = std::move(value);
    }

    [[nodiscard]] const std::string& get_status() const noexcept
    {
        return status;
    }

    std::string id{"1"};
    std::string email{"invite@example.com"};
    std::string status{"sent"};
};

class InvitesResponse
{
public:
    void set_invites(std::vector<InviteDto> value)
    {
        invites = std::move(value);
    }

    [[nodiscard]] const std::vector<InviteDto>& get_invites() const noexcept
    {
        return invites;
    }

    std::vector<InviteDto> invites{};
};

class NotificationDto
{
public:
    void set_id(std::string value)
    {
        id = std::move(value);
    }

    [[nodiscard]] const std::string& get_id() const noexcept
    {
        return id;
    }

    void set_message(std::string value)
    {
        message = std::move(value);
    }

    [[nodiscard]] const std::string& get_message() const noexcept
    {
        return message;
    }

    void set_type(std::string value)
    {
        type = std::move(value);
    }

    [[nodiscard]] const std::string& get_type() const noexcept
    {
        return type;
    }

    std::string id{"1"};
    std::string message{"Ghost notification"};
    std::string type{"info"};
};

class NotificationsResponse
{
public:
    void set_notifications(std::vector<NotificationDto> value)
    {
        notifications = std::move(value);
    }

    [[nodiscard]] const std::vector<NotificationDto>& get_notifications() const noexcept
    {
        return notifications;
    }

    std::vector<NotificationDto> notifications{};
};

class WebhookDto
{
public:
    void set_id(std::string value)
    {
        id = std::move(value);
    }

    [[nodiscard]] const std::string& get_id() const noexcept
    {
        return id;
    }

    void set_event(std::string value)
    {
        event = std::move(value);
    }

    [[nodiscard]] const std::string& get_event() const noexcept
    {
        return event;
    }

    void set_target_url(std::string value)
    {
        target_url = std::move(value);
    }

    [[nodiscard]] const std::string& get_target_url() const noexcept
    {
        return target_url;
    }

    void set_name(std::string value)
    {
        name = std::move(value);
    }

    [[nodiscard]] const std::string& get_name() const noexcept
    {
        return name;
    }

    std::string id{"1"};
    std::string event{"post.published"};
    std::string target_url{"https://example.com/hook"};
    std::string name{"Webhook"};
};

class WebhooksResponse
{
public:
    void set_webhooks(std::vector<WebhookDto> value)
    {
        webhooks = std::move(value);
    }

    [[nodiscard]] const std::vector<WebhookDto>& get_webhooks() const noexcept
    {
        return webhooks;
    }

    std::vector<WebhookDto> webhooks{};
};

} // namespace blog::dto::invites

// ---------------------------------------------------------------------------
// Shared invite helpers. Kept in this already-registered partition so every
// invite step/handler uses the same Ghost-shaped primitives (CONTRACTS.md):
//   core::query  - NQL / option parsing, filtering, pagination
//   core::auth   - Ghost permission service
//   blog::mapping- output clean (token stripping) + envelopes
//   outbox_shim  - observable invite-mail side effects
// No shared file outside include/blog/handlers/auth/invites/ is modified.
// ---------------------------------------------------------------------------
export namespace blog::handlers::invites::support {

inline constexpr std::string_view kDocName{"invites"};

[[nodiscard]] inline std::string quote(std::string_view text)
{
    return blog::api::JsonBody{}.quote(text);
}

[[nodiscard]] inline std::string unquote(std::string_view value)
{
    auto text = value;
    while (!text.empty() && std::isspace(static_cast<unsigned char>(text.front()))) {
        text.remove_prefix(1);
    }
    while (!text.empty() && std::isspace(static_cast<unsigned char>(text.back()))) {
        text.remove_suffix(1);
    }
    if (text.size() >= 2 && text.front() == '"' && text.back() == '"') {
        std::string out;
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

[[nodiscard]] inline std::string
plain_field(const blog::api::RecordDto& record, std::string_view key)
{
    const auto it = record.find(std::string{key});
    return it == record.end() ? std::string{} : unquote(it->second);
}

/** @brief model/invite.js#toJSON deletes `token` before serialisation. */
[[nodiscard]] inline std::string clean(std::string_view json)
{
    return blog::dto::invites::local_mapping::clean(json, "invite");
}

[[nodiscard]] inline std::string wrap(std::string_view rows, std::string_view meta = {})
{
    return blog::dto::invites::local_mapping::wrap(kDocName, rows, meta);
}

/** @brief Opaque stand-in for security.tokens.generateFromEmail. */
[[nodiscard]] inline std::string new_token()
{
    std::random_device rd;
    std::mt19937_64 rng{
        rd() ^
        static_cast<std::uint64_t>(std::chrono::steady_clock::now().time_since_epoch().count())
    };
    std::ostringstream out;
    out << std::hex << std::setw(32) << std::setfill('0') << rng() << std::setw(32)
        << std::setfill('0') << rng();
    return "it_" + out.str();
}

[[nodiscard]] inline std::string now_iso()
{
    const auto t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    const std::tm tm = *std::gmtime(&t);
    char buf[32]{};
    std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &tm);
    return std::string{buf};
}

/** @brief moment().add(1,'week').valueOf() - epoch milliseconds. */
[[nodiscard]] inline std::string expires_millis()
{
    constexpr long long kWeekMs = 7LL * 24 * 60 * 60 * 1'000;
    const auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
                         std::chrono::system_clock::now().time_since_epoch()
    )
                         .count();
    return std::to_string(now + kWeekMs);
}

/** @brief Full invite wire record (includes the internal `token`). */
[[nodiscard]] inline std::string full_invite(
    std::string_view id,
    std::string_view role_id,
    std::string_view email,
    std::string_view status,
    std::string_view expires,
    std::string_view created,
    std::string_view updated,
    std::string_view token
)
{
    return "{\"id\":" + quote(id) + ",\"role_id\":" + quote(role_id) +
           ",\"status\":" + quote(status) + ",\"token\":" + quote(token) +
           ",\"email\":" + quote(email) + ",\"expires\":" + std::string{expires} +
           ",\"created_at\":" + quote(created) + ",\"updated_at\":" + quote(updated) + "}";
}

/** @brief Ghost api-framework Location: <request path>/<id>/ (headers.ts:146-178). */
[[nodiscard]] inline std::string location_for(std::string_view path, std::string_view id)
{
    auto request_path = path.substr(0, path.find('?'));
    while (!request_path.empty() && request_path.back() == '/') {
        request_path.remove_suffix(1);
    }
    std::string out{request_path};
    if (out.empty()) {
        out = "/invites";
    }
    out += "/";
    out += id;
    out += "/";
    return out;
}

/** @brief Emit an invite side effect to the context-local outbox. */
inline void emit(std::string_view type, std::string_view payload, std::string_view aggregate_id)
{
    blog::handlers::outbox::global_outbox().emit(
        blog::handlers::outbox::Event{
            .type = std::string{type},
            .payload = std::string{payload},
            .aggregate_id = std::string{aggregate_id}
        }
    );
}

} // namespace blog::handlers::invites::support

template<>
struct serde::Serializable<blog::dto::invites::RoleDto>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &blog::dto::invites::RoleDto::get_id,
                &blog::dto::invites::RoleDto::set_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "name",
                &blog::dto::invites::RoleDto::get_name,
                &blog::dto::invites::RoleDto::set_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "description",
                &blog::dto::invites::RoleDto::get_description,
                &blog::dto::invites::RoleDto::set_description,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "role_dto";
    }
};

template<>
struct serde::Serializable<blog::dto::invites::RolesResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "roles",
            &blog::dto::invites::RolesResponse::get_roles,
            &blog::dto::invites::RolesResponse::set_roles,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "roles_response";
    }
};

template<>
struct serde::Serializable<blog::dto::invites::InviteDto>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &blog::dto::invites::InviteDto::get_id,
                &blog::dto::invites::InviteDto::set_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "email",
                &blog::dto::invites::InviteDto::get_email,
                &blog::dto::invites::InviteDto::set_email,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &blog::dto::invites::InviteDto::get_status,
                &blog::dto::invites::InviteDto::set_status,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "invite_dto";
    }
};

template<>
struct serde::Serializable<blog::dto::invites::InvitesResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "invites",
            &blog::dto::invites::InvitesResponse::get_invites,
            &blog::dto::invites::InvitesResponse::set_invites,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "invites_response";
    }
};

template<>
struct serde::Serializable<blog::dto::invites::NotificationDto>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &blog::dto::invites::NotificationDto::get_id,
                &blog::dto::invites::NotificationDto::set_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "message",
                &blog::dto::invites::NotificationDto::get_message,
                &blog::dto::invites::NotificationDto::set_message,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "type",
                &blog::dto::invites::NotificationDto::get_type,
                &blog::dto::invites::NotificationDto::set_type,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "notification_dto";
    }
};

template<>
struct serde::Serializable<blog::dto::invites::NotificationsResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "notifications",
            &blog::dto::invites::NotificationsResponse::get_notifications,
            &blog::dto::invites::NotificationsResponse::set_notifications,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "notifications_response";
    }
};

template<>
struct serde::Serializable<blog::dto::invites::WebhookDto>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &blog::dto::invites::WebhookDto::get_id,
                &blog::dto::invites::WebhookDto::set_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "event",
                &blog::dto::invites::WebhookDto::get_event,
                &blog::dto::invites::WebhookDto::set_event,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "target_url",
                &blog::dto::invites::WebhookDto::get_target_url,
                &blog::dto::invites::WebhookDto::set_target_url,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "name",
                &blog::dto::invites::WebhookDto::get_name,
                &blog::dto::invites::WebhookDto::set_name,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "webhook_dto";
    }
};

template<>
struct serde::Serializable<blog::dto::invites::WebhooksResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "webhooks",
            &blog::dto::invites::WebhooksResponse::get_webhooks,
            &blog::dto::invites::WebhooksResponse::set_webhooks,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "webhooks_response";
    }
};
