export module memory_backends:search;

import std;
import core_search;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

namespace core::search {

/** @brief Reference backend: substring search over in-memory documents. */
export class MemorySearchBackend final : public SearchBackend
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
        for (const auto& [id, document] : m_documents) {
            if (!query.term().empty() &&
                document.body.find(query.term()) == std::string::npos) {
                continue;
            }
            if (const auto type = query.filter("type"); type && *type != document.type) {
                continue;
            }
            const auto parsed = core_serde::JsonCodec{}.decode<serde::Value>(document.body);
            bool filters_ok = true;
            for (const auto& [key, value] : query.filters()) {
                if (key == "type") continue;
                if (!parsed || field_of(*parsed, key) != value) {
                    filters_ok = false;
                    break;
                }
            }
            if (!filters_ok) continue;
            if (!query.where_expression().empty() &&
                ( !parsed || !matches_where(*parsed, query.where_expression()))) {
                continue;
            }
            hits.emplace_back(id, document.type, 1.0, document.body.substr(0, 80));
            auto& hit = hits.back();
            if (!query.field_names().empty() && parsed) {
                apply_projection(hit, *parsed, query.field_names());
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

    [[nodiscard]] static std::string field_of(const serde::Value& doc, std::string_view key)
    {
        const auto* object = std::get_if<serde::Value::Object>(&doc.get());
        if (object == nullptr) return {};
        const auto field = object->get(std::string{key});
        if (!field) return {};
        if (const auto* text = std::get_if<std::string>(&field->get())) return *text;
        return fragment(*field);
    }

    [[nodiscard]] static bool matches_where(const serde::Value& doc, std::string_view expression)
    {
        bool any_group = false;
        for (;;) {
            const auto comma = expression.find(',');
            auto group = expression.substr(0, comma);
            bool all = true;
            for (;;) {
                const auto plus = group.find('+');
                const auto term = group.substr(0, plus);
                const auto colon = term.find(':');
                if (colon != std::string_view::npos) {
                    auto key = term.substr(0, colon);
                    auto value = term.substr(colon + 1);
                    char op = '=';
                    if (!value.empty() && (value.front() == '-' || value.front() == '~' ||
                                           value.front() == '>' || value.front() == '<')) {
                        op = value.front() == '-' ? '!' : value.front();
                        value.remove_prefix(1);
                    }
                    if (value.size() >= 2 && value.front() == '\'' && value.back() == '\'') {
                        value = value.substr(1, value.size() - 2);
                    }
                    const auto actual = field_of(doc, key);
                    std::string lowered = actual;
                    std::string needle{value};
                    std::ranges::transform(lowered, lowered.begin(), [](char c) { return static_cast<char>(std::tolower(static_cast<unsigned char>(c))); });
                    std::ranges::transform(needle, needle.begin(), [](char c) { return static_cast<char>(std::tolower(static_cast<unsigned char>(c))); });
                    const double left = std::strtod(actual.c_str(), nullptr);
                    const double right = std::strtod(value.data(), nullptr);
                    const bool hit = op == '!' ? actual != value
                                   : op == '~' ? lowered.find(needle) != std::string::npos
                                   : op == '>' ? left > right
                                   : op == '<' ? left < right
                                               : actual == value;
                    all = all && hit;
                }
                if (plus == std::string_view::npos) break;
                group.remove_prefix(plus + 1);
            }
            any_group = any_group || all;
            if (comma == std::string_view::npos) break;
            expression.remove_prefix(comma + 1);
        }
        return any_group;
    }

    void apply_projection(SearchHit& hit, const serde::Value& doc, const std::vector<std::string>& names) const
    {
        hit.put("id", fragment(serde::Value{hit.id()}));
        for (const auto& name : names) {
            const auto value = field_value(doc, name);
            if (value) hit.put(name, fragment(*value));
        }
    }

    [[nodiscard]] static std::optional<serde::Value> field_value(const serde::Value& doc, std::string_view key)
    {
        const auto* object = std::get_if<serde::Value::Object>(&doc.get());
        if (object == nullptr) return std::nullopt;
        const auto value = object->get(std::string{key});
        if (!value) return std::nullopt;
        return *value;
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
