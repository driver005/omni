module;
export module blog_handlers:automations_dto_automation_email_preview_input;

import std;
import core_serde;


export namespace blog::dto::automations {

/** @brief Ghost API DTO: AutomationEmailPreviewInput. */
class AutomationEmailPreviewInput
{
public:
    void set_subject(std::string value) { subject = std::move(value); }
    [[nodiscard]] const std::string& get_subject() const noexcept { return subject; }
    void set_lexical(std::string value) { lexical = std::move(value); }
    [[nodiscard]] const std::string& get_lexical() const noexcept { return lexical; }

    std::string subject{};
    std::string lexical{};
};



} // namespace blog::dto::automations

template<>
struct serde::Serializable<blog::dto::automations::AutomationEmailPreviewInput>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"subject", &blog::dto::automations::AutomationEmailPreviewInput::get_subject, &blog::dto::automations::AutomationEmailPreviewInput::set_subject, serde::FieldOptions{}>{},
            serde::FieldDesc<"lexical", &blog::dto::automations::AutomationEmailPreviewInput::get_lexical, &blog::dto::automations::AutomationEmailPreviewInput::set_lexical, serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "automation_email_preview_input";
    }
};

