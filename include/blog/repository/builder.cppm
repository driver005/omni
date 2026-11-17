export module blog_repository:builder;

import std;
import core_database;
import :repository;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::repository {

/** @brief Builds a PostRepository bound to a Database handle. */
class RepositoryBuilder
{
public:
    explicit RepositoryBuilder(core::db::Database& db) :
        m_db{db}
    {
    }

    [[nodiscard]] PostRepository build() const
    {
        return PostRepository{m_db};
    }

private:
    core::db::Database& m_db;
};

} // namespace blog::repository

#ifdef CONGELADO_TEST
namespace blog::repository::tests {
using namespace boost::ut;
suite<"blog_repository_builder"> blog_repository_builder_suite = []
{
    "builds a repository"_test = []
    {
        expect(true);
    };
};
} // namespace blog::repository::tests
#endif
