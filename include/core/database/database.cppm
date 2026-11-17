export module core_database:database;

import std;
import :db_value;
import :row;
import :query_result;
import :query_spec;
import :query;
import :compare;
import :sort_order;
import :predicate;
import :entity_meta;
import :sql_generator;
import :entity_mapper;
import :database_runtime;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

// ─── Reflection helpers (module-local, deliberately not exported) ────────────

namespace core::db::detail {

template<reflect::ReflectedEntity E>
consteval bool has_primary_key()
{
    return []<std::size_t... I>(std::index_sequence<I...>) {
        return (... || reflect::field_at_t<E, I>::primary_key);
    }(std::make_index_sequence<
      std::tuple_size_v<typename reflect::EntityMeta<E>::fields>>{});
}

template<reflect::ReflectedEntity E>
consteval std::string_view primary_key_column()
{
    std::string_view result;
    [&]<std::size_t... I>(std::index_sequence<I...>) {
        ((reflect::field_at_t<E, I>::primary_key
              ? (result = std::string_view{reflect::field_at_t<E, I>::name}, 0)
              : 0),
         ...);
    }(std::make_index_sequence<
      std::tuple_size_v<typename reflect::EntityMeta<E>::fields>>{});
    if (result.empty()) {
        throw "CORE_DB: entity has no reflected primary key";
    }
    return result;
}

} // namespace core::db::detail

export namespace core::db {

/**
 * @brief The only handle a consumer ever holds. It hides the backend, the
 * connection and every cursor: callers get typed CRUD and query methods and
 * nothing else. Construct it through DatabaseBuilder.
 */
class Database
{
public:
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;
    Database(Database&&) noexcept;
    Database& operator=(Database&&) noexcept;
    ~Database();

    [[nodiscard]] bool is_open() const noexcept;
    [[nodiscard]] std::string_view engine_name() const noexcept;

    template<reflect::ReflectedEntity E>
    [[nodiscard]] std::vector<E> all() const;

    template<reflect::ReflectedEntity E>
    [[nodiscard]] std::optional<E> find(const DbValue& id) const;

    template<reflect::ReflectedEntity E, auto Member>
    [[nodiscard]] std::optional<E> find_first_by(Compare comparison, DbValue value) const;

    template<reflect::ReflectedEntity E>
    [[nodiscard]] Query<E> query() const;

    [[nodiscard]] QueryResult execute(const QuerySpec& spec) const;

    template<reflect::ReflectedEntity E>
    E insert(E entity);

    template<reflect::ReflectedEntity E, auto Member>
    std::uint64_t update_where(const E& entity, Compare comparison, DbValue value);

    template<reflect::ReflectedEntity E>
    std::uint64_t remove(const DbValue& id);

    template<reflect::ReflectedEntity E>
    [[nodiscard]] std::uint64_t count() const;

    void close() noexcept;

private:
    struct Runtime;
    std::unique_ptr<Runtime> m_runtime{};

    explicit Database(std::unique_ptr<Runtime> runtime) noexcept;

    template<reflect::ReflectedEntity E>
    [[nodiscard]] std::vector<E> collect(const QuerySpec& spec) const;

    [[nodiscard]] static std::vector<std::string> split(std::string_view joined);

    friend class DatabaseBuilder;
};

} // namespace core::db

// ─── Implementation (module purview — not reachable by importers) ────────────

namespace core::db {

struct Database::Runtime
{
    explicit Runtime(std::unique_ptr<detail::DatabaseRuntime> value) noexcept :
        impl{std::move(value)}
    {
    }

