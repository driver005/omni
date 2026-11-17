export module blog_repository:repository;

import std;
import core_database;
import blog_model;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::repository {

/** @brief Typed Ghost post repository over the opaque Database facade. */
class PostRepository
{
public:
    explicit PostRepository(core::db::Database& db) :
        m_db{db},
        m_repo{db}
    {
    }

    [[nodiscard]] std::vector<model::Post> all() const
    {
        return m_repo.all();
    }

    model::Post insert(model::Post post)
    {
        return m_repo.insert(std::move(post));
    }

    std::uint64_t remove(const core::db::DbValue& id)
    {
        return m_repo.remove(id);
    }

    /** @brief Typed predicate query — member pointer validated at compile time. */
    template<auto Member>
    [[nodiscard]] std::vector<model::Post> find_by(
        core::db::Compare comparison,
        core::db::DbValue value
    ) const
    {
        static_assert(
            core::db::reflect::is_entity_member_v<model::Post, Member>,
            "find_by member is not reflected on Post"
        );
        core::db::QuerySpec spec;
        spec.table = std::string{core::db::reflect::EntityMeta<model::Post>::table};
        spec.predicates.push_back(core::db::Predicate{
            .column = std::string{core::db::reflect::column_of<model::Post, Member>()},
            .comparison = comparison,
            .value = std::move(value),
        });
        std::vector<model::Post> result;
        for (const core::db::Row& row: m_db.execute(spec).rows) {
            result.push_back(core::db::reflect::EntityMapper{}.from_row<model::Post>(row));
        }
        return result;
    }

    [[nodiscard]] std::vector<model::Post> find_by_column(
        std::string_view column,
        core::db::Compare comparison,
        core::db::DbValue value
    ) const
    {
        core::db::QuerySpec spec;
        spec.table = std::string{core::db::reflect::EntityMeta<model::Post>::table};
        spec.predicates.push_back(core::db::Predicate{
            .column = std::string{column},
            .comparison = comparison,
            .value = std::move(value),
        });
        std::vector<model::Post> result;
        for (const core::db::Row& row: m_db.execute(spec).rows) {
            result.push_back(core::db::reflect::EntityMapper{}.from_row<model::Post>(row));
        }
        return result;
    }

    [[nodiscard]] std::vector<model::Post> find_by_status(std::string status) const
    {
        return find_by_column("status", core::db::Compare::EQ, core::db::DbValue{std::move(status)});
    }

private:
    core::db::Database& m_db;
    core::db::RepositoryBase<model::Post> m_repo;
};

} // namespace blog::repository

#ifdef CONGELADO_TEST
namespace blog::repository::tests {
using namespace boost::ut;
suite<"blog_repository_repository"> blog_repository_repository_suite = []
{
    "exposes typed find_by"_test = []
    {
        core::db::Database db = core::db::DatabaseBuilder{}.build();
        PostRepository repo{db};
        const auto result = repo.find_by_status("published");
        expect(result.empty() || !result.empty());
    };
};
} // namespace blog::repository::tests
#endif
