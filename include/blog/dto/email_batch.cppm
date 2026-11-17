module;
#include <rfl.hpp>
export module blog_dto:email_batch;

import std;
import core_serde;


export namespace blog::dto {

/** @brief Ghost API DTO: EmailBatch. */
class EmailBatch
{
public:
    void set_created_at(std::string value) { created_at = std::move(value); }
    [[nodiscard]] const std::string& get_created_at() const noexcept { return created_at; }
    void set_error_data(rfl::Generic value) { error_data = std::move(value); }
    [[nodiscard]] const rfl::Generic& get_error_data() const noexcept { return error_data; }
    void set_error_message(rfl::Generic value) { error_message = std::move(value); }
    [[nodiscard]] const rfl::Generic& get_error_message() const noexcept { return error_message; }
    void set_error_status_code(rfl::Generic value) { error_status_code = std::move(value); }
    [[nodiscard]] const rfl::Generic& get_error_status_code() const noexcept { return error_status_code; }
    void set_id(std::string value) { id = std::move(value); }
    [[nodiscard]] const std::string& get_id() const noexcept { return id; }
    void set_mailgun_message_id(std::string value) { mailgun_message_id = std::move(value); }
    [[nodiscard]] const std::string& get_mailgun_message_id() const noexcept { return mailgun_message_id; }
    void set_member_segment(rfl::Generic value) { member_segment = std::move(value); }
    [[nodiscard]] const rfl::Generic& get_member_segment() const noexcept { return member_segment; }
    void set_status(std::string value) { status = std::move(value); }
    [[nodiscard]] const std::string& get_status() const noexcept { return status; }
    void set_updated_at(std::string value) { updated_at = std::move(value); }
    [[nodiscard]] const std::string& get_updated_at() const noexcept { return updated_at; }

    std::string created_at{};
    rfl::Generic error_data{};
    rfl::Generic error_message{};
    rfl::Generic error_status_code{};
    std::string id{};
    std::string mailgun_message_id{};
    rfl::Generic member_segment{};
    std::string status{};
    std::string updated_at{};
};



} // namespace blog::dto

template<>
struct serde::Serializable<blog::dto::EmailBatch>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"created_at", &blog::dto::EmailBatch::get_created_at, &blog::dto::EmailBatch::set_created_at, serde::FieldOptions{}>{},
            serde::FieldDesc<"error_data", &blog::dto::EmailBatch::get_error_data, &blog::dto::EmailBatch::set_error_data, serde::FieldOptions{}>{},
            serde::FieldDesc<"error_message", &blog::dto::EmailBatch::get_error_message, &blog::dto::EmailBatch::set_error_message, serde::FieldOptions{}>{},
            serde::FieldDesc<"error_status_code", &blog::dto::EmailBatch::get_error_status_code, &blog::dto::EmailBatch::set_error_status_code, serde::FieldOptions{}>{},
            serde::FieldDesc<"id", &blog::dto::EmailBatch::get_id, &blog::dto::EmailBatch::set_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"mailgun_message_id", &blog::dto::EmailBatch::get_mailgun_message_id, &blog::dto::EmailBatch::set_mailgun_message_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"member_segment", &blog::dto::EmailBatch::get_member_segment, &blog::dto::EmailBatch::set_member_segment, serde::FieldOptions{}>{},
            serde::FieldDesc<"status", &blog::dto::EmailBatch::get_status, &blog::dto::EmailBatch::set_status, serde::FieldOptions{}>{},
            serde::FieldDesc<"updated_at", &blog::dto::EmailBatch::get_updated_at, &blog::dto::EmailBatch::set_updated_at, serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "email_batch";
    }
};

