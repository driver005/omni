export module blog_api:query;

import std;
import :json;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::api {

/**
 * @brief Ghost query-string semantics for API browse endpoints.
 *
 * Supports:
 *  - `filter=status:published+featured:true` (AND within a group, `,` for OR)
 *  - `filter[status]=published` (legacy bracket form)
 *  - `fields=title,slug` (projection; `id` is always kept, as in Ghost)
 *  - `include=authors,tags` (embedding, applied by the caller via embed())
 */
class Query
{
public:
    /** @brief One flat JSON object: field name -> raw JSON value fragment. */
    using Record = std::map<std::string, std::string>;

    explicit Query(const std::map<std::string, std::string>& params)
    {
        m_fields = parse_list(value_of(params, "fields"));
        m_include = parse_list(value_of(params, "include"));

        for (const auto& group : split(value_of(params, "filter"), ',')) {
            std::vector<Condition> ands;
            for (const auto& condition : split(group, '+')) {
                const auto colon = condition.find(':');
                if (colon == std::string::npos) {
                    continue;
                }
                std::string value{trim(condition.substr(colon + 1))};
                if (value.size() >= 2 && value.front() == '\'' && value.back() == '\'') {
                    value = value.substr(1, value.size() - 2);
                }
                ands.emplace_back(std::string{trim(condition.substr(0, colon))}, std::move(value));
            }
            if (!ands.empty()) {
                m_filters.push_back(std::move(ands));
            }
        }

        for (const auto& [key, value] : params) {
            if (key.starts_with("filter[") && key.ends_with(']')) {
                m_filters.push_back({{key.substr(7, key.size() - 8), value}});
            }
        }
    }

    [[nodiscard]] const std::vector<std::string>& includes() const noexcept
    {
        return m_include;
    }

    /** @brief Filter and project a set of records, preserving input order. */
    [[nodiscard]] std::vector<Record> apply(const std::vector<Record>& records) const
    {
        std::vector<Record> out;
        for (const auto& record : records) {
            if (matches(record)) {
                out.push_back(project(record));
            }
        }
        return out;
    }

    /** @brief Add embedded relation arrays to every record for requested includes. */
    void embed(
        std::vector<Record>& records,
        const std::map<std::string, std::string>& available
    ) const
    {
        if (m_include.empty()) {
            return;
        }
        for (auto& record : records) {
            for (const auto& name : m_include) {
                if (const auto it = available.find(name); it != available.end()) {
                    record[name] = it->second;
                }
            }
        }
    }

    [[nodiscard]] std::string serialize(const std::vector<Record>& records) const
    {
        std::string out = "[";
        bool first = true;
        for (const auto& record : records) {
            out += (first ? "" : ",") + to_json(record);
            first = false;
        }
        return out + "]";
    }

    /** @brief Parse a flat JSON array into records with raw value fragments. */
    [[nodiscard]] static std::vector<Record> parse_array(std::string_view text)
    {
        std::vector<Record> out;
        std::size_t i = 0;
        while ((i = text.find('{', i)) != std::string_view::npos) {
            const auto end = find_object_end(text, i);
            if (end == std::string_view::npos) {
                break;
            }
            out.push_back(parse_object(text.substr(i, end - i + 1)));
            i = end + 1;
        }
        return out;
    }

    [[nodiscard]] static std::string to_json(const Record& record)
    {
        std::string out = "{";
        bool first = true;
        for (const auto& [key, value] : record) {
            out += (first ? "" : ",") + JsonBody{}.quote(key) + ":" + value;
            first = false;
        }
        return out + "}";
    }

private:
    using Condition = std::pair<std::string, std::string>;

    [[nodiscard]] bool matches(const Record& record) const
    {
        if (m_filters.empty()) {
            return true;
        }
        for (const auto& group : m_filters) {
            bool all = true;
            for (const auto& [field, expected] : group) {
                const auto it = record.find(field);
                if (it == record.end() || unquote(it->second) != expected) {
                    all = false;
                    break;
                }
            }
            if (all) {
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] Record project(const Record& record) const
    {
        std::vector<std::string> keys;
        if (record.contains("id")) {
            keys.push_back("id");
        }
        if (m_fields.empty()) {
            for (const auto& [key, value] : record) {
                (void)value;
                if (key != "id") {
                    keys.push_back(key);
                }
            }
        } else {
            for (const auto& field : m_fields) {
                if (field != "id" && record.contains(field)) {
                    keys.push_back(field);
                }
            }
        }
        Record out;
        for (const auto& key : keys) {
            out[key] = record.at(key);
        }
        return out;
    }

    [[nodiscard]] static std::string value_of(
        const std::map<std::string, std::string>& params,
        std::string_view key
    )
    {
        const auto it = params.find(std::string{key});
        return it == params.end() ? std::string{} : it->second;
    }

    [[nodiscard]] static std::vector<std::string> parse_list(std::string_view text)
    {
        std::vector<std::string> out;
        for (const auto& item : split(text, ',')) {
            const auto trimmed = trim(item);
            if (!trimmed.empty()) {
                out.emplace_back(trimmed);
            }
        }
        return out;
    }

    [[nodiscard]] static std::vector<std::string_view> split(
        std::string_view text,
        char delimiter
    )
    {
        std::vector<std::string_view> out;
        std::size_t start = 0;
        for (;;) {
            const auto pos = text.find(delimiter, start);
            if (pos == std::string_view::npos) {
                out.push_back(text.substr(start));
                break;
            }
            out.push_back(text.substr(start, pos - start));
            start = pos + 1;
        }
        return out;
    }

    [[nodiscard]] static std::string_view trim(std::string_view text)
    {
        while (!text.empty() && std::isspace(static_cast<unsigned char>(text.front()))) {
            text.remove_prefix(1);
        }
        while (!text.empty() && std::isspace(static_cast<unsigned char>(text.back()))) {
            text.remove_suffix(1);
        }
        return text;
    }

    [[nodiscard]] static std::string unquote(std::string_view raw)
    {
        if (raw.size() < 2 || raw.front() != '"' || raw.back() != '"') {
            return std::string{raw};
        }
        std::string out;
        for (std::size_t i = 1; i + 1 < raw.size(); ++i) {
            if (raw[i] == '\\' && i + 2 < raw.size()) {
                out += raw[++i];
            } else {
                out += raw[i];
            }
        }
        return out;
    }

    [[nodiscard]] static std::size_t find_quote_end(std::string_view text, std::size_t start)
    {
        bool escaped = false;
        for (std::size_t i = start; i < text.size(); ++i) {
            if (escaped) {
                escaped = false;
            } else if (text[i] == '\\') {
                escaped = true;
            } else if (text[i] == '"') {
                return i;
            }
        }
        return std::string_view::npos;
    }

    [[nodiscard]] static std::size_t find_object_end(std::string_view text, std::size_t start)
    {
        int depth = 0;
        bool in_string = false;
        bool escaped = false;
        for (std::size_t i = start; i < text.size(); ++i) {
            const char c = text[i];
            if (in_string) {
                if (escaped) {
                    escaped = false;
                } else if (c == '\\') {
                    escaped = true;
                } else if (c == '"') {
                    in_string = false;
                }
                continue;
            }
            if (c == '"') {
                in_string = true;
            } else if (c == '{') {
                ++depth;
            } else if (c == '}') {
                if (--depth == 0) {
                    return i;
                }
            }
        }
        return std::string_view::npos;
    }

    [[nodiscard]] static Record parse_object(std::string_view object)
    {
        Record record;
        std::size_t i = 1;
        while (i + 1 < object.size()) {
            while (i < object.size() && (std::isspace(static_cast<unsigned char>(object[i])) || object[i] == ',')) {
                ++i;
            }
            if (i >= object.size() || object[i] == '}') {
                break;
            }
            if (object[i] != '"') {
                ++i;
                continue;
            }
            const auto key_end = find_quote_end(object, i + 1);
            if (key_end == std::string_view::npos) {
                break;
            }
            std::string key{object.substr(i + 1, key_end - i - 1)};
            i = key_end + 1;
            while (i < object.size() && (std::isspace(static_cast<unsigned char>(object[i])) || object[i] == ':')) {
                ++i;
            }
            std::string raw;
            if (i < object.size() && object[i] == '"') {
                const auto value_end = find_quote_end(object, i + 1);
                if (value_end == std::string_view::npos) {
                    break;
                }
                raw = std::string{object.substr(i, value_end - i + 1)};
                i = value_end + 1;
            } else {
                const auto comma = object.find(',', i);
                const auto brace = object.find('}', i);
                const auto stop = std::min(comma, brace);
                if (stop == std::string_view::npos) {
                    break;
                }
                raw = std::string{trim(object.substr(i, stop - i))};
                i = stop;
            }
            record[std::move(key)] = std::move(raw);
        }
        return record;
    }

    std::vector<std::string> m_fields{};
    std::vector<std::string> m_include{};
    std::vector<std::vector<Condition>> m_filters{};
};

} // namespace blog::api

#ifdef CONGELADO_TEST
namespace blog::api::tests {
using namespace boost::ut;
suite<"blog_api_query"> blog_api_query_suite = []
{
    "filters, projects and embeds like Ghost"_test = []
    {
        const std::string array = R"([{"id":1,"title":"Hello","status":"published"},{"id":2,"title":"Second","status":"draft"}])";
        const auto records = Query::parse_array(array);
        expect(records.size() == 2_u);

        const Query draft{{{"filter", "status:draft"}}};
        const auto filtered = draft.apply(records);
        expect(filtered.size() == 1_u);
        expect(filtered[0].at("title") == R"("Second")");

        const Query projected{{{"fields", "title"}}};
        const auto subset = projected.apply(records);
        expect(subset.size() == 2_u);
        expect(subset[0].contains("id"));
        expect(subset[0].contains("title"));
        expect(!subset[0].contains("status"));

        Query embedded{{{"include", "authors"}}};
        auto with_embed = embedded.apply(records);
        embedded.embed(with_embed, {{"authors", R"([{"id":9,"name":"Ghost"}])"}});
        expect(Query::serialize(with_embed).contains(R"("authors":[)"));
    };
};
} // namespace blog::api::tests
#endif
