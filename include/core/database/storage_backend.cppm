export module core_database:storage_backend;

import std;
import :db_value;
import :row;
import :query_result;
import :query_spec;
import :predicate;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

namespace core::db::detail {

/** @brief Instance functor: total ordering over DbValue alternatives. */
class CompareValues
{
public:
    [[nodiscard]] int operator()(const DbValue& a, const DbValue& b) const noexcept
    {
        return std::visit(
            [&](const auto& x, const auto& y) noexcept -> int {
                using X = std::decay_t<decltype(x)>;
                using Y = std::decay_t<decltype(y)>;
                if constexpr (std::same_as<X, Null> || std::same_as<Y, Null>) {
                    return a.index() < b.index() ? -1 : (a.index() > b.index() ? 1 : 0);
                } else if constexpr (std::is_arithmetic_v<X> && std::is_arithmetic_v<Y>) {
                    const long double l = static_cast<long double>(x);
                    const long double r = static_cast<long double>(y);
                    return l < r ? -1 : (l > r ? 1 : 0);
                } else if constexpr (std::same_as<X, Y>) {
                    return x < y ? -1 : (y < x ? 1 : 0);
                } else {
                    DbValueLiteral literal;
                    const auto l = literal(DbValue{x});
                    const auto r = literal(DbValue{y});
                    return l < r ? -1 : (l > r ? 1 : 0);
                }
            },
            a,
            b
        );
    }
};

/** @brief Instance functor evaluating a predicate against a row. */
class RowMatches
{
public:
    [[nodiscard]] bool operator()(const Row& row, const Predicate& predicate) const noexcept
    {
        const auto it = row.cells.find(predicate.column);
        if (it == row.cells.end()) {
            return false;
        }
        CompareValues compare;
        const int cmp = compare(it->second, predicate.value);
        if (predicate.comparison == Compare::LIKE) {
            const auto* text = std::get_if<std::string>(&it->second);
            const auto* pattern = std::get_if<std::string>(&predicate.value);
            if (!text || !pattern) {
                return false;
            }
            std::string p = *pattern;
            const bool leading = !p.empty() && p.front() == '%';
            const bool trailing = !p.empty() && p.back() == '%';
            if (leading) {
                p.erase(p.begin());
            }
            if (trailing && !p.empty()) {
                p.pop_back();
            }
            return leading && trailing ? text->find(p) != std::string::npos
                 : leading ? text->ends_with(p)
                 : trailing ? text->starts_with(p)
                            : *text == p;
        }
        switch (predicate.comparison) {
            case Compare::LIKE:
                return false;
            case Compare::EQ:
            case Compare::IN:
                return cmp == 0;
            case Compare::NE:
                return cmp != 0;
            case Compare::LT:
                return cmp < 0;
            case Compare::LE:
                return cmp <= 0;
            case Compare::GT:
                return cmp > 0;
            case Compare::GE:
                return cmp >= 0;
        }
        return false;
    }
};

/**
 * @brief The backend contract the facade talks to. Purely internal — concrete
 * engines and this interface never leave the module.
 */
class StorageBackend
{
public:
    virtual ~StorageBackend() = default;

    [[nodiscard]] virtual std::string_view name() const noexcept = 0;
    [[nodiscard]] virtual bool is_open() const noexcept = 0;

    virtual void open() = 0;
    virtual void close() = 0;

    virtual void create_table(std::string_view table) = 0;
    [[nodiscard]] virtual QueryResult run(const QuerySpec& spec) = 0;
    virtual std::uint64_t insert(std::string_view table, const std::map<std::string, DbValue>& values) = 0;
    virtual std::uint64_t update(
        std::string_view table,
        const std::map<std::string, DbValue>& values,
        std::string_view key,
        const DbValue& key_value
    ) = 0;
    virtual std::uint64_t remove(
        std::string_view table,
        std::string_view key,
        const DbValue& key_value
    ) = 0;
};

} // namespace core::db::detail

#ifdef CONGELADO_TEST
namespace core::db::detail::tests {
using namespace boost::ut;

suite<"core_db_storage_backend"> core_db_storage_backend_suite = []
{
    "backend contract is abstract"_test = []
    {
        expect(std::is_abstract_v<StorageBackend>);
    };

    "predicates evaluate against rows"_test = []
    {
        Row row;
        row.cells["age"] = std::int64_t{30};
        row.cells["name"] = std::string{"Ada"};
        RowMatches matches;
        CompareValues compare;
        expect(matches(row, Predicate{"age", Compare::GE, to_db_value(18)}));
        expect(!matches(row, Predicate{"age", Compare::LT, to_db_value(18)}));
        expect(matches(row, Predicate{"name", Compare::LIKE, to_db_value(std::string{"%da"})}));
        expect(compare(to_db_value(2), to_db_value(1)) > 0);
    };
};

} // namespace core::db::detail::tests
#endif
