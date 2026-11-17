export module core_storage:storage;

import std;
import :backend;
import :ref;
import :result;
import :runtime;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::storage {

/**
 * @brief Opaque storage handle. All members inline, shared_ptr pImpl,
 * no free/static functions. Built by StorageBuilder.
 */
class Storage
{
public:
    Storage() :
        m_runtime{std::make_shared<StorageRuntime>(nullptr)}
    {
    }

    void use_backend(std::unique_ptr<StorageBackend> backend)
    {
        m_runtime = std::make_shared<StorageRuntime>(std::move(backend));
    }

    Storage(const Storage&) = delete;
    Storage& operator=(const Storage&) = delete;
    Storage(Storage&&) noexcept = default;
    Storage& operator=(Storage&&) noexcept = default;
    ~Storage() = default;

    [[nodiscard]] StorageResult write(std::string_view payload)
    {
        return m_runtime->backend().write(payload);
    }

    [[nodiscard]] StorageResult read(std::string_view uri) const
    {
        return m_runtime->backend().read(uri);
    }

    bool remove(std::string_view uri)
    {
        return m_runtime->backend().remove(uri);
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return m_runtime->backend().size();
    }

    [[nodiscard]] std::string_view backend_name() const noexcept
    {
        return m_runtime->backend().name();
    }

private:
    std::shared_ptr<StorageRuntime> m_runtime{};
};

} // namespace core::storage

#ifdef CONGELADO_TEST
namespace core::storage::tests {
using namespace boost::ut;

suite<"core_storage_storage"> core_storage_storage_suite = []
{
    "facade round-trips payloads"_test = []
    {
        Storage storage;
        const auto written = storage.write("payload");
        expect(written.ok());
        expect(storage.read(written.ref().uri()).data() == "payload");
        expect(storage.size() == 1);
        expect(storage.remove(written.ref().uri()));
        expect(storage.size() == 0);
    };
};

} // namespace core::storage::tests
#endif
