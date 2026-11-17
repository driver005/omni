export module core_cache:cache_builder;

import std;
import :cache_engine;
import :eviction_policy;
import :cache_value;
import :ttl;
import :cache_compare;
import :cache_context;
import :cache;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::cache {

/**
 * @brief Fluent entry point. Selects a backend, sizing, policy and options,
 * then builds an opaque Cache handle. No backend class is exposed.
 */
class CacheBuilder
{
public:
    CacheBuilder& engine(CacheEngine engine)
    {
        m_context.set_engine(engine);
        return *this;
    }

    CacheBuilder& capacity(std::size_t count)
    {
        m_context.set_capacity(count);
        return *this;
    }

    CacheBuilder& policy(EvictionPolicy policy)
    {
        m_context.set_policy(policy);
        return *this;
    }

    CacheBuilder& option(std::string key, std::string value)
    {
        m_context.set_option(std::move(key), std::move(value));
        return *this;
    }

    [[nodiscard]] Cache build() const
    {
        auto runtime = std::make_unique<Cache::Runtime>(m_context.build());
        return Cache{std::move(runtime)};
    }

private:
    detail::CacheContext m_context{};
};

} // namespace core::cache

#ifdef CONGELADO_TEST
namespace core::cache::tests {
using namespace boost::ut;

suite<"core_cache_cache_builder"> core_cache_cache_builder_suite = []
{
    "builds an open handle and performs typed CRUD through the facade"_test = []
    {
        CacheBuilder builder;
        Cache cache = builder.engine(CacheEngine::LOCAL)
                          .capacity(64)
                          .policy(EvictionPolicy::LRU)
                          .option("namespace", "test")
                          .build();
        expect(cache.is_open());
        expect(cache.backend_name() == "local");
        expect(cache.size() == 0);

        const CacheUser user{7, "Ada", true};
        cache.put<CacheUser>(user, Ttl::seconds(60));
        const auto restored = cache.get<CacheUser>(user);
        expect(restored.has_value());
        expect(restored->id == 7);
        expect(restored->name == "Ada");
        expect(restored->admin);

        cache.put("raw:key", to_cache_value(std::int64_t{42}));
        expect(value_as<std::int64_t>(cache.get("raw:key").value) == 42);
        expect(cache.size() == 2);
        expect(cache.stats().writes == 2);

        auto query = cache.query<CacheUser>();
        query.where<&CacheUser::id>(CacheCompare::GE, "1");
        const auto found = cache.query<CacheUser>(query.spec());
        expect(found.size() == 1);

        expect(cache.erase<CacheUser>(user));
        expect(cache.get<CacheUser>(user) == std::nullopt);
        cache.clear();
        expect(cache.size() == 0);
    };
};

} // namespace core::cache::tests
#endif
