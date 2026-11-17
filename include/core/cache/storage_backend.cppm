export module core_cache:storage_backend;

import std;
import :entry;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::cache {

/**
 * @brief Backend contract the facade talks to. Purely internal — concrete
 * backends and this interface never leave the module.
 */
class CacheStorageBackend
{
public:
    virtual ~CacheStorageBackend() = default;

    [[nodiscard]] virtual std::string_view name() const noexcept = 0;
    [[nodiscard]] virtual bool is_open() const noexcept = 0;

    virtual void open() = 0;
    virtual void close() = 0;

    [[nodiscard]] virtual CacheEntry get(std::string_view key) const = 0;
    virtual void put(const CacheEntry& entry) = 0;
    virtual bool erase(std::string_view key) = 0;
    virtual void clear() = 0;
    [[nodiscard]] virtual std::vector<CacheEntry> entries() const = 0;
    [[nodiscard]] virtual std::size_t size() const noexcept = 0;
};

} // namespace core::cache

#ifdef CONGELADO_TEST
namespace core::cache::tests {
using namespace boost::ut;

suite<"core_cache_storage_backend"> core_cache_storage_backend_suite = []
{
    "backend contract is abstract"_test = []
    {
        expect(std::is_abstract_v<CacheStorageBackend>);
    };
};

} // namespace core::cache::tests
#endif
