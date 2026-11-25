export module core_database:builder;

import std;
import :storage_engine;
import :entity_meta;
import :db_context;
import :database;
import :storage_backend;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::db {

/**
 * @brief Fluent entry point into the abstraction. Registers entities and picks
 * a backend, then builds an opaque Database handle. Nothing here exposes a
 * connection, driver or backend class.
 */
class DatabaseBuilder
{
public:
    DatabaseBuilder& engine(StorageEngine engine)
    {
        m_context.set_engine(engine);
        return *this;
    }

    DatabaseBuilder& use_backend(std::unique_ptr<StorageBackend> backend)
    {
        m_context.use_backend(std::move(backend));
        return *this;
    }

    DatabaseBuilder& option(std::string key, std::string value)
    {
        m_context.set_option(std::move(key), std::move(value));
        return *this;
    }

    template<reflect::ReflectedEntity E>
    DatabaseBuilder& register_entity()
    {
        m_context.register_entity<E>();
        return *this;
    }

    [[nodiscard]] Database build()
    {
        Database db;
        db.use_backend(m_context.take_backend());
        return db;
    }

private:
    DbContext m_context{};
};

} // namespace core::db

#ifdef CONGELADO_TEST
namespace core::db::tests {

struct BuiltSample
{
    std::int64_t id{};
};

CORE_DB_REFLECT(
    BuiltSample,
    "built_samples",
    CORE_DB_FIELD("id", &BuiltSample::id, ColumnType::BIGINT, FieldFlags::PRIMARY_KEY)
);

using namespace boost::ut;

suite<"core_db_database_builder"> core_db_database_builder_suite = []
{
    "builds an open handle with registered entities"_test = []
    {
        DatabaseBuilder builder;
        Database db = builder.engine(StorageEngine::IN_MEMORY)
                          .option("schema", "public")
                          .register_entity<BuiltSample>()
                          .build();
        expect(db.is_open());
        expect(db.engine_name() == "in_memory");
        expect(db.count<BuiltSample>() == 0);
    };
};

} // namespace core::db::tests
#endif
