export module core_database:storage_engine;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::db {

/**
 * @brief The only backend choice a consumer gets. Concrete engine classes never
 * leave the internal partitions of this module.
 */
enum class StorageEngine : std::uint8_t
{
    IN_MEMORY,
    /** @brief Reserved for the postgres plugin wired behind the facade. */
    POSTGRES
};

/** @brief Instance functor mapping a StorageEngine to its name. */
class EngineName
{
public:
    [[nodiscard]] constexpr std::string_view operator()(StorageEngine engine) const noexcept
    {
        return engine == StorageEngine::POSTGRES ? "postgres" : "in_memory";
    }
};

} // namespace core::db

#ifdef CONGELADO_TEST
namespace core::db::tests {
using namespace boost::ut;

suite<"core_db_storage_engine"> core_db_storage_engine_suite = []
{
    "exposes backend names only"_test = []
    {
        expect(EngineName{}(StorageEngine::IN_MEMORY) == "in_memory");
        expect(EngineName{}(StorageEngine::POSTGRES) == "postgres");
    };
};

} // namespace core::db::tests
#endif
