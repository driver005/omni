export module blog_handlers:outbox_shim;

import std;

/**
 * @file outbox.cppm
 * @brief Transactional outbox for events / jobs / side effects (CONTRACTS.md §4).
 *
 * Design notes
 * ------------
 * - "Context-local" means all state (the pending queue AND the transaction stack)
 *   lives on the `Outbox` instance itself. There is no external DB and no shared
 *   global outbox state for transactions: two `Outbox` objects are two independent
 *   contexts and never observe each other's emits or transactions.
 * - `emit()` inside a transaction is staged, not queued. It becomes visible to
 *   `drain()`/`pending()` only on `commit()`; `rollback()` discards it. This is the
 *   observable transaction contract.
 * - Nested transactions are supported: an inner `commit()` merges its staged events
 *   into the parent transaction's buffer (or into the pending queue when no parent
 *   exists); an inner `rollback()` discards only its own buffer.
 * - `drain()` is the flush point: it hands every committed event to each registered
 *   `Adapter::dispatch()` and, where practical, reuses `core_events` by publishing
 *   the drained event onto the ambient event bus when one is configured.
 */

export namespace blog::handlers::outbox {

/** One record in the outbox: an event type, an opaque JSON-ish payload and the id of the
 * aggregate that produced it. Mirrors the Ghost "events / jobs / side effects" tuple. */
struct Event
{
    std::string type;
    std::string payload;
    std::string aggregate_id;
};

/** A sink for drained outbox events (e.g. a real external adapter — see `core_adapters`). */
struct Adapter
{
    virtual ~Adapter() = default;
    virtual void dispatch(const Event&) = 0;
};

namespace detail {

    /** @brief Optional host event sink, wired by the host instead of importing core_events here. */
    using EventSink = void (*)(const Event&);

    [[nodiscard]] inline EventSink& event_sink() noexcept
    {
        static EventSink sink{nullptr};
        return sink;
    }

    /** Process-wide adapter registry. Function-local static avoids static-initialisation order
     * issues; the registry is intentionally global because `register_adapter()` is a free
     * function with no `Outbox` context argument. */
    [[nodiscard]] inline std::vector<std::shared_ptr<Adapter>>& adapters()
    {
        static std::vector<std::shared_ptr<Adapter>> registry;
        return registry;
    }

} // namespace detail

/** Context-local transactional outbox. */
class Outbox
{
public:
    /** Adds an event. Inside a transaction it is staged; otherwise it is queued now. */
    void emit(Event event)
    {
        if (m_transactions.empty()) {
            m_pending.push_back(std::move(event));
        } else {
            m_transactions.back().push_back(std::move(event));
        }
    }

    /** Removes and returns every committed event in emit order, then clears the queue.
     * Staged (uncommitted) events are never returned. Each drained event is dispatched to
     * every registered adapter and, when an event bus is active, published on it. */
    [[nodiscard]] std::vector<Event> drain()
    {
        std::vector<Event> drained;
        drained.swap(m_pending);

        for (const Event& event: drained) {
            for (const auto& adapter: detail::adapters()) {
                if (adapter != nullptr) {
                    adapter->dispatch(event);
                }
            }
            if (detail::event_sink() != nullptr) {
                detail::event_sink()(event);
            }
        }
        return drained;
    }

    /** Opens a (possibly nested) transaction. Emits after this are staged. */
    void begin_transaction()
    {
        m_transactions.emplace_back();
    }

    /** Commits the innermost transaction: its staged events merge into the parent
     * transaction (if any) or into the pending queue, preserving emit order. No-op if
     * no transaction is open. */
    void commit()
    {
        if (m_transactions.empty()) {
            return;
        }
        auto staged = std::move(m_transactions.back());
        m_transactions.pop_back();
        auto& target = m_transactions.empty() ? m_pending : m_transactions.back();
        target.insert(
            target.end(),
            std::make_move_iterator(staged.begin()),
            std::make_move_iterator(staged.end())
        );
    }

    /** Rolls back the innermost transaction, discarding its staged events. No-op if no
     * transaction is open. */
    void rollback() noexcept
    {
        if (!m_transactions.empty()) {
            m_transactions.pop_back();
        }
    }

    /** Number of committed events currently waiting to be drained. Staged events are
     * deliberately excluded — transactions are observable through this value and `drain()`. */
    [[nodiscard]] std::size_t pending() const noexcept
    {
        return m_pending.size();
    }

private:
    std::vector<Event> m_pending;
    std::vector<std::vector<Event>> m_transactions; // stack; back() is innermost
};

/** Registers a global adapter that receives every event passed to `Outbox::drain()`.
 * Null adapters are ignored. */
inline void register_adapter(std::shared_ptr<Adapter> adapter)
{
    if (adapter != nullptr) {
        detail::adapters().push_back(std::move(adapter));
    }
}

/** @brief Host wiring: install a sink that forwards outbox events to the host bus. */
inline void set_event_sink(detail::EventSink sink) noexcept
{
    detail::event_sink() = sink;
}

/** @brief Process-wide outbox instance for adapter hooks and host integration. */
[[nodiscard]] inline Outbox& global_outbox() noexcept
{
    static Outbox instance;
    return instance;
}

} // namespace blog::handlers::outbox

