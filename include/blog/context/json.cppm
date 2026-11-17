export module blog_context:json;

import std;
import core_database;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::api {

/** @brief Minimal flat JSON object parser/serializer for admin API bodies. */
class JsonBody
{
public:
    [[nodiscard]] std::map<std::string, std::string> parse(std::string_view text) const
    {
        std::map<std::string, std::string> result;
        std::size_t i = 0;
        while (i < text.size()) {
            i = text.find('"', i);
            if (i == std::string_view::npos) {
                break;
            }
            const auto key_end = text.find('"', i + 1);
            if (key_end == std::string_view::npos) {
                break;
            }
            const std::string key{text.substr(i + 1, key_end - i - 1)};
            const auto colon = text.find(':', key_end + 1);
            if (colon == std::string_view::npos) {
                break;
            }
            std::size_t value_start = colon + 1;
            while (value_start < text.size() && std::isspace(static_cast<unsigned char>(text[value_start]))) {
                ++value_start;
            }
            std::string value;
            if (value_start < text.size() && text[value_start] == '"') {
                const auto value_end = text.find('"', value_start + 1);
                if (value_end == std::string_view::npos) {
                    break;
                }
                value = std::string{text.substr(value_start + 1, value_end - value_start - 1)};
                i = value_end + 1;
            } else {
                auto value_end = text.find_first_of(",}", value_start);
                if (value_end == std::string_view::npos) {
                    value_end = text.size();
                }
                value = std::string{text.substr(value_start, value_end - value_start)};
                i = value_end;
            }
            result[key] = value;
        }
        return result;
    }

    /**
     * @brief Like parse(), but keeps each value as its raw JSON fragment so
     * numbers, booleans and strings retain their type on re-serialization.
     */
    [[nodiscard]] std::map<std::string, std::string> parse_raw(std::string_view text) const
    {
        std::map<std::string, std::string> result;
        std::size_t i = 0;
        while (i < text.size()) {
            i = text.find('"', i);
            if (i == std::string_view::npos) {
                break;
            }
            const auto key_end = text.find('"', i + 1);
            if (key_end == std::string_view::npos) {
                break;
            }
            const std::string key{text.substr(i + 1, key_end - i - 1)};
            const auto colon = text.find(':', key_end + 1);
            if (colon == std::string_view::npos) {
                break;
            }
            std::size_t value_start = colon + 1;
            while (value_start < text.size() &&
                   std::isspace(static_cast<unsigned char>(text[value_start]))) {
                ++value_start;
            }
            if (value_start >= text.size()) {
                break;
            }
            std::string raw;
            if (text[value_start] == '"') {
                bool escaped = false;
                auto value_end = value_start + 1;
                for (; value_end < text.size(); ++value_end) {
                    if (escaped) {
                        escaped = false;
                    } else if (text[value_end] == '\\') {
                        escaped = true;
                    } else if (text[value_end] == '"') {
                        break;
                    }
                }
                if (value_end >= text.size()) {
                    break;
                }
                raw = std::string{text.substr(value_start, value_end - value_start + 1)};
                i = value_end + 1;
            } else {
                const auto comma = text.find(',', value_start);
                const auto brace = text.find('}', value_start);
                const auto stop = std::min(comma, brace);
                if (stop == std::string_view::npos) {
                    break;
                }
                raw = std::string{trim_view(text.substr(value_start, stop - value_start))};
                i = stop;
            }
            result[key] = std::move(raw);
        }
        return result;
    }

    [[nodiscard]] std::string_view trim_view(std::string_view text) const
    {
        while (!text.empty() && std::isspace(static_cast<unsigned char>(text.front()))) {
            text.remove_prefix(1);
        }
        while (!text.empty() && std::isspace(static_cast<unsigned char>(text.back()))) {
            text.remove_suffix(1);
        }
        return text;
    }

    [[nodiscard]] std::string quote(std::string_view text) const
    {
        std::string out = "\"";
        for (const char c: text) {
            if (c == '"' || c == '\\') {
                out += '\\';
                out += c;
            } else {
                out += c;
            }
        }
        return out + "\"";
    }
};

/** @brief Reflection-driven JSON conversion for one entity type. */
template<core::db::reflect::ReflectedEntity E>
class EntityJson
{
public:
    [[nodiscard]] E from_json(std::string_view text) const
    {
        const JsonBody parser;
        const auto values = parser.parse(text);
        E entity{};
        for (std::size_t i = 0; i < m_names.size(); ++i) {
            const auto it = values.find(std::string{m_names[i]});
            if (it != values.end()) {
                m_setters[i](entity, it->second);
            }
        }
        return entity;
    }

    [[nodiscard]] std::string to_json(const E& entity) const
    {
        std::string out = "{";
        for (std::size_t i = 0; i < m_names.size(); ++i) {
            if (i > 0) {
                out += ",";
            }
            out += JsonBody{}.quote(m_names[i]) + ":" + m_getters[i](entity);
        }
        return out + "}";
    }

private:
    template<std::size_t... I>
    [[nodiscard]] static std::array<std::string_view, sizeof...(I)> names(std::index_sequence<I...>)
    {
        return {std::string_view{core::db::reflect::field_at_t<E, I>::name}...};
    }

    template<std::size_t... I>
    [[nodiscard]] static std::array<std::function<void(E&, std::string_view)>, sizeof...(I)>
    setters(std::index_sequence<I...>)
    {
        return {+[](E& entity, std::string_view value) {
            using F = core::db::reflect::field_at_t<E, I>;
            using V = typename F::value_type;
            if constexpr (std::same_as<V, std::string>) {
                core::db::reflect::set_field(entity, F{}, std::string{value});
            } else if constexpr (std::same_as<V, bool>) {
                core::db::reflect::set_field(entity, F{}, value == "true" || value == "1");
            } else if constexpr (std::is_integral_v<V>) {
                try {
                    core::db::reflect::set_field(entity, F{}, static_cast<V>(std::stoll(std::string{value})));
                } catch (...) {
                }
            } else if constexpr (std::is_floating_point_v<V>) {
                try {
                    core::db::reflect::set_field(entity, F{}, static_cast<V>(std::stod(std::string{value})));
                } catch (...) {
                }
            }
        }...};
    }

    template<std::size_t... I>
    [[nodiscard]] static std::array<std::function<std::string(const E&)>, sizeof...(I)>
    getters(std::index_sequence<I...>)
    {
        return {+[](const E& entity) -> std::string {
            using F = core::db::reflect::field_at_t<E, I>;
            using V = typename F::value_type;
            const auto& value = core::db::reflect::get_field(entity, F{});
            if constexpr (std::same_as<V, std::string>) {
                return JsonBody{}.quote(value);
            } else if constexpr (std::same_as<V, bool>) {
                return value ? "true" : "false";
            } else if constexpr (std::is_arithmetic_v<V>) {
                return std::format("{}", value);
            } else {
                return "null";
            }
        }...};
    }

    std::array<std::string_view, core::db::reflect::field_count_v<E>> m_names{
        names(std::make_index_sequence<core::db::reflect::field_count_v<E>>{})
    };
    std::array<std::function<void(E&, std::string_view)>, core::db::reflect::field_count_v<E>>
        m_setters{setters(std::make_index_sequence<core::db::reflect::field_count_v<E>>{})};
    std::array<std::function<std::string(const E&)>, core::db::reflect::field_count_v<E>>
        m_getters{getters(std::make_index_sequence<core::db::reflect::field_count_v<E>>{})};
};

} // namespace blog::api
