module;
export module app_http:query;

import std;
import interfaces;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::http {

/** @brief Query-string parameters with basic percent decoding; later duplicates win. */
class QueryParams
{
public:
    QueryParams() = default;

    explicit QueryParams(std::string_view query)
    {
        parse(query);
    }

    [[nodiscard]] std::optional<std::string> get(std::string_view key) const
    {
        const auto it{m_params.find(std::string{key})};
        if (it == m_params.end()) {
            return std::nullopt;
        }
        return it->second;
    }

private:
    void parse(std::string_view query)
    {
        std::size_t start{0};
        while (start < query.size()) {
            const std::size_t amp{query.find('&', start)};
            const std::size_t end{amp == std::string_view::npos ? query.size() : amp};
            const std::string_view pair{query.substr(start, end - start)};
            if (!pair.empty()) {
                const std::size_t eq{pair.find('=')};
                const std::string_view raw_key{
                    eq == std::string_view::npos ? pair : pair.substr(0, eq)
                };
                const std::string_view raw_value{
                    eq == std::string_view::npos ? std::string_view{} : pair.substr(eq + 1)
                };
                m_params.insert_or_assign(decode(raw_key), decode(raw_value));
            }
            if (amp == std::string_view::npos) {
                break;
            }
            start = amp + 1;
        }
    }

    [[nodiscard]] std::string decode(std::string_view value) const
    {
        std::string out{};
        out.reserve(value.size());
        for (std::size_t i{0}; i < value.size(); ++i) {
            const char ch{value[i]};
            if (ch == '+') {
                out.push_back(' ');
                continue;
            }
            if (ch == '%' && i + 2 < value.size()) {
                const int high{hex_digit(value[i + 1])};
                const int low{hex_digit(value[i + 2])};
                if (high >= 0 && low >= 0) {
                    out.push_back(static_cast<char>((high << 4) | low));
                    i += 2;
                    continue;
                }
            }
            out.push_back(ch);
        }
        return out;
    }

    [[nodiscard]] int hex_digit(char ch) const noexcept
    {
        if (ch >= '0' && ch <= '9') {
            return ch - '0';
        }
        if (ch >= 'a' && ch <= 'f') {
            return ch - 'a' + 10;
        }
        if (ch >= 'A' && ch <= 'F') {
            return ch - 'A' + 10;
        }
        return -1;
    }

    std::unordered_map<std::string, std::string> m_params{};
};

/** @brief Instance functor parsing the query string following '?' in a request path. */
class RequestQuery
{
public:
    [[nodiscard]] QueryParams operator()(interfaces::io::IRequest& request) const
    {
        const std::string_view path{request.get_path()};
        const std::size_t query{path.find('?')};
        return QueryParams{
            query == std::string_view::npos ? std::string_view{} : path.substr(query + 1)
        };
    }
};

} // namespace app::http

#ifdef CONGELADO_TEST
namespace app::http::tests {
using namespace boost::ut;

suite<"app_http_query"> query_suite = []
{
    "keys decode plus and percent escapes"_test = []
    {
        const QueryParams params{"limit=10&q=hello+world&id=%41%2B%20x&bad=%ZZ"};

        expect(params.get("limit").value() == "10");
        expect(params.get("q").value() == "hello world");
        expect(params.get("id").value() == "A+ x");
        expect(params.get("bad").value() == "%ZZ");
        expect(!params.get("missing").has_value());
    };

    "later duplicate values win and bare keys map to empty strings"_test = []
    {
        const QueryParams params{"a=1&a=2&flag"};
        expect(params.get("a").value() == "2");
        expect(params.get("flag").value().empty());
    };

    "request query reads only the text after the question mark"_test = []
    {
        struct FakeRequest : interfaces::io::IRequest
        {
            explicit FakeRequest(std::string value) :
                interfaces::io::IRequest{0},
                m_path{std::move(value)}
            {
            }

            [[nodiscard]] std::string_view get_path() const noexcept override
            {
                return m_path;
            }

            std::string m_path{};
        };

        FakeRequest with_query{"/payments?limit=5&sort=desc"};
        FakeRequest without_query{"/payments"};
        const QueryParams parsed{RequestQuery{}(with_query)};
        const QueryParams empty{RequestQuery{}(without_query)};

        expect(parsed.get("limit").value() == "5");
        expect(parsed.get("sort").value() == "desc");
        expect(!empty.get("limit").has_value());
    };
};

} // namespace app::http::tests
#endif
