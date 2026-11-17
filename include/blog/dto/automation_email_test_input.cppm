module;
export module blog_dto:automation_email_test_input;

import std;
import core_serde;


export namespace blog::dto {

/** @brief Ghost API DTO: AutomationEmailTestInput. */
class AutomationEmailTestInput
{
public:
    void set_email(std::string value) { email = std::move(value); }
    [[nodiscard]] const std::string& get_email() const noexcept { return email; }
    void set_subject(std::string value) { subject = std::move(value); }
    [[nodiscard]] const std::string& get_subject() const noexcept { return subject; }
    void set_lexical(std::string value) { lexical = std::move(value); }
    [[nodiscard]] const std::string& get_lexical() const noexcept { return lexical; }

    std::string email{};
    std::string subject{};
    std::string lexical{};
};



} // namespace blog::dto

template<>
struct serde::Serializable<blog::dto::AutomationEmailTestInput>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"email", &blog::dto::AutomationEmailTestInput::get_email, &blog::dto::AutomationEmailTestInput::set_email, serde::FieldOptions{}>{},
            serde::FieldDesc<"subject", &blog::dto::AutomationEmailTestInput::get_subject, &blog::dto::AutomationEmailTestInput::set_subject, serde::FieldOptions{}>{},
            serde::FieldDesc<"lexical", &blog::dto::AutomationEmailTestInput::get_lexical, &blog::dto::AutomationEmailTestInput::set_lexical, serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "automation_email_test_input";
    }
};

