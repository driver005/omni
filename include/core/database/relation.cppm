export module core_database:relation;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::db {

/** @brief A join relation between two tables, expressed as compile-time data. */
class Relation
{
public:
    Relation() = default;

    Relation(
        std::string from_table,
        std::string from_column,
        std::string to_table,
        std::string to_column
    ) :
        m_from_table{std::move(from_table)},
        m_from_column{std::move(from_column)},
        m_to_table{std::move(to_table)},
        m_to_column{std::move(to_column)}
    {
    }

    [[nodiscard]] const std::string& from_table() const noexcept
    {
        return m_from_table;
    }

    [[nodiscard]] const std::string& from_column() const noexcept
    {
        return m_from_column;
    }

    [[nodiscard]] const std::string& to_table() const noexcept
    {
        return m_to_table;
    }

    [[nodiscard]] const std::string& to_column() const noexcept
    {
        return m_to_column;
    }

    /** @brief Renders the JOIN clause for this relation. */
    [[nodiscard]] std::string join_clause() const
    {
        return std::format(
            " JOIN {} ON {}.{} = {}.{}",
            m_to_table,
            m_from_table,
            m_from_column,
            m_to_table,
            m_to_column
        );
    }

private:
    std::string m_from_table{};
    std::string m_from_column{};
    std::string m_to_table{};
    std::string m_to_column{};
};

} // namespace core::db

#ifdef CONGELADO_TEST
namespace core::db::tests {
using namespace boost::ut;

suite<"core_db_relation"> core_db_relation_suite = []
{
    "renders a join clause"_test = []
    {
        const Relation relation{"posts", "author_id", "authors", "id"};
        expect(relation.from_table() == "posts");
        expect(relation.join_clause() ==
               " JOIN authors ON posts.author_id = authors.id");
    };
};

} // namespace core::db::tests
#endif
