export module core_storage:storage_backend;

import std;
import :storage_ref;
import :storage_result;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::storage::detail {

/**
 * @brief Internal backend contract. The facade owns an implementation; plugins
 * (P1/P2) plug in through the bridge without the facade exposing storage details.
 */
class StorageBackend
{
public:
    virtual ~StorageBackend() = default;

    [[nodiscard]] virtual std::string_view name() const noexcept = 0;
    [[nodiscard]] virtual StorageResult write(std::string_view payload) = 0;
    [[nodiscard]] virtual StorageResult read(std::string_view uri) const = 0;
    virtual bool remove(std::string_view uri) = 0;
    [[nodiscard]] virtual std::size_t size() const noexcept = 0;
};

} // namespace core::storage::detail

#ifdef CONGELADO_TEST
namespace core::storage::detail::tests {
using namespace boost::ut;

suite<"core_storage_storage_backend"> core_storage_storage_backend_suite = []
{
    "backend contract is abstract"_test = []
    {
        expect(std::is_abstract_v<StorageBackend>);
    };
};

} // namespace core::storage::detail::tests
#endif
