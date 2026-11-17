export module core_storage:storage_builder;

import std;
import :storage;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::storage {

/** @brief Fluent entry point — sole public way to construct a Storage handle. */
class StorageBuilder
{
public:
    StorageBuilder& option(std::string key, std::string value)
    {
        m_options.insert_or_assign(std::move(key), std::move(value));
        return *this;
    }

    [[nodiscard]] Storage build() const
    {
        return Storage{};
    }

private:
    std::map<std::string, std::string> m_options{};
};

} // namespace core::storage

#ifdef CONGELADO_TEST
namespace core::storage::tests {
using namespace boost::ut;

suite<"core_storage_storage_builder"> core_storage_storage_builder_suite = []
{
    "builds a working Storage handle"_test = []
    {
        const Storage storage = StorageBuilder{}.option("bucket", "test").build();
        expect(storage.backend_name() == "memory");
    };
};

} // namespace core::storage::tests
#endif
