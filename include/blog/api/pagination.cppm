export module blog_api:pagination;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::api {

/** @brief Ghost-compatible meta.pagination serializer. */
class PaginationMeta
{
public:
    [[nodiscard]] std::string json(std::size_t total, std::size_t page, std::size_t limit) const
    {
        if (limit == 0) {
            limit = 15;
        }
        const std::size_t pages = total == 0 ? 1 : (total + limit - 1) / limit;
        const std::string next = page < pages ? std::to_string(page + 1) : "null";
        const std::string prev = page > 1 ? std::to_string(page - 1) : "null";
        return std::format(
            R"({{"limit":{},"next":{},"page":{},"pages":{},"prev":{},"total":{}}})",
            limit, next, page, pages, prev, total
        );
    }
};

} // namespace blog::api

#ifdef CONGELADO_TEST
namespace blog::api::tests {
using namespace boost::ut;
suite<"blog_api_pagination"> blog_api_pagination_suite = []
{
    "matches Ghost snapshot pagination shape"_test = []
    {
        const PaginationMeta meta;
        const auto json = meta.json(11, 2, 1);
        expect(json.contains(R"("limit":1)"));
        expect(json.contains(R"("next":3)"));
        expect(json.contains(R"("page":2)"));
        expect(json.contains(R"("pages":11)"));
        expect(json.contains(R"("prev":1)"));
        expect(json.contains(R"("total":11)"));
    };
};
} // namespace blog::api::tests
#endif
