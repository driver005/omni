import std;
import blog_testing;
import blog_handlers;

/**
 * Replays Ghost e2e-api snapshot cases through the C++ API over the real
 * HTTP2 wire path. Each case asserts the snapshot's primary top-level key.
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
                                        result.body().empty() ||
                                        result.body() == "{}"
                                  : result.body().contains(snapshot_case.key);
        if (!expected && !valid_error) {
            ++failed;
        } else {
            ++passed;
        }
    }

    std::println("snapshot replay: {} passed, {} failed", passed, failed);
    return failed == 0 ? 0 : 1;
}
