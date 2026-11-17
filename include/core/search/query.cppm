export module core_search:query;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::search {

/** @brief A search request with a free-text term, filters and paging. */
class SearchQuery
{
public:
    SearchQuery() = default;

    explicit SearchQuery(std::string term) :
        m_term{std::move(term)}
    {
    }

    [[nodiscard]] const std::string& term() const noexcept
    {
        return m_term;
    }

    SearchQuery& filter(std::string key, std::string value)
    {
        m_filters.insert_or_assign(std::move(key), std::move(value));
        return *this;
    }

    [[nodiscard]] std::optional<std::string> filter(std::string_view key) const
    {
        const auto it = m_filters.find(std::string{key});
        return it == m_filters.end() ? std::nullopt : std::optional<std::string>{it->second};
    }

    SearchQuery& limit(std::size_t count)
    {
        m_limit = count;
        return *this;
    }

    SearchQuery& offset(std::size_t count)
    {
        m_offset = count;
        return *this;
    }

    [[nodiscard]] std::size_t limit_count() const noexcept
    {
        return m_limit;
    }

    [[nodiscard]] std::size_t offset_count() const noexcept
    {
        return m_offset;
    }

private:
    std::string m_term{};
    std::map<std::string, std::string> m_filters{};
    std::size_t m_limit{10};
    std::size_t m_offset{0};
};

} // namespace core::search

#ifdef CONGELADO_TEST
namespace core::search::tests {
using namespace boost::ut;

suite<"core_search_search_query"> core_search_search_query_suite = []
{
    "carries term, filters and paging"_test = []
    {
        SearchQuery query{"ghost"};
        query.filter("type", "post").limit(5).offset(2);
        expect(query.term() == "ghost");
        expect(query.filter("type") == std::string{"post"});
        expect(query.limit_count() == 5);
        expect(query.offset_count() == 2);
    };
};

} // namespace core::search::tests
#endif
