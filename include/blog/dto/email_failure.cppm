module;
#include <rfl.hpp>
export module blog_dto:email_failure;

import std;
import core_serde;


export namespace blog::dto {

/** @brief Ghost API DTO: EmailFailure. */
class EmailFailure
{
public:
    void set_code(int value) { code = std::move(value); }
    [[nodiscard]] const int& get_code() const noexcept { return code; }
    void set_email_recipient(rfl::Generic value) { email_recipient = std::move(value); }
    [[nodiscard]] const rfl::Generic& get_email_recipient() const noexcept { return email_recipient; }
    void set_enhanced_code(std::string value) { enhanced_code = std::move(value); }
    [[nodiscard]] const std::string& get_enhanced_code() const noexcept { return enhanced_code; }
    void set_event_id(std::string value) { event_id = std::move(value); }
    [[nodiscard]] const std::string& get_event_id() const noexcept { return event_id; }
    void set_failed_at(std::string value) { failed_at = std::move(value); }
    [[nodiscard]] const std::string& get_failed_at() const noexcept { return failed_at; }
    void set_id(std::string value) { id = std::move(value); }
    [[nodiscard]] const std::string& get_id() const noexcept { return id; }
    void set_member(rfl::Generic value) { member = std::move(value); }
    [[nodiscard]] const rfl::Generic& get_member() const noexcept { return member; }
    void set_message(std::string value) { message = std::move(value); }
    [[nodiscard]] const std::string& get_message() const noexcept { return message; }
    void set_severity(std::string value) { severity = std::move(value); }
    [[nodiscard]] const std::string& get_severity() const noexcept { return severity; }

    int code{555};
    rfl::Generic email_recipient{};
    std::string enhanced_code{};
    std::string event_id{};
    std::string failed_at{};
    std::string id{};
    rfl::Generic member{};
    std::string message{};
    std::string severity{};
};



} // namespace blog::dto

template<>
struct serde::Serializable<blog::dto::EmailFailure>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"code", &blog::dto::EmailFailure::get_code, &blog::dto::EmailFailure::set_code, serde::FieldOptions{}>{},
            serde::FieldDesc<"email_recipient", &blog::dto::EmailFailure::get_email_recipient, &blog::dto::EmailFailure::set_email_recipient, serde::FieldOptions{}>{},
            serde::FieldDesc<"enhanced_code", &blog::dto::EmailFailure::get_enhanced_code, &blog::dto::EmailFailure::set_enhanced_code, serde::FieldOptions{}>{},
            serde::FieldDesc<"event_id", &blog::dto::EmailFailure::get_event_id, &blog::dto::EmailFailure::set_event_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"failed_at", &blog::dto::EmailFailure::get_failed_at, &blog::dto::EmailFailure::set_failed_at, serde::FieldOptions{}>{},
            serde::FieldDesc<"id", &blog::dto::EmailFailure::get_id, &blog::dto::EmailFailure::set_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"member", &blog::dto::EmailFailure::get_member, &blog::dto::EmailFailure::set_member, serde::FieldOptions{}>{},
            serde::FieldDesc<"message", &blog::dto::EmailFailure::get_message, &blog::dto::EmailFailure::set_message, serde::FieldOptions{}>{},
            serde::FieldDesc<"severity", &blog::dto::EmailFailure::get_severity, &blog::dto::EmailFailure::set_severity, serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "email_failure";
    }
};

