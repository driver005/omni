module;
#include <rfl/Generic.hpp>
export module blog_dto:admin_resources;

import std;
import core_serde;

export namespace blog::dto {

class RoleDto
{
public:
    void set_id(std::string value) { id = std::move(value); }
    [[nodiscard]] const std::string& get_id() const noexcept { return id; }
    void set_name(std::string value) { name = std::move(value); }
    [[nodiscard]] const std::string& get_name() const noexcept { return name; }
    void set_description(std::string value) { description = std::move(value); }
    [[nodiscard]] const std::string& get_description() const noexcept { return description; }
    std::string id{"1"};
    std::string name{"Administrator"};
    std::string description{"Full access"};
};

class RolesResponse
{
public:
    void set_roles(std::vector<RoleDto> value) { roles = std::move(value); }
    [[nodiscard]] const std::vector<RoleDto>& get_roles() const noexcept { return roles; }
    std::vector<RoleDto> roles{};
};

class InviteDto
{
public:
    void set_id(std::string value) { id = std::move(value); }
    [[nodiscard]] const std::string& get_id() const noexcept { return id; }
    void set_email(std::string value) { email = std::move(value); }
    [[nodiscard]] const std::string& get_email() const noexcept { return email; }
    void set_status(std::string value) { status = std::move(value); }
    [[nodiscard]] const std::string& get_status() const noexcept { return status; }
    std::string id{"1"};
    std::string email{"invite@example.com"};
    std::string status{"sent"};
};

class InvitesResponse
{
public:
    void set_invites(std::vector<InviteDto> value) { invites = std::move(value); }
    [[nodiscard]] const std::vector<InviteDto>& get_invites() const noexcept { return invites; }
    std::vector<InviteDto> invites{};
};

class NotificationDto
{
public:
    void set_id(std::string value) { id = std::move(value); }
    [[nodiscard]] const std::string& get_id() const noexcept { return id; }
    void set_message(std::string value) { message = std::move(value); }
    [[nodiscard]] const std::string& get_message() const noexcept { return message; }
    void set_type(std::string value) { type = std::move(value); }
    [[nodiscard]] const std::string& get_type() const noexcept { return type; }
    std::string id{"1"};
    std::string message{"Ghost notification"};
    std::string type{"info"};
};

class NotificationsResponse
{
public:
    void set_notifications(std::vector<NotificationDto> value) { notifications = std::move(value); }
    [[nodiscard]] const std::vector<NotificationDto>& get_notifications() const noexcept { return notifications; }
    std::vector<NotificationDto> notifications{};
};

class WebhookDto
{
public:
    void set_id(std::string value) { id = std::move(value); }
    [[nodiscard]] const std::string& get_id() const noexcept { return id; }
    void set_event(std::string value) { event = std::move(value); }
    [[nodiscard]] const std::string& get_event() const noexcept { return event; }
    void set_target_url(std::string value) { target_url = std::move(value); }
    [[nodiscard]] const std::string& get_target_url() const noexcept { return target_url; }
    void set_name(std::string value) { name = std::move(value); }
    [[nodiscard]] const std::string& get_name() const noexcept { return name; }
    std::string id{"1"};
    std::string event{"post.published"};
    std::string target_url{"https://example.com/hook"};
    std::string name{"Webhook"};
};

class WebhooksResponse
{
public:
    void set_webhooks(std::vector<WebhookDto> value) { webhooks = std::move(value); }
    [[nodiscard]] const std::vector<WebhookDto>& get_webhooks() const noexcept { return webhooks; }
    std::vector<WebhookDto> webhooks{};
};

} // namespace blog::dto

#define MODEL_SERDE(Type, TableName, ...)                                                      \
    template<> struct serde::Serializable<blog::dto::Type>                                      \
    {                                                                                           \
        static constexpr auto fields() { return std::tuple{__VA_ARGS__}; }                      \
        static constexpr std::string_view table_name() { return TableName; }                    \
    };

MODEL_SERDE(RoleDto, "role_dto",
    serde::FieldDesc<"id", &blog::dto::RoleDto::get_id, &blog::dto::RoleDto::set_id, serde::FieldOptions{}>{},
    serde::FieldDesc<"name", &blog::dto::RoleDto::get_name, &blog::dto::RoleDto::set_name, serde::FieldOptions{}>{},
    serde::FieldDesc<"description", &blog::dto::RoleDto::get_description, &blog::dto::RoleDto::set_description, serde::FieldOptions{}>{})
MODEL_SERDE(RolesResponse, "roles_response",
    serde::FieldDesc<"roles", &blog::dto::RolesResponse::get_roles, &blog::dto::RolesResponse::set_roles, serde::FieldOptions{}>{})
MODEL_SERDE(InviteDto, "invite_dto",
    serde::FieldDesc<"id", &blog::dto::InviteDto::get_id, &blog::dto::InviteDto::set_id, serde::FieldOptions{}>{},
    serde::FieldDesc<"email", &blog::dto::InviteDto::get_email, &blog::dto::InviteDto::set_email, serde::FieldOptions{}>{},
    serde::FieldDesc<"status", &blog::dto::InviteDto::get_status, &blog::dto::InviteDto::set_status, serde::FieldOptions{}>{})
MODEL_SERDE(InvitesResponse, "invites_response",
    serde::FieldDesc<"invites", &blog::dto::InvitesResponse::get_invites, &blog::dto::InvitesResponse::set_invites, serde::FieldOptions{}>{})
MODEL_SERDE(NotificationDto, "notification_dto",
    serde::FieldDesc<"id", &blog::dto::NotificationDto::get_id, &blog::dto::NotificationDto::set_id, serde::FieldOptions{}>{},
    serde::FieldDesc<"message", &blog::dto::NotificationDto::get_message, &blog::dto::NotificationDto::set_message, serde::FieldOptions{}>{},
    serde::FieldDesc<"type", &blog::dto::NotificationDto::get_type, &blog::dto::NotificationDto::set_type, serde::FieldOptions{}>{})
MODEL_SERDE(NotificationsResponse, "notifications_response",
    serde::FieldDesc<"notifications", &blog::dto::NotificationsResponse::get_notifications, &blog::dto::NotificationsResponse::set_notifications, serde::FieldOptions{}>{})
MODEL_SERDE(WebhookDto, "webhook_dto",
    serde::FieldDesc<"id", &blog::dto::WebhookDto::get_id, &blog::dto::WebhookDto::set_id, serde::FieldOptions{}>{},
    serde::FieldDesc<"event", &blog::dto::WebhookDto::get_event, &blog::dto::WebhookDto::set_event, serde::FieldOptions{}>{},
    serde::FieldDesc<"target_url", &blog::dto::WebhookDto::get_target_url, &blog::dto::WebhookDto::set_target_url, serde::FieldOptions{}>{},
    serde::FieldDesc<"name", &blog::dto::WebhookDto::get_name, &blog::dto::WebhookDto::set_name, serde::FieldOptions{}>{})
MODEL_SERDE(WebhooksResponse, "webhooks_response",
    serde::FieldDesc<"webhooks", &blog::dto::WebhooksResponse::get_webhooks, &blog::dto::WebhooksResponse::set_webhooks, serde::FieldOptions{}>{})
