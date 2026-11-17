export module core_search:hit;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::search {

/** @brief One search result with an opaque id, type and relevance score. */
class SearchHit
{
public:
    SearchHit() = default;

    SearchHit(std::string id, std::string type, double score, std::string snippet) :
        m_id{std::move(id)},
        m_type{std::move(type)},
        m_score{score},
        m_snippet{std::move(snippet)}
    {
    }

    [[nodiscard]] const std::string& id() const noexcept
    {
        return m_id;
    }

    [[nodiscard]] const std::string& type() const noexcept
    {
        return m_type;
    }

    [[nodiscard]] double score() const noexcept
    {
        return m_score;
    }

    [[nodiscard]] const std::string& snippet() const noexcept
    {
        return m_snippet;
    }

private:
    std::string m_id{};
    std::string m_type{};
    double m_score{0.0};
    std::string m_snippet{};
};

} // namespace core::search

#ifdef CONGELADO_TEST
namespace core::search::tests {
using namespace boost::ut;

suite<"core_search_search_hit"> core_search_search_hit_suite = []
{
    "carries hit metadata"_test = []
    {
        const SearchHit hit{"1", "post", 0.9, "snippet"};
        expect(hit.id() == "1");
        expect(hit.type() == "post");
        expect(hit.score() == 0.9);
        expect(hit.snippet() == "snippet");
    };
};

} // namespace core::search::tests
#endif
