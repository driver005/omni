export module core_storage:storage_runtime;

import std;
import :storage_backend;
import :memory_storage_backend;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::storage::detail {
class StorageRuntime;
}

namespace core::storage::detail {

/** @brief Internal state owned by the facade. */
class StorageRuntime
{
public:
    StorageBackend& backend() noexcept
    {
        return m_backend;
    }

    [[nodiscard]] const StorageBackend& backend() const noexcept
    {
        return m_backend;
    }

private:
    MemoryStorageBackend m_backend{};
};

} // namespace core::storage::detail

#ifdef CONGELADO_TEST
namespace core::storage::detail::tests {
using namespace boost::ut;

suite<"core_storage_runtime"> core_storage_runtime_suite = []
{
    "owns a memory backend"_test = []
    {
        StorageRuntime runtime;
        expect(runtime.backend().name() == "memory");
    };
};

} // namespace core::storage::detail::tests
#endif
