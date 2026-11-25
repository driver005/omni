export module core_database:query;

import std;
import :db_value;
import :compare;
import :sort_order;
import :sql_generator;
import :entity_meta;
import :query_spec;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::db {

/**
 * @brief Fluent, entity-typed query builder.
 *
 * Member-pointer arguments are checked at compile time: a pointer that is not a
 * reflected member of `E` fails to compile. The built QuerySpec is plain data —
 * no connection, driver or cursor ever leaks to the caller.
 */
template<reflect::ReflectedEntity E>
class Query
{
public:
    Query() :
        m_spec{
            .table = std::string{reflect::EntityMeta<E>::table},
            .columns = split_columns(reflect::SqlGenerator{}.select_columns<E>()),
        }
    {
    }

    /** @brief Restricts the projection to the listed reflected members. */
    template<auto... Members>
    Query& select()
    {
        static_assert(sizeof...(Members) > 0, "select() needs at least one member");
        static_assert(
            (reflect::is_entity_member_v<E, Members> && ...),
            "select() member is not reflected on this entity"
        );
        m_spec.columns = {std::string{reflect::column_of<E, Members>()}...};
        return *this;
    }

    /** @brief Adds a typed WHERE predicate; the member must be reflected. */
    template<auto Member>
    Query& where(Compare comparison, DbValue value)
    {
        static_assert(
            reflect::is_entity_member_v<E, Member>,
            "where() member is not reflected on this entity"
        );
        m_spec.predicates.push_back(
            Predicate{
                .column = std::string{reflect::column_of<E, Member>()},
                .comparison = comparison,
                .value = std::move(value),
            }
        );
        return *this;
    }

    /** @brief Convenience: equality predicate on a reflected member. */
    template<auto Member>
    Query& where_eq(DbValue value)
    {
        return where<Member>(Compare::EQ, std::move(value));
    }

    /** @brief Adds an ORDER BY; the member must be reflected. */
    template<auto Member>
    Query& order_by(SortOrder direction = SortOrder::ASC)
    {
        static_assert(
            reflect::is_entity_member_v<E, Member>,
            "order_by() member is not reflected on this entity"
        );
        m_spec.ordering = std::make_pair(std::string{reflect::column_of<E, Member>()}, direction);
        return *this;
    }

    Query& limit(std::size_t count)
    {
        m_spec.limit_count = count;
        return *this;
    }

    Query& offset(std::size_t count)
    {
        m_spec.offset_count = count;
        return *this;
    }

    [[nodiscard]] const QuerySpec& spec() const noexcept
    {
        return m_spec;
    }

    [[nodiscard]] std::string to_sql() const
    {
        return m_spec.to_sql();
    }

private:
    [[nodiscard]] std::vector<std::string> split_columns(std::string_view joined)
    {
        std::vector<std::string> columns;
        for (const auto part: std::views::split(joined, ',')) {
            columns.emplace_back(std::ranges::begin(part), std::ranges::end(part));
        }
        return columns;
    }

    QuerySpec m_spec{};
};

} // namespace core::db

#ifdef CONGELADO_TEST
namespace core::db::tests {

struct Widget
{
    std::int64_t id{};
    std::string label{};
};

CORE_DB_REFLECT(
    Widget,
    "widgets",
    CORE_DB_FIELD("id", &Widget::id, ColumnType::BIGINT, FieldFlags::PRIMARY_KEY),
    CORE_DB_FIELD("label", &Widget::label, ColumnType::TEXT)
);

using namespace boost::ut;

suite<"core_db_query"> core_db_query_suite = []
{
    "default query selects all reflected columns"_test = []
    {
        Query<Widget> query;
        expect(query.to_sql() == "SELECT id,label FROM widgets");
    };

    "typed predicates, ordering and paging compose"_test = []
    {
        Query<Widget> query;
        query.where<&Widget::id>(Compare::GT, to_db_value(3))
            .where<&Widget::label>(Compare::LIKE, to_db_value(std::string{"a%"}))
            .order_by<&Widget::id>(SortOrder::DESC)
            .limit(10)
            .offset(5);
        expect(
            query.to_sql() == "SELECT id,label FROM widgets WHERE id > 3 AND label LIKE 'a%' "
                              "ORDER BY id DESC LIMIT 10 OFFSET 5"
        );
    };

    "select restricts the projection"_test = []
    {
        Query<Widget> query;
        query.select<&Widget::label>();
        expect(query.to_sql() == "SELECT label FROM widgets");
    };
};

} // namespace core::db::tests
#endif
