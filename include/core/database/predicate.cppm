export module core_database:predicate;

import std;
import :db_value;
import :compare;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::db {

/** @brief Instance functor escaping a SQL string literal. */
class QuoteSql
{
public:
    [[nodiscard]] std::string operator()(std::string_view text) const
    {
        std::string out = "'";
        for (const char c: text) {
            out += (c == '\'') ? "''" : std::string_view{&c, 1};
        }
        out += "'";
        return out;
    }
};

/** @brief Instance functor rendering a DbValue as a SQL literal. */
class DbValueLiteral
{
public:
    [[nodiscard]] std::string operator()(const DbValue& value) const
    {
        return std::visit(
            [this](const auto& v) -> std::string
            {
                using T = std::decay_t<decltype(v)>;
                if constexpr (std::same_as<T, Null>) {
                    return "NULL";
                } else if constexpr (std::same_as<T, bool>) {
                    return v ? "TRUE" : "FALSE";
                } else if constexpr (std::same_as<T, std::string>) {
                    return m_quote(v);
                } else if constexpr (std::same_as<T, std::vector<std::uint8_t>>) {
                    static constexpr char DIGITS[] = "0123456789ABCDEF";
                    std::string hex = "X'";
                    for (const std::uint8_t byte: v) {
                        hex += DIGITS[(byte >> 4) & 0xF];
                        hex += DIGITS[byte & 0xF];
                    }
                    return hex + "'";
                } else if constexpr (std::same_as<T, std::vector<std::string>>) {
                    std::string out = "(";
                    for (std::size_t i = 0; i < v.size(); ++i) {
                        out += (i == 0 ? "" : ",") + m_quote(v[i]);
                    }
                    return out + ")";
                } else if constexpr (std::same_as<T, std::chrono::system_clock::time_point>) {
                    return m_quote(std::format("{:%Y-%m-%d %H:%M:%S}", v));
                } else {
                    return std::format("{}", v);
                }
            },
            value
        );
    }

private:
    QuoteSql m_quote{};
};

/** @brief A single bound predicate in a query. */
class Predicate
{
public:
    std::string column{};
    Compare comparison{Compare::EQ};
    DbValue value{Null{}};

    [[nodiscard]] std::string to_sql() const
    {
        QuoteSql quote;
        DbValueLiteral literal;
        if (comparison == Compare::IN) {
            if (const auto* list = std::get_if<std::vector<std::string>>(&value)) {
                std::string out = "(";
                for (std::size_t i = 0; i < list->size(); ++i) {
                    out += (i == 0 ? "" : ",") + quote((*list)[i]);
                }
                return column + " IN " + out + ")";
            }
        }
        return column + " " + std::string{CompareSql{}(comparison)} + " " + literal(value);
    }
};

} // namespace core::db

#ifdef CONGELADO_TEST
namespace core::db::tests {
using namespace boost::ut;

suite<"core_db_predicate"> core_db_predicate_suite = []
{
    "renders comparisons and IN lists"_test = []
    {
        Predicate eq{"age", Compare::GE, to_db_value(18)};
        expect(eq.to_sql() == "age >= 18");
        Predicate name{"name", Compare::EQ, to_db_value(std::string{"O'Neil"})};
        expect(name.to_sql() == "name = 'O''Neil'");
        Predicate in{"id", Compare::IN, std::vector<std::string>{"a", "b"}};
        expect(in.to_sql() == "id IN ('a','b')");
    };
};

} // namespace core::db::tests
#endif
