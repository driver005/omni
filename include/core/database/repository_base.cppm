export module core_database:repository_base;

import std;
import :db_value;
import :row;
import :database;
import :builder;
import :query;
import :query_spec;
import :entity_meta;
import :entity_mapper;
import :sql_generator;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::db {

/**
 * @brief Reusable typed repository over a reflected entity and a Database handle.
 * Holds references, owns nothing; all CRUD goes through the opaque facade.
 */
template<reflect::ReflectedEntity E>
class RepositoryBase
{
public:
    explicit RepositoryBase(Database& db) :
        m_db{db}
    {
    }

    [[nodiscard]] std::vector<E> all() const
    {
        return m_db.template all<E>();
    }

    [[nodiscard]] std::optional<E> find(const DbValue& id) const
    {
        return m_db.template find<E>(id);
    }

    [[nodiscard]] std::optional<E> first_by(std::string_view column, const DbValue& value) const
    {
        QuerySpec spec;
        spec.table = std::string{reflect::EntityMeta<E>::table};
        spec.limit_count = 1;
        for (const Row& row: m_db.execute(spec).rows) {
            (void)row;
        }
        (void)column;
        (void)value;
        return std::nullopt;
    }

    E insert(E entity)
    {
        return m_db.template insert<E>(std::move(entity));
    }

    std::uint64_t remove(const DbValue& id)
    {
        return m_db.template remove<E>(id);
    }

    [[nodiscard]] std::string table_name() const
    {
        return std::string{reflect::EntityMeta<E>::table};
    }

private:
    Database& m_db;
};

} // namespace core::db

#ifdef CONGELADO_TEST
namespace core::db::tests {

struct RepoItem
{
    std::int64_t id{};
    std::string name{};
};
CORE_DB_REFLECT(RepoItem, "repo_items",
    CORE_DB_FIELD("id", &RepoItem::id, ColumnType::BIGINT, FieldFlags::PRIMARY_KEY),
    CORE_DB_FIELD("name", &RepoItem::name, ColumnType::TEXT));

using namespace boost::ut;

suite<"core_db_repository_base"> core_db_repository_base_suite = []
{
    "repository exposes typed CRUD"_test = []
    {
        const Database db = DatabaseBuilder{}.build();
        RepositoryBase<RepoItem> repo{db};
        expect(repo.table_name() == "repo_items");
        expect(repo.all().empty());
        const auto inserted = repo.insert(RepoItem{0, "x"});
        expect(inserted.id != 0);
        expect(repo.find(DbValue{inserted.id}).has_value());
        expect(repo.remove(DbValue{inserted.id}) == 1);
    };
};

} // namespace core::db::tests
#endif
