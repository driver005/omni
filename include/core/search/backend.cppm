export module core_search:backend;

import std;
import :query;
import :result;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::search {

/** @brief Internal search backend contract; concrete engines are plugins. */
class SearchBackend
{
public:
    virtual ~SearchBackend() = default;

    [[nodiscard]] virtual std::string_view name() const noexcept = 0;
    virtual void index(std::string id, std::string type, std::string document) = 0;
    virtual bool remove(std::string_view id) = 0;
    [[nodiscard]] virtual SearchResult search(const SearchQuery& query) const = 0;
    [[nodiscard]] virtual std::size_t size() const noexcept = 0;
};

} // namespace core::search

#ifdef CONGELADO_TEST
namespace core::search::tests {
using namespace boost::ut;

suite<"core_search_search_backend"> core_search_search_backend_suite = []
{
    "backend contract is abstract"_test = []
    {
        expect(std::is_abstract_v<SearchBackend>);
    };
};

} // namespace core::search::tests
#endif
