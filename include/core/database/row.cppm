module;
#include <uuid.h>

export module core_database:row;

import std;
import :db_value;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::db {

/** @brief One result row, keyed by column name. */
class Row
{
public:
    std::map<std::string, DbValue> cells{};

    [[nodiscard]] std::size_t size() const noexcept
    {
        return cells.size();
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return cells.empty();
    }

    [[nodiscard]] bool contains(std::string_view column) const noexcept
    {
        return cells.find(std::string{column}) != cells.end();
    }

    /**
     * @brief Fetches a cell and converts to T. Supports the exact variant
     * alternatives plus numeric/bool/string conveniences for text-ish backends.
     * @throws std::out_of_range when the column is absent.
     * @throws std::bad_variant_access when the cell holds an incompatible type.
     */
    template<typename T>
    [[nodiscard]] T get(std::string_view column) const
    {
        const auto it = cells.find(std::string{column});
        if (it == cells.end()) {
            throw std::out_of_range(std::format("Row has no column '{}'", column));
        }
        const DbValue& value = it->second;
        if constexpr (std::same_as<T, std::string>) {
            if (const auto* s = std::get_if<std::string>(&value)) {
                return *s;
            }
        } else if constexpr (std::same_as<T, uuids::uuid>) {
            if (const auto* s = std::get_if<std::string>(&value)) {
                return uuids::uuid::from_string(*s).value_or(uuids::uuid{});
            }
        } else if constexpr (std::integral<T> && !std::same_as<T, bool>) {
            if (const auto* n = std::get_if<std::int64_t>(&value)) {
                return static_cast<T>(*n);
            }
            if (const auto* u = std::get_if<std::uint64_t>(&value)) {
                return static_cast<T>(*u);
            }
        } else if constexpr (std::same_as<T, bool>) {
            if (const auto* b = std::get_if<bool>(&value)) {
                return *b;
            }
        } else {
            return std::get<T>(value);
        }
        return T{};
    }

    /** @brief Fetches a cell or returns `fallback` on any failure. */
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

} // namespace core::db

#ifdef CONGELADO_TEST
namespace core::db::tests {
using namespace boost::ut;

suite<"core_db_row"> core_db_row_suite = []
{
    "stores, detects and converts cells"_test = []
    {
        Row row;
        expect(row.empty());
        row.cells["n"] = std::int64_t{42};
        row.cells["s"] = std::string{"hello"};
        expect(row.size() == 2);
        expect(row.contains("n"));
        expect(!row.contains("z"));
        expect(row.get<std::int64_t>("n") == 42);
        expect(row.get<std::string>("s") == "hello");
        expect(row.get_or<std::int64_t>("missing", 0) == 0);
    };
};

} // namespace core::db::tests
#endif