// shared local query implementation
// local self-contained query/outbox implementations (avoid cross-partition imports)
namespace local_query {

struct ParsedQuery
{
    std::map<std::string, std::string> options;               // allowlisted Ghost options
    std::vector<std::pair<std::string, std::string>> filters; // field,value pairs
    std::string raw_filter;                                   // bare `filter` NQL
    std::string order;                                        // e.g. "created_at desc"
    std::size_t limit{15};
    std::size_t page{1};
    std::size_t offset{0};
    std::set<std::string> fields;  // empty = all
    std::set<std::string> include; // relations to eager-load
};

} // namespace local_query

// ---------------------------------------------------------------------------
// Module-internal implementation. Nothing in this namespace is exported.
// ---------------------------------------------------------------------------
namespace local_query::detail {

inline constexpr std::size_t kAll = std::numeric_limits<std::size_t>::max();

// ------------------------------- text helpers -----------------------------

[[nodiscard]] inline std::string trim(std::string_view s)
{
    const auto b = s.find_first_not_of(" \t\r\n");
    if (b == std::string_view::npos) {
        return {};
    }
    const auto e = s.find_last_not_of(" \t\r\n");
    return std::string{s.substr(b, e - b + 1)};
}

[[nodiscard]] inline std::string lower(std::string_view s)
{
    std::string out;
    out.reserve(s.size());
    for (const unsigned char c: s) {
        out.push_back(static_cast<char>(std::tolower(c)));
    }
    return out;
}

[[nodiscard]] inline bool iequals(std::string_view a, std::string_view b)
{
    if (a.size() != b.size()) {
        return false;
    }
    for (std::size_t i = 0; i < a.size(); ++i) {
        if (std::tolower(static_cast<unsigned char>(a[i])) !=
            std::tolower(static_cast<unsigned char>(b[i]))) {
            return false;
        }
    }
    return true;
}

[[nodiscard]] inline bool all_digits(std::string_view s)
{
    if (s.empty()) {
        return false;
    }
    for (const char c: s) {
        if (c < '0' || c > '9') {
            return false;
        }
    }
    return true;
}

[[nodiscard]] inline std::size_t to_size(std::string_view s, std::size_t fallback)
{
    if (!all_digits(s)) {
        return fallback;
    }
    std::size_t value = 0;
    for (const char c: s) {
        const std::size_t digit = static_cast<std::size_t>(c - '0');
        if (value > (kAll - digit) / 10) {
            return kAll;
        }
        value = value * 10 + digit;
    }
    return value;
}

[[nodiscard]] inline bool ident_char(char c)
{
    const unsigned char u = static_cast<unsigned char>(c);
    return std::isalnum(u) != 0 || c == '_' || c == '.';
}

[[nodiscard]] inline bool is_ident(std::string_view s)
{
    if (s.empty()) {
        return false;
    }
    const unsigned char first = static_cast<unsigned char>(s.front());
    if (!(std::isalpha(first) != 0 || s.front() == '_')) {
        return false;
    }
    for (const char c: s) {
        if (!ident_char(c)) {
            return false;
        }
    }
    return true;
}

[[nodiscard]] inline std::vector<std::string> split_ws(std::string_view s)
{
    std::vector<std::string> out;
    std::size_t i = 0;
    while (i < s.size()) {
        while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i]))) {
            ++i;
        }
        const auto b = i;
        while (i < s.size() && !std::isspace(static_cast<unsigned char>(s[i]))) {
            ++i;
        }
        if (i > b) {
            out.emplace_back(s.substr(b, i - b));
        }
    }
    return out;
}

[[nodiscard]] inline std::vector<std::string> split_csv(std::string_view s)
{
    std::vector<std::string> out;
    for (;;) {
        const auto comma = s.find(',');
        auto part = trim(s.substr(0, comma));
        if (!part.empty()) {
            out.push_back(std::move(part));
        }
        if (comma == std::string_view::npos) {
            break;
        }
        s.remove_prefix(comma + 1);
    }
    return out;
}

[[nodiscard]] inline int hex_value(char c)
{
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }
    if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    }
    return -1;
}

/** @brief Decode percent escapes. `+` is left literal: in Ghost NQL it is the
 *         AND operator, not a space. Spaces are normally sent as `%20`. */
[[nodiscard]] inline std::string url_decode(std::string_view s)
{
    std::string out;
    out.reserve(s.size());
    for (std::size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '%' && i + 2 < s.size()) {
            const int hi = hex_value(s[i + 1]);
            const int lo = hex_value(s[i + 2]);
            if (hi >= 0 && lo >= 0) {
                out.push_back(static_cast<char>((hi << 4) | lo));
                i += 2;
                continue;
            }
        }
        out.push_back(s[i]);
    }
    return out;
}

/** @brief Remove one pair of matching quotes and undo NQL's only escapes. */
[[nodiscard]] inline std::string strip_quotes(std::string_view s)
{
    std::string_view v = trim(s);
    if (v.size() >= 2 && (v.front() == '\'' || v.front() == '"') && v.back() == v.front()) {
        v = v.substr(1, v.size() - 2);
    }
    std::string out;
    out.reserve(v.size());
    for (std::size_t i = 0; i < v.size(); ++i) {
        if (v[i] == '\\' && i + 1 < v.size() && (v[i + 1] == '\'' || v[i + 1] == '"')) {
            out.push_back(v[i + 1]);
            ++i;
            continue;
        }
        out.push_back(v[i]);
    }
    return out;
}

// ------------------------------ JSON value --------------------------------

struct JsonValue
{
    enum class Type
    {
        Null,
        Bool,
        Number,
        String,
        Array,
        Object
    };

