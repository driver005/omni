export module memory_backends:search;

import std;
import core_search;
import :query;
import :hit;
import :result;
import :backend;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

namespace core::search {

/** @brief Reference backend: substring search over in-memory documents. */
class MemorySearchBackend final : public SearchBackend
{
public:
    [[nodiscard]] std::string_view name() const noexcept override
    {
        return "memory";
    }

    void index(std::string id, std::string type, std::string document) override
    {
        m_documents.insert_or_assign(id, Document{std::move(type), std::move(document)});
    }

    bool remove(std::string_view id) override
    {
        return m_documents.erase(std::string{id}) != 0;
    }

    [[nodiscard]] SearchResult search(const SearchQuery& query) const override
    {
        std::vector<SearchHit> hits;
        for (const auto& [id, document]: m_documents) {
            if (!query.term().empty() &&
                document.body.find(query.term()) == std::string::npos) {
                continue;
            }
            if (const auto type = query.filter("type"); type && *type != document.type) {
                continue;
            }
            hits.emplace_back(id, document.type, 1.0, document.body.substr(0, 80));
        }
        const std::size_t total = hits.size();
        const std::size_t start = std::min(query.offset_count(), hits.size());
        if (start > 0) {
            hits.erase(hits.begin(), hits.begin() + static_cast<std::ptrdiff_t>(start));
        }
        if (hits.size() > query.limit_count()) {
            hits.resize(query.limit_count());
        }
        return SearchResult{std::move(hits), total};
    }

    [[nodiscard]] std::size_t size() const noexcept override
    {
        return m_documents.size();
    }

private:
    struct Document
    {
        std::string type{};
        std::string body{};
    };

    std::map<std::string, Document> m_documents{};
};

} // namespace core::search

#ifdef CONGELADO_TEST
namespace core::search::tests {
using namespace boost::ut;

suite<"core_search_memory_search_backend"> core_search_memory_search_backend_suite = []
{
    "indexes and searches documents"_test = []
    {
        MemorySearchBackend backend;
        backend.index("1", "post", "hello ghost world");
        backend.index("2", "page", "unrelated");
        expect(backend.size() == 2);
        const auto result = backend.search(SearchQuery{"ghost"});
        expect(result.total() == 1);
        expect(result.hits().size() == 1);
        expect(backend.remove("1"));
        expect(backend.size() == 1);
    };
};

} // namespace core::search::tests
#endif
