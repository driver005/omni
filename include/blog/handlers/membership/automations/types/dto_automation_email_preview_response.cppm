module;
export module blog_handlers:automations_dto_automation_email_preview_response;

import std;
import core_serde;
import :automations_dto_automation_email_preview;

export namespace blog::dto::automations {

/** @brief Ghost API DTO: AutomationEmailPreviewResponse. */
class AutomationEmailPreviewResponse
{
public:
    void set_automation_email_previews(std::vector<AutomationEmailPreview> value) { automation_email_previews = std::move(value); }
    [[nodiscard]] const std::vector<AutomationEmailPreview>& get_automation_email_previews() const noexcept { return automation_email_previews; }

    std::vector<AutomationEmailPreview> automation_email_previews{};
};



} // namespace blog::dto::automations

template<>
struct serde::Serializable<blog::dto::automations::AutomationEmailPreviewResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"automation_email_previews", &blog::dto::automations::AutomationEmailPreviewResponse::get_automation_email_previews, &blog::dto::automations::AutomationEmailPreviewResponse::set_automation_email_previews, serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "automation_email_preview_response";
    }
};

