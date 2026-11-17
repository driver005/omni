export module memory_backends:storage;

import std;
import core_storage;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

namespace core::storage {

/** @brief Reference backend: in-memory key/value store with generated URIs. */
class MemoryStorageBackend final : public StorageBackend
{
public:
    [[nodiscard]] std::string_view name() const noexcept override
    {
        return "memory";
    }

    [[nodiscard]] StorageResult write(std::string_view payload) override
    {
        const std::string uri = std::format("mem://{}", m_next++);
        m_values.emplace(uri, std::string{payload});
        return StorageResult{StorageResult::Status::OK, std::string{payload}, StorageRef{uri}};
    }

    [[nodiscard]] StorageResult read(std::string_view uri) const override
    {
        const auto it = m_values.find(std::string{uri});
        if (it == m_values.end()) {
            return StorageResult{StorageResult::Status::NOT_FOUND, {}, StorageRef{}};
        }
        return StorageResult{StorageResult::Status::OK, it->second, StorageRef{std::string{uri}}};
    }

    bool remove(std::string_view uri) override
    {
        return m_values.erase(std::string{uri}) != 0;
    }

    [[nodiscard]] std::size_t size() const noexcept override
    {
        return m_values.size();
    }

private:
    std::map<std::string, std::string> m_values{};
    std::uint64_t m_next{1};
};

} // namespace core::storage

#ifdef CONGELADO_TEST
namespace core::storage::tests {
using namespace boost::ut;

suite<"core_storage_memory_storage_backend"> core_storage_memory_storage_backend_suite = []
{
    "writes, reads and removes payloads"_test = []
    {
        MemoryStorageBackend backend;
        expect(backend.name() == "memory");
        const auto written = backend.write("hello");
        expect(written.ok());
        expect(backend.read(written.ref().uri()).data() == "hello");
        expect(backend.remove(written.ref().uri()));
        expect(!backend.read(written.ref().uri()).ok());
    };
};

} // namespace core::storage::tests
#endif
