export module core_search:runtime;

import std;
import :backend;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::search {
class SearchRuntime;
}

namespace core::search {

/** @brief Internal state owned by the facade. */
class SearchRuntime
{
public:
    explicit SearchRuntime(std::unique_ptr<SearchBackend> backend) :
        m_backend{std::move(backend)}
    {
    }

    SearchBackend& backend() noexcept
    {
        return *m_backend;
    }

    [[nodiscard]] const SearchBackend& backend() const noexcept
    {
        return *m_backend;
    }

private:
    std::unique_ptr<SearchBackend> m_backend{};
};

} // namespace core::search

#ifdef CONGELADO_TEST
namespace core::search::tests {
using namespace boost::ut;

suite<"core_search_runtime"> core_search_runtime_suite = []
{
    "owns a memory backend"_test = []
    {
        SearchRuntime runtime;
        expect(runtime.backend().name() == "memory");
    };
};

} // namespace core::search::tests
#endif
