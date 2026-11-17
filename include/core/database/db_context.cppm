export module core_database:db_context;

import std;
import :storage_engine;
import :entity_meta;
import :sql_generator;
import :runtime;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

namespace core::db {

/** @brief Accumulates builder configuration and compiles it into a runtime. */
class DbContext
{
public:
    void set_engine(StorageEngine engine) noexcept
    {
        m_engine = engine;
    }

    std::unique_ptr<StorageBackend> take_backend()
    {
        return std::move(m_backend);
    }

    void use_backend(std::unique_ptr<StorageBackend> backend)
    {
        m_backend = std::move(backend);
    }

    void set_option(std::string key, std::string value)
    {
        m_options.insert_or_assign(std::move(key), std::move(value));
    }

    template<reflect::ReflectedEntity E>
    void register_entity()
    {
        const std::string table{reflect::EntityMeta<E>::table};
        if (!m_tables.contains(table)) {
            m_tables.emplace(table, reflect::SqlGenerator{}.create_table<E>());
        }
    }

    [[nodiscard]] std::unique_ptr<DatabaseRuntime> build()
    {
        auto runtime = std::make_unique<DatabaseRuntime>(std::move(m_backend));
        for (const auto& [table, ddl]: m_tables) {
            (void)ddl; // DDL is retained for diagnostics; the backend keys by table name.
            runtime->register_table(table);
        }
        return runtime;
    }

    [[nodiscard]] StorageEngine engine() const noexcept
    {
        return m_engine;
    }

    [[nodiscard]] const std::map<std::string, std::string>& options() const noexcept
    {
        return m_options;
    }

    [[nodiscard]] std::map<std::string, std::string>& options() noexcept
    {
        return m_options;
    }

private:
    std::unique_ptr<StorageBackend> m_backend{};
    StorageEngine m_engine{StorageEngine::IN_MEMORY};
    std::map<std::string, std::string> m_options{};
    std::map<std::string, std::string> m_tables{};
};

} // namespace core::db

#ifdef CONGELADO_TEST
namespace core::db::tests {

struct ContextSample
{
    std::int64_t id{};
};
CORE_DB_REFLECT(ContextSample, "context_samples",
    CORE_DB_FIELD("id", &ContextSample::id, ColumnType::BIGINT, FieldFlags::PRIMARY_KEY));

using namespace boost::ut;

suite<"core_db_db_context"> core_db_db_context_suite = []
{
    "accumulates config and builds a runtime with registered tables"_test = []
    {
        DbContext context;
        context.set_engine(StorageEngine::IN_MEMORY);
        context.set_option("schema", "public");
        context.register_entity<ContextSample>();
        expect(context.engine() == StorageEngine::IN_MEMORY);
        expect(context.options().at("schema") == "public");

        auto runtime = context.build();
        expect(runtime->table_registered("context_samples"));
        expect(runtime->is_open());
    };
};

} // namespace core::db::tests
#endif
