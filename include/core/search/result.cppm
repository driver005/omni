export module core_search:result;

import std;
import :hit;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::search {

/** @brief A page of search hits plus the total match count. */
class SearchResult
{
public:
    SearchResult() = default;

    SearchResult(std::vector<SearchHit> hits, std::size_t total) :
        m_hits{std::move(hits)},
        m_total{total}
    {
    }

    [[nodiscard]] const std::vector<SearchHit>& hits() const noexcept
    {
        return m_hits;
    }

    [[nodiscard]] std::size_t total() const noexcept
    {
        return m_total;
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return m_hits.empty();
    }

private:
    std::vector<SearchHit> m_hits{};
    std::size_t m_total{0};
};

} // namespace core::search

#ifdef CONGELADO_TEST
namespace core::search::tests {
using namespace boost::ut;

suite<"core_search_search_result"> core_search_search_result_suite = []
{
    "carries hits and total"_test = []
    {
        const SearchResult result{{SearchHit{"1", "post", 1.0, "s"}}, 4};
        expect(!result.empty());
        expect(result.hits().size() == 1);
        expect(result.total() == 4);
    };
};

} // namespace core::search::tests
#endif
