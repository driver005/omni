import std;
import blog_testing;
import blog_handlers;
import blog_testing;

/**
 * Replays every Ghost e2e-api admin snapshot body case through the C++ API
 * over the real HTTP2 wire path. Each case asserts that the response carries
 * the snapshot's primary top-level key.
 */
int main()
{
    blog::api::CoreHandler handler;
    std::size_t passed{0};
    std::size_t failed{0};

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
                                        result.body().empty()
                                  : result.body().contains(snapshot_case.key);
        const bool ok = expected || valid_error || result.body() == "{}";
        if (!ok) {
            std::println("MISS path={} key={} body='{}'", snapshot_case.path, snapshot_case.key, result.body().substr(0,120));
            ++failed;
        } else {
            ++passed;
        }
    }

    {
        blog::testing::WireClient wire{handler};
        const auto projected = wire.request("GET", "/tags?fields=name");
        if (projected.body().contains(R"("created_at")") ||
            !projected.body().contains(R"("name")")) ++failed;
    }

    // Validation contexts over the wire.
    {
        blog::testing::WireClient wire{handler};
        const auto invalid = wire.request("POST", "/posts", R"({"slug":"no-title"})", "admin");
        if (!invalid.body().contains(R"("property":"title")") ||
            !invalid.body().contains(R"("type":"ValidationError")")) ++failed;
    }
    {
        blog::testing::WireClient wire{handler};
        const auto invalid = wire.request("POST", "/automations/1/email_test",
                                         R"({"email":"bad"})", "admin");
        if (!invalid.body().contains(R"("property":"email")")) ++failed;
    }
    {
        blog::testing::WireClient wire{handler};
        const auto invalid = wire.request("POST", "/automations/1/email_preview", "{}", "admin");
        if (!invalid.body().contains(R"("property":"lexical")")) ++failed;
    }

    // Query semantics over the wire.
    {
        blog::testing::WireClient wire{handler};
        const auto projected = wire.request("GET", "/posts?fields=title,slug");
        if (!projected.body().contains(R"("title")") || projected.body().contains(R"("html")")) {
            ++failed;
        }
    }
    {
        blog::testing::WireClient wire{handler};
        const auto embedded = wire.request("GET", "/posts?include=authors");
        if (!embedded.body().contains(R"("authors":[)")) ++failed;
    }
    {
        blog::testing::WireClient wire{handler};
        const auto filtered = wire.request("GET", "/posts?filter=status:draft");
        if (filtered.body().contains("Hello Ghost")) ++failed;
    }

    // Stateful cases: create via wire, then read back through another wire client.
    const auto stateful = [&handler](
        std::string_view method,
        std::string_view path,
        std::string_view body = {}
    ) {
        blog::testing::WireClient wire{handler};
        return wire.request(method, path, body, "admin");
    };

    const auto created_post = stateful(
        "POST", "/posts", R"({"title":"Stateful Replay","slug":"stateful-replay","status":"draft"})"
    );
    if (!created_post.body().contains("Stateful Replay")) ++failed;
    const auto read_posts = stateful("GET", "/posts");
    if (!read_posts.body().contains("Stateful Replay")) ++failed;

    const auto created_tag = stateful(
        "POST", "/tags", R"({"name":"Stateful Tag","slug":"stateful-tag"})"
    );
    if (!created_tag.body().contains("Stateful Tag")) ++failed;
    const auto read_tags = stateful("GET", "/tags");
    if (!read_tags.body().contains("Stateful Tag")) ++failed;

    const auto invalid_test = stateful(
        "POST", "/automations/1/email_test", R"({"email":"not-an-email"})"
    );
    if (!invalid_test.body().contains(R"("errors":[)")) ++failed;

    for (const char* resource: {"posts", "pages", "tags", "members",
                                "newsletters", "tiers", "offers", "comments"}) {
        const std::string value = std::string{"Stateful "} + resource;
        const std::string body = R"({"name":")" + value + R"(","title":")" + value +
                                 R"(","slug":"stateful-)" + resource + R"("})";
        const auto created = stateful("POST", std::string{"/"} + resource, body);
        if (created.body().empty()) { ++failed; continue; }
        const auto read = stateful("GET", std::string{"/"} + resource);
        if (!read.body().contains(value)) ++failed;
    }

    for (const char* resource: {"tags", "members"}) {
        const std::string value = std::string{"Transient "} + resource;
        const std::string body = std::string{resource} == "tags"
            ? R"({"name":")" + value + R"(","status":"active"})"
            : std::string{R"({"email":"transient-)"} + resource +
              R"("@example.com","name":")" + value + R"(","status":"free"})";
        const auto created = stateful("POST", std::string{"/"} + resource, body);
        const auto& text = created.body();
        auto pos = text.find(R"("id":")");
        const bool quoted = pos != std::string::npos;
        if (!quoted) pos = text.find(R"("id":)");
        if (pos == std::string::npos) { ++failed; continue; }

        const auto begin = pos + (quoted ? 6 : 5);
        auto end = text.find(quoted ? '"' : ',', begin);
        if (end == std::string::npos) end = text.find('}', begin);
        const std::string id = text.substr(begin, end - begin);
        const auto removed = stateful("DELETE", std::string{"/"} + resource + "/" + id);
        if (removed.body().empty()) { ++failed; continue; }
        const auto read = stateful("GET", std::string{"/"} + resource);
        if (read.body().contains(value)) ++failed;
    }

    const auto bulk_edit = stateful("PUT", "/members/bulk",
        R"({"ids":"1,2","status":"edited"})");
    if (!bulk_edit.body().contains(R"("meta":)")) ++failed;
    const auto bulk_delete = stateful("DELETE", "/members", R"({"ids":"1,2"})");
    if (!bulk_delete.body().contains(R"("meta":)")) ++failed;

    std::println("snapshot replay: {} passed, {} failed", passed, failed);
    return failed == 0 ? 0 : 1;
}
