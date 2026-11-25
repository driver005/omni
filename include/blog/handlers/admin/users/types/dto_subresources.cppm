module;
#include <rfl/Generic.hpp>
export module blog_handlers:users_dto_subresources;

import std;
import core_serde;

export namespace blog::dto::users {

class EmailPreview
{
public:
    void set_subject(std::string value)
    {
        subject = std::move(value);
    }

    [[nodiscard]] const std::string& get_subject() const noexcept
    {
        return subject;
    }

    void set_html(std::string value)
    {
        html = std::move(value);
    }

    [[nodiscard]] const std::string& get_html() const noexcept
    {
        return html;
    }

    void set_plaintext(std::string value)
    {
        plaintext = std::move(value);
    }

    [[nodiscard]] const std::string& get_plaintext() const noexcept
    {
        return plaintext;
    }

    std::string subject{"Preview"};
    std::string html{"<p>Lorem ipsum.</p>"};
    std::string plaintext{"Lorem ipsum."};
};

class EmailPreviewsResponse
{
public:
    void set_email_previews(std::vector<EmailPreview> value)
    {
        email_previews = std::move(value);
    }

    [[nodiscard]] const std::vector<EmailPreview>& get_email_previews() const noexcept
    {
        return email_previews;
    }

    std::vector<EmailPreview> email_previews{};
};

class PasswordResponse
{
public:
    void set_password(rfl::Generic value)
    {
        password = std::move(value);
    }

    [[nodiscard]] const rfl::Generic& get_password() const noexcept
    {
        return password;
    }

    rfl::Generic password{};
};

class CommentSubResponse
{
public:
    void set_comments(std::vector<rfl::Generic> value)
    {
        comments = std::move(value);
    }

    [[nodiscard]] const std::vector<rfl::Generic>& get_comments() const noexcept
    {
        return comments;
    }

    std::vector<rfl::Generic> comments{};
};

} // namespace blog::dto::users

template<>
struct serde::Serializable<blog::dto::users::EmailPreview>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "subject",
                &blog::dto::users::EmailPreview::get_subject,
                &blog::dto::users::EmailPreview::set_subject,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "html",
                &blog::dto::users::EmailPreview::get_html,
                &blog::dto::users::EmailPreview::set_html,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "plaintext",
                &blog::dto::users::EmailPreview::get_plaintext,
                &blog::dto::users::EmailPreview::set_plaintext,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "email_preview";
    }
};

template<>
struct serde::Serializable<blog::dto::users::EmailPreviewsResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "email_previews",
            &blog::dto::users::EmailPreviewsResponse::get_email_previews,
            &blog::dto::users::EmailPreviewsResponse::set_email_previews,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "email_previews_response";
    }
};

template<>
struct serde::Serializable<blog::dto::users::PasswordResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "password",
            &blog::dto::users::PasswordResponse::get_password,
            &blog::dto::users::PasswordResponse::set_password,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "password_response";
    }
};

template<>
struct serde::Serializable<blog::dto::users::CommentSubResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "comments",
            &blog::dto::users::CommentSubResponse::get_comments,
            &blog::dto::users::CommentSubResponse::set_comments,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "comment_sub_response";
    }
};
