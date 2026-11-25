export module core_storage:result;

import std;
import :ref;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::storage {

/** @brief Outcome of a storage read or write. */
class StorageResult
{
public:
    enum class Status : std::uint8_t
    {
        OK,
        NOT_FOUND,
        FAILED
    };

    StorageResult() = default;

    StorageResult(Status status, std::string data, StorageRef ref) :
        m_status{status},
        m_data{std::move(data)},
        m_ref{std::move(ref)}
    {
    }

    [[nodiscard]] Status status() const noexcept
    {
        return m_status;
    }

    [[nodiscard]] bool ok() const noexcept
    {
        return m_status == Status::OK;
    }

    [[nodiscard]] const std::string& data() const noexcept
    {
        return m_data;
    }

    [[nodiscard]] const StorageRef& ref() const noexcept
    {
        return m_ref;
    }

private:
    Status m_status{Status::FAILED};
    std::string m_data{};
    StorageRef m_ref{};
};

} // namespace core::storage

#ifdef CONGELADO_TEST
namespace core::storage::tests {
using namespace boost::ut;

suite<"core_storage_storage_result"> core_storage_storage_result_suite = []
{
    "carries status, data and ref"_test = []
    {
        const StorageResult result{StorageResult::Status::OK, "payload", StorageRef{"mem://1"}};
        expect(result.ok());
        expect(result.data() == "payload");
        expect(result.ref().uri() == "mem://1");
    };
};

} // namespace core::storage::tests
#endif
