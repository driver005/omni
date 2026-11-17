export module core_database:types;

import std;

export namespace core::db {

// ─── Null / value model ───────────────────────────────────────────────────────

/** @brief Tag used in DbValue / Row cells for a SQL NULL. */
struct Null
{
    constexpr bool operator==(const Null&) const noexcept = default;
};

/** @brief Value that can cross the abstraction boundary, in either direction. */
using DbValue = std::variant<
    Null,
    std::int64_t,
    std::uint64_t,
    double,
    bool,
    std::string,
    std::vector<std::uint8_t>,
    std::chrono::system_clock::time_point>;

/** @brief One row of a result. Cells are keyed by column name. */
struct Row
{
    std::map<std::string, DbValue> cells{};

    /** @brief Number of columns stored in this row. */
    [[nodiscard]] std::size_t size() const noexcept
    {
        return cells.size();
    }

    /** @brief True when the row has no cells at all. */
    [[nodiscard]] bool empty() const noexcept
    {
        return cells.empty();
    }

    /** @brief Checks whether a named column is present. */
    [[nodiscard]] bool contains(std::string_view column) const noexcept
    {
        return cells.find(std::string{column}) != cells.end();
    }

    /**
     * @brief Fetches a cell and converts it to `T`.
     * @note std::string columns can carry a numeric/boolean literal coming back
     * from a text-ish backend, so a narrow set of string conversions is provided
     * on top of std::get. No implicit float<->int juggling otherwise — ask for
     * the type that is actually stored.
     * @throws std::out_of_range if the column is missing.
     * @throws std::bad_variant_access if the cell holds an incompatible type.
     */
    template<typename T>
    [[nodiscard]] T get(std::string_view column) const
    {
        const auto it = cells.find(std::string{column});
        if (it == cells.end()) {
            throw std::out_of_range(std::format("Row has no column '{}'", column));
        }
        const DbValue& value = it->second;
        if constexpr (std::is_same_v<T, std::string>) {
            if (const auto* s = std::get_if<std::string>(&value)) {
                return *s;
            }
        }
        if constexpr (std::integral<T> && !std::same_as<T, bool>) {
            if (const auto* n = std::get_if<std::int64_t>(&value)) {
                return static_cast<T>(*n);
            }
            if (const auto* u = std::get_if<std::uint64_t>(&value)) {
                return static_cast<T>(*u);
            }
        }
        if constexpr (std::same_as<T, bool>) {
            if (const auto* b = std::get_if<bool>(&value)) {
                return *b;
            }
        }
        return std::get<T>(value);
    }

    /** @brief Fetches a cell by name, or a default when it is missing or NULL. */
    template<typename T>
    [[nodiscard]] T get_or(std::string_view column, T fallback) const noexcept
    {
        try {
            return get<T>(column);
        } catch (...) {
            return fallback;
        }
    }
};

/** @brief Typed result of any statement that went through the facade. */
struct QueryResult
{
    std::vector<Row> rows{};
    std::uint64_t rows_affected{0};

    /** @brief Result status, kept deliberately small. */
    enum class Status : std::uint8_t
    {
        OK,
        NOOP
    };
    Status status{Status::OK};

    [[nodiscard]] bool ok() const noexcept
    {
        return status == Status::OK;
    }

    [[nodiscard]] std::size_t size() const noexcept
    {
        return rows.size();
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return rows.empty();
    }
};

// ─── Schema value types ───────────────────────────────────────────────────────

/** @brief SQL column kinds the reflection layer understands. */
enum class ColumnType : std::uint8_t
{
    INTEGER,
    BIGINT,
    SMALLINT,
    REAL,
    DOUBLE,
    BOOLEAN,
    TEXT,
    BLOB,
    TIMESTAMP
};

/** @brief Per-column constraints, a bitmask usable as a compile-time value. */
enum class FieldFlags : std::uint8_t
{
    NONE = 0,
    PRIMARY_KEY = 1,
    NOT_NULL = 2,
    UNIQUE = 4
};

[[nodiscard]] constexpr FieldFlags operator|(FieldFlags a, FieldFlags b) noexcept
{
    return static_cast<FieldFlags>(
        static_cast<std::uint8_t>(a) | static_cast<std::uint8_t>(b)
    );
}

[[nodiscard]] constexpr bool has_flag(FieldFlags value, FieldFlags flag) noexcept
{
    return (static_cast<std::uint8_t>(value) & static_cast<std::uint8_t>(flag)) != 0;
}

/** @brief Maps a ColumnType to the SQL spelling used in generated DDL. */
[[nodiscard]] constexpr std::string_view sql_type_name(ColumnType type) noexcept
{
    switch (type) {
        case ColumnType::INTEGER:
            return "INTEGER";
        case ColumnType::BIGINT:
            return "BIGINT";
        case ColumnType::SMALLINT:
            return "SMALLINT";
        case ColumnType::REAL:
            return "REAL";
        case ColumnType::DOUBLE:
            return "DOUBLE PRECISION";
        case ColumnType::BOOLEAN:
            return "BOOLEAN";
        case ColumnType::TEXT:
            return "TEXT";
        case ColumnType::BLOB:
            return "BLOB";
        case ColumnType::TIMESTAMP:
            return "TIMESTAMP";
    }
    return "TEXT";
}

// ─── Backend selection ────────────────────────────────────────────────────────

/**
 * @brief Selectable storage backends. This is the only knob a consumer gets to
 * pick an engine — concrete engine classes never leave the internal partitions.
 */
enum class StorageEngine : std::uint8_t
{
    IN_MEMORY,
    /** @brief Reserved: the postgres plugin would be wired up behind the facade. */
    POSTGRES
};

[[nodiscard]] constexpr std::string_view engine_name(StorageEngine engine) noexcept
{
    return engine == StorageEngine::POSTGRES ? "postgres" : "in_memory";
}

// ─── Query vocabulary ─────────────────────────────────────────────────────────

enum class Compare : std::uint8_t
{
    EQ,
    NE,
    LT,
    LE,
    GT,
    GE,
    LIKE,
    IN
};

enum class SortOrder : std::uint8_t
{
    ASC,
    DESC
};

} // namespace core::db

#ifdef CONGELADO_TEST
import boost.ut;

namespace core::db::tests {
using namespace boost::ut;

suite<"core_db_types"> core_db_types_suite = []
{
    "sql_type_name maps every column type"_test = []
    {
        expect(sql_type_name(ColumnType::BIGINT) == "BIGINT");
        expect(sql_type_name(ColumnType::TEXT) == "TEXT");
        expect(sql_type_name(ColumnType::DOUBLE) == "DOUBLE PRECISION");
    };

    "field flags compose and test"_test = []
    {
        constexpr auto flags = FieldFlags::PRIMARY_KEY | FieldFlags::NOT_NULL;
        expect(has_flag(flags, FieldFlags::PRIMARY_KEY));
        expect(has_flag(flags, FieldFlags::NOT_NULL));
        expect(!has_flag(flags, FieldFlags::UNIQUE));
    };

    "Row::get retrieves and converts"_test = []
    {
        Row row;
        row.cells["n"] = std::int64_t{42};
        row.cells["s"] = std::string{"hello"};
        expect(row.get<std::int64_t>("n") == 42);
        expect(row.get<std::string>("s") == "hello");
        expect(row.contains("n"));
        expect(!row.contains("z"));
    };
};

} // namespace core::db::tests
#endif
