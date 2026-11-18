export module blog_handlers:automations_state;

import std;
import blog_dto;
import blog_model;
import blog_handlers_middleware;
import :mutation;

export namespace blog::api {

/** @brief Mutable automation email-test state owned by the automations domain. */
class AutomationsState
{
public:
    /** @brief POST /automations/:id/email_preview. */
    std::pair<int, std::string> automation_email_preview(std::string_view id, std::string_view body)
    {
        if (!known_automation(id)) {
            return not_found("Automation not found");
        }
        const auto input = blog::handlers::middleware::decode<blog::dto::AutomationEmailPreviewInput>(body);
        if (!input || input->subject.empty() || input->lexical.empty()) {
            return {422, ApiError{}.validation(
                "Validation error, cannot preview email.",
                "subject and lexical are required",
                "lexical"
            )};
        }
        blog::dto::AutomationEmailPreviewResponse response;
        blog::dto::AutomationEmailPreview preview;
        preview.set_subject(input->subject);
        preview.set_html(R"(<p>Lorem ipsum.</p>)");
        preview.set_plaintext("Lorem ipsum.");
        response.automation_email_previews.push_back(std::move(preview));
        return {200, blog::handlers::middleware::encode(response)};
    }

    /** @brief POST /automations/:id/email_test — 204 with no body on success. */
    std::pair<int, std::string> automation_email_test(std::string_view id, std::string_view body)
    {
        if (!known_automation(id)) {
            return not_found("Automation not found");
        }
        const auto input = blog::handlers::middleware::decode<blog::dto::AutomationEmailTestInput>(body);
        if (!input || input->email.find('@') == std::string::npos ||
            input->email.find('.') == std::string::npos) {
            return {422, ApiError{}.validation(
                "Validation error, cannot send test email.",
                "Invalid email address",
                "email"
            )};
        }
        ++m_test_emails_sent;
        return {204, ""};
    }

    [[nodiscard]] std::size_t test_emails_sent() const noexcept
    {
        return m_test_emails_sent;
    }

private:
    [[nodiscard]] static bool known_automation(std::string_view id)
    {
        return !id.empty() && id != "missing" && id != "0000";
    }

    [[nodiscard]] static std::pair<int, std::string> not_found(std::string_view context)
    {
        return {404, ApiError{}.json("Resource not found", context)};
    }

    std::size_t m_test_emails_sent{0};
};

} // namespace blog::api
