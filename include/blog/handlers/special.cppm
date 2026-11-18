export module blog_handlers:special;

import std;
import blog_dto;
import blog_context;
import :mutation;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::api {

/** @brief Result of a special admin endpoint call. */
class SpecialResult
{
public:
    SpecialResult() = default;

    SpecialResult(int status, std::string body) :
        m_status{status},
        m_body{std::move(body)}
    {
    }

    [[nodiscard]] int status() const noexcept
    {
        return m_status;
    }

    [[nodiscard]] const std::string& body() const noexcept
    {
        return m_body;
    }

private:
    int m_status{200};
    std::string m_body{};
};

/**
 * @brief Endpoint-specific handlers for Admin API routes whose payload is not a
 * plain resource collection. Every payload is a real reflected model struct
 * from `blog_model`, serialized through `ModelJson` — no hand-built JSON.
 */
class SpecialEndpoints
{
public:
    SpecialEndpoints()
    {
        m_analytics.latest.jobName = "email-analytics-latest-others";
        m_analytics.latestOpened.jobName = "email-analytics-latest-opened";
        m_analytics.missing.jobName = "email-analytics-missing";
        m_analytics.scheduled.jobName = "email-analytics-scheduled";
    }

    /** @brief POST /automations/:id/email_preview. */
    SpecialResult automation_email_preview(std::string_view id, std::string_view body)
    {
        if (!known_automation(id)) {
            return not_found("Automation not found");
        }
        const auto input = blog::dto::ModelJson<
            blog::dto::AutomationEmailPreviewInput
        >{}.from_json(body);
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
        return ok(response);
    }

    /** @brief POST /automations/:id/email_test — 204 with no body on success. */
    SpecialResult automation_email_test(std::string_view id, std::string_view body)
    {
        if (!known_automation(id)) {
            return not_found("Automation not found");
        }
        const auto input = blog::dto::ModelJson<
            blog::dto::AutomationEmailTestInput
        >{}.from_json(body);
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

    /** @brief GET /emails/:id/analytics. */
    SpecialResult emails_analytics_get(std::string_view /*id*/) const
    {
        return ok(m_analytics);
    }

    /** @brief PUT /emails/:id/analytics — flips a job's `running` flag. */
    SpecialResult emails_analytics_put(std::string_view /*id*/, std::string_view body)
    {
        const auto input = blog::dto::ModelJson<
            blog::dto::AnalyticsStatusInput
        >{}.from_json(body);
        if (input) {
            auto* job = job_for(input->jobName);
            if (job != nullptr) {
                job->running = input->running;
            }
        }
        return ok(m_analytics);
    }

    /** @brief DELETE /emails/analytics — resets all job flags. */
    SpecialResult emails_analytics_delete()
    {
        m_analytics = blog::dto::AnalyticsStatus{};
        m_analytics.latest.jobName = "email-analytics-latest-others";
        m_analytics.latestOpened.jobName = "email-analytics-latest-opened";
        m_analytics.missing.jobName = "email-analytics-missing";
        m_analytics.scheduled.jobName = "email-analytics-scheduled";
        return {200, "{}"};
    }

    /** @brief GET /emails/:id/batches. */
    SpecialResult emails_batches(std::string_view /*id*/) const
    {
        return SpecialResult{200, std::format(
            R"({{"batches":[{}],"meta":{{"pagination":{{"limit":15,"page":1,"pages":1,"prev":null,"next":null,"total":1}}}}}})",
            blog::dto::GhostJson::batch()
        )};
    }

    /** @brief GET /emails/:id/recipient-failures. */
    SpecialResult emails_failures(std::string_view /*id*/) const
    {
        return SpecialResult{200, std::format(
            R"({{"failures":[{}],"meta":{{"pagination":{{"limit":15,"page":1,"pages":1,"prev":null,"next":null,"total":1}}}}}})",
            blog::dto::GhostJson::failure()
        )};
    }

    /** @brief POST /members/:id/subscriptions/. */
    SpecialResult member_subscription_create(std::string_view id, std::string_view body)
    {
        const auto input = blog::dto::ModelJson<
            blog::dto::MemberSubscriptionInput
        >{}.from_json(body);
        blog::dto::MemberSubscription subscription;
        subscription.id = std::to_string(++m_next_subscription);
        subscription.status = input ? input->status : std::string{"active"};
        subscription.tier.name = input ? input->tier : std::string{"free"};
        subscription.current_period_end = "1970-01-01T00:00:00.000Z";
        m_subscriptions[std::string{id}].push_back(std::move(subscription));
        return ok(member_response(id));
    }

    /** @brief PUT /members/:id/subscriptions/:subscription_id. */
    SpecialResult member_subscription_edit(
        std::string_view id,
        std::string_view subscription_id,
        std::string_view body
    )
    {
        const auto input = blog::dto::ModelJson<
            blog::dto::MemberSubscriptionInput
        >{}.from_json(body);
        for (auto& subscription : m_subscriptions[std::string{id}]) {
            if (subscription.id == subscription_id) {
                if (!input) {
                    continue;
                }
                subscription.status = input->status;
                subscription.tier.name = input->tier.empty() ? subscription.tier.name : input->tier;
            }
        }
        return ok(member_response(id));
    }

    /** @brief GET /members/:id/signin_urls. */
    SpecialResult member_signin_urls(std::string_view id) const
    {
        blog::dto::SigninUrlsResponse response;
        blog::dto::SigninUrl signin;
        signin.set_member_id(std::string{id});
        signin.set_url("https://ghost.local/signin/" + std::string{id});
        response.signin_urls.push_back(std::move(signin));
        return ok(response);
    }

    /** @brief GET /stats/mrr and /members/stats/mrr. */
    SpecialResult stats_mrr() const
    {
        blog::dto::StatsMrrResponse response;
        blog::dto::StatsMrrEntry first;
        first.set_currency("usd");
        first.set_date("1970-01-01");
        first.set_mrr(0);
        blog::dto::StatsMrrEntry second;
        second.set_currency("usd");
        second.set_date("1970-01-01");
        second.set_mrr(1000);
        response.stats.push_back(std::move(first));
        response.stats.push_back(std::move(second));
        response.meta.totals.push_back(blog::dto::StatsMrrTotal{});
        return ok(response);
    }

    /** @brief GET /stats/member_count and /members/stats/count. */
    SpecialResult stats_member() const
    {
        blog::dto::StatsMemberResponse response;
        response.stats.push_back(blog::dto::StatsMemberEntry{});
        return ok(response);
    }

    /** @brief Other stats endpoints — empty stats array. */
    SpecialResult stats_empty() const
    {
        return ok(blog::dto::StatsEmptyResponse{});
    }

    /** @brief GET /email_previews/posts/:id. */
    SpecialResult email_previews() const
    {
        blog::dto::EmailPreviewsResponse response;
        response.email_previews.push_back(blog::dto::EmailPreview{});
        return ok(response);
    }

    /** @brief PUT /users/password. */
    SpecialResult password_change() const
    {
        return ok(blog::dto::PasswordResponse{});
    }

    /** @brief GET /comments/:id/{replies,likes,reports,dislikes}. */
    SpecialResult comment_sub(std::string_view path) const
    {
        std::string_view key{"comments"};
        if (path.ends_with("/likes")) key = "comment_likes";
        else if (path.ends_with("/dislikes")) key = "comment_dislikes";
        else if (path.ends_with("/reports")) key = "comment_reports";
        return SpecialResult{200, std::format(
            R"({{"{}":[{}],"meta":{{"pagination":{{"limit":15,"page":1,"pages":1,"prev":null,"next":null,"total":1}}}}}})",
            key, key == "comments" ? blog::dto::GhostJson::comment([]{ model::Comment c; c.set_id(1); return c; }())
                                   : blog::dto::GhostJson::comment_sub()
        )};
    }

    SpecialResult oembed() const
    {
        return ok(blog::dto::OembedResponse{});
    }

    SpecialResult feedback() const
    {
        return ok(blog::dto::FeedbackResponse{});
    }

    SpecialResult search_index(std::string_view kind) const
    {
        if (kind == "pages") {
            blog::dto::SearchPagesResponse r; r.pages.push_back(blog::dto::RecordDto{}); return ok(r);
        }
        if (kind == "posts") {
            blog::dto::SearchPostsResponse r; r.posts.push_back(blog::dto::RecordDto{}); return ok(r);
        }
        if (kind == "tags") {
            blog::dto::SearchTagsResponse r; r.tags.push_back(blog::dto::RecordDto{}); return ok(r);
        }
        if (kind == "authors") {
            blog::dto::SearchAuthorsResponse r;
            r.authors.push_back(blog::dto::RecordDto{});
            return ok(r);
        }
        blog::dto::SearchUsersResponse r; r.users.push_back(blog::dto::RecordDto{}); return ok(r);
    }

    SpecialResult automated_emails() const
    {
        blog::dto::AutomatedEmailsResponse response;
        response.automated_emails.push_back(blog::dto::RecordDto{});
        return ok(response);
    }

    SpecialResult custom_theme_settings() const
    {
        return SpecialResult{200, std::format(
            R"({{"custom_theme_settings":[{}]}})",
            blog::dto::GhostJson::custom_theme_setting()
        )};
    }

    SpecialResult integrations() const
    {
        blog::dto::IntegrationsResponse response;
        response.integrations.push_back(blog::dto::RecordDto{});
        return ok(response);
    }

    SpecialResult labels() const
    {
        blog::dto::LabelsResponse response;
        response.labels.push_back(blog::dto::NamedRecordDto{});
        return ok(response);
    }

    SpecialResult snippets() const
    {
        blog::dto::SnippetsResponse response;
        response.snippets.push_back(blog::dto::NamedRecordDto{});
        return ok(response);
    }

    SpecialResult recommendations() const
    {
        blog::dto::RecommendationsResponse response;
        response.recommendations.push_back(blog::dto::NamedRecordDto{});
        return ok(response);
    }

    SpecialResult webhooks() const
    {
        blog::dto::WebhooksListResponse response;
        response.webhooks.push_back(blog::dto::WebhooksDto{});
        return ok(response);
    }

    SpecialResult gift_links() const
    {
        blog::dto::GiftLinksResponse response;
        response.gift_links.push_back(blog::dto::LinkRecord{});
        return ok(response);
    }

    SpecialResult automation_links() const
    {
        blog::dto::AutomationLinksResponse response;
        response.links.push_back(blog::dto::LinkRecord{});
        return ok(response);
    }

    SpecialResult roles() const
    {
        blog::dto::RolesResponse response;
        response.roles.push_back(blog::dto::RoleDto{});
        return ok(response);
    }

    SpecialResult invites() const
    {
        blog::dto::InvitesResponse response;
        response.invites.push_back(blog::dto::InviteDto{});
        return ok(response);
    }

    SpecialResult notifications() const
    {
        blog::dto::NotificationsResponse response;
        response.notifications.push_back(blog::dto::NotificationDto{});
        return ok(response);
    }

    /** @brief GET /site. */
    SpecialResult site() const
    {
        return ok(blog::dto::SiteResponse{});
    }

    /** @brief GET /config. */
    SpecialResult config() const
    {
        return ok(blog::dto::ConfigEnvelope{});
    }

    [[nodiscard]] std::size_t test_emails_sent() const noexcept
    {
        return m_test_emails_sent;
    }

private:
    [[nodiscard]] SpecialResult not_found(std::string_view context) const
    {
        return {404, ApiError{}.json("Resource not found", context)};
    }

    [[nodiscard]] SpecialResult ok(const auto& model) const
    {
        return {200, blog::dto::ModelJson<std::decay_t<decltype(model)>>{}.to_json(model)};
    }

    [[nodiscard]] bool known_automation(std::string_view id) const
    {
        return !id.empty() && id != "missing" && id != "0000";
    }

    [[nodiscard]] blog::dto::AnalyticsJob* job_for(std::string_view name)
    {
        if (name == "email-analytics-latest-opened") {
            return &m_analytics.latestOpened;
        }
        if (name == "email-analytics-missing") {
            return &m_analytics.missing;
        }
        return &m_analytics.latest;
    }

    [[nodiscard]] blog::dto::MemberSubscriptionResponse member_response(
        std::string_view id
    ) const
    {
        blog::dto::SubscriptionMember member;
        member.email = "member@example.com";
        member.id = std::string{id};
        member.status = "paid";
        const auto it = m_subscriptions.find(std::string{id});
        if (it != m_subscriptions.end()) {
            member.subscriptions = it->second;
        }
        blog::dto::MemberSubscriptionResponse response;
        response.members.push_back(std::move(member));
        return response;
    }

    std::map<std::string, std::vector<blog::dto::MemberSubscription>> m_subscriptions{};
    blog::dto::AnalyticsStatus m_analytics{};
    std::uint64_t m_next_subscription{0};
    std::size_t m_test_emails_sent{0};
};

} // namespace blog::api

#ifdef CONGELADO_TEST
namespace blog::api::tests {
using namespace boost::ut;
suite<"blog_api_special"> blog_api_special_suite = []
{
    "special endpoints return reflected snapshot-shaped bodies with state"_test = []
    {
        SpecialEndpoints endpoints;
        const auto preview = endpoints.automation_email_preview(
            "1", R"({"subject":"Hi","lexical":"{}"})"
        );
        expect(preview.status() == 200);
        expect(preview.body().contains(R"("automation_email_previews":["));

        const auto analytics = endpoints.emails_analytics_get("1");
        expect(analytics.body().contains(R"("jobName":"email-analytics-scheduled")"));
        endpoints.emails_analytics_put(
            "1", R"({"jobName":"email-analytics-latest-opened","running":true})"
        );
        expect(endpoints.emails_analytics_get("1").body().contains(R"("running":true)"));

        endpoints.member_subscription_create("1", R"({"tier":"gold"})");
        expect(endpoints.member_subscription_edit(
            "1", "1", R"({"status":"canceled"})"
        ).body().contains("canceled"));
    };
};
} // namespace blog::api::tests
#endif