    std::unique_ptr<detail::DatabaseRuntime> impl{};
};

Database::Database(std::unique_ptr<Runtime> runtime) noexcept :
    m_runtime{std::move(runtime)}
{
}

Database::Database(Database&&) noexcept = default;
Database& Database::operator=(Database&&) noexcept = default;
Database::~Database() = default;

bool Database::is_open() const noexcept
{
    return m_runtime && m_runtime->impl && m_runtime->impl->is_open();
}

std::string_view Database::engine_name() const noexcept
{
    return m_runtime && m_runtime->impl ? m_runtime->impl->name() : "none";
}

QueryResult Database::execute(const QuerySpec& spec) const
{
    return m_runtime->impl->driver().run(spec);
}

void Database::close() noexcept
{
    if (m_runtime && m_runtime->impl) {
        m_runtime->impl->driver().close();
    }
}

std::vector<std::string> Database::split(std::string_view joined)
{
    std::vector<std::string> columns;
    for (const auto part: joined | std::views::split(',')) {
        columns.emplace_back(std::ranges::begin(part), std::ranges::end(part));
    }
    return columns;
}

template<reflect::ReflectedEntity E>
std::vector<E> Database::collect(const QuerySpec& spec) const
{
    const QueryResult result = execute(spec);
    std::vector<E> entities;
    entities.reserve(result.rows.size());
    for (const Row& row: result.rows) {
        entities.push_back(reflect::EntityMapper::from_row<E>(row));
    }
    return entities;
}

template<reflect::ReflectedEntity E>
std::vector<E> Database::all() const
{
    const std::string table{reflect::EntityMeta<E>::table};
    m_runtime->impl->require_table(table);
    return collect<E>(Query<E>{}.spec());
}

template<reflect::ReflectedEntity E>
std::optional<E> Database::find(const DbValue& id) const
{
    static_assert(detail::has_primary_key<E>(), "find() requires a reflected primary key");
    const std::string table{reflect::EntityMeta<E>::table};
    m_runtime->impl->require_table(table);

    QuerySpec spec;
    spec.table = table;
    spec.columns = split(reflect::SqlGenerator::select_columns<E>());
    spec.predicates.push_back(Predicate{
        .column = std::string{detail::primary_key_column<E>()},
        .comparison = Compare::EQ,
        .value = id,
    });
    spec.limit_count = 1;
    const QueryResult result = execute(spec);
    if (result.rows.empty()) {
        return std::nullopt;
    }
    return reflect::EntityMapper::from_row<E>(result.rows.front());
}

template<reflect::ReflectedEntity E, auto Member>
std::optional<E> Database::find_first_by(Compare comparison, DbValue value) const
{
    static_assert(
        reflect::is_entity_member_v<E, Member>,
        "find_first_by() member is not reflected on this entity"
    );
    const std::string table{reflect::EntityMeta<E>::table};
    m_runtime->impl->require_table(table);

    QuerySpec spec;
    spec.table = table;
    spec.columns = split(reflect::SqlGenerator::select_columns<E>());
    spec.predicates.push_back(Predicate{
        .column = std::string{reflect::column_of<E, Member>()},
        .comparison = comparison,
        .value = std::move(value),
    });
    spec.limit_count = 1;
    const QueryResult result = execute(spec);
    if (result.rows.empty()) {
        return std::nullopt;
    }
    return reflect::EntityMapper::from_row<E>(result.rows.front());
}

template<reflect::ReflectedEntity E>
Query<E> Database::query() const
{
    const std::string table{reflect::EntityMeta<E>::table};
    m_runtime->impl->require_table(table);
    return Query<E>{};
}

template<reflect::ReflectedEntity E>
E Database::insert(E entity)
{
    const std::string table{reflect::EntityMeta<E>::table};
    m_runtime->impl->require_table(table);
    auto values = reflect::EntityMapper::to_values(entity);

    if constexpr (detail::has_primary_key<E>()) {
        const std::string pk{detail::primary_key_column<E>()};
        if (!values.contains(pk)) {
            const std::uint64_t id = m_runtime->impl->driver().next_id();
            values[pk] = to_db_value(id);
            [&]<std::size_t... I>(std::index_sequence<I...>) {
                auto assign = [&](auto descriptor) {
                    using F = decltype(descriptor);
                    if constexpr (F::primary_key) {
                        if constexpr (std::integral<typename F::value_type>) {
                            entity.*F::member = static_cast<typename F::value_type>(id);
                        } else if constexpr (std::same_as<typename F::value_type, std::string>) {
                            entity.*F::member = std::to_string(id);
                        }
                    }
                };
                (assign(reflect::field_at_t<E, I>{}), ...);
            }(std::make_index_sequence<
              std::tuple_size_v<typename reflect::EntityMeta<E>::fields>>{});
        }
    }
    m_runtime->impl->driver().insert(table, values);
    return entity;
}

template<reflect::ReflectedEntity E, auto Member>
std::uint64_t Database::update_where(const E& entity, Compare comparison, DbValue value)
{
    static_assert(
        reflect::is_entity_member_v<E, Member>,
        "update_where() member is not reflected on this entity"
    );
    const std::string table{reflect::EntityMeta<E>::table};
    m_runtime->impl->require_table(table);
    auto values = reflect::EntityMapper::to_values(entity);
    values.erase(std::string{detail::primary_key_column<E>()});
    return m_runtime->impl->driver().update(
        table,
        values,
        std::string{reflect::column_of<E, Member>()},
        std::move(value)
    );
}

template<reflect::ReflectedEntity E>
std::uint64_t Database::remove(const DbValue& id)
{
    static_assert(detail::has_primary_key<E>(), "remove() requires a reflected primary key");
    const std::string table{reflect::EntityMeta<E>::table};
    m_runtime->impl->require_table(table);
    return m_runtime->impl->driver().remove(
        table,
        detail::primary_key_column<E>(),
        id
    );
}

template<reflect::ReflectedEntity E>
std::uint64_t Database::count() const
{
    return static_cast<std::uint64_t>(all<E>().size());
}

} // namespace core::db

#ifdef CONGELADO_TEST
namespace core::db::tests {

struct Person
{
    std::int64_t id{};
    std::string name{};
    std::int64_t age{};
};
CORE_DB_REFLECT(Person, "people",
    CORE_DB_FIELD("id", &Person::id, ColumnType::BIGINT, FieldFlags::PRIMARY_KEY),
    CORE_DB_FIELD("name", &Person::name, ColumnType::TEXT, FieldFlags::NOT_NULL),
    CORE_DB_FIELD("age", &Person::age, ColumnType::INTEGER));

using namespace boost::ut;

suite<"core_db_database"> core_db_database_suite = []
{
    "primary key metadata drives the facade's key handling"_test = []
    {
        static_assert(detail::has_primary_key<Person>());
        expect(detail::primary_key_column<Person>() == "id");
    };
};

} // namespace core::db::tests
#endif
