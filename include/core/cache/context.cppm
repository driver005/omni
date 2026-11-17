export module core_cache:context;

import std;
import :engine;
import :eviction_policy;
import :runtime;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

namespace core::cache {

/** @brief Accumulates builder configuration and compiles it into a runtime. */
class CacheContext
{
public:
    void set_engine(CacheEngine engine) noexcept
    {
        m_engine = engine;
    }

    void set_capacity(std::size_t capacity) noexcept
    {
        m_capacity = capacity;
    }

    void set_policy(EvictionPolicy policy) noexcept
    {
        m_policy = policy;
    }

    void use_backend(std::unique_ptr<CacheStorageBackend> backend)
    {
        m_backend = std::move(backend);
    }

    void set_option(std::string key, std::string value)
    {
        m_options.insert_or_assign(std::move(key), std::move(value));
    }

    [[nodiscard]] std::unique_ptr<CacheRuntime> build()
    {
        return std::make_unique<CacheRuntime>(m_engine, m_capacity, m_policy);
    }

    [[nodiscard]] CacheEngine engine() const noexcept
    {
        return m_engine;
    }

    [[nodiscard]] std::size_t capacity() const noexcept
    {
        return m_capacity;
    }

    [[nodiscard]] EvictionPolicy policy() const noexcept
    {
        return m_policy;
    }

    [[nodiscard]] const std::map<std::string, std::string>& options() const noexcept
    {
        return m_options;
    }

private:
    std::unique_ptr<CacheStorageBackend> m_backend{};
    CacheEngine m_engine{CacheEngine::LOCAL};
    std::size_t m_capacity{1024};
    EvictionPolicy m_policy{EvictionPolicy::LRU};
    std::map<std::string, std::string> m_options{};
};

} // namespace core::cache

#ifdef CONGELADO_TEST
namespace core::cache::tests {
using namespace boost::ut;

suite<"core_cache_context"> core_cache_context_suite = []
{
    "accumulates config and builds a runtime"_test = []
    {
        CacheContext context;
        context.set_engine(CacheEngine::LOCAL);
        context.set_capacity(32);
        context.set_policy(EvictionPolicy::LFU);
        expect(context.engine() == CacheEngine::LOCAL);
        expect(context.capacity() == 32);
        expect(context.policy() == EvictionPolicy::LFU);

        auto runtime = context.build();
        expect(runtime->is_open());
        expect(runtime->name() == "local");
    };
};

} // namespace core::cache::tests
#endif
