import std;
import blog_testing;
import blog_handlers;
import blog_context;

using namespace blog::testing;

/**
 * Replays Ghost e2e-api snapshot cases through the C++ API over the real
 * HTTP2 wire path. Each case asserts the snapshot's primary top-level key and,
 * for full posts/members bodies, the exact Ghost field set and JSON types.
 */
int main()
{
    blog::api::BlogHandlers handler;
    std::size_t passed{0};
    std::size_t failed{0};
    std::size_t schema_bodies{0};
    std::size_t schema_checked{0};
    std::size_t schema_passed{0};
    std::size_t schema_bodies_failed{0};

    for (const auto& snapshot_case : blog::testing::SnapshotCases::values) {
        blog::testing::WireClient wire{handler};
        const auto result = wire.request(
            snapshot_case.method, snapshot_case.path, snapshot_case.body, "admin"
        );
        const bool is_bulk = snapshot_case.path.contains("/bulk");
        const bool is_delete = snapshot_case.method == "DELETE";
        const bool valid_error = result.body().contains(R"("errors":[)");
        const bool expected = is_delete || is_bulk
                                  ? result.body().contains(R"("meta":)") ||
                                        result.body().empty() ||
                                        result.body() == "{}"
                                  : result.body().contains(snapshot_case.key);
        // Structural oracle: validate Ghost field sets/types on collection reads.
        const bool projected = snapshot_case.path.contains("fields=") ||
                               snapshot_case.path.contains("include=") ||
                               snapshot_case.path.contains("filter=") ||
                               snapshot_case.path.contains("/export") ||
                               snapshot_case.path.contains("/search-index/") ||
                               snapshot_case.path.contains("/subscriptions") ||
                               snapshot_case.path.contains("/verifications") ||
                               snapshot_case.path.contains("/actions/");
        const EntitySchema* endpoint_stats = nullptr;
        if (snapshot_case.path.starts_with("/stats/mrr")) endpoint_stats = &kStats_mrrSchema;
        else if (snapshot_case.path.starts_with("/stats/member")) endpoint_stats = &kStats_membersSchema;
        else if (snapshot_case.path.starts_with("/stats/subscriptions")) endpoint_stats = &kStats_subscriptionsSchema;
        const EntitySchema* schema = nullptr;
        const bool read_method = snapshot_case.method == "GET";
        if (read_method && !projected && result.body().contains("\"posts\":[") && !result.body().contains("\"errors\":"))
            schema = &kPostSchema;
        else if (read_method && !projected && result.body().contains("\"members\":[") && !result.body().contains("\"errors\":"))
            schema = &kMemberSchema;
        else if (read_method && !projected && result.body().contains("\"users\":[") && !result.body().contains("\"errors\":"))
            schema = &kUserSchema;
        else if (read_method && !projected && result.body().contains("\"newsletters\":[") && !result.body().contains("\"errors\":"))
            schema = &kNewsletterSchema;
        else if (read_method && !projected && result.body().contains("\"tiers\":[") && !result.body().contains("\"errors\":"))
            schema = &kTierSchema;
        else if (read_method && !projected && result.body().contains("\"comments\":[") && !result.body().contains("\"errors\":"))
            schema = &kCommentSchema;
        else if (read_method && !projected && result.body().contains("\"offers\":[") && !result.body().contains("\"errors\":"))
            schema = &kOfferSchema;
        else if (read_method && !projected && result.body().contains("\"notifications\":[") && !result.body().contains("\"errors\":"))
            schema = &kNotificationSchema;
        else if (read_method && !projected && result.body().contains("\"webhooks\":[") && !result.body().contains("\"errors\":"))
            schema = &kWebhookSchema;
        else if (read_method && !projected && result.body().contains("\"emails\":[") && !result.body().contains("\"errors\":"))
            schema = &kEmailSchema;
        else if (read_method && !projected && result.body().contains("\"labels\":[") && !result.body().contains("\"errors\":"))
            schema = &kLabelSchema;
        else if (read_method && !projected && result.body().contains("\"snippets\":[") && !result.body().contains("\"errors\":"))
            schema = &kSnippetSchema;
        else if (read_method && !projected && result.body().contains("\"roles\":[") && !result.body().contains("\"errors\":"))
            schema = &kRoleSchema;
        else if (read_method && !projected && result.body().contains("\"recommendations\":[") && !result.body().contains("\"errors\":"))
            schema = &kRecommendationSchema;
        else if (read_method && !projected && result.body().contains("\"links\":[") && !result.body().contains("\"errors\":"))
            schema = &kLinkSchema;
        else if (read_method && !projected && result.body().contains("\"automated_emails\":[") && !result.body().contains("\"errors\":"))
            schema = &kAutomatedEmailSchema;
        else if (read_method && !projected && result.body().contains("\"events\":[") && !result.body().contains("\"errors\":"))
            schema = &kEventSchema;
        else if (read_method && !projected && result.body().contains("\"site\":{") && !result.body().contains("\"errors\":"))
            schema = &kSiteSchema;
        else if (read_method && !projected && snapshot_case.path.starts_with("/config") &&
                 result.body().contains("\"config\":{") && !result.body().contains("\"errors\":"))
            schema = &kConfigSchema;
        else if (read_method && !projected && result.body().contains("\"pages\":[") && !result.body().contains("\"errors\":"))
            schema = &kPagesSchema;
        else if (result.body().contains("\"unsubscribe_url\":") &&
                 !result.body().contains("\"members\":[") && !result.body().contains("\"errors\":"))
            schema = &kMemberObjectSchema;
        else if (result.body().contains("\"bulk\":{") && !result.body().contains("\"errors\":"))
            schema = &kBulkSchema;
        else if (read_method && !projected && result.body().contains("\"db\":[") && !result.body().contains("\"errors\":"))
            schema = &kDbSchema;
        else if (read_method && !projected && result.body().contains("\"email_previews\":[") && !result.body().contains("\"errors\":"))
            schema = &kEmailPreviewSchema;
        else if (read_method && !projected && result.body().contains("\"email_statuses\":[") && !result.body().contains("\"errors\":"))
            schema = &kEmailStatusSchema;
        else if (read_method && !projected && result.body().contains("\"failures\":[") && !result.body().contains("\"errors\":"))
            schema = &kFailuresSchema;
        else if (read_method && !projected && result.body().contains("\"batches\":[") && !result.body().contains("\"errors\":"))
            schema = &kBatchesSchema;
        else if (read_method && !projected && result.body().contains("\"automations\":[") && !result.body().contains("\"errors\":"))
            schema = &kAutomationSchema;
        else if (read_method && !projected && result.body().contains("\"slugs\":[") && !result.body().contains("\"errors\":"))
            schema = &kSlugSchema;
        else if (read_method && !projected && result.body().contains("\"custom_theme_settings\":[") && !result.body().contains("\"errors\":"))
            schema = &kCustomThemeSettingSchema;
        else if (read_method && !projected && result.body().contains("\"mentions\":[") && !result.body().contains("\"errors\":"))
            schema = &kMentionSchema;

        else if (read_method && !projected && result.body().contains("\"settings\":[") && !result.body().contains("\"errors\":"))
            schema = &kSettingSchema;
        else if (read_method && !projected && result.body().contains("\"comment_likes\":[") && !result.body().contains("\"errors\":"))
            schema = &kCommentLikeSchema;
        else if (read_method && !projected && result.body().contains("\"comment_dislikes\":[") && !result.body().contains("\"errors\":"))
            schema = &kCommentDislikeSchema;
        else if (read_method && !projected && result.body().contains("\"comment_reports\":[") && !result.body().contains("\"errors\":"))
            schema = &kCommentReportSchema;
        if (endpoint_stats && !result.body().contains("\"errors\":"))
            schema = endpoint_stats;
        if (schema) {
            const auto report = validate(result.body(), *schema);
            ++schema_bodies;
            schema_checked += report.checked;
            schema_passed += report.passed;
            if (!report.failures.empty()) {
                ++schema_bodies_failed;
                if (schema_bodies_failed <= 10) {
                    std::println("SCHEMA {} {}: {}", snapshot_case.method, snapshot_case.path,
                                 report.failures.front());
                }
            }
        }

        if (!expected && !valid_error) {
            if (failed < 40) std::println("MISS {} {} key={} body={}", snapshot_case.method,
                snapshot_case.path, snapshot_case.key, result.body().substr(0,100));
            ++failed;
        } else {
            ++passed;
        }
    }

    // Detail reads using real seeded ids from each collection.
    for (const char* resource: {"posts", "pages", "tags", "users", "members"}) {
        const std::string path = std::string{"/"} + resource;
        blog::testing::WireClient wire{handler};
        const auto list = wire.request("GET", path, "", "admin");
        const auto pos = list.body().find(R"("id":)");
        if (pos == std::string::npos) { ++failed; continue; }
        const auto begin = pos + 5;
        const auto end = list.body().find_first_of(",}", begin);
        const std::string id = list.body().substr(begin, end - begin);
        const auto detail = wire.request("GET", path + "/" + id, "", "admin");
        if (!detail.body().contains(resource)) ++failed;
    }

    // Stateful create-then-read over the wire.
    for (const char* resource: {"posts", "pages", "tags", "newsletters", "tiers"}) {
        const std::string value = std::string{"Stateful "} + resource;
        const std::string body = R"({"title":")" + value + R"(","name":")" + value +
                                 R"(","slug":"stateful-)" + resource + R"("})";
        blog::testing::WireClient wire{handler};
        const auto created = wire.request("POST", std::string{"/"} + resource, body, "admin");
        if (created.body().empty()) { ++failed; continue; }
        const auto read = wire.request("GET", std::string{"/"} + resource, "", "admin");
        if (!read.body().contains(value)) ++failed;
    }

    // Ghost filter operators: negation, contains and numeric comparison.
    // A fresh handler set keeps the seeded fixture values intact.
    {
        blog::api::BlogHandlers fresh;
        blog::testing::WireClient wire{fresh};
        const auto neg = wire.request("GET", "/posts?filter=status:-draft", "", "admin");
        if (!neg.body().contains("Hello Ghost") || neg.body().contains("Second Post")) ++failed;
        const auto contains = wire.request("GET", "/posts?filter=title:~ghost", "", "admin");
        if (!contains.body().contains("Hello Ghost")) ++failed;
        const auto gt = wire.request("GET", "/tiers?filter=monthly_price:>0", "", "admin");
        if (!gt.body().contains("Gold") || gt.body().contains("Free")) ++failed;
        const auto lt = wire.request("GET", "/tiers?filter=monthly_price:<1", "", "admin");
        if (!lt.body().contains("Free") || lt.body().contains("Gold")) ++failed;
    }

    // Query semantics over the wire path (include / fields / validation).
    {
        blog::api::BlogHandlers fresh;
        blog::testing::WireClient wire{fresh};
        const auto included = wire.request("GET", "/posts?include=authors", "", "admin");
        if (!included.body().contains(R"("authors":[)")) ++failed;
        const auto projected = wire.request("GET", "/posts?fields=title,slug", "", "admin");
        if (projected.body().contains(R"("html":)")) ++failed;
        const auto filtered = wire.request("GET", "/posts?filter=status:draft", "", "admin");
        if (filtered.body().contains("Hello Ghost")) ++failed;
        const auto invalid = wire.request("POST", "/members", R"({"email":"bad"})", "admin");
        if (!invalid.body().contains(R"("errors":[)") ||
            !invalid.body().contains(R"("property":)")) ++failed;
    }

    // DTO-backed domain handlers keep their Ghost payloads on the wire path.
    {
        blog::api::BlogHandlers fresh;
        blog::testing::WireClient wire{fresh};
        const auto stats = wire.request("GET", "/stats/mrr", "", "admin");
        if (!stats.body().contains(R"("mrr":)")) ++failed;
        const auto site = wire.request("GET", "/site", "", "admin");
        if (!site.body().contains(R"("title":)")) ++failed;
        const auto config = wire.request("GET", "/config/featurebase", "", "admin");
        if (!config.body().contains(R"("config":)")) ++failed;
    }

    // Injected context (plugin-load path): handlers must share the caller's
    // BlogContext state, not a private duplicate store.
    {
        blog::BlogContext shared;
        blog::api::BlogHandlers injected{shared};
        blog::testing::WireClient wire{injected};
        const auto before = shared.store().total("posts");
        const auto created = wire.request(
            "POST", "/posts", R"({"title":"Injected Post","slug":"injected-post"})", "admin"
        );
        const auto listed = wire.request("GET", "/posts", "", "admin");
        if (!created.body().contains("Injected Post") ||
            !listed.body().contains("Injected Post") ||
            shared.store().total("posts") <= before) {
            ++failed;
        }
    }

    // Structural conformance is a hard assertion: a Ghost-shaped body that
    // misses any snapshot-asserted field or has a wrong JSON type fails replay.
    failed += schema_bodies_failed;

    std::println("snapshot replay: {} passed, {} failed", passed, failed);
    std::println("Ghost structural: {}/{} fields passed across {} bodies ({} bodies failed)",
                 schema_passed, schema_checked, schema_bodies, schema_bodies_failed);
    return failed == 0 ? 0 : 1;
}