    Type type{Type::Null};
    bool boolean{false};
    std::string text;             // number lexeme or string content
    std::string member_key;       // set when value is an object member
    std::vector<JsonValue> array; // elements, or object members
};

// Each element is a member value carrying its key in `member_key`. Storing the
// key on the value keeps the recursive type complete (no vector of incomplete
// pair<string, JsonValue>).
using JsonObject = std::vector<JsonValue>;

class JsonParser
{
public:
    explicit JsonParser(std::string_view text) :
        m_text{text}
    {
    }

    [[nodiscard]] std::optional<JsonValue> parse()
    {
        skip_ws();
        auto value = parse_value();
        if (!value) {
            return std::nullopt;
        }
        skip_ws();
        if (m_pos != m_text.size()) {
            return std::nullopt;
        }
        return value;
    }

private:
    void skip_ws()
    {
        while (m_pos < m_text.size() && std::isspace(static_cast<unsigned char>(m_text[m_pos]))) {
            ++m_pos;
        }
    }

    [[nodiscard]] std::optional<JsonValue> parse_value()
    {
        skip_ws();
        if (m_pos >= m_text.size()) {
            return std::nullopt;
        }
        const char c = m_text[m_pos];
        if (c == '{') {
            return parse_object();
        }
        if (c == '[') {
            return parse_array();
        }
        if (c == '"') {
            auto s = parse_string();
            if (!s) {
                return std::nullopt;
            }
            JsonValue v;
            v.type = JsonValue::Type::String;
            v.text = std::move(*s);
            return v;
        }
        if (c == 't') {
            return parse_literal("true", true);
        }
        if (c == 'f') {
            return parse_literal("false", false);
        }
        if (c == 'n') {
            return parse_null();
        }
        if (c == '-' || (c >= '0' && c <= '9')) {
            return parse_number();
        }
        return std::nullopt;
    }

    [[nodiscard]] std::optional<JsonValue> parse_literal(std::string_view word, bool value)
    {
        if (m_text.substr(m_pos, word.size()) != word) {
            return std::nullopt;
        }
        m_pos += word.size();
        JsonValue v;
        v.type = JsonValue::Type::Bool;
        v.boolean = value;
        return v;
    }

    [[nodiscard]] std::optional<JsonValue> parse_null()
    {
        if (m_text.substr(m_pos, 4) != "null") {
            return std::nullopt;
        }
        m_pos += 4;
        return JsonValue{};
    }

