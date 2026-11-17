export module core_search:builder;

import std;
import :search;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::search {

/** @brief Fluent entry point — sole public way to construct a Search handle. */
class SearchBuilder
{
public:
    SearchBuilder& option(std::string key, std::string value)
    {
        m_options.insert_or_assign(std::move(key), std::move(value));
        return *this;
    }

    [[nodiscard]] Search build() const
    {
        return Search{};
    }

private:
    std::map<std::string, std::string> m_options{};
};

} // namespace core::search

#ifdef CONGELADO_TEST
namespace core::search::tests {
using namespace boost::ut;

suite<"core_search_search_builder"> core_search_search_builder_suite = []
{
    "builds a Search handle"_test = []
    {
        const Search search = SearchBuilder{}.option("engine", "memory").build();
        expect(search.backend_name() == "memory");
    };
};

} // namespace core::search::tests
#endif
