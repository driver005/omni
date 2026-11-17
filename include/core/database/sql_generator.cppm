export module core_database:sql_generator;

import std;
import :column_type;
import :entity_meta;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::db::reflect {

/** @brief Turns reflected entity metadata into SQL — all compile-time driven. */
class SqlGenerator
{
public:
    /** @brief Comma-joined column list, e.g. `"id,name"`. */
    template<ReflectedEntity E>
    [[nodiscard]] std::string select_columns()
    {
        std::string out;
        [&]<std::size_t... I>(std::index_sequence<I...>) {
            ((out += (I == 0 ? "" : ",") +
                      std::string{std::string_view{field_at_t<E, I>::name}}),
             ...);
        }(std::make_index_sequence<field_count_v<E>>{});
        return out;
    }

    /** @brief Alias-qualified column list, e.g. `"u.id,u.name"`. */
    template<ReflectedEntity E>
    [[nodiscard]] std::string qualified_columns(std::string_view alias = {})
    {
        const std::string prefix = alias.empty() ? std::string{} : std::string{alias} + ".";
        std::string out;
        [&]<std::size_t... I>(std::index_sequence<I...>) {
            ((out += (I == 0 ? "" : ",") + prefix +
                      std::string{std::string_view{field_at_t<E, I>::name}}),
             ...);
        }(std::make_index_sequence<field_count_v<E>>{});
        return out;
    }

    /** @brief CREATE TABLE with kinds, NOT NULL, UNIQUE and composite PK. */
    template<ReflectedEntity E>
    [[nodiscard]] std::string create_table(bool if_not_exists = true)
    {
        std::string out = "CREATE TABLE ";
        if (if_not_exists) {
            out += "IF NOT EXISTS ";
        }
        out += std::string{EntityMeta<E>::table} + " (";

        std::vector<std::string> keys;
        [&]<std::size_t... I>(std::index_sequence<I...>) {
            auto append = [&](auto descriptor) {
                using F = decltype(descriptor);
                out += std::string{std::string_view{F::name}} + " " +
                       std::string{SqlTypeName{}(F::type)};
                if (F::not_null) {
                    out += " NOT NULL";
                }
                if (F::unique) {
                    out += " UNIQUE";
                }
                out += ",";
                if constexpr (F::primary_key) {
                    keys.emplace_back(std::string_view{F::name});
                }
            };
            (append(field_at_t<E, I>{}), ...);
        }(std::make_index_sequence<field_count_v<E>>{});

        if (!keys.empty()) {
            out += "PRIMARY KEY (";
            for (std::size_t i = 0; i < keys.size(); ++i) {
                out += (i == 0 ? "" : ",") + keys[i];
            }
            out += ")";
        } else if (!out.empty() && out.back() == ',') {
            out.pop_back();
        }
        out += ")";
        return out;
    }

    /** @brief DROP TABLE statement. */
    template<ReflectedEntity E>
    [[nodiscard]] std::string drop_table(bool if_exists = true)
    {
        return std::string{"DROP TABLE "} + (if_exists ? "IF EXISTS " : "") +
               std::string{EntityMeta<E>::table};
    }
};

} // namespace core::db::reflect

#ifdef CONGELADO_TEST
namespace core::db::reflect::tests {

struct SqlSample
{
    std::int64_t id{};
    std::string name{};
    bool active{};
};
CORE_DB_REFLECT(SqlSample, "sql_samples",
    CORE_DB_FIELD("id", &SqlSample::id, ColumnType::BIGINT, FieldFlags::PRIMARY_KEY),
    CORE_DB_FIELD("name", &SqlSample::name, ColumnType::TEXT, FieldFlags::NOT_NULL),
    CORE_DB_FIELD("active", &SqlSample::active, ColumnType::BOOLEAN));

using namespace boost::ut;

suite<"core_db_sql_generator"> core_db_sql_generator_suite = []
{
    "generates projections and DDL from metadata"_test = []
    {
        expect(SqlGenerator{}.select_columns<SqlSample>() == "id,name,active");
        expect(SqlGenerator{}.qualified_columns<SqlSample>("s") == "s.id,s.name,s.active");
        expect(SqlGenerator{}.drop_table<SqlSample>() == "DROP TABLE IF EXISTS sql_samples");
        const std::string ddl = SqlGenerator{}.create_table<SqlSample>();
        expect(ddl.contains("CREATE TABLE IF NOT EXISTS sql_samples"));
        expect(ddl.contains("id BIGINT NOT NULL"));
        expect(ddl.contains("PRIMARY KEY (id)"));
    };
};

} // namespace core::db::reflect::tests
#endif
