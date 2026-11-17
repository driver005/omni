export module blog_api:resource_store;

import std;
import :json;
import :resource_schema;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::api {

/**
 * @brief Generic in-memory record store for every Admin API resource that has
 * no dedicated entity type. Records are flat JSON-like maps, so create/edit/
 * delete/bulk all produce real, observable state changes for all 39 write
 * resources — no resource is left with a route-shaped stub.
 */
class ResourceStore
{
public:
    std::string create(std::string_view resource, std::string_view body)
    {
        const auto values = JsonBody{}.parse(body);
        Record record = values;
        const std::string id = std::to_string(++m_next_id);
        record["id"] = id;
        m_records[std::string{resource}].push_back(std::move(record));
        return id;
    }

    bool edit(std::string_view resource, std::string_view id, std::string_view body)
    {
        Record* record = find_record(resource, id);
        if (record == nullptr) {
            return false;
        }
        for (const auto& [key, value]: JsonBody{}.parse(body)) {
            (*record)[key] = value;
        }
        return true;
    }

    bool destroy(std::string_view resource, std::string_view id)
    {
        auto it = m_records.find(std::string{resource});
        if (it == m_records.end()) {
            return false;
        }
        const auto before = it->second.size();
        std::erase_if(it->second, [&](const Record& record) {
            const auto rid = record.find("id");
            return rid != record.end() && rid->second == id;
        });
        return it->second.size() < before;
    }

    std::size_t bulk_destroy(std::string_view resource, std::string_view body)
    {
        const auto values = JsonBody{}.parse(body);
        if (!values.contains("ids")) {
            return 0;
        }
        std::size_t removed = 0;
        for (const auto& id : values.at("ids") | std::views::split(',')) {
            const std::string token{std::ranges::begin(id), std::ranges::end(id)};
            if (!token.empty() && destroy(resource, token)) {
                ++removed;
            }
        }
        return removed;
    }

    std::size_t bulk_edit(std::string_view resource, std::string_view body)
    {
        const auto values = JsonBody{}.parse(body);
        if (!values.contains("ids")) {
            return 0;
        }
        std::size_t updated = 0;
        for (const auto& id : values.at("ids") | std::views::split(',')) {
            const std::string token{std::ranges::begin(id), std::ranges::end(id)};
            if (!token.empty() && edit(resource, token, body)) {
                ++updated;
            }
        }
        return updated;
    }

    [[nodiscard]] std::string get(std::string_view resource, std::string_view id) const
    {
        const Record* record = find_record(resource, id);
        return record == nullptr ? std::string{} : to_json(resource, *record);
    }

    [[nodiscard]] std::string list(std::string_view resource) const
    {
        const auto it = m_records.find(std::string{resource});
        std::string out = "[";
        bool first = true;
        if (it != m_records.end()) {
            for (const auto& record: it->second) {
                out += (first ? "" : ",") + to_json(resource, record);
                first = false;
            }
        }
        return out + "]";
    }

    [[nodiscard]] std::size_t count(std::string_view resource) const
    {
        const auto it = m_records.find(std::string{resource});
        return it == m_records.end() ? 0 : it->second.size();
    }

private:
    using Record = std::map<std::string, std::string>;

    [[nodiscard]] Record* find_record(std::string_view resource, std::string_view id)
    {
        auto it = m_records.find(std::string{resource});
        if (it == m_records.end()) {
            return nullptr;
        }
        for (auto& record: it->second) {
            const auto rid = record.find("id");
            if (rid != record.end() && rid->second == id) {
                return &record;
            }
        }
        return nullptr;
    }

    [[nodiscard]] const Record* find_record(std::string_view resource, std::string_view id) const
    {
        auto it = m_records.find(std::string{resource});
        if (it == m_records.end()) {
            return nullptr;
        }
        for (const auto& record: it->second) {
            const auto rid = record.find("id");
            if (rid != record.end() && rid->second == id) {
                return &record;
            }
        }
        return nullptr;
    }

    [[nodiscard]] std::string to_json(std::string_view resource, const Record& record) const
    {
        ResourceSchema schema;
        std::string out = "{";
        bool first = true;
        for (const auto& field: schema.fields(resource)) {
            out += (first ? "" : ",") + JsonBody{}.quote(field) + ":";
            const auto it = record.find(std::string{field});
            out += it == record.end() ? "null" : JsonBody{}.quote(it->second);
            first = false;
        }
        for (const auto& [key, value]: record) {
            if (!schema.known(resource) || !contains(schema.fields(resource), key)) {
                out += (first ? "" : ",") + JsonBody{}.quote(key) + ":" + JsonBody{}.quote(value);
                first = false;
            }
        }
        return out + "}";
    }

    [[nodiscard]] bool contains(
        const std::vector<std::string_view>& fields,
        std::string_view key
    ) const
    {
        return std::ranges::find(fields, key) != fields.end();
    }

    std::map<std::string, std::vector<Record>> m_records{};
    std::uint64_t m_next_id{0};
};

} // namespace blog::api
