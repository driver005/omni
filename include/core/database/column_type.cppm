export module core_database:column_type;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::db {

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

/** @brief Instance functor mapping a ColumnType to its SQL spelling. */
export class SqlTypeName
{
public:
    [[nodiscard]] constexpr std::string_view operator()(ColumnType type) const noexcept
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
};

} // namespace core::db

#ifdef CONGELADO_TEST
namespace core::db::tests {
using namespace boost::ut;

suite<"core_db_column_type"> core_db_column_type_suite = []
{
    "maps each column kind to SQL spelling"_test = []
    {
        expect(SqlTypeName{}(ColumnType::BIGINT) == "BIGINT");
        expect(SqlTypeName{}(ColumnType::DOUBLE) == "DOUBLE PRECISION");
        expect(SqlTypeName{}(ColumnType::BOOLEAN) == "BOOLEAN");
        expect(SqlTypeName{}(ColumnType::TIMESTAMP) == "TIMESTAMP");
    };
};

} // namespace core::db::tests
#endif
