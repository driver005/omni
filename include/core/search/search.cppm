export module core_search:search;

import std;
import :backend;
import :query;
import :result;
import :runtime;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::search {

/** @brief Opaque search handle. All members inline, shared_ptr pImpl. */
class Search
{
public:
    Search() :
        m_runtime{std::make_shared<SearchRuntime>(nullptr)}
    {
    }

    void use_backend(std::unique_ptr<SearchBackend> backend)
    {
        m_runtime = std::make_shared<SearchRuntime>(std::move(backend));
    }

    Search(const Search&) = delete;
    Search& operator=(const Search&) = delete;
    Search(Search&&) noexcept = default;
    Search& operator=(Search&&) noexcept = default;
    ~Search() = default;

    void index(std::string id, std::string type, std::string document)
    {
        m_runtime->backend().index(std::move(id), std::move(type), std::move(document));
    }

    bool remove(std::string_view id)
    {
        return m_runtime->backend().remove(id);
    }

    [[nodiscard]] SearchResult search(const SearchQuery& query) const
    {
        return m_runtime->backend().search(query);
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return m_runtime->backend().size();
    }

    [[nodiscard]] std::string_view backend_name() const noexcept
    {
        return m_runtime->backend().name();
    }

private:
    std::shared_ptr<SearchRuntime> m_runtime{};
};

} // namespace core::search

#ifdef CONGELADO_TEST
namespace core::search::tests {
using namespace boost::ut;

suite<"core_search_search"> core_search_search_suite = []
{
    "facade indexes and searches"_test = []
    {
        Search search;
        search.index("1", "post", "ghost post content");
        expect(search.size() == 1);
        const auto result = search.search(SearchQuery{"ghost"});
        expect(result.total() == 1);
        expect(search.backend_name() == "memory");
    };
};

} // namespace core::search::tests
#endif
