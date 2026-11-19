export module memory_backends:search;

import std;
import core_search;
import core_serde;
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
            auto& hit = hits.back();
            if (!query.field_names().empty()) {
                apply_projection(hit, document.body, query.field_names());
            }
            if (!query.include_names().empty()) {
                apply_includes(hit, query.include_names());
            }
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

    [[nodiscard]] static std::string fragment(const serde::Value& value)
    {
        return core_serde::JsonCodec{}.encode(value);
    }

    void apply_projection(
        SearchHit& hit,
        std::string_view document,
        const std::vector<std::string>& names
    ) const
    {
        const auto parsed = core_serde::JsonCodec{}.decode<serde::Value>(document);
        if (!parsed) return;
        const auto* object = std::get_if<serde::Value::Object>(&parsed->get());
        if (object == nullptr) return;
        hit.put("id", fragment(serde::Value{hit.id()}));
        for (const auto& name : names) {
            if (const auto field = object->get(name)) {
                hit.put(name, fragment(*field));
            }
        }
    }

    void apply_includes(SearchHit& hit, const std::vector<std::string>& names) const
    {
        for (const auto& name : names) {
            std::vector<SearchHit> related;
            for (const auto& [id, document] : m_documents) {
                if (document.type == name) {
                    related.emplace_back(id, document.type, 1.0, document.body.substr(0, 80));
                }
            }
            hit.embed(name, std::move(related));
        }
    }

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
