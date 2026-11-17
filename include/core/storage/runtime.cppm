export module core_storage:runtime;

import std;
import :backend;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::storage {
class StorageRuntime;
}

namespace core::storage {

/** @brief Internal state owned by the facade. */
class StorageRuntime
{
public:
    explicit StorageRuntime(std::unique_ptr<StorageBackend> backend) :
        m_backend{std::move(backend)}
    {
    }

    StorageBackend& backend() noexcept
    {
        return *m_backend;
    }

    [[nodiscard]] const StorageBackend& backend() const noexcept
    {
        return *m_backend;
    }

private:
    std::unique_ptr<StorageBackend> m_backend{};
};

} // namespace core::storage

#ifdef CONGELADO_TEST
namespace core::storage::tests {
using namespace boost::ut;

suite<"core_storage_runtime"> core_storage_runtime_suite = []
{
    "owns a memory backend"_test = []
    {
        StorageRuntime runtime;
        expect(runtime.backend().name() == "memory");
    };
};

} // namespace core::storage::tests
#endif
