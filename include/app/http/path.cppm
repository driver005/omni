module;
export module app_http:path;

import std;
import interfaces;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::http {

/** @brief Path split into non-empty segments between '/' separators. */
class PathSegments
{
public:
    explicit PathSegments(std::string_view path) :
        m_path{path}
    {
        std::size_t start{0};
        while (start < m_path.size()) {
            const std::size_t slash{m_path.find('/', start)};
            const std::size_t end{slash == std::string_view::npos ? m_path.size() : slash};
            if (end > start) {
                m_parts.emplace_back(start, end - start);
            }
            if (slash == std::string_view::npos) {
                break;
            }
            start = slash + 1;
        }
    }

    [[nodiscard]] std::size_t count() const noexcept
    {
        return m_parts.size();
    }

    [[nodiscard]] std::optional<std::string_view> at(std::size_t index) const noexcept
    {
        if (index >= m_parts.size()) {
            return std::nullopt;
        }
        const auto [start, length]{m_parts[index]};
        return std::string_view{m_path}.substr(start, length);
    }

    /** @brief Segments after the first one, or nullopt when there is no second segment. */
    [[nodiscard]] std::optional<std::string_view> tail() const noexcept
    {
        if (m_parts.size() < 2) {
            return std::nullopt;
        }
        const std::size_t start{m_parts[1].first};
        const auto [last_start, last_length]{m_parts.back()};
        return std::string_view{m_path}.substr(start, last_start + last_length - start);
    }

private:
    std::string m_path{};
    std::vector<std::pair<std::size_t, std::size_t>> m_parts{};
};

/** @brief Instance functor extracting path segments from a request, query stripped. */
class RequestPath
{
public:
    [[nodiscard]] PathSegments operator()(interfaces::io::IRequest& request) const
    {
        const std::string_view path{request.get_path()};
        const std::size_t query{path.find('?')};
        return PathSegments{query == std::string_view::npos ? path : path.substr(0, query)};
    }
};

} // namespace app::http

#ifdef CONGELADO_TEST
namespace app::http::tests {
using namespace boost::ut;

suite<"app_http_path"> path_suite = []
{
    "segments split on slashes and skip empty ones"_test = []
    {
        const PathSegments segments{"/payments/pay_1/refunds"};
        expect(segments.count() == 3u);
        expect(segments.at(0).value() == "payments");
        expect(segments.at(1).value() == "pay_1");
        expect(segments.at(2).value() == "refunds");
        expect(!segments.at(3).has_value());
    };

    "tail returns everything after the first segment"_test = []
    {
        const PathSegments segments{"/payments/pay_1/refunds/"};
        expect(segments.count() == 3u);
        expect(segments.tail().value() == "pay_1/refunds");

        const PathSegments single{"/payments"};
        expect(!single.tail().has_value());
        const PathSegments empty{""};
        expect(!empty.tail().has_value());
    };

    "request path strips the query string"_test = []
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

        FakeRequest request{"/payments/pay_1?limit=10&offset=0"};
        const PathSegments segments{RequestPath{}(request)};

        expect(segments.count() == 2u);
        expect(segments.at(0).value() == "payments");
        expect(segments.at(1).value() == "pay_1");
    };
};

} // namespace app::http::tests
#endif
