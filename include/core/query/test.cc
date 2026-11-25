import std;
import core_query;

using namespace core::query;

namespace {

class Check
{
public:
    void yes(bool condition, std::string_view label)
    {
        if (!condition) {
            std::println(std::cerr, "FAIL {}", label);
            ++m_failures;
        }
    }

    void eq(std::string_view actual, std::string_view expected, std::string_view label)
    {
        if (actual != expected) {
            std::println(std::cerr, "FAIL {}: expected '{}', got '{}'", label, expected, actual);
            ++m_failures;
        }
    }

    int failures() const
    {
        return m_failures;
    }

private:
    int m_failures{0};
};

constexpr auto kAll = std::numeric_limits<std::size_t>::max();

constexpr std::array<std::string_view, 7>
    kAllow{"filter", "fields", "include", "order", "limit", "page", "offset"};

constexpr std::string_view kRows = R"([
{"id":"1","title":"Hello Ghost","status":"published","featured":true,"monthly_price":0,"created_at":"2024-01-01"},
{"id":"2","title":"Second Post","status":"draft","featured":false,"monthly_price":5,"created_at":"2024-02-01"},
{"id":"3","title":"Third Ghost","status":"published","featured":true,"monthly_price":10,"created_at":"2024-03-01"}
])";

std::size_t count(std::string_view haystack, std::string_view needle)
{
    std::size_t n = 0;
    for (std::size_t pos = 0; (pos = haystack.find(needle, pos)) != std::string_view::npos; ++pos) {
        ++n;
    }
    return n;
}

} // namespace

