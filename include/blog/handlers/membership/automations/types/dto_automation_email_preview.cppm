module;
export module blog_handlers:automations_dto_automation_email_preview;

import std;
import core_serde;

export namespace blog::dto::automations {

/** @brief Ghost API DTO: AutomationEmailPreview. */
class AutomationEmailPreview
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

    std::string subject{};
    std::string html{};
    std::string plaintext{};
};


} // namespace blog::dto::automations

template<>
struct serde::Serializable<blog::dto::automations::AutomationEmailPreview>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "subject",
                &blog::dto::automations::AutomationEmailPreview::get_subject,
                &blog::dto::automations::AutomationEmailPreview::set_subject,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "html",
                &blog::dto::automations::AutomationEmailPreview::get_html,
                &blog::dto::automations::AutomationEmailPreview::set_html,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "plaintext",
                &blog::dto::automations::AutomationEmailPreview::get_plaintext,
                &blog::dto::automations::AutomationEmailPreview::set_plaintext,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "automation_email_preview";
    }
};
