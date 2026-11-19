export module blog_handlers:automations_operations;

import std;
import core_cache;
import blog_dto;
import blog_handlers_middleware;
import :mutation;

export namespace blog::api::automations {

namespace detail {

constexpr std::string_view kTestEmailsSent{"automations:test_emails_sent"};

[[nodiscard]] bool known_automation(std::string_view id)
{
    return !id.empty() && id != "missing" && id != "0000";
}

[[nodiscard]] std::pair<int, std::string> not_found(std::string_view context)
{
    return {404, ApiError{}.json("Resource not found", context)};
}

} // namespace detail

using namespace detail;
class AutomationsOperations
{
public:
    explicit AutomationsOperations(core::cache::Cache& cache) noexcept :
        m_cache{cache}
    {
    }

/** @brief POST /automations/:id/email_preview. */
[[nodiscard]] std::pair<int, std::string> automation_email_preview(
    std::string_view id,
    std::string_view body
)
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
[[nodiscard]] std::pair<int, std::string> automation_email_test(
    std::string_view id,
    std::string_view body
)
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
    const auto sent = core::cache::value_as<std::int64_t>(m_cache.get(kTestEmailsSent).value, 0) + 1;
    m_cache.put(kTestEmailsSent, core::cache::to_cache_value(sent), core::cache::Ttl{}.never(), {"automations"});
    return {204, ""};
}

private:
    core::cache::Cache& m_cache;
};

} // namespace blog::api::automations