int main()
{
    Check check;

    // --- parse: bracket filters, projection, embedding, paging, allowlist ---
    {
        const auto q = parse(
            "?filter[status]=published&fields=id,title&include=author,tags"
            "&order=created_at%20desc&limit=all&page=2&bogus=ignored",
            kAllow
        );
        check.eq(q.options.at("limit"), "all", "option limit preserved");
        check.yes(!q.options.contains("bogus"), "non-allowlisted option dropped");
        check.yes(
            q.filters.size() == 1 && q.filters[0].first == "status" &&
                q.filters[0].second == "published",
            "bracket filter"
        );
        check.yes(q.fields.contains("id") && q.fields.contains("title"), "fields parsed");
        check.yes(q.include.contains("author") && q.include.contains("tags"), "include parsed");
        check.eq(q.order, "created_at desc", "order decoded");
        check.yes(q.limit == std::numeric_limits<std::size_t>::max(), "limit=all");
        check.yes(q.page == 2, "page parsed");
    }

    // --- parse: bare NQL keeps `+` as AND and `,` as OR ---
    {
        const auto q = parse("?filter=status:draft,status:published+featured:true", kAllow);
        check.eq(q.raw_filter, "status:draft,status:published+featured:true", "raw filter");
        check.yes(q.options.contains("filter"), "filter option recorded");
    }

    // --- restricted-field guard ---
    check.yes(has_restricted_field("password:foo"), "password blocked");
    check.yes(has_restricted_field("user.token:x"), "nested token blocked");
    check.yes(has_restricted_field("gift_link_token:abc"), "gift_link_token blocked");
    check.yes(has_restricted_field("secret desc"), "order field blocked");
    check.yes(!has_restricted_field("status:secret"), "value not treated as field");
    check.yes(!has_restricted_field("status:draft"), "ordinary field allowed");

    // --- valid_option ---
    check.yes(valid_option("limit", "all"), "limit all valid");
    check.yes(valid_option("limit", "25"), "limit number valid");
    check.yes(!valid_option("limit", "abc"), "limit junk invalid");
    check.yes(valid_option("order", "created_at desc"), "order valid");
    check.yes(!valid_option("order", "created_at sideways"), "order direction invalid");
    check.yes(valid_option("fields", "id,title"), "fields valid");
    check.yes(!valid_option("fields", "id;title"), "fields invalid");
    check.yes(valid_option("filter", "status:[draft,published]"), "bracket filter valid");

    // --- apply: bracket filter = AND; OR groups; include preserved ---
    {
        ParsedQuery q;
        q.filters.emplace_back("status", "published");
        q.limit = kAll;
        const auto out = apply(kRows, q);
        check.yes(count(out, R"("id":)") == 2, "filter[x]=y matches two rows");
        check.yes(out.find("Second Post") == std::string::npos, "draft row removed");
    }
    {
        ParsedQuery q;
        q.raw_filter = "status:draft,status:published+featured:true";
        q.limit = kAll;
        const auto out = apply(kRows, q);
        check.yes(count(out, R"("id":)") == 3, "OR + AND yields all three rows");
    }
    {
        ParsedQuery q;
        q.raw_filter = "status:[draft,published]";
        q.limit = kAll;
        check.yes(count(apply(kRows, q), R"("id":)") == 3, "[a,b] IN filter");
    }
    {
        ParsedQuery q;
        q.raw_filter = "title:~ghost";
        q.limit = kAll;
        const auto out = apply(kRows, q);
        check.yes(count(out, R"("id":)") == 2, "contains operator");
        check.yes(out.find("Second Post") == std::string::npos, "non-match removed");
    }
    {
        ParsedQuery q;
        q.raw_filter = "status:-draft";
        q.limit = kAll;
        const auto out = apply(kRows, q);
        check.yes(out.find("Second Post") == std::string::npos, "negation operator");
        check.yes(count(out, R"("id":)") == 2, "negation keeps two");
    }
    {
        ParsedQuery q;
        q.raw_filter = "monthly_price:>0";
        q.limit = kAll;
        const auto out = apply(kRows, q);
        check.yes(out.find(R"("id":"1")") == std::string::npos, "numeric greater excludes zero");
        check.yes(count(out, R"("id":)") == 2, "numeric greater keeps two");
    }
    {
        ParsedQuery q;
        q.raw_filter = "missing:null";
        q.limit = kAll;
        check.yes(count(apply(kRows, q), R"("id":)") == 3, "null matches missing fields");
    }
    {
        ParsedQuery q;
        q.raw_filter = R"(title:'Hello Ghost')";
        q.limit = kAll;
        check.yes(count(apply(kRows, q), R"("id":)") == 1, "quoted value with space");
    }

    // --- fields projection, include preservation, order, paging ---
    {
        ParsedQuery q;
        q.limit = kAll;
        q.fields.insert("id");
        q.fields.insert("title");
        q.include.insert("authors");
        const auto out = apply(kRows, q);
        check.yes(out.find(R"("html":)") == std::string::npos, "projection drops keys");
        check.yes(
            out.find(R"("title":"Hello Ghost")") != std::string::npos,
            "projection keeps key"
        );
        check.yes(q.include.contains("authors"), "include untouched by apply");
    }
    {
        ParsedQuery q;
        q.limit = kAll;
        q.order = "created_at desc";
        const auto out = apply(kRows, q);
        check.yes(out.find(R"("id":"3")") < out.find(R"("id":"1")"), "descending order");
    }
    {
        ParsedQuery q;
        q.limit = 2;
        q.page = 2;
        q.order = "created_at asc";
        const auto out = apply(kRows, q);
        check.yes(count(out, R"("id":)") == 1, "page 2 of limit 2 has one row");
        check.yes(out.find(R"("id":"3")") != std::string::npos, "page 2 last row");
    }
    {
        ParsedQuery q;
        q.limit = 2;
        q.offset = 1;
        const auto out = apply(kRows, q);
        check.yes(count(out, R"("id":)") == 2, "offset honored with limit");
        check.yes(out.find(R"("id":"1")") == std::string::npos, "offset skips first row");
    }

    // --- pagination meta + run envelope ---
    {
        ParsedQuery q;
        q.limit = 2;
        q.page = 1;
        check.eq(
            pagination_meta(5, q),
            R"({"page":1,"limit":2,"pages":3,"total":5,"next":2,"prev":null})",
            "meta page 1"
        );
        q.page = 3;
        check.eq(
            pagination_meta(5, q),
            R"({"page":3,"limit":2,"pages":3,"total":5,"next":null,"prev":2})",
            "meta last page"
        );
        q.limit = kAll;
        q.page = 1;
        check.eq(
            pagination_meta(5, q),
            R"({"page":1,"limit":5,"pages":1,"total":5,"next":null,"prev":null})",
            "meta all"
        );
    }
    {
        ParsedQuery q;
        q.limit = 2;
        const auto out = run(kRows, q);
        check.yes(out.starts_with(R"({"data":[)"), "run data key");
        check.yes(out.contains(R"("meta":{"pagination":)"), "run meta key");
        check.yes(out.contains(R"("total":3)"), "run total");
    }

    // --- restricted field guard throws before filtering ---
    {
        bool threw = false;
        try {
            ParsedQuery q;
            q.raw_filter = "password:foo";
            (void)apply(kRows, q);
        } catch (const std::runtime_error&) {
            threw = true;
        }
        check.yes(threw, "restricted field throws");
    }

    if (check.failures() == 0) {
        std::println("core_query tests passed");
        return 0;
    }
    std::println("{} core_query checks failed", check.failures());
    return 1;
}
