import std;
import blog_testing;

int main()
{
    auto api = blog::api::TestApi{};
    if (api.route_count() != 261) return 1;
    if (api.content_route_count() != 20) return 2;

    const auto posts = api.dispatch(blog::api::ApiRequest{"GET", "/posts"});
    if (!posts.ok() || !posts.body().contains("Hello Ghost")) {
        return 3;
    }
    for (const char* magic: {"\"posts\":", "\"meta\":", "\"pagination\":",
                             "\"limit\":", "\"page\":", "\"pages\":",
                             "\"prev\":", "\"next\":", "\"total\":"}) {
        if (!posts.body().contains(magic)) return 21;
    }
    // Ghost query semantics: fields / filter / include.
    const auto projected = api.dispatch(blog::api::ApiRequest{"GET", "/posts?fields=title,slug"});
    if (!projected.ok() || !projected.body().contains("\"title\"") ||
        !projected.body().contains("\"slug\"") ||
        projected.body().contains("\"html\"") ||
        projected.body().contains("\"status\"")) return 39;
    const auto filtered_draft = api.dispatch(blog::api::ApiRequest{"GET", "/posts?filter=status:draft"});
    if (!filtered_draft.ok() || !filtered_draft.body().contains("Second Post") ||
        filtered_draft.body().contains("Hello Ghost")) return 40;
    const auto filtered_or = api.dispatch(
        blog::api::ApiRequest{"GET", "/posts?filter=status:draft,status:published"}
    );
    if (!filtered_or.ok() || !filtered_or.body().contains("Second Post") ||
        !filtered_or.body().contains("Hello Ghost")) return 41;
    const auto filtered_legacy = api.dispatch(
        blog::api::ApiRequest{"GET", "/posts?filter[status]=published"}
    );
    if (!filtered_legacy.ok() || !filtered_legacy.body().contains("Hello Ghost") ||
        filtered_legacy.body().contains("Second Post")) return 42;
    const auto embedded = api.dispatch(blog::api::ApiRequest{"GET", "/posts?include=authors"});
    if (!embedded.ok() || !embedded.body().contains("\"authors\":[")) return 43;

    const auto one = api.dispatch(blog::api::ApiRequest{"GET", "/posts/1"});
    if (!one.ok() || !one.body().contains("Hello Ghost")) return 4;

    for (const char* path: {"/pages", "/tags", "/authors", "/settings", "/newsletters", "/tiers"}) {
        const auto response = api.dispatch(blog::api::ApiRequest{"GET", path});
        if (!response.ok()) return 5;
        if (response.body() == "[]" || response.body() == "{}") return 6;
    }

    if (api.dispatch(blog::api::ApiRequest{"GET", "/members"}).status() != 401) return 7;
    const auto allowed = api.dispatch(blog::api::ApiRequest{"GET", "/members"}.token("t"));
    if (!allowed.ok() || !allowed.body().contains("members")) return 8;

    // Admin create: real state change + Ghost-shaped response.
    const auto created = api.dispatch(blog::api::ApiRequest{"POST", "/posts"}
        .token("admin")
        .body(R"({"title":"New Admin Post","slug":"new-admin-post","status":"draft"})"));
    if (!created.ok()) return 9;
    if (!created.body().contains("New Admin Post")) return 10;
    if (!created.body().contains("\"posts\":")) return 11;
    const auto after_create = api.dispatch(blog::api::ApiRequest{"GET", "/posts"});
    if (!after_create.body().contains("New Admin Post")) return 12;

    // Admin edit: real field change.
    const auto edited = api.dispatch(blog::api::ApiRequest{"PUT", "/posts/1"}
        .token("admin")
        .body(R"({"title":"Edited Title","status":"published"})"));
    if (!edited.ok() || !edited.body().contains("Edited Title")) return 13;
    const auto after_edit = api.dispatch(blog::api::ApiRequest{"GET", "/posts/1"});
    if (!after_edit.body().contains("Edited Title")) return 14;

    // Admin delete: state removed.
    const auto deleted = api.dispatch(blog::api::ApiRequest{"DELETE", "/posts/1"}.token("admin"));
    if (!deleted.ok()) return 15;
    const auto after_delete = api.dispatch(blog::api::ApiRequest{"GET", "/posts/1"});
    if (after_delete.ok()) return 16;

    // Filtering + pagination.
    const auto drafts = api.dispatch(blog::api::ApiRequest{"GET", "/posts?filter[status]=draft"});
    if (!drafts.ok() || drafts.body().contains(R"("status":"published")")) return 17;
    const auto limited = api.dispatch(blog::api::ApiRequest{"GET", "/posts?limit=1"});
    if (!limited.ok()) return 18;
    const auto all_posts = api.dispatch(blog::api::ApiRequest{"GET", "/posts"});
    const auto full_closes = std::ranges::count(all_posts.body(), '}');
    const auto limited_closes = std::ranges::count(limited.body(), '}');
    if (limited_closes >= full_closes) return 19;

    // Validation: title required for post creation.
    const auto invalid = api.dispatch(blog::api::ApiRequest{"POST", "/posts"}
        .token("admin").body(R"({"slug":"no-title"})"));
    if (invalid.status() != 422 || !invalid.body().contains("title is required")) return 20;
    for (const char* magic: {"\"errors\":", "\"id\":", "\"code\":null",
                             "\"context\":", "\"details\":null",
                             "\"ghostErrorCode\":null", "\"message\":"}) {
        if (!invalid.body().contains(magic)) return 22;
    }

    // Remaining admin resources: webhooks, automations, comments, integrations, offers, emails.
    for (const char* resource: {"webhooks", "automations", "comments", "offers"}) {
        const std::string path = std::string{"/"} + resource;
        const auto created_res = api.dispatch(blog::api::ApiRequest{"POST", path}
            .token("admin").body(R"({"name":"probe","status":"active"})"));
        if (!created_res.ok()) return 23;
        // Resources with a browse route expose the new record via GET.
        if (std::string{resource} != "webhooks" && std::string{resource} != "comments") {
            const auto listed = api.dispatch(blog::api::ApiRequest{"GET", path}.token("admin"));
            if (!listed.ok() || !listed.body().contains("probe")) {
                return 24;
            }
            for (const char* field: {"\"created_at\":", "\"updated_at\":", "\"id\":"}) {
                if (!listed.body().contains(field)) return 37;
            }
        }
        const std::string& body_text = created_res.body();
        const auto id_pos = body_text.find("\"id\":\"");
        const std::string created_id = id_pos == std::string::npos
            ? std::string{"1"}
            : body_text.substr(id_pos + 6, body_text.find('"', id_pos + 6) - id_pos - 6);
        if (std::string{resource} == "webhooks" || std::string{resource} == "integrations") {
            const auto deleted_res = api.dispatch(
                blog::api::ApiRequest{"DELETE", path + "/" + created_id}.token("admin")
            );
            if (!deleted_res.ok()) return 25;
        } else {
            const auto edited_res = api.dispatch(
                blog::api::ApiRequest{"PUT", path + "/" + created_id}.token("admin")
                    .body(R"({"status":"edited"})")
            );
            if (!edited_res.ok()) return 25;
        }
    }

    const auto email_retry = api.dispatch(blog::api::ApiRequest{"PUT", "/emails/1/retry"}
        .token("admin").body(R"({"status":"queued"})"));
    if (!email_retry.ok()) return 29;

    // Bulk operations.
    const auto bulk_a = api.dispatch(blog::api::ApiRequest{"POST", "/webhooks"}.token("admin")
        .body(R"({"name":"a"})"));
    const auto bulk_b = api.dispatch(blog::api::ApiRequest{"POST", "/webhooks"}.token("admin")
        .body(R"({"name":"b"})"));
    if (!bulk_a.ok() || !bulk_b.ok()) return 28;
    const auto bulk = api.dispatch(blog::api::ApiRequest{"DELETE", "/webhooks"}
        .token("admin").body(R"({"ids":"2,3"})"));
    if (!bulk.ok() || !bulk.body().contains("deleted")) return 26;
    const auto bulk_edit = api.dispatch(blog::api::ApiRequest{"PUT", "/webhooks/bulk"}
        .token("admin").body(R"({"ids":"4","status":"edited"})"));
    if (!bulk_edit.ok() || !bulk_edit.body().contains("updated")) return 27;

    // Stripe endpoints: real config state observable across requests.
    const auto stripe_before = api.dispatch(blog::api::ApiRequest{"GET", "/stripe/checkout/config"}.token("admin"));
    if (!stripe_before.body().contains("\"session_mode\":\"payment\"")) return 32;
    const auto stripe_put = api.dispatch(blog::api::ApiRequest{"PUT", "/stripe/checkout/config"}
        .token("admin").body(R"({"session_mode":"subscription","currency":"eur"})"));
    if (!stripe_put.ok()) return 33;
    const auto stripe_after = api.dispatch(blog::api::ApiRequest{"GET", "/stripe/checkout/config"}.token("admin"));
    if (!stripe_after.body().contains("\"session_mode\":\"subscription\"")) return 34;
    const auto stripe_preview = api.dispatch(blog::api::ApiRequest{"POST", "/stripe/checkout/preview"}
        .token("admin").body(R"({"amount":500})"));
    if (!stripe_preview.body().contains("\"currency\":\"eur\"")) {
        return 35;
    }
    const auto stripe_branding = api.dispatch(blog::api::ApiRequest{"GET", "/stripe/checkout/branding"}.token("admin"));
    if (!stripe_branding.body().contains("branding")) return 36;

    // Special admin endpoints — shapes from Ghost e2e-api snapshots.
    const auto preview = api.dispatch(
        blog::api::ApiRequest{"POST", "/automations/1/email_preview"}.token("admin")
            .body(R"({"subject":"Automation Subject","lexical":"{}"})")
    );
    if (preview.status() != 200 ||
        !preview.body().contains(R"("automation_email_previews":[)") ||
        !preview.body().contains(R"("subject":"Automation Subject")") ||
        !preview.body().contains(R"("html":)") ||
        !preview.body().contains(R"("plaintext":)")) return 44;
    const auto preview_invalid = api.dispatch(
        blog::api::ApiRequest{"POST", "/automations/1/email_preview"}.token("admin").body("{}")
    );
    if (preview_invalid.status() != 422 ||
        !preview_invalid.body().contains(R"("property":"lexical")") ||
        !preview_invalid.body().contains(R"("type":"ValidationError")")) return 45;
    const auto email_test = api.dispatch(
        blog::api::ApiRequest{"POST", "/automations/1/email_test"}.token("admin")
            .body(R"({"email":"test@ghost.org","subject":"Hi","lexical":"{}"})")
    );
    if (email_test.status() != 204 || !email_test.body().empty()) return 46;
    const auto email_test_invalid = api.dispatch(
        blog::api::ApiRequest{"POST", "/automations/1/email_test"}.token("admin")
            .body(R"({"email":"not-an-email"})")
    );
    if (email_test_invalid.status() != 422 ||
        !email_test_invalid.body().contains(R"("property":"email")")) return 47;

    const auto batches = api.dispatch(
        blog::api::ApiRequest{"GET", "/emails/1/batches"}.token("admin")
    );
    for (const char* magic: {R"("batches":[)", R"("status":"submitted")",
                             R"("mailgun_message_id":)", R"("error_data":null)",
                             R"("error_message":null)", R"("error_status_code":null)",
                             R"("member_segment":null)", R"("next":null)",
                             R"("prev":null)", R"("meta":)", R"("pagination":)"}) {
        if (!batches.body().contains(magic)) return 48;
    }
    const auto failures = api.dispatch(
        blog::api::ApiRequest{"GET", "/emails/1/recipient-failures"}.token("admin")
    );
    for (const char* magic: {R"("failures":[)", R"("severity":"temporary")",
                             R"("enhanced_code":"5.5.5")", R"("code":555)"}) {
        if (!failures.body().contains(magic)) return 49;
    }
    const auto analytics = api.dispatch(
        blog::api::ApiRequest{"GET", "/emails/1/analytics"}.token("admin")
    );
    for (const char* magic: {R"("jobName":"email-analytics-latest-others")",
                             R"("jobName":"email-analytics-latest-opened")",
                             R"("jobName":"email-analytics-missing")",
                             R"("jobName":"email-analytics-scheduled")",
                             R"("fetchedThrough":null)", R"("lagSeconds":null)"}) {
        if (!analytics.body().contains(magic)) return 50;
    }
    const auto analytics_put = api.dispatch(
        blog::api::ApiRequest{"PUT", "/emails/1/analytics"}.token("admin")
            .body(R"({"jobName":"email-analytics-latest-opened","running":true})")
    );
    if (!analytics_put.ok()) return 51;
    const auto analytics_after = api.dispatch(
        blog::api::ApiRequest{"GET", "/emails/1/analytics"}.token("admin")
    );
    if (!analytics_after.body().contains(R"("running":true)")) return 52;
    const auto analytics_delete = api.dispatch(
        blog::api::ApiRequest{"DELETE", "/emails/analytics"}.token("admin")
    );
    if (!analytics_delete.ok()) return 53;

    const auto sub_create = api.dispatch(
        blog::api::ApiRequest{"POST", "/members/1/subscriptions/"}.token("admin")
            .body(R"({"tier":"gold","status":"active"})")
    );
    for (const char* magic: {R"("members":[)", R"("subscriptions":[)",
                             R"("cancel_at_period_end":false)",
                             R"("cancellation_reason":null)",
                             R"("current_subscription":null)", R"("name":null)"}) {
        if (!sub_create.body().contains(magic)) return 54;
    }
    const auto sub_edit = api.dispatch(
        blog::api::ApiRequest{"PUT", "/members/1/subscriptions/1"}.token("admin")
            .body(R"({"tier":"gold","status":"canceled"})")
    );
    if (!sub_edit.body().contains(R"("status":"canceled")")) return 55;
    const auto signin_urls = api.dispatch(
        blog::api::ApiRequest{"GET", "/members/1/signin_urls"}.token("admin")
    );
    if (!signin_urls.body().contains(R"("signin_urls":[)")) return 56;

    // Stats + config special endpoints — Ghost snapshot keys.
    const auto stats_mrr = api.dispatch(blog::api::ApiRequest{"GET", "/stats/mrr"}.token("admin"));
    for (const char* magic: {R"("stats":[)", R"("mrr":1000)", R"("totals":[)",
                             R"("currency":"usd")", R"("meta":)"}) {
        if (!stats_mrr.body().contains(magic)) return 57;
    }
    const auto stats_members = api.dispatch(
        blog::api::ApiRequest{"GET", "/stats/member_count"}.token("admin")
    );
    for (const char* magic: {R"("paid":5)", R"("free":3)", R"("totals":{)"}) {
        if (!stats_members.body().contains(magic)) return 58;
    }
    const auto stats_empty = api.dispatch(
        blog::api::ApiRequest{"GET", "/stats/top-posts"}.token("admin")
    );
    if (!stats_empty.body().contains(R"("stats":[)")) return 59;
    const auto config = api.dispatch(
        blog::api::ApiRequest{"GET", "/config/featurebase"}.token("admin")
    );
    for (const char* magic: {R"("auth":{)", R"("engine":"legacy")",
                             R"("database":"mysql8")", R"("emailAnalytics":true)"}) {
        if (!config.body().contains(magic)) return 60;
    }

    // Every admin route must return a Ghost-shaped body, never the generic stub.
    std::size_t admin_probed = 0;
    for (const auto& route: api.routes()) {
        if (!route.admin()) {
            continue;
        }
        std::string concrete{route.path()};
        for (;;) {
            const auto colon = concrete.find(':');
            if (colon == std::string::npos) break;
            const auto slash = concrete.find('/', colon);
            concrete.replace(colon, slash == std::string::npos ? concrete.size() - colon : slash - colon, "1");
        }
        const auto response = api.dispatch(
            blog::api::ApiRequest{route.method(), concrete}.token("admin").body(R"({"name":"coverage"})")
        );
        if (response.status() == 404) {
            continue;
        }
        if (response.body().contains(R"("resource":)")) return 32;
        if (route.action() != "delete" && (response.body().size() < 3 ||
                                           response.body() == "{}")) return 37;
        ++admin_probed;
    }
    if (admin_probed < 100) return 33;

    // Every admin collection POST route must produce a record readable via GET.
    std::size_t write_resources = 0;
    for (const auto& route: api.routes()) {
        if (!route.admin() || route.action() != "post") {
            continue;
        }
        const auto& path = route.path();
        if (path.size() < 2 || path.find('/', 1) != std::string::npos ||
            path.find(':') != std::string::npos) {
            continue;
        }
        if (path == "/notifications" || path == "/invites" || path == "/roles" ||
            path == "/roles/" || path == "/labels" || path == "/snippets" ||
            path == "/recommendations" || path == "/automated_emails" ||
            path == "/custom_theme_settings" || path == "/integrations" ||
            path == "/comments") {
            continue; // snapshot-shaped GET handler owns this resource
        }
        const bool readable = std::ranges::any_of(api.routes(), [&](const auto& candidate) {
            return candidate.method() == "GET" && candidate.path() == path;
        });
        if (!readable) {
            continue;
        }
        const auto created = api.dispatch(
            blog::api::ApiRequest{"POST", path}.token("admin")
                .body(R"({"name":"route-probe","title":"route-probe","slug":"route-probe"})")
        );
        if (!created.ok()) return 34;
        const auto listed = api.dispatch(blog::api::ApiRequest{"GET", path}.token("admin"));
        if (!listed.ok() || !listed.body().contains("route-probe")) {
            return 35;
        }
        ++write_resources;
    }
    if (write_resources < 5) return 36;

    return 0;
}
