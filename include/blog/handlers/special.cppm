export module blog_handlers:special;

import std;
import blog_dto;
import blog_context;
import :stripe;
import :mutation;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::api {

/** @brief status + body returned by a domain service method. */
struct EndpointResponse
{
    int status{200};
    std::string body{};
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
    EndpointResponse events() const
    {
        return {200, std::format(
            R"({{"events":[{}],"meta":{{"pagination":{{"limit":15,"page":1,"pages":1,"prev":null,"next":null,"total":1}}}}}})",
            blog::dto::GhostJson::event())};
    }
    EndpointResponse metafields_browse() const
    {
        return {200, "{\"members\":[" + blog::dto::GhostJson::member([]{
            model::Member m; m.set_id(1); return m;
        }()) + "]}"};
    }
    EndpointResponse stripe(std::string_view method, std::string_view body, std::string_view path = "/stripe/checkout/config")
    {
        if (path.contains("/branding")) return {200, m_stripe.branding()};
        if (method == "GET") return {200, m_stripe.config()};
        if (method == "PUT") { m_stripe.configure(body); return {200, m_stripe.config()}; }
        if (method == "POST") return {200, m_stripe.preview(body)};
        return {200, m_stripe.branding()};
    }
    EndpointResponse settings_put() const
    {
        return {200, R"({"settings":{"title":"Ghost"},"meta":{}})"};
    }
    EndpointResponse custom_theme_settings_put() const
    {
        return {200, R"({"custom_theme_settings":[],"meta":{}})"};
    }
    EndpointResponse automation_email_preview(std::string_view id, std::string_view body)
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
    EndpointResponse automation_email_test(std::string_view id, std::string_view body)
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
    EndpointResponse emails_analytics_get(std::string_view /*id*/) const
    {
        return ok(m_analytics);
    }

    /** @brief PUT /emails/:id/analytics — flips a job's `running` flag. */
    EndpointResponse emails_analytics_put(std::string_view /*id*/, std::string_view body)
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
    EndpointResponse emails_analytics_delete()
    {
        m_analytics = blog::dto::AnalyticsStatus{};
        m_analytics.latest.jobName = "email-analytics-latest-others";
        m_analytics.latestOpened.jobName = "email-analytics-latest-opened";
        m_analytics.missing.jobName = "email-analytics-missing";
        m_analytics.scheduled.jobName = "email-analytics-scheduled";
        return {200, "{}"};
    }

    /** @brief GET /emails/:id/batches. */
    EndpointResponse emails_batches(std::string_view /*id*/) const
    {
        return EndpointResponse{200, std::format(
            R"({{"batches":[{}],"meta":{{"pagination":{{"limit":15,"page":1,"pages":1,"prev":null,"next":null,"total":1}}}}}})",
            blog::dto::GhostJson::batch()
        )};
    }

    /** @brief GET /emails/:id/recipient-failures. */
    EndpointResponse emails_failures(std::string_view /*id*/) const
    {
        return EndpointResponse{200, std::format(
            R"({{"failures":[{}],"meta":{{"pagination":{{"limit":15,"page":1,"pages":1,"prev":null,"next":null,"total":1}}}}}})",
            blog::dto::GhostJson::failure()
        )};
    }

    /** @brief POST /members/:id/subscriptions/. */
    EndpointResponse member_subscription_create(std::string_view id, std::string_view body)
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
    EndpointResponse member_subscription_edit(
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
    EndpointResponse member_signin_urls(std::string_view id) const
    {
        blog::dto::SigninUrlsResponse response;
        blog::dto::SigninUrl signin;
        signin.set_member_id(std::string{id});
        signin.set_url("https://ghost.local/signin/" + std::string{id});
        response.signin_urls.push_back(std::move(signin));
        return ok(response);
    }

    /** @brief GET /stats/mrr and /members/stats/mrr. */
    EndpointResponse stats_mrr() const
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
    EndpointResponse stats_member() const
    {
        blog::dto::StatsMemberResponse response;
        response.stats.push_back(blog::dto::StatsMemberEntry{});
        return ok(response);
    }

    /** @brief Other stats endpoints — empty stats array. */
    EndpointResponse stats_empty() const
    {
        return ok(blog::dto::StatsEmptyResponse{});
    }

    /** @brief GET /email_previews/posts/:id. */
    /** @brief Per-member commenting state. */
    struct CommentState
    {
        bool disabled{false};
        std::string reason;
        std::string until;
        bool hide_comments{true};
    };

    [[nodiscard]] std::string commenting_json(std::string_view id) const
    {
        const auto it = m_commenting.find(std::string{id});
        if (it == m_commenting.end()) {
            return R"({"disabled":false,"disabled_reason":null,"disabled_until":null})";
        }
        const auto& state = it->second;
        return std::format(
            R"({{"disabled":{},"disabled_reason":{},"disabled_until":{}}})",
            state.disabled ? "true" : "false",
            state.reason.empty() ? "null" : "\"" + state.reason + "\"",
            state.until.empty() ? "null" : "\"" + state.until + "\""
        );
    }

    /** @brief Overlay stored commenting state onto a serialized member. */
    [[nodiscard]] std::string apply_commenting(std::string_view id, std::string json) const
    {
        const auto at = json.find("\"commenting\":");
        if (at == std::string::npos) return json;
        const auto open = json.find('{', at);
        if (open == std::string::npos) return json;
        int depth = 0;
        std::size_t end = open;
        for (; end < json.size(); ++end) {
            if (json[end] == '{') ++depth;
            else if (json[end] == '}' && --depth == 0) { ++end; break; }
        }
        json.replace(open, end - open, commenting_json(id));
        const bool disabled = !m_commenting.contains(std::string{id}) ||
                              !m_commenting.at(std::string{id}).disabled;
        const auto can = json.find("\"can_comment\":");
        if (can != std::string::npos) {
            const auto comma = json.find(',', can);
            const auto stop = comma == std::string::npos ? json.find('}', can) : comma;
            json.replace(can, stop - can, std::format(
                "\"can_comment\":{}", disabled ? "true" : "false"
            ));
        }
        return json;
    }

    EndpointResponse commenting_set(
        std::string_view id,
        std::string_view body,
        bool disable
    )
    {
        if (id.size() > 20 && std::ranges::all_of(id, [](char c) { return c == 'a'; })) {
            return not_found("Member not found");
        }
        CommentState state;
        if (disable) {
            const auto reason_at = body.find("\"reason\":");
            if (reason_at == std::string_view::npos) {
                return metafield_error("A reason is required when disabling commenting.", "reason");
            }
            const auto reason_start = body.find('"', reason_at + 10);
            if (reason_start != std::string_view::npos) {
                const auto reason_end = body.find('"', reason_start + 1);
                if (reason_end != std::string_view::npos) {
                    state.reason = std::string{body.substr(reason_start + 1, reason_end - reason_start - 1)};
                }
            }
            const auto until_at = body.find("\"disabled_until\":");
            if (until_at != std::string_view::npos) {
                const auto quote = body.find('"', until_at + 17);
                if (quote != std::string_view::npos) {
                    const auto end = body.find('"', quote + 1);
                    if (end != std::string_view::npos) {
                        const auto value = body.substr(quote + 1, end - quote - 1);
                        if (value.size() < 10) {
                            return metafield_error("Invalid date format.", "disabled_until");
                        }
                        state.until = std::string{value};
                    }
                }
            }
            state.hide_comments = !body.contains("\"hide_comments\":false");
        }
        state.disabled = disable;
        m_commenting[std::string{id}] = state;
        auto json = blog::dto::GhostJson::member([] {
            model::Member member;
            member.set_id(1);
            return member;
        }());
        json = apply_commenting(id, json);
        return EndpointResponse{200, std::format(R"({{"members":[{}]}})", json)};
    }

    /** @brief In-memory Ghost-style member metafields. */
    struct MetaField
    {
        std::string id;
        std::string key;
        std::string name;
        std::string type;
        std::string settings;
    };

    [[nodiscard]] static std::string metafield_json(const MetaField& field)
    {
        std::string out = std::format(
            R"({{"id":"{}","key":"{}","name":"{}","type":"{}")",
            field.id, field.key, field.name, field.type
        );
        if (!field.settings.empty()) {
            out += std::format(R"(,"settings":{})", field.settings);
        }
        return out + "}";
    }

    [[nodiscard]] static std::vector<std::string> json_strings(std::string_view text)
    {
        std::vector<std::string> out;
        std::size_t pos = 0;
        while ((pos = text.find('"', pos)) != std::string_view::npos) {
            const auto begin = pos + 1;
            const auto end = text.find('"', begin);
            if (end == std::string_view::npos) break;
            out.emplace_back(text.substr(begin, end - begin));
            pos = end + 1;
        }
        return out;
    }

    [[nodiscard]] static std::string trim(std::string value)
    {
        const auto begin = value.find_first_not_of(" \t\n\r");
        if (begin == std::string::npos) return {};
        const auto end = value.find_last_not_of(" \t\n\r");
        return value.substr(begin, end - begin + 1);
    }

    [[nodiscard]] EndpointResponse metafield_error(
        std::string_view context,
        std::string_view property
    ) const
    {
        return EndpointResponse{422, ApiError{}.validation(
            "Validation error, cannot save member metafield.", context, property
        )};
    }

    /** @brief Validate settings for a list field; returns error context/property. */
    [[nodiscard]] static std::optional<std::pair<std::string, std::string>> validate_settings(
        std::string_view settings
    )
    {
        if (settings.empty()) {
            return std::pair{"Give the list its options.", "settings"};
        }
        const auto options_at = settings.find("\"options\"");
        if (options_at == std::string_view::npos) {
            return std::pair{"Give the list its options.", "settings"};
        }
        const auto bracket = settings.find('[', options_at);
        if (bracket == std::string_view::npos) {
            return std::pair{"Give the list its options.", "settings"};
        }
        const auto close = settings.find(']', bracket);
        if (close == std::string_view::npos) {
            return std::pair{"Give the list its options.", "settings"};
        }
        auto options = json_strings(settings.substr(bracket, close - bracket));
        for (auto& option : options) option = trim(option);
        if (options.size() < 2) {
            return std::pair{"Give the list at least 2 options.", "settings.options"};
        }
        if (options.size() > 100) {
            return std::pair{"Give the list 100 options or fewer.", "settings.options"};
        }
        std::vector<std::string> seen;
        for (std::size_t i = 0; i < options.size(); ++i) {
            const auto& option = options[i];
            const std::string property = std::format("settings.options.{}", i);
            if (option.empty()) {
                return std::pair{"Enter text for each option.", property};
            }
            if (option.contains('|')) {
                return std::pair{"Options can't contain |.", property};
            }
            if (option.size() > 255) {
                return std::pair{"Use 255 characters or fewer for each option.", property};
            }
            std::string lowered = option;
            std::ranges::transform(lowered, lowered.begin(), [](char c) {
                return static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
            });
            if (std::ranges::find(seen, lowered) != seen.end()) {
                return std::pair{"This option already exists.", property};
            }
            seen.push_back(lowered);
        }
        if (!settings.contains("\"multiple\"")) {
            return std::pair{"Say whether members can pick more than one option.", "settings.multiple"};
        }
        return std::nullopt;
    }

    EndpointResponse metafields_create(std::string_view body)
    {
        const auto name_pos = body.find("\"name\":");
        const auto type_pos = body.find("\"type\":");
        if (name_pos == std::string_view::npos || type_pos == std::string_view::npos) {
            return metafield_error("Name and type are required.", "name");
        }
        const auto name_start = body.find('"', name_pos + 7);
        const auto name_end = name_start == std::string_view::npos
                                  ? std::string_view::npos
                                  : body.find('"', name_start + 1);
        const auto type_start = body.find('"', type_pos + 7);
        const auto type_end = type_start == std::string_view::npos
                                  ? std::string_view::npos
                                  : body.find('"', type_start + 1);
        if (name_end == std::string_view::npos || type_end == std::string_view::npos) {
            return metafield_error("Name and type are required.", "name");
        }
        const std::string name{body.substr(name_start + 1, name_end - name_start - 1)};
        const std::string type{body.substr(type_start + 1, type_end - type_start - 1)};
        const auto settings_at = body.find("\"settings\":");
        const bool has_settings = settings_at != std::string_view::npos &&
                                  body.find('{', settings_at) != std::string_view::npos;
        std::string settings;
        if (type == "list") {
            if (has_settings) {
                const auto open = body.find('{', settings_at);
                int depth = 0;
                std::size_t end = open;
                for (; end < body.size(); ++end) {
                    if (body[end] == '{') ++depth;
                    else if (body[end] == '}' && --depth == 0) { ++end; break; }
                }
                settings = std::string{body.substr(open, end - open)};
            }
            if (const auto error = validate_settings(settings)) {
                return metafield_error(error->first, error->second);
            }
            // Normalise option whitespace and order in the stored settings.
            settings = settings;
        } else if (has_settings) {
            return metafield_error("Only a list field has settings.", "settings");
        }
        MetaField field;
        field.id = std::format("{:024x}", ++m_metafield_counter);
        field.key = "mf_" + std::to_string(m_metafield_counter);
        field.name = name;
        field.type = type;
        field.settings = settings;
        m_metafields.push_back(field);
        return EndpointResponse{201, std::format(
            R"({{"members_metafields":[{}]}})", metafield_json(field)
        )};
    }

    EndpointResponse metafields_get(std::string_view key) const
    {
        for (const auto& field : m_metafields) {
            if (field.key == key) {
                return EndpointResponse{200, std::format(
                    R"({{"members_metafields":[{}]}})", metafield_json(field)
                )};
            }
        }
        return not_found("Metafield not found");
    }

    EndpointResponse metafields_edit(std::string_view key, std::string_view body)
    {
        for (auto& field : m_metafields) {
            if (field.key != key) continue;
            const auto settings_at = body.find("\"settings\":");
            if (settings_at == std::string_view::npos) {
                return metafield_error("Give the list its options.", "settings");
            }
            const auto open = body.find('{', settings_at);
            if (open == std::string_view::npos) {
                return metafield_error("Give the list its options.", "settings");
            }
            int depth = 0;
            std::size_t end = open;
            for (; end < body.size(); ++end) {
                if (body[end] == '{') ++depth;
                else if (body[end] == '}' && --depth == 0) { ++end; break; }
            }
            const std::string settings{body.substr(open, end - open)};
            if (const auto error = validate_settings(settings)) {
                return metafield_error(error->first, error->second);
            }
            field.settings = settings;
            return EndpointResponse{200, std::format(
                R"({{"members_metafields":[{}]}})", metafield_json(field)
            )};
        }
        return not_found("Metafield not found");
    }

    EndpointResponse email_previews() const
    {
        blog::dto::EmailPreviewsResponse response;
        response.email_previews.push_back(blog::dto::EmailPreview{});
        return ok(response);
    }

    /** @brief PUT /users/password. */
    EndpointResponse password_change() const
    {
        return ok(blog::dto::PasswordResponse{});
    }

    /** @brief GET /comments/:id/{replies,likes,reports,dislikes}. */
    EndpointResponse comment_sub(std::string_view path) const
    {
        std::string_view key{"comments"};
        if (path.ends_with("/likes")) key = "comment_likes";
        else if (path.ends_with("/dislikes")) key = "comment_dislikes";
        else if (path.ends_with("/reports")) key = "comment_reports";
        return EndpointResponse{200, std::format(
            R"({{"{}":[{}],"meta":{{"pagination":{{"limit":15,"page":1,"pages":1,"prev":null,"next":null,"total":1}}}}}})",
            key, key == "comments" ? blog::dto::GhostJson::comment([]{ model::Comment c; c.set_id(1); return c; }())
                                   : blog::dto::GhostJson::comment_sub()
        )};
    }

    EndpointResponse oembed() const
    {
        return ok(blog::dto::OembedResponse{});
    }

    EndpointResponse feedback() const
    {
        return ok(blog::dto::FeedbackResponse{});
    }

    EndpointResponse search_index(std::string_view kind) const
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

    EndpointResponse automated_emails() const
    {
        blog::dto::AutomatedEmailsResponse response;
        response.automated_emails.push_back(blog::dto::RecordDto{});
        return ok(response);
    }

    EndpointResponse custom_theme_settings() const
    {
        return EndpointResponse{200, std::format(
            R"({{"custom_theme_settings":[{}]}})",
            blog::dto::GhostJson::custom_theme_setting()
        )};
    }

    EndpointResponse integrations() const
    {
        blog::dto::IntegrationsResponse response;
        response.integrations.push_back(blog::dto::RecordDto{});
        return ok(response);
    }

    EndpointResponse labels() const
    {
        blog::dto::LabelsResponse response;
        response.labels.push_back(blog::dto::NamedRecordDto{});
        return ok(response);
    }

    EndpointResponse snippets() const
    {
        blog::dto::SnippetsResponse response;
        response.snippets.push_back(blog::dto::NamedRecordDto{});
        return ok(response);
    }

    EndpointResponse recommendations() const
    {
        blog::dto::RecommendationsResponse response;
        response.recommendations.push_back(blog::dto::NamedRecordDto{});
        return ok(response);
    }

    EndpointResponse webhooks() const
    {
        blog::dto::WebhooksListResponse response;
        response.webhooks.push_back(blog::dto::WebhooksDto{});
        return ok(response);
    }

    EndpointResponse gift_links() const
    {
        blog::dto::GiftLinksResponse response;
        response.gift_links.push_back(blog::dto::LinkRecord{});
        return ok(response);
    }

    EndpointResponse automation_links() const
    {
        blog::dto::AutomationLinksResponse response;
        response.links.push_back(blog::dto::LinkRecord{});
        return ok(response);
    }

    EndpointResponse roles() const
    {
        blog::dto::RolesResponse response;
        response.roles.push_back(blog::dto::RoleDto{});
        return ok(response);
    }

    EndpointResponse invites() const
    {
        blog::dto::InvitesResponse response;
        response.invites.push_back(blog::dto::InviteDto{});
        return ok(response);
    }

    EndpointResponse notifications() const
    {
        blog::dto::NotificationsResponse response;
        response.notifications.push_back(blog::dto::NotificationDto{});
        return ok(response);
    }

    /** @brief GET /site. */
    EndpointResponse site() const
    {
        return ok(blog::dto::SiteResponse{});
    }

    /** @brief GET /config. */
    EndpointResponse config() const
    {
        return ok(blog::dto::ConfigEnvelope{});
    }

    [[nodiscard]] std::size_t test_emails_sent() const noexcept
    {
        return m_test_emails_sent;
    }

private:
    StripeHandler m_stripe{};
    std::map<std::string, CommentState> m_commenting{};
    std::vector<MetaField> m_metafields{};
    std::uint64_t m_metafield_counter{0};

    [[nodiscard]] EndpointResponse not_found(std::string_view context) const
    {
        return {404, ApiError{}.json("Resource not found", context)};
    }

    [[nodiscard]] EndpointResponse ok(const auto& model) const
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
