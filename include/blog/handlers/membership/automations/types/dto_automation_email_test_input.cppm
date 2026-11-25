module;
export module blog_handlers:automations_dto_automation_email_test_input;

import std;
import core_serde;

export namespace blog::dto::automations {

/** @brief Ghost API DTO: AutomationEmailTestInput. */
class AutomationEmailTestInput
{
public:
    void set_email(std::string value)
    {
        email = std::move(value);
    }

    [[nodiscard]] const std::string& get_email() const noexcept
    {
        return email;
    }

    void set_subject(std::string value)
    {
        subject = std::move(value);
    }

    [[nodiscard]] const std::string& get_subject() const noexcept
    {
        return subject;
    }

    void set_lexical(std::string value)
    {
        lexical = std::move(value);
    }

    [[nodiscard]] const std::string& get_lexical() const noexcept
    {
        return lexical;
    }

    std::string email{};
    std::string subject{};
    std::string lexical{};
};


} // namespace blog::dto::automations

template<>
struct serde::Serializable<blog::dto::automations::AutomationEmailTestInput>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "email",
                &blog::dto::automations::AutomationEmailTestInput::get_email,
                &blog::dto::automations::AutomationEmailTestInput::set_email,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "subject",
                &blog::dto::automations::AutomationEmailTestInput::get_subject,
                &blog::dto::automations::AutomationEmailTestInput::set_subject,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "lexical",
                &blog::dto::automations::AutomationEmailTestInput::get_lexical,
                &blog::dto::automations::AutomationEmailTestInput::set_lexical,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "automation_email_test_input";
    }
};
