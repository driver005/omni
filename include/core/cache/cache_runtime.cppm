export module core_cache:cache_runtime;

import std;
import :cache_engine;
import :cache_driver;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

namespace core::cache::detail {

/** @brief Internal state owned by the facade: the single cache driver. */
class CacheRuntime
{
public:
    CacheRuntime(CacheEngine engine, std::size_t capacity, EvictionPolicy policy) :
        m_driver{engine, capacity, policy}
    {
        m_driver.open();
    }

    CacheRuntime(const CacheRuntime&) = delete;
    CacheRuntime& operator=(const CacheRuntime&) = delete;
    CacheRuntime(CacheRuntime&&) noexcept = default;
    CacheRuntime& operator=(CacheRuntime&&) noexcept = default;

    [[nodiscard]] bool is_open() const noexcept
    {
        return m_driver.is_open();
    }

    [[nodiscard]] std::string_view name() const noexcept
    {
        return m_driver.name();
    }

    [[nodiscard]] CacheDriver& driver() noexcept
    {
        return m_driver;
    }

private:
    CacheDriver m_driver;
};

} // namespace core::cache::detail

#ifdef CONGELADO_TEST
namespace core::cache::detail::tests {
using namespace boost::ut;

suite<"core_cache_runtime"> core_cache_runtime_suite = []
{
    "owns an open driver"_test = []
    {
        CacheRuntime runtime{CacheEngine::LOCAL, 8, EvictionPolicy::LRU};
        expect(runtime.is_open());
        expect(runtime.name() == "local");
    };
};

} // namespace core::cache::detail::tests
#endif
