export module core_database:in_memory_backend;

import std;
import :db_value;
import :row;
import :query_result;
import :query_spec;
import :storage_backend;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

namespace core::db::detail {

/** @brief Reference backend: tables are vectors of rows held in process memory. */
class InMemoryBackend final : public StorageBackend
{
public:
    [[nodiscard]] std::string_view name() const noexcept override
    {
        return "in_memory";
    }

    [[nodiscard]] bool is_open() const noexcept override
    {
        return m_open;
    }

    void open() override
    {
        m_open = true;
    }

    void close() override
    {
        m_open = false;
        m_tables.clear();
    }

    void create_table(std::string_view table) override
    {
        m_tables.try_emplace(std::string{table});
    }

    [[nodiscard]] QueryResult run(const QuerySpec& spec) override
    {
        QueryResult result;
        const auto it = m_tables.find(spec.table);
        if (it == m_tables.end()) {
            result.status = QueryResult::Status::NOOP;
            return result;
        }

        std::vector<Row> matched;
        for (const Row& row: it->second) {
            bool ok = true;
            for (const Predicate& predicate: spec.predicates) {
                if (!RowMatches{}(row, predicate)) {
                    ok = false;
                    break;
                }
            }
            if (ok) {
                matched.push_back(row);
            }
        }

        if (spec.ordering) {
            const std::string& column = spec.ordering->first;
            std::ranges::stable_sort(matched, [&](const Row& a, const Row& b) {
                const DbValue lhs = a.cells.count(column) ? a.cells.at(column) : DbValue{Null{}};
                const DbValue rhs = b.cells.count(column) ? b.cells.at(column) : DbValue{Null{}};
                const int cmp = CompareValues{}(lhs, rhs);
                return spec.ordering->second == SortOrder::DESC ? cmp > 0 : cmp < 0;
            });
        }
        if (spec.offset_count) {
            const std::size_t skip = std::min(*spec.offset_count, matched.size());
            matched.erase(matched.begin(), matched.begin() + static_cast<std::ptrdiff_t>(skip));
        }
        if (spec.limit_count && matched.size() > *spec.limit_count) {
            matched.resize(*spec.limit_count);
        }

        for (const Row& row: matched) {
            result.rows.push_back(project(row, spec.columns));
        }
        result.rows_affected = static_cast<std::uint64_t>(result.rows.size());
        return result;
    }

    std::uint64_t insert(
        std::string_view table,
        const std::map<std::string, DbValue>& values
    ) override
    {
        auto& rows = m_tables[std::string{table}];
        Row row;
        row.cells = values;
        rows.push_back(std::move(row));
        return m_next_id++;
    }

    std::uint64_t update(
        std::string_view table,
        const std::map<std::string, DbValue>& values,
        std::string_view key,
        const DbValue& key_value
    ) override
    {
        auto& rows = m_tables[std::string{table}];
        const Predicate predicate{std::string{key}, Compare::EQ, key_value};
        std::uint64_t count = 0;
        for (Row& row: rows) {
            if (RowMatches{}(row, predicate)) {
                for (const auto& [column, value]: values) {
                    row.cells[column] = value;
                }
                ++count;
            }
        }
        return count;
    }

    std::uint64_t remove(
        std::string_view table,
        std::string_view key,
        const DbValue& key_value
    ) override
    {
        auto& rows = m_tables[std::string{table}];
        const Predicate predicate{std::string{key}, Compare::EQ, key_value};
        const auto before = rows.size();
        std::erase_if(rows, [&](const Row& row) { return RowMatches{}(row, predicate); });
        return static_cast<std::uint64_t>(before - rows.size());
    }

private:
    [[nodiscard]] Row project(const Row& source, const std::vector<std::string>& columns)
    {
        if (columns.empty()) {
            return source;
        }
        Row out;
        for (const std::string& column: columns) {
            if (const auto it = source.cells.find(column); it != source.cells.end()) {
                out.cells.emplace(column, it->second);
            }
        }
        return out;
    }

    std::map<std::string, std::vector<Row>> m_tables{};
    std::uint64_t m_next_id{1};
    bool m_open{false};
};

} // namespace core::db::detail

#ifdef CONGELADO_TEST
namespace core::db::detail::tests {
using namespace boost::ut;

suite<"core_db_in_memory_backend"> core_db_in_memory_backend_suite = []
{
    "insert, query, update and remove"_test = []
    {
        InMemoryBackend backend;
        backend.open();
        backend.create_table("users");
        expect(backend.is_open());
        expect(backend.name() == "in_memory");

        Row a;
        a.cells["id"] = std::int64_t{1};
        a.cells["name"] = std::string{"Ada"};
        Row b;
        b.cells["id"] = std::int64_t{2};
        b.cells["name"] = std::string{"Grace"};
        backend.insert("users", a.cells);
        backend.insert("users", b.cells);

        QuerySpec spec;
        spec.table = "users";
        spec.columns = {"id", "name"};
        spec.predicates.push_back(Predicate{"id", Compare::GE, to_db_value(1)});
        spec.ordering = std::make_pair(std::string{"id"}, SortOrder::DESC);
        const QueryResult result = backend.run(spec);
        expect(result.rows.size() == 2);
        expect(result.rows[0].get<std::string>("name") == "Grace");

        expect(backend.update("users", {{"name", to_db_value(std::string{"A"})}}, "id",
                              to_db_value(1)) == 1);
        expect(backend.remove("users", "id", to_db_value(2)) == 1);
        expect(backend.run(spec).rows.size() == 1);
    };
};

} // namespace core::db::detail::tests
#endif
