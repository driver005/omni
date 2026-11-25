export module core_database:database;

import std;
import :storage_engine;
import :db_value;
import :row;
import :query_result;
import :query_spec;
import :query;
import :compare;
import :predicate;
import :entity_meta;
import :sql_generator;
import :entity_mapper;
import :storage_backend;
import :runtime;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::db {

/**
 * @brief The only handle a consumer holds. All members inline, shared_ptr pImpl,
 * no free/static functions. Built by DatabaseBuilder.
 */
class Database
{
public:
    Database() :
        m_runtime{std::make_shared<DatabaseRuntime>(nullptr)}
    {
    }

    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;
    Database(Database&&) noexcept = default;
    Database& operator=(Database&&) noexcept = default;
    ~Database() = default;

    void use_backend(std::unique_ptr<StorageBackend> backend)
    {
        m_runtime = std::make_shared<DatabaseRuntime>(std::move(backend));
    }

    [[nodiscard]] bool is_open() const noexcept
    {
        return m_runtime && m_runtime->is_open();
    }

    [[nodiscard]] std::string_view engine_name() const noexcept
    {
        return m_runtime ? m_runtime->name() : "none";
    }

    [[nodiscard]] QueryResult execute(const QuerySpec& spec) const
    {
        return m_runtime->driver().run(spec);
    }

    void register_table(std::string_view table)
    {
        m_runtime->register_table(table);
    }

    [[nodiscard]] std::vector<std::string> split(std::string_view joined) const
    {
        std::vector<std::string> columns;
        for (const auto part: std::views::split(joined, ',')) {
            columns.emplace_back(std::ranges::begin(part), std::ranges::end(part));
        }
        return columns;
    }

    template<reflect::ReflectedEntity E>
    [[nodiscard]] std::vector<E> collect(const QuerySpec& spec) const
    {
        const QueryResult result = execute(spec);
        std::vector<E> entities;
        entities.reserve(result.rows.size());
        for (const Row& row: result.rows) {
            entities.push_back(reflect::EntityMapper{}.from_row<E>(row));
        }
        return entities;
    }

    template<reflect::ReflectedEntity E>
    [[nodiscard]] std::vector<E> all() const
    {
        const std::string table{std::string{reflect::EntityMeta<E>::table}};
        m_runtime->require_table(table);
        return collect<E>(Query<E>{}.spec());
    }

    template<reflect::ReflectedEntity E>
    [[nodiscard]] std::optional<E> find(const DbValue& id) const
    {
        static_assert(
            []<std::size_t... I>(std::index_sequence<I...>)
            {
                return (... || reflect::field_at_t<E, I>::primary_key);
            }(std::make_index_sequence<
                std::tuple_size_v<typename reflect::EntityMeta<E>::fields>>{}),
            "find() requires a reflected primary key"
        );
        const std::string table{std::string{reflect::EntityMeta<E>::table}};
        m_runtime->require_table(table);
        QuerySpec spec;
        spec.table = table;
        spec.columns = split(reflect::SqlGenerator{}.select_columns<E>());
        spec.predicates.push_back(
            Predicate{
                .column = std::string{pk_column<E>()},
                .comparison = Compare::EQ,
                .value = id,
            }
        );
        spec.limit_count = 1;
        const QueryResult result = execute(spec);
        if (result.rows.empty()) {
            return std::nullopt;
        }
        return reflect::EntityMapper{}.from_row<E>(result.rows.front());
    }

    template<reflect::ReflectedEntity E, auto Member>
    [[nodiscard]] std::optional<E> find_first_by(Compare comparison, DbValue value) const
    {
        static_assert(
            reflect::is_entity_member_v<E, Member>,
            "find_first_by() member is not reflected on this entity"
        );
        const std::string table{std::string{reflect::EntityMeta<E>::table}};
        m_runtime->require_table(table);
        QuerySpec spec;
        spec.table = table;
        spec.columns = split(reflect::SqlGenerator{}.select_columns<E>());
        spec.predicates.push_back(
            Predicate{
                .column = std::string{reflect::column_of<E, Member>()},
                .comparison = comparison,
                .value = std::move(value),
            }
        );
        spec.limit_count = 1;
        const QueryResult result = execute(spec);
        if (result.rows.empty()) {
            return std::nullopt;
        }
        return reflect::EntityMapper{}.from_row<E>(result.rows.front());
    }

    template<reflect::ReflectedEntity E>
    [[nodiscard]] Query<E> query() const
    {
        const std::string table{std::string{reflect::EntityMeta<E>::table}};
        m_runtime->require_table(table);
        return Query<E>{};
    }

    template<reflect::ReflectedEntity E>
    E insert(E entity)
    {
        const std::string table{std::string{reflect::EntityMeta<E>::table}};
        m_runtime->require_table(table);
        auto values = reflect::EntityMapper{}.to_values(entity);
        if constexpr (
            []<std::size_t... I>(std::index_sequence<I...>)
            {
                return (... || reflect::field_at_t<E, I>::primary_key);
            }(std::make_index_sequence<
                std::tuple_size_v<typename reflect::EntityMeta<E>::fields>>{})
        ) {
            const std::string pk{std::string{pk_column<E>()}};
            const auto pk_it = values.find(pk);
            const bool pk_unset = pk_it == values.end() ||
                                  std::holds_alternative<Null>(pk_it->second) ||
                                  (std::holds_alternative<std::int64_t>(pk_it->second) &&
                                   std::get<std::int64_t>(pk_it->second) == 0);
            if (pk_unset) {
                const std::uint64_t id = m_runtime->driver().next_id();
                values[pk] = to_db_value(id);
                [&]<std::size_t... I>(std::index_sequence<I...>)
                {
                    auto assign = [&](auto descriptor)
                    {
                        using F = decltype(descriptor);
                        if constexpr (F::primary_key) {
                            if constexpr (std::integral<typename F::value_type>) {
                                set_field(entity, F{}, static_cast<typename F::value_type>(id));
                            } else if constexpr (
                                std::same_as<typename F::value_type, std::string>
                            ) {
                                set_field(entity, F{}, std::to_string(id));
                            }
                        }
                    };
                    (assign(reflect::field_at_t<E, I>{}), ...);
                }(std::make_index_sequence<
                    std::tuple_size_v<typename reflect::EntityMeta<E>::fields>>{});
            }
        }
        m_runtime->driver().insert(table, values);
        return entity;
    }

    template<reflect::ReflectedEntity E, auto Member>
    std::uint64_t update_where(const E& entity, Compare comparison, DbValue value)
    {
        static_assert(
            reflect::is_entity_member_v<E, Member>,
            "update_where() member is not reflected on this entity"
        );
        const std::string table{std::string{reflect::EntityMeta<E>::table}};
        m_runtime->require_table(table);
        auto values = reflect::EntityMapper{}.to_values(entity);
        values.erase(std::string{pk_column<E>()});
        return m_runtime->driver()
            .update(table, values, std::string{reflect::column_of<E, Member>()}, std::move(value));
    }

    template<reflect::ReflectedEntity E>
    std::uint64_t
    update_where_column(const E& entity, std::string_view column, Compare comparison, DbValue value)
    {
        const std::string table{std::string{reflect::EntityMeta<E>::table}};
        m_runtime->require_table(table);
        auto values = reflect::EntityMapper{}.to_values(entity);
        values.erase(std::string{pk_column<E>()});
        return m_runtime->driver().update(table, values, std::string{column}, std::move(value));
    }

    template<reflect::ReflectedEntity E>
    std::uint64_t remove(const DbValue& id)
    {
        static_assert(
            []<std::size_t... I>(std::index_sequence<I...>)
            {
                return (... || reflect::field_at_t<E, I>::primary_key);
            }(std::make_index_sequence<
                std::tuple_size_v<typename reflect::EntityMeta<E>::fields>>{}),
            "remove() requires a reflected primary key"
        );
        const std::string table{std::string{reflect::EntityMeta<E>::table}};
        m_runtime->require_table(table);
        return m_runtime->driver().remove(table, pk_column<E>(), id);
    }

    template<reflect::ReflectedEntity E>
    [[nodiscard]] std::uint64_t count() const
    {
        return static_cast<std::uint64_t>(all<E>().size());
    }

    void close() noexcept
    {
        if (m_runtime) {
            m_runtime->driver().close();
        }
    }

private:
    template<reflect::ReflectedEntity E>
    [[nodiscard]] constexpr bool has_pk() const
    {
        return []<std::size_t... I>(std::index_sequence<I...>)
        {
            return (... || reflect::field_at_t<E, I>::primary_key);
        }(std::make_index_sequence<std::tuple_size_v<typename reflect::EntityMeta<E>::fields>>{});
    }

    template<reflect::ReflectedEntity E>
    [[nodiscard]] constexpr std::string_view pk_column() const
    {
        std::string_view result;
        [&]<std::size_t... I>(std::index_sequence<I...>)
        {
            ((reflect::field_at_t<E, I>::primary_key
                  ? (result = std::string_view{reflect::field_at_t<E, I>::name}, 0)
                  : 0),
             ...);
        }(std::make_index_sequence<std::tuple_size_v<typename reflect::EntityMeta<E>::fields>>{});
        return result;
    }

    std::shared_ptr<DatabaseRuntime> m_runtime{};
};

} // namespace core::db

#ifdef CONGELADO_TEST
namespace core::db::tests {
using namespace boost::ut;

struct DbFacadeItem
{
    std::int64_t id{};
    std::string name{};
};

CORE_DB_REFLECT(
    DbFacadeItem,
    "facade_items",
    CORE_DB_FIELD("id", &DbFacadeItem::id, ColumnType::BIGINT, FieldFlags::PRIMARY_KEY),
    CORE_DB_FIELD("name", &DbFacadeItem::name, ColumnType::TEXT)
);

suite<"core_db_facade"> core_db_facade_suite = []
{
    "facade works through pImpl"_test = []
    {
        Database db;
        expect(!db.is_open());
    };
};

} // namespace core::db::tests
#endif