    [[nodiscard]] std::optional<std::string> parse_string()
    {
        if (m_pos >= m_text.size() || m_text[m_pos] != '"') {
            return std::nullopt;
        }
        ++m_pos;
        std::string out;
        while (m_pos < m_text.size()) {
            const char c = m_text[m_pos++];
            if (c == '"') {
                return out;
            }
            if (c != '\\') {
                out.push_back(c);
                continue;
            }
            if (m_pos >= m_text.size()) {
                return std::nullopt;
            }
            const char e = m_text[m_pos++];
            switch (e) {
                case '"':
                    out.push_back('"');
                    break;
                case '\\':
                    out.push_back('\\');
                    break;
                case '/':
                    out.push_back('/');
                    break;
                case 'b':
                    out.push_back('\b');
                    break;
                case 'f':
                    out.push_back('\f');
                    break;
                case 'n':
                    out.push_back('\n');
                    break;
                case 'r':
                    out.push_back('\r');
                    break;
                case 't':
                    out.push_back('\t');
                    break;
                case 'u':
                    {
                        auto cp = parse_hex4();
                        if (!cp) {
                            return std::nullopt;
                        }
                        std::uint32_t code = *cp;
                        if (code >= 0xD8'00 && code <= 0xDB'FF && m_pos + 1 < m_text.size() &&
                            m_text[m_pos] == '\\' && m_text[m_pos + 1] == 'u') {
                            m_pos += 2;
                            auto low = parse_hex4();
                            if (!low || *low < 0xDC'00 || *low > 0xDF'FF) {
                                return std::nullopt;
                            }
                            code = 0x1'00'00 + ((code - 0xD8'00) << 10) + (*low - 0xDC'00);
                        }
                        append_utf8(out, code);
                        break;
                    }
                default:
                    return std::nullopt;
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] std::optional<std::uint32_t> parse_hex4()
    {
        if (m_pos + 4 > m_text.size()) {
            return std::nullopt;
        }
        std::uint32_t value = 0;
        for (int i = 0; i < 4; ++i) {
            const int n = hex_value(m_text[m_pos++]);
            if (n < 0) {
                return std::nullopt;
            }
            value = value * 16 + static_cast<std::uint32_t>(n);
        }
        return value;
    }

    static void append_utf8(std::string& out, std::uint32_t cp)
    {
        if (cp <= 0x7F) {
            out.push_back(static_cast<char>(cp));
        } else if (cp <= 0x7'FF) {
            out.push_back(static_cast<char>(0xC0 | (cp >> 6)));
            out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
        } else if (cp <= 0xFF'FF) {
            out.push_back(static_cast<char>(0xE0 | (cp >> 12)));
            out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
            out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
        } else {
            out.push_back(static_cast<char>(0xF0 | (cp >> 18)));
            out.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
            out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
            out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
        }
    }

    [[nodiscard]] std::optional<JsonValue> parse_number()
    {
        const auto begin = m_pos;
        if (m_text[m_pos] == '-') {
            ++m_pos;
        }
        while (m_pos < m_text.size() &&
               ((m_text[m_pos] >= '0' && m_text[m_pos] <= '9') || m_text[m_pos] == '.' ||
                m_text[m_pos] == 'e' || m_text[m_pos] == 'E' || m_text[m_pos] == '+' ||
                m_text[m_pos] == '-')) {
            ++m_pos;
        }
        if (m_pos == begin) {
            return std::nullopt;
        }
        JsonValue v;
        v.type = JsonValue::Type::Number;
        v.text = std::string{m_text.substr(begin, m_pos - begin)};
        return v;
    }

    [[nodiscard]] std::optional<JsonValue> parse_array()
    {
        ++m_pos; // '['
        JsonValue v;
        v.type = JsonValue::Type::Array;
        skip_ws();
        if (m_pos < m_text.size() && m_text[m_pos] == ']') {
            ++m_pos;
            return v;
        }
        for (;;) {
            auto item = parse_value();
            if (!item) {
                return std::nullopt;
            }
            v.array.push_back(std::move(*item));
            skip_ws();
            if (m_pos >= m_text.size()) {
                return std::nullopt;
            }
            if (m_text[m_pos] == ']') {
                ++m_pos;
                return v;
            }
            if (m_text[m_pos] != ',') {
                return std::nullopt;
            }
            ++m_pos;
        }
    }

    [[nodiscard]] std::optional<JsonValue> parse_object()
    {
        ++m_pos; // '{'
        JsonValue v;
        v.type = JsonValue::Type::Object;
        skip_ws();
        if (m_pos < m_text.size() && m_text[m_pos] == '}') {
            ++m_pos;
            return v;
        }
        for (;;) {
            skip_ws();
            auto key = parse_string();
            if (!key) {
                return std::nullopt;
            }
            skip_ws();
            if (m_pos >= m_text.size() || m_text[m_pos] != ':') {
                return std::nullopt;
            }
            ++m_pos;
            auto value = parse_value();
            if (!value) {
                return std::nullopt;
            }
            value->member_key = std::move(*key);
            v.array.push_back(std::move(*value));
            skip_ws();
            if (m_pos >= m_text.size()) {
                return std::nullopt;
            }
            if (m_text[m_pos] == '}') {
                ++m_pos;
                return v;
            }
            if (m_text[m_pos] != ',') {
                return std::nullopt;
            }
            ++m_pos;
        }
    }

    std::string_view m_text;
    std::size_t m_pos{0};
};

[[nodiscard]] inline std::vector<JsonObject> parse_json_array(std::string_view text)
{
    JsonParser parser{text};
    auto value = parser.parse();
    if (!value || value->type != JsonValue::Type::Array) {
        return {};
    }
    std::vector<JsonObject> out;
    out.reserve(value->array.size());
    for (auto& item: value->array) {
        if (item.type == JsonValue::Type::Object) {
            out.push_back(std::move(item.array));
        }
    }
    return out;
}

inline void write_json_string(std::string& out, std::string_view s)
{
    out.push_back('"');
    for (const unsigned char c: s) {
        switch (c) {
            case '"':
                out += "\\\"";
                break;
            case '\\':
                out += "\\\\";
                break;
            case '\b':
                out += "\\b";
                break;
            case '\f':
                out += "\\f";
                break;
            case '\n':
                out += "\\n";
                break;
            case '\r':
                out += "\\r";
                break;
            case '\t':
                out += "\\t";
                break;
            default:
                if (c < 0x20) {
                    out += std::format("\\u{:04x}", static_cast<unsigned>(c));
                } else {
                    out.push_back(static_cast<char>(c));
                }
        }
    }
    out.push_back('"');
}

[[nodiscard]] inline std::string serialize(const JsonValue& v)
{
    switch (v.type) {
        case JsonValue::Type::Null:
            return "null";
        case JsonValue::Type::Bool:
            return v.boolean ? "true" : "false";
        case JsonValue::Type::Number:
            return v.text;
        case JsonValue::Type::String:
            {
                std::string out;
                write_json_string(out, v.text);
                return out;
            }
        case JsonValue::Type::Array:
            {
                std::string out{"["};
                for (std::size_t i = 0; i < v.array.size(); ++i) {
                    if (i) {
                        out.push_back(',');
                    }
                    out += serialize(v.array[i]);
                }
                out.push_back(']');
                return out;
            }
        case JsonValue::Type::Object:
            {
                std::string out{"{"};
                for (std::size_t i = 0; i < v.array.size(); ++i) {
                    if (i) {
                        out.push_back(',');
                    }
                    write_json_string(out, v.array[i].member_key);
                    out.push_back(':');
                    out += serialize(v.array[i]);
                }
                out.push_back('}');
                return out;
            }
    }
    return "null";
}

[[nodiscard]] inline std::string
serialize(const JsonObject& obj, const std::set<std::string>& fields)
{
    std::string out{"{"};
    bool first = true;
    for (const auto& value: obj) {
        if (!fields.empty() && !fields.contains(value.member_key)) {
            continue;
        }
        if (!first) {
            out.push_back(',');
        }
        first = false;
        write_json_string(out, value.member_key);
        out.push_back(':');
        out += serialize(value);
    }
    out.push_back('}');
    return out;
}

[[nodiscard]] inline std::string
serialize_array(const std::vector<JsonObject>& rows, const std::set<std::string>& fields)
{
    std::string out{"["};
    for (std::size_t i = 0; i < rows.size(); ++i) {
        if (i) {
            out.push_back(',');
        }
        out += serialize(rows[i], fields);
    }
    out.push_back(']');
    return out;
}

[[nodiscard]] inline const JsonValue* find_field(const JsonObject& obj, std::string_view key)
{
    for (const auto& value: obj) {
        if (value.member_key == key) {
            return &value;
        }
    }
    return nullptr;
}

[[nodiscard]] inline std::string scalar_text(const JsonValue& v)
{
    switch (v.type) {
        case JsonValue::Type::Null:
            return {};
        case JsonValue::Type::Bool:
            return v.boolean ? "true" : "false";
        case JsonValue::Type::Number:
        case JsonValue::Type::String:
            return v.text;
        default:
            return serialize(v);
    }
}

// ------------------------------- NQL --------------------------------------

/** @brief Split on a delimiter only at bracket depth zero and outside quotes. */
[[nodiscard]] inline std::vector<std::string> split_top(std::string_view s, char delim)
{
    std::vector<std::string> out;
    std::size_t start = 0;
    int depth = 0;
    char quote = 0;
    bool escaped = false;
    for (std::size_t i = 0; i < s.size(); ++i) {
        const char c = s[i];
        if (quote) {
            if (escaped) {
                escaped = false;
            } else if (c == '\\') {
                escaped = true;
            } else if (c == quote) {
                quote = 0;
            }
            continue;
        }
        if (c == '\'' || c == '"') {
            quote = c;
        } else if (c == '[') {
            ++depth;
        } else if (c == ']') {
            if (depth > 0) {
                --depth;
            }
        } else if (c == delim && depth == 0) {
            out.push_back(trim(s.substr(start, i - start)));
            start = i + 1;
        }
    }
    out.push_back(trim(s.substr(start)));
    return out;
}

[[nodiscard]] inline std::size_t find_top_colon(std::string_view s)
{
    int depth = 0;
    char quote = 0;
    bool escaped = false;
    for (std::size_t i = 0; i < s.size(); ++i) {
        const char c = s[i];
        if (quote) {
            if (escaped) {
                escaped = false;
            } else if (c == '\\') {
                escaped = true;
            } else if (c == quote) {
                quote = 0;
            }
            continue;
        }
        if (c == '\'' || c == '"') {
            quote = c;
        } else if (c == '[') {
            ++depth;
        } else if (c == ']') {
            if (depth > 0) {
                --depth;
            }
        } else if (c == ':' && depth == 0) {
            return i;
        }
    }
    return std::string_view::npos;
}

struct Predicate
{
    enum class Op
    {
        Eq,
        NotEq,
        Contains,
        Greater,
        GreaterEq,
        Less,
        LessEq,
        IsNull,
        NotNull
    };

    std::string field;
    Op op{Op::Eq};
    std::vector<std::string> values;
};

[[nodiscard]] inline Predicate parse_clause(std::string_view clause)
{
    Predicate p;
    const auto colon = find_top_colon(clause);
    if (colon == std::string_view::npos) {
        p.field = trim(clause);
        return p;
    }
    p.field = trim(clause.substr(0, colon));
    auto value = trim(clause.substr(colon + 1));

    if (value.size() >= 2 && value.front() == '[' && value.back() == ']') {
        auto parts = split_top(value.substr(1, value.size() - 2), ',');
        for (auto& part: parts) {
            auto item = strip_quotes(part);
            if (!item.empty()) {
                p.values.push_back(std::move(item));
            }
        }
        return p;
    }

    const bool quoted = value.size() >= 2 && (value.front() == '\'' || value.front() == '"') &&
                        value.back() == value.front();

    if (!quoted) {
        if (value.starts_with(">=")) {
            p.op = Predicate::Op::GreaterEq;
            value = trim(value.substr(2));
        } else if (value.starts_with("<=")) {
            p.op = Predicate::Op::LessEq;
            value = trim(value.substr(2));
        } else if (value.starts_with('>')) {
            p.op = Predicate::Op::Greater;
            value = trim(value.substr(1));
        } else if (value.starts_with('<')) {
            p.op = Predicate::Op::Less;
            value = trim(value.substr(1));
        } else if (value.starts_with('~')) {
            p.op = Predicate::Op::Contains;
            value = trim(value.substr(1));
        } else if (value.starts_with('-') || value.starts_with('!')) {
            p.op = Predicate::Op::NotEq;
            value = trim(value.substr(1));
        }

        if (iequals(value, "null")) {
            p.op = (p.op == Predicate::Op::NotEq) ? Predicate::Op::NotNull : Predicate::Op::IsNull;
            return p;
        }
    }

    p.values.push_back(strip_quotes(value));
    return p;
}

[[nodiscard]] inline bool compare_number(const JsonValue* v, std::string_view rhs, Predicate::Op op)
{
    if (!v || (v->type != JsonValue::Type::Number && v->type != JsonValue::Type::String)) {
        return false;
    }
    double left = 0.0;
    double right = 0.0;
    auto [lptr, lec] = std::from_chars(v->text.data(), v->text.data() + v->text.size(), left);
    auto [rptr, rec] = std::from_chars(rhs.data(), rhs.data() + rhs.size(), right);
    if (lec != std::errc{} || rec != std::errc{} || lptr != v->text.data() + v->text.size() ||
        rptr != rhs.data() + rhs.size()) {
        return false;
    }
    switch (op) {
        case Predicate::Op::Greater:
            return left > right;
        case Predicate::Op::GreaterEq:
            return left >= right;
        case Predicate::Op::Less:
            return left < right;
        case Predicate::Op::LessEq:
            return left <= right;
        default:
            return false;
    }
}

[[nodiscard]] inline bool matches(const JsonObject& obj, const Predicate& p)
{
    const JsonValue* v = find_field(obj, p.field);
    switch (p.op) {
        case Predicate::Op::IsNull:
            return v == nullptr || v->type == JsonValue::Type::Null;
        case Predicate::Op::NotNull:
            return v != nullptr && v->type != JsonValue::Type::Null;
        case Predicate::Op::Eq:
            {
                if (!v || v->type == JsonValue::Type::Null) {
                    return false;
                }
                const auto text = scalar_text(*v);
                for (const auto& value: p.values) {
                    if (iequals(text, value)) {
                        return true;
                    }
                }
                return false;
            }
        case Predicate::Op::NotEq:
            {
                if (!v || v->type == JsonValue::Type::Null) {
                    return true;
                }
                const auto text = scalar_text(*v);
                for (const auto& value: p.values) {
                    if (iequals(text, value)) {
                        return false;
                    }
                }
                return true;
            }
        case Predicate::Op::Contains:
            {
                if (!v || v->type == JsonValue::Type::Null) {
                    return false;
                }
                const auto text = lower(scalar_text(*v));
                for (const auto& value: p.values) {
                    if (text.contains(lower(value))) {
                        return true;
                    }
                }
                return false;
            }
        case Predicate::Op::Greater:
        case Predicate::Op::GreaterEq:
        case Predicate::Op::Less:
        case Predicate::Op::LessEq:
            return !p.values.empty() && compare_number(v, p.values.front(), p.op);
    }
    return false;
}

using OrGroups = std::vector<std::vector<Predicate>>;

[[nodiscard]] inline bool matches_any(const JsonObject& obj, const OrGroups& groups)
{
    for (const auto& group: groups) {
        bool all = true;
        for (const auto& predicate: group) {
            if (!matches(obj, predicate)) {
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

[[nodiscard]] inline OrGroups build_predicates(const ParsedQuery& q)
{
    std::vector<Predicate> explicit_predicates;
    for (const auto& [field, value]: q.filters) {
        Predicate p;
        p.field = trim(field);
        auto parsed = parse_clause(std::string{field} + ":" + value);
        p.op = parsed.op;
        p.values = std::move(parsed.values);
        explicit_predicates.push_back(std::move(p));
    }

    OrGroups groups;
    if (!q.raw_filter.empty()) {
        for (auto& group: split_top(q.raw_filter, ',')) {
            std::vector<Predicate> and_clauses;
            for (auto& clause: split_top(group, '+')) {
                auto predicate = parse_clause(clause);
                if (!predicate.field.empty()) {
                    and_clauses.push_back(std::move(predicate));
                }
            }
            if (!and_clauses.empty()) {
                groups.push_back(std::move(and_clauses));
            }
        }
    }

    if (groups.empty()) {
        groups.emplace_back();
    }
    for (auto& group: groups) {
        group.insert(group.end(), explicit_predicates.begin(), explicit_predicates.end());
    }
    return groups;
}

// -------------------------- restricted-field guard ------------------------

[[nodiscard]] inline bool restricted_token(std::string_view token)
{
    static constexpr std::string_view
        kRestricted[]{"password", "token", "gift_link_token", "secret"};
    const auto low = lower(token);
    std::size_t start = 0;
    for (;;) {
        const auto dot = low.find('.', start);
        const auto segment =
            low.substr(start, dot == std::string_view::npos ? std::string_view::npos : dot - start);
        if (!segment.empty()) {
            for (const auto r: kRestricted) {
                if (segment == r) {
                    return true;
                }
            }
        }
        if (dot == std::string_view::npos) {
            if (!segment.empty()) {
                for (const auto r: kRestricted) {
                    if (r.ends_with(segment)) {
                        return true; // suffix rule for unqualified fields
                    }
                }
            }
            break;
        }
        start = dot + 1;
    }
    return false;
}

[[nodiscard]] inline bool has_restricted(std::string_view nql)
{
    bool saw_colon = false;
    for (std::size_t i = 0; i < nql.size(); ++i) {
        if (nql[i] != ':') {
            continue;
        }
        saw_colon = true;
        std::size_t j = i;
        while (j > 0 && ident_char(nql[j - 1])) {
            --j;
        }
        const auto token = nql.substr(j, i - j);
        if (!token.empty() && restricted_token(token)) {
            return true;
        }
    }
    if (!saw_colon) {
        // No colon: treat the whole string as a list of order fields.
        std::size_t i = 0;
        while (i < nql.size()) {
            if (!ident_char(nql[i])) {
                ++i;
                continue;
            }
            const auto b = i;
            while (i < nql.size() && ident_char(nql[i])) {
                ++i;
            }
            if (restricted_token(nql.substr(b, i - b))) {
                return true;
            }
        }
    }
    return false;
}

[[nodiscard]] inline bool
has_restricted_filters(const std::vector<std::pair<std::string, std::string>>& filters)
{
    for (const auto& [field, value]: filters) {
        (void)value;
        if (restricted_token(field)) {
            return true;
        }
    }
    return false;
}

// ---------------------------- option validation ---------------------------

[[nodiscard]] inline bool valid_option_impl(std::string_view key, std::string_view value)
{
    const std::string base =
        key.starts_with("filter[") && key.ends_with("]") ? std::string{"filter"} : std::string{key};
    if (base == "limit") {
        return iequals(value, "all") || all_digits(value);
    }
    if (base == "page" || base == "offset") {
        return all_digits(value);
    }
    if (base == "order") {
        const auto text = trim(value);
        if (text.empty()) {
            return true;
        }
        for (const auto& clause: split_top(text, ',')) {
            const auto tokens = split_ws(clause);
            if (tokens.empty() || tokens.size() > 2) {
                return false;
            }
            if (!is_ident(tokens[0])) {
                return false;
            }
            if (tokens.size() == 2 && !iequals(tokens[1], "asc") && !iequals(tokens[1], "desc")) {
                return false;
            }
        }
        return true;
    }
    if (base == "fields" || base == "include") {
        if (trim(value).empty()) {
            return true;
        }
        for (const auto& field: split_csv(value)) {
            if (!is_ident(field)) {
                return false;
            }
        }
        return true;
    }
    if (base == "filter") {
        char quote = 0;
        int depth = 0;
        bool escaped = false;
        for (const char c: value) {
            if (c == '\0' || c == '\n' || c == '\r' || c == '\t') {
                return false;
            }
            if (quote) {
                if (escaped) {
                    escaped = false;
                } else if (c == '\\') {
                    escaped = true;
                } else if (c == quote) {
                    quote = 0;
                }
                continue;
            }
            if (c == '\'' || c == '"') {
                quote = c;
            } else if (c == '[') {
                ++depth;
            } else if (c == ']') {
                if (--depth < 0) {
                    return false;
                }
            }
        }
        return quote == 0 && depth == 0;
    }
    return value.size() <= 4'096 && value.find('\0') == std::string_view::npos;
}

// ------------------------------ paging maths ------------------------------

[[nodiscard]] inline std::size_t effective_limit(const ParsedQuery& q, std::size_t def)
{
    return q.limit == 0 ? def : q.limit;
}

[[nodiscard]] inline std::size_t page_size(const ParsedQuery& q, std::size_t def)
{
    const auto limit = effective_limit(q, def);
    return limit == kAll ? def : limit;
}

[[nodiscard]] inline std::size_t effective_page(const ParsedQuery& q, std::size_t def)
{
    if (q.page != 0) {
        return q.page;
    }
    if (q.offset == 0) {
        return 1;
    }
    const auto size = page_size(q, def);
    return size == 0 ? 1 : q.offset / size + 1;
}

[[nodiscard]] inline std::size_t effective_offset(const ParsedQuery& q, std::size_t def)
{
    if (q.offset != 0) {
        return q.offset;
    }
    const auto page = effective_page(q, def);
    return (page - 1) * page_size(q, def);
}

// ------------------------------ query pipeline ----------------------------

[[nodiscard]] inline std::vector<JsonObject>
select(std::string_view json_array, const ParsedQuery& q)
{
    auto rows = parse_json_array(json_array);
    const auto predicates = build_predicates(q);
    std::vector<JsonObject> filtered;
    for (auto& row: rows) {
        if (matches_any(row, predicates)) {
            filtered.push_back(std::move(row));
        }
    }

    const auto clauses = split_top(trim(q.order), ',');
    // Apply from last key to first so earlier keys stay primary under stable_sort.
    for (auto it = clauses.rbegin(); it != clauses.rend(); ++it) {
        const auto tokens = split_ws(*it);
        if (tokens.empty()) {
            continue;
        }
        const auto& field = tokens[0];
        const bool descending = tokens.size() > 1 && iequals(tokens[1], "desc");
        std::stable_sort(
            filtered.begin(),
            filtered.end(),
            [&](const JsonObject& a, const JsonObject& b)
            {
                const JsonValue* va = find_field(a, field);
                const JsonValue* vb = find_field(b, field);
                const bool na = !va || va->type == JsonValue::Type::Null;
                const bool nb = !vb || vb->type == JsonValue::Type::Null;
                if (na && nb) {
                    return false;
                }
                if (na != nb) {
                    return descending ? na : !na;
                }
                const bool numeric =
                    va->type == JsonValue::Type::Number && vb->type == JsonValue::Type::Number;
                if (numeric) {
                    double left = 0.0;
                    double right = 0.0;
                    std::from_chars(va->text.data(), va->text.data() + va->text.size(), left);
                    std::from_chars(vb->text.data(), vb->text.data() + vb->text.size(), right);
                    return descending ? left > right : left < right;
                }
                const auto left = lower(scalar_text(*va));
                const auto right = lower(scalar_text(*vb));
                return descending ? left > right : left < right;
            }
        );
    }
    return filtered;
}

[[nodiscard]] inline std::vector<JsonObject>
paginate(const std::vector<JsonObject>& rows, const ParsedQuery& q, std::size_t def)
{
    const auto offset = effective_offset(q, def);
    if (offset >= rows.size()) {
        return {};
    }
    const auto limit = effective_limit(q, def);
    const auto remaining = rows.size() - offset;
    const auto count = limit == kAll ? remaining : std::min(remaining, limit);
    return {
        rows.begin() + static_cast<std::ptrdiff_t>(offset),
        rows.begin() + static_cast<std::ptrdiff_t>(offset + count)
    };
}

[[nodiscard]] inline void guard(const ParsedQuery& q)
{
    if (has_restricted(q.raw_filter) || has_restricted(q.order) ||
        has_restricted_filters(q.filters)) {
        throw std::runtime_error{"restricted field used in Ghost query"};
    }
}

} // namespace local_query::detail

// ---------------------------------------------------------------------------
// Public API.
// ---------------------------------------------------------------------------
namespace local_query {

[[nodiscard]] inline ParsedQuery
parse(std::string_view path, std::span<const std::string_view> allowed_options)
{
    ParsedQuery q;
    std::set<std::string_view> allow{allowed_options.begin(), allowed_options.end()};

    auto query = path;
    if (const auto qpos = query.find('?'); qpos != std::string_view::npos) {
        query.remove_prefix(qpos + 1);
    }
    if (const auto hpos = query.find('#'); hpos != std::string_view::npos) {
        query = query.substr(0, hpos);
    }

    std::size_t start = 0;
    while (start <= query.size()) {
        const auto amp = query.find('&', start);
        const auto pair = query.substr(
            start,
            amp == std::string_view::npos ? std::string_view::npos : amp - start
        );
        if (!pair.empty()) {
            const auto eq = pair.find('=');
            const auto key =
                detail::url_decode(eq == std::string_view::npos ? pair : pair.substr(0, eq));
            const auto value = detail::url_decode(
                eq == std::string_view::npos ? std::string_view{} : pair.substr(eq + 1)
            );
            const bool is_bracket_filter = key.starts_with("filter[") && key.ends_with("]");
            const std::string base = is_bracket_filter ? std::string{"filter"} : key;
            const bool allowed =
                allow.contains(std::string_view{base}) || allow.contains(std::string_view{key});
            if (allowed && detail::valid_option_impl(key, value)) {
                q.options.insert_or_assign(key, value);
                if (key == "filter") {
                    q.raw_filter = value;
                } else if (is_bracket_filter) {
                    const auto field =
                        detail::trim(std::string_view{key}.substr(7, key.size() - 8));
                    if (!field.empty()) {
                        q.filters.emplace_back(field, value);
                    }
                } else if (key == "fields") {
                    for (auto& f: detail::split_csv(value)) {
                        q.fields.insert(std::move(f));
                    }
                } else if (key == "include") {
                    for (auto& i: detail::split_csv(value)) {
                        q.include.insert(std::move(i));
                    }
                } else if (key == "order") {
                    q.order = value;
                } else if (key == "limit") {
                    q.limit = detail::iequals(value, "all") ? detail::kAll
                                                            : detail::to_size(value, q.limit);
                } else if (key == "page") {
                    q.page = detail::to_size(value, q.page);
                    if (q.page == 0) {
                        q.page = 1;
                    }
                } else if (key == "offset") {
                    q.offset = detail::to_size(value, q.offset);
                }
            }
        }
        if (amp == std::string_view::npos) {
            break;
        }
        start = amp + 1;
    }
    return q;
}

[[nodiscard]] inline bool has_restricted_field(std::string_view nql)
{
    return detail::has_restricted(nql);
}

[[nodiscard]] inline bool valid_option(std::string_view key, std::string_view value)
{
    return detail::valid_option_impl(key, value);
}

[[nodiscard]] inline std::string
apply(std::string_view json_array, const ParsedQuery& q, std::size_t default_limit = 15)
{
    detail::guard(q);
    auto rows = detail::select(json_array, q);
    return detail::serialize_array(detail::paginate(rows, q, default_limit), q.fields);
}

[[nodiscard]] inline std::string
pagination_meta(std::size_t total, const ParsedQuery& q, std::size_t default_limit = 15)
{
    const auto limit = detail::effective_limit(q, default_limit);
    const auto shown_limit = limit == detail::kAll ? total : limit;
    const auto pages = shown_limit == 0 ? std::size_t{1} : (total + shown_limit - 1) / shown_limit;
    const auto page = detail::effective_page(q, default_limit);
    const auto next = page < pages ? page + 1 : 0;
    const auto prev = page > 1 ? page - 1 : 0;
    return std::format(
        R"({{"page":{},"limit":{},"pages":{},"total":{},"next":{},"prev":{}}})",
        page,
        shown_limit,
        pages,
        total,
        next == 0 ? std::string{"null"} : std::to_string(next),
        prev == 0 ? std::string{"null"} : std::to_string(prev)
    );
}

[[nodiscard]] inline std::string
run(std::string_view json_array, const ParsedQuery& q, std::size_t default_limit = 15)
{
    detail::guard(q);
    auto rows = detail::select(json_array, q);
    const auto total = rows.size();
    const auto page = detail::paginate(rows, q, default_limit);
    return std::format(
        R"({{"data":{},"meta":{{"pagination":{}}}}})",
        detail::serialize_array(page, q.fields),
        pagination_meta(total, q, default_limit)
    );
}

} // namespace local_query


namespace local_outbox = blog::handlers::outbox;

namespace local_mapping {
[[nodiscard]] inline std::string
wrap(std::string_view resource, std::string_view rows, std::string_view meta = {})
{
    return "{\"" + std::string{resource} + "\":" + std::string{rows} +
           (meta.empty() ? std::string{} : ",\"meta\":" + std::string{meta}) + "}";
}

[[nodiscard]] inline std::string clean(std::string_view json, std::string_view)
{
    return std::string{json};
}
} // namespace local_mapping

using Query = local_query::ParsedQuery;
