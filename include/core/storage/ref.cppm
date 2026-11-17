export module core_storage:ref;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::storage {

/** @brief Opaque reference returned by a successful write. */
class StorageRef
{
public:
    StorageRef() = default;

    explicit StorageRef(std::string uri) :
        m_uri{std::move(uri)}
    {
    }

    [[nodiscard]] const std::string& uri() const noexcept
    {
        return m_uri;
    }

    [[nodiscard]] bool valid() const noexcept
    {
        return !m_uri.empty();
    }

private:
    std::string m_uri{};
};

} // namespace core::storage

#ifdef CONGELADO_TEST
namespace core::storage::tests {
using namespace boost::ut;

suite<"core_storage_storage_ref"> core_storage_storage_ref_suite = []
{
    "carries a URI"_test = []
    {
        const StorageRef ref{"s3://bucket/key"};
        expect(ref.valid());
        expect(ref.uri() == "s3://bucket/key");
        expect(!StorageRef{}.valid());
    };
};

} // namespace core::storage::tests
#endif
