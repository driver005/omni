export module core_cache:cache;

import std;
import :cache_value;
import :cache_entry;
import :cache_result;
import :cache_stats;
import :ttl;
import :cache_query_spec;
import :cache_query;
import :cache_predicate;
import :cache_entity_meta;
import :key_builder;
import :entity_codec;
import :cache_runtime;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

namespace core::cache::detail {

/** @brief Keys must be non-empty; enforced on every write through the facade. */
[[nodiscard]] inline bool valid_key(std::string_view key) noexcept
{
    return !key.empty();
}

} // namespace core::cache::detail

export namespace core::cache {

/**
 * @brief The only handle a consumer ever holds. It hides the backend, the
 * connection and every cursor: callers get typed get/put/query operations and
 * nothing that can talk to storage directly. Built by CacheBuilder.
 */
class Cache
{
public:
    Cache(const Cache&) = delete;
    Cache& operator=(const Cache&) = delete;
    Cache(Cache&&) noexcept;
    Cache& operator=(Cache&&) noexcept;
    ~Cache();

    [[nodiscard]] bool is_open() const noexcept;
    [[nodiscard]] std::string_view backend_name() const noexcept;
    [[nodiscard]] CacheStats stats() const noexcept;
    [[nodiscard]] std::size_t size() const noexcept;

    [[nodiscard]] CacheResult get(std::string_view key) const;
    void put(
        std::string_view key,
        CacheValue value,
        Ttl ttl = Ttl::never(),
        std::vector<std::string> tags = {}
    );

    template<reflect::ReflectedCacheEntity E, typename Entity>
    [[nodiscard]] std::optional<E> get(const Entity& entity) const;

    template<reflect::ReflectedCacheEntity E, typename Entity>
    void put(const Entity& entity, Ttl ttl = Ttl::never());

    bool erase(std::string_view key);

    template<reflect::ReflectedCacheEntity E, typename Entity>
    bool erase(const Entity& entity);

    [[nodiscard]] std::vector<CacheEntry> execute(const CacheQuerySpec& spec) const;

    template<reflect::ReflectedCacheEntity E>
    [[nodiscard]] std::vector<E> query(const CacheQuerySpec& spec) const;

    template<reflect::ReflectedCacheEntity E>
    [[nodiscard]] CacheQuery<E> query() const;

    void clear();
    void close() noexcept;

private:
    struct Runtime;
    std::unique_ptr<Runtime> m_runtime{};

    explicit Cache(std::unique_ptr<Runtime> runtime) noexcept;

    friend class CacheBuilder;
};

} // namespace core::cache

// ─── Implementation (module purview — not reachable by importers) ────────────

namespace core::cache {

struct Cache::Runtime
{
    explicit Runtime(std::unique_ptr<detail::CacheRuntime> value) noexcept :
        impl{std::move(value)}
    {
    }

    std::unique_ptr<detail::CacheRuntime> impl{};
};

Cache::Cache(std::unique_ptr<Runtime> runtime) noexcept :
    m_runtime{std::move(runtime)}
{
}

Cache::Cache(Cache&&) noexcept = default;
Cache& Cache::operator=(Cache&&) noexcept = default;
Cache::~Cache() = default;

bool Cache::is_open() const noexcept
{
    return m_runtime && m_runtime->impl && m_runtime->impl->is_open();
}

std::string_view Cache::backend_name() const noexcept
{
    return m_runtime && m_runtime->impl ? m_runtime->impl->name() : "none";
}

CacheStats Cache::stats() const noexcept
{
    return m_runtime && m_runtime->impl ? m_runtime->impl->driver().stats() : CacheStats{};
}

std::size_t Cache::size() const noexcept
{
    return m_runtime && m_runtime->impl ? m_runtime->impl->driver().size() : 0;
}

CacheResult Cache::get(std::string_view key) const
{
    const CacheEntry entry = m_runtime->impl->driver().get(key);
    if (is_miss(entry.value)) {
        return CacheResult::miss(key);
    }
    CacheResult result = CacheResult::found(key, entry.value, entry.tags);
    result.attributes = entry.attributes;
    return result;
}

void Cache::put(std::string_view key, CacheValue value, Ttl ttl, std::vector<std::string> tags)
{
    if (!detail::valid_key(key)) {
        throw std::invalid_argument("core_cache: cache key must not be empty");
    }
    CacheEntry entry;
    entry.key = std::string{key};
    entry.value = std::move(value);
    entry.tags = std::move(tags);
    entry.expires_at = ttl.deadline_from(CacheEntry::Clock::now());
    m_runtime->impl->driver().put(entry);
}

template<reflect::ReflectedCacheEntity E, typename Entity>
std::optional<E> Cache::get(const Entity& entity) const
{
    const std::string key = reflect::KeyBuilder::build<E>(entity);
    const CacheResult result = get(key);
    if (!result.hit) {
        return std::nullopt;
    }
    CacheEntry entry;
    entry.key = key;
    entry.value = result.value;
    entry.tags = result.tags;
    entry.attributes = result.attributes;
    return reflect::EntityCodec::decode<E>(entry);
}

template<reflect::ReflectedCacheEntity E, typename Entity>
void Cache::put(const Entity& entity, Ttl ttl)
{
    CacheEntry entry = reflect::EntityCodec::encode<E>(entity);
    entry.expires_at = ttl.deadline_from(CacheEntry::Clock::now());
    m_runtime->impl->driver().put(entry);
}

bool Cache::erase(std::string_view key)
{
    return m_runtime->impl->driver().erase(key);
}

template<reflect::ReflectedCacheEntity E, typename Entity>
bool Cache::erase(const Entity& entity)
{
    return erase(reflect::KeyBuilder::build<E>(entity));
}

std::vector<CacheEntry> Cache::execute(const CacheQuerySpec& spec) const
{
    return m_runtime->impl->driver().query(spec);
}

template<reflect::ReflectedCacheEntity E>
std::vector<E> Cache::query(const CacheQuerySpec& spec) const
{
    std::vector<E> entities;
    for (const CacheEntry& entry: execute(spec)) {
        if (auto decoded = reflect::EntityCodec::decode<E>(entry)) {
            entities.push_back(*decoded);
        }
    }
    return entities;
}

template<reflect::ReflectedCacheEntity E>
CacheQuery<E> Cache::query() const
{
    return CacheQuery<E>{};
}

void Cache::clear()
{
    m_runtime->impl->driver().clear();
}

void Cache::close() noexcept
{
    if (m_runtime && m_runtime->impl) {
        m_runtime->impl->driver().close();
    }
}

} // namespace core::cache

#ifdef CONGELADO_TEST
namespace core::cache::tests {

struct CacheUser
{
    std::int64_t id{};
    std::string name{};
    bool admin{};
};
CORE_CACHE_ENTITY(CacheUser, "users",
    CORE_CACHE_FIELD("id", &CacheUser::id, FieldRole::KEY),
    CORE_CACHE_FIELD("name", &CacheUser::name, FieldRole::TAG),
    CORE_CACHE_FIELD("admin", &CacheUser::admin));

using namespace boost::ut;

suite<"core_cache_cache"> core_cache_cache_suite = []
{
    "facade rejects empty keys on write"_test = []
    {
        expect(detail::valid_key("users:v1:7"));
        expect(!detail::valid_key(""));
    };
};

} // namespace core::cache::tests
#endif
