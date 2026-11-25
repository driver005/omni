export module blog_handlers:upgrades_browse_step;
import :domain_all;
import std;
import core_workflow;
import core_serde;
import core_auth;
import interfaces;
import blog_handlers_middleware;
import :outbox_shim;

// ---------------------------------------------------------------------------
// Shared strict-parity support for the three upgrades routes. Kept in this
// already-registered partition (CONTRACTS.md: shared helpers must live in
// registered partitions; a dedicated `upgrades_support` partition needs the
// central BUILD/all.cppm pass and is recorded in the parity reports).
//
// Primitives used:
//   core_auth:ghost + blog_handlers_middleware:ghost_permission — permissions:true
//   blog_handlers:outbox_shim — observable host-adapter acceptance side effect
//   core_serde::JsonCodec — input/adapter JSON parsing
//   ctx.store() — config/settings overrides for the host adapter stand-in
//
// No `blog_handlers:mapping`, `core_adapters` or rate-limit target is a dep of
// this target; those new-dependency needs are recorded in the parity reports.
// ---------------------------------------------------------------------------
export namespace blog::handlers::upgrades::support {

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

    [[nodiscard]] inline std::string pagination(
        std::size_t total,
        std::size_t limit,
        std::size_t page,
        std::size_t pages,
        std::string_view prev,
        std::string_view next
    )
    {
        return std::format(
            R"("total":{},"limit":{},"page":{},"pages":{},"prev":{},"next":{})",
            total,
            limit,
            page,
            pages,
            prev.empty() ? std::string{"null"} : std::string{prev},
            next.empty() ? std::string{"null"} : std::string{next}
        );
    }
} // namespace local_mapping

using Output = std::map<std::string, std::string>;
using Object = serde::Value::Object;

inline constexpr std::string_view kDocName{"upgrades"};
inline constexpr std::string_view kNoopStatus{R"({"supported":false,"reason":"not-configured"})"};
inline constexpr std::string_view kUnavailableStatus{
    R"({"supported":true,"availability":"unavailable","one_click":[],"manual":[]})"
};
inline constexpr std::string_view kValidationHelp{
    "Generate a lowercase UUID v4 idempotency_key once and reuse it when retrying."
};

// Defined with the adapter model below; adapter_error needs it earlier.
[[nodiscard]] inline std::string adapter_option(blog::BlogContext& ctx, std::string_view name);

[[nodiscard]] inline std::string json_quote(std::string_view text)
{
    std::string out = "\"";
    for (const unsigned char c: text) {
        switch (c) {
            case '"':
                out += "\\\"";
                break;
            case '\\':
                out += "\\\\";
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
    out += '"';
    return out;
}

[[nodiscard]] inline std::string unquote(std::string_view value)
{
    if (value.size() >= 2 && value.front() == '"' && value.back() == '"') {
        return std::string{value.substr(1, value.size() - 2)};
    }
    return std::string{value};
}

[[nodiscard]] inline std::size_t key_count(const Object* object)
{
    std::size_t count{0};
    if (object != nullptr) {
        for (const auto& [key, value]: *object) {
            (void)key;
            (void)value;
            ++count;
        }
    }
    return count;
}

[[nodiscard]] inline const serde::Value* value_for(const Object* object, std::string_view key)
{
    if (object == nullptr) {
        return nullptr;
    }
    for (const auto& [name, value]: *object) {
        if (name == key) {
            return &value;
        }
    }
    return nullptr;
}

[[nodiscard]] inline bool has_key(const Object* object, std::string_view key)
{
    return value_for(object, key) != nullptr;
}

/** @brief JSON scalar as text; rfl::Generic keeps booleans as bool, not string. */
[[nodiscard]] inline std::string scalar_text(const serde::Value& value)
{
    if (const auto* text = std::get_if<std::string>(&value.get())) {
        return *text;
    }
    if (const auto* flag = std::get_if<bool>(&value.get())) {
        return *flag ? "true" : "false";
    }
    return {};
}

[[nodiscard]] inline std::string field(const Object* object, std::string_view key)
{
    const auto* value = value_for(object, key);
    return value == nullptr ? std::string{} : scalar_text(*value);
}

/** @brief Local equivalent of local_mapping::wrap (that partition is not registered yet). */
[[nodiscard]] inline std::string wrap(std::string_view rows)
{
    return "{\"upgrades\":[" + std::string{rows} + "]}";
}

[[nodiscard]] inline Output result(int status, std::string body)
{
    return {{"status", std::to_string(status)}, {"body", std::move(body)}};
}

/** @brief Ghost-shaped error envelope; `details` is raw JSON when present. */
[[nodiscard]] inline std::string error_envelope(
    std::string_view message,
    std::string_view type,
    std::string_view code,
    std::string_view help,
    std::string_view property = {},
    std::string_view details = {}
)
{
    static std::atomic<unsigned long long> counter{0};
    const auto id = std::format("{:032x}", ++counter);
    return std::format(
        R"({{"errors":[{{"id":"{}","code":{},"context":null,"details":{},"ghostErrorCode":null,"help":{},"message":{},"property":{},"type":{}}}]}})",
        id,
        code.empty() ? "null" : json_quote(code),
        details.empty() ? "null" : std::string{details},
        help.empty() ? "null" : json_quote(help),
        json_quote(message),
        property.empty() ? "null" : json_quote(property),
        json_quote(type)
    );
}

[[nodiscard]] inline Output validation_error(
    std::string_view message,
    std::string_view help = {},
    std::string_view property = {}
)
{
    return result(422, error_envelope(message, "ValidationError", {}, help, property));
}

[[nodiscard]] inline Output no_permission_error(std::string_view message)
{
    return result(403, error_envelope(message, "NoPermissionError", {}, {}, {}));
}

[[nodiscard]] inline Output adapter_error(blog::BlogContext& ctx, std::string_view code)
{
    struct Spec
    {
        std::string_view code;
        int status;
        std::string_view type;
        std::string_view message;
        std::string_view help;
        std::string_view ghost_code;
    };

    static constexpr std::array<Spec, 6> kSpecs{{
        {"unsupported",
         409,
         "DisabledFeatureError",
         "This host does not support updating Ghost from Admin.",
         "Ask your host to configure an upgrade adapter.",
         "UPGRADE_UNSUPPORTED"},
        {"unavailable",
         502,
         "InternalServerError",
         "The update service is temporarily unavailable.",
         "Retry later. Reuse the same idempotency_key if an update request lost its response.",
         "UPGRADE_UNAVAILABLE"},
        {"busy",
         409,
         "ConflictError",
         "An update is already in progress.",
         "Wait for the active job to finish before requesting another update.",
         "UPGRADE_BUSY"},
        {"target-unapproved",
         422,
         "ValidationError",
         "This update is not available as a one-click update.",
         "Refresh the available updates before choosing one.",
         "UPGRADE_TARGET_UNAPPROVED"},
        {"idempotency-conflict",
         409,
         "ConflictError",
         "This idempotency_key was already used for a different update.",
         "Use a new idempotency_key for a new update request.",
         "UPGRADE_IDEMPOTENCY_CONFLICT"},
        {"request-expired",
         409,
         "ConflictError",
         "This update request has expired.",
         "Check the outcome of the earlier request before making a new one.",
         "UPGRADE_REQUEST_EXPIRED"},
    }};

    const Spec* selected = nullptr;
    for (const auto& spec: kSpecs) {
        if (spec.code == code) {
            selected = &spec;
            break;
        }
    }
    if (selected == nullptr) {
        selected = &kSpecs[1]; // unknown adapter errors become UPGRADE_UNAVAILABLE (502)
    }

    std::string details;
    const auto diagnostics = adapter_option(ctx, "diagnostics_json");
    if (!diagnostics.empty() && diagnostics.front() == '[') {
        details = "{\"diagnostics\":" + diagnostics + "}";
    }
    return result(
        selected->status,
        error_envelope(
            selected->message,
            selected->type,
            selected->ghost_code,
            selected->help,
            {},
            details
        )
    );
}

// ---------------------------------------------------------------------------
// SHA-256 (std-only, matching node:crypto createHash('sha256')). A shared
// `utils:hash` dependency is recorded as a new-partition need; until then the
// same primitive used by the featurebase partition is inlined here.
// ---------------------------------------------------------------------------
namespace detail {

    inline constexpr std::array<std::uint32_t, 64> kSha256K{
        0x42'8A'2F'98u, 0x71'37'44'91u, 0xB5'C0'FB'CFu, 0xE9'B5'DB'A5u, 0x39'56'C2'5Bu,
        0x59'F1'11'F1u, 0x92'3F'82'A4u, 0xAB'1C'5E'D5u, 0xD8'07'AA'98u, 0x12'83'5B'01u,
        0x24'31'85'BEu, 0x55'0C'7D'C3u, 0x72'BE'5D'74u, 0x80'DE'B1'FEu, 0x9B'DC'06'A7u,
        0xC1'9B'F1'74u, 0xE4'9B'69'C1u, 0xEF'BE'47'86u, 0x0F'C1'9D'C6u, 0x24'0C'A1'CCu,
        0x2D'E9'2C'6Fu, 0x4A'74'84'AAu, 0x5C'B0'A9'DCu, 0x76'F9'88'DAu, 0x98'3E'51'52u,
        0xA8'31'C6'6Du, 0xB0'03'27'C8u, 0xBF'59'7F'C7u, 0xC6'E0'0B'F3u, 0xD5'A7'91'47u,
        0x06'CA'63'51u, 0x14'29'29'67u, 0x27'B7'0A'85u, 0x2E'1B'21'38u, 0x4D'2C'6D'FCu,
        0x53'38'0D'13u, 0x65'0A'73'54u, 0x76'6A'0A'BBu, 0x81'C2'C9'2Eu, 0x92'72'2C'85u,
        0xA2'BF'E8'A1u, 0xA8'1A'66'4Bu, 0xC2'4B'8B'70u, 0xC7'6C'51'A3u, 0xD1'92'E8'19u,
        0xD6'99'06'24u, 0xF4'0E'35'85u, 0x10'6A'A0'70u, 0x19'A4'C1'16u, 0x1E'37'6C'08u,
        0x27'48'77'4Cu, 0x34'B0'BC'B5u, 0x39'1C'0C'B3u, 0x4E'D8'AA'4Au, 0x5B'9C'CA'4Fu,
        0x68'2E'6F'F3u, 0x74'8F'82'EEu, 0x78'A5'63'6Fu, 0x84'C8'78'14u, 0x8C'C7'02'08u,
        0x90'BE'FF'FAu, 0xA4'50'6C'EBu, 0xBE'F9'A3'F7u, 0xC6'71'78'F2u
    };

    [[nodiscard]] inline constexpr std::uint32_t rotr(std::uint32_t x, unsigned n) noexcept
    {
        return (x >> n) | (x << (32u - n));
    }

    [[nodiscard]] inline std::array<std::uint8_t, 32> sha256(std::string_view data)
    {
        const std::uint64_t bit_length = static_cast<std::uint64_t>(data.size()) * 8u;
        std::vector<std::uint8_t> message(data.begin(), data.end());
        message.push_back(0x80u);
        while (message.size() % 64u != 56u) {
            message.push_back(0u);
        }
        for (int i = 7; i >= 0; --i) {
            message.push_back(static_cast<std::uint8_t>(bit_length >> (i * 8)));
        }

        std::uint32_t h[8]{
            0x6A'09'E6'67u,
            0xBB'67'AE'85u,
            0x3C'6E'F3'72u,
            0xA5'4F'F5'3Au,
            0x51'0E'52'7Fu,
            0x9B'05'68'8Cu,
            0x1F'83'D9'ABu,
            0x5B'E0'CD'19u
        };
        std::uint32_t w[64]{};
        for (std::size_t offset = 0; offset < message.size(); offset += 64u) {
            for (int i = 0; i < 16; ++i) {
                w[i] = (static_cast<std::uint32_t>(message[offset + i * 4u]) << 24u) |
                       (static_cast<std::uint32_t>(message[offset + i * 4u + 1u]) << 16u) |
                       (static_cast<std::uint32_t>(message[offset + i * 4u + 2u]) << 8u) |
                       static_cast<std::uint32_t>(message[offset + i * 4u + 3u]);
            }
            for (int i = 16; i < 64; ++i) {
                const std::uint32_t s0 =
                    rotr(w[i - 15], 7u) ^ rotr(w[i - 15], 18u) ^ (w[i - 15] >> 3u);
                const std::uint32_t s1 =
                    rotr(w[i - 2], 17u) ^ rotr(w[i - 2], 19u) ^ (w[i - 2] >> 10u);
                w[i] = w[i - 16] + s0 + w[i - 7] + s1;
            }
            std::uint32_t a = h[0], b = h[1], c = h[2], d = h[3];
            std::uint32_t e = h[4], f = h[5], g = h[6], hh = h[7];
            for (int i = 0; i < 64; ++i) {
                const std::uint32_t s1 = rotr(e, 6u) ^ rotr(e, 11u) ^ rotr(e, 25u);
                const std::uint32_t ch = (e & f) ^ ((~e) & g);
                const std::uint32_t t1 =
                    hh + s1 + ch + kSha256K[static_cast<std::size_t>(i)] + w[i];
                const std::uint32_t s0 = rotr(a, 2u) ^ rotr(a, 13u) ^ rotr(a, 22u);
                const std::uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
                const std::uint32_t t2 = s0 + maj;
                hh = g;
                g = f;
                f = e;
                e = d + t1;
                d = c;
                c = b;
                b = a;
                a = t1 + t2;
            }
            h[0] += a;
            h[1] += b;
            h[2] += c;
            h[3] += d;
            h[4] += e;
            h[5] += f;
            h[6] += g;
            h[7] += hh;
        }
        std::array<std::uint8_t, 32> digest{};
        for (int i = 0; i < 8; ++i) {
            digest[static_cast<std::size_t>(i) * 4u] = static_cast<std::uint8_t>(h[i] >> 24u);
            digest[static_cast<std::size_t>(i) * 4u + 1u] = static_cast<std::uint8_t>(h[i] >> 16u);
            digest[static_cast<std::size_t>(i) * 4u + 2u] = static_cast<std::uint8_t>(h[i] >> 8u);
            digest[static_cast<std::size_t>(i) * 4u + 3u] = static_cast<std::uint8_t>(h[i]);
        }
        return digest;
    }

} // namespace detail

/** @brief node:crypto sha256(JSON.stringify([userId, clientKey])).digest('hex'). */
[[nodiscard]] inline std::string
scoped_idempotency_key(std::string_view user_id, std::string_view client_key)
{
    const auto input = "[" + json_quote(user_id) + "," + json_quote(client_key) + "]";
    const auto digest = detail::sha256(input);
    static constexpr std::string_view kHex{"0123456789abcdef"};
    std::string out;
    out.reserve(digest.size() * 2u);
    for (const std::uint8_t byte: digest) {
        out.push_back(kHex[byte >> 4u]);
        out.push_back(kHex[byte & 0x0Fu]);
    }
    return out;
}

// ---------------------------------------------------------------------------
// Host upgrade adapter stand-in. Ghost resolves the adapter per call; the
// default NoopUpgradeAdapter is mirrored exactly. A configured adapter is
// modeled with context config/settings and UPGRADE_* env overrides until a
// `core_adapters:upgrades` partition is registered (recorded as a need).
// ---------------------------------------------------------------------------
[[nodiscard]] inline std::string
env_or_config(blog::BlogContext& ctx, std::string_view name, std::string_view env_suffix)
{
    const std::string env_key = "UPGRADE_" + std::string{env_suffix};
    if (const char* value = std::getenv(env_key.c_str()); value != nullptr && *value != '\0') {
        return std::string{value};
    }

    const auto lookup = [&](const std::vector<blog::api::RecordDto>& rows)
    {
        for (const auto& row: rows) {
            const auto key = row.find("key");
            if (key == row.end()) {
                continue;
            }
            const auto stored = unquote(key->second);
            if (stored != std::string{name} && stored != "upgrade:" + std::string{name}) {
                continue;
            }
            const auto value = row.find("value");
            if (value != row.end()) {
                return unquote(value->second);
            }
        }
        return std::string{};
    };

    auto found = lookup(blog::api::parse_array(ctx.store().json_for("settings", {})));
    if (!found.empty()) {
        return found;
    }
    found = lookup(blog::api::parse_array(ctx.store().resources().list("config")));
    return found;
}

[[nodiscard]] inline std::string adapter_option(blog::BlogContext& ctx, std::string_view name)
{
    std::string suffix;
    for (const char c: name) {
        suffix.push_back(
            c == ':' ? '_' : static_cast<char>(std::toupper(static_cast<unsigned char>(c)))
        );
    }
    return env_or_config(ctx, name, suffix);
}

[[nodiscard]] inline bool adapter_active(blog::BlogContext& ctx)
{
    const auto active = adapter_option(ctx, "active");
    if (active.empty()) {
        return false; // default NoopUpgradeAdapter
    }
    return active != "NoopUpgradeAdapter" && active != "noop" && active != "false" && active != "0";
}

[[nodiscard]] inline std::string error_code(blog::BlogContext& ctx)
{
    const auto code = adapter_option(ctx, "error_code");
    static constexpr std::array<std::string_view, 6> kCodes{
        "unsupported",
        "unavailable",
        "busy",
        "target-unapproved",
        "idempotency-conflict",
        "request-expired"
    };
    for (const auto known: kCodes) {
        if (code == known) {
            return code;
        }
    }
    return {};
}

[[nodiscard]] inline std::string object_from_json(std::string_view json)
{
    if (json.empty()) {
        return {};
    }
    const auto parsed = core_serde::JsonCodec{}.decode<serde::Value>(json);
    if (!parsed || std::get_if<Object>(&parsed->get()) == nullptr) {
        return {};
    }
    return std::string{json};
}

[[nodiscard]] inline std::string field_from_json(std::string_view json, std::string_view name)
{
    const auto parsed = core_serde::JsonCodec{}.decode<serde::Value>(json);
    const auto* object = parsed ? std::get_if<Object>(&parsed->get()) : nullptr;
    return field(object, name);
}

[[nodiscard]] inline std::string
inject_default(std::string json, std::string_view name, std::string_view value)
{
    const auto parsed = core_serde::JsonCodec{}.decode<serde::Value>(json);
    const auto* object = parsed ? std::get_if<Object>(&parsed->get()) : nullptr;
    if (has_key(object, name)) {
        return json;
    }
    const auto closing = json.find_last_of('}');
    if (closing == std::string::npos) {
        return json;
    }
    const std::string fragment = json_quote(name) + ":" + std::string{value};
    if (closing > 0 && json[closing - 1] == '{') {
        return json.substr(0, closing) + fragment + json.substr(closing);
    }
    return json.substr(0, closing) + "," + fragment + json.substr(closing);
}

[[nodiscard]] inline bool valid_availability(std::string_view value)
{
    return value == "ready" || value == "busy" || value == "unavailable" || value == "stale" ||
           value == "incompatible";
}

[[nodiscard]] inline bool valid_component(std::string_view value)
{
    if (value.empty() || value.size() > 32 || value.front() < 'a' || value.front() > 'z') {
        return false;
    }
    for (std::size_t i = 1; i < value.size(); ++i) {
        const char c = value[i];
        if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-')) {
            return false;
        }
    }
    return true;
}

[[nodiscard]] inline bool valid_version(std::string_view value)
{
    // Ghost upgradeVersionSchema: opaque 1..128 character string.
    return !value.empty() && value.size() <= 128;
}

[[nodiscard]] inline bool is_lower_hex(char c)
{
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f');
}

[[nodiscard]] inline bool valid_uuid_v4(std::string_view value)
{
    if (value.size() != 36 || value[8] != '-' || value[13] != '-' || value[18] != '-' ||
        value[23] != '-' || value[14] != '4') {
        return false;
    }
    if (value[19] != '8' && value[19] != '9' && value[19] != 'a' && value[19] != 'b') {
        return false;
    }
    for (std::size_t i = 0; i < value.size(); ++i) {
        if (i == 8 || i == 13 || i == 18 || i == 23) {
            continue;
        }
        if (!is_lower_hex(value[i])) {
            return false;
        }
    }
    return true;
}

[[nodiscard]] inline bool valid_job_id(std::string_view value)
{
    if (value.empty() || value.size() > 64) {
        return false;
    }
    const char first = value.front();
    if (!((first >= 'a' && first <= 'z') || (first >= '0' && first <= '9'))) {
        return false;
    }
    for (std::size_t i = 1; i < value.size(); ++i) {
        const char c = value[i];
        if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-')) {
            return false;
        }
    }
    return true;
}

[[nodiscard]] inline std::string last_segment(std::string_view path)
{
    path = path.substr(0, path.find('?'));
    while (path.size() > 1 && path.back() == '/') {
        path.remove_suffix(1);
    }
    const auto slash = path.find_last_of('/');
    return std::string{slash == std::string_view::npos ? path : path.substr(slash + 1)};
}

[[nodiscard]] inline bool
valid_job(std::string_view json, std::string_view component, std::string_view to_version)
{
    const auto parsed = core_serde::JsonCodec{}.decode<serde::Value>(json);
    const auto* object = parsed ? std::get_if<Object>(&parsed->get()) : nullptr;
    if (object == nullptr) {
        return false;
    }
    static constexpr std::array<std::string_view, 5>
        kRequired{"id", "state", "component", "to_version", "created_at"};
    for (const auto name: kRequired) {
        if (field(object, name).empty()) {
            return false;
        }
    }
    if (field(object, "updated_at").empty()) {
        return false;
    }
    return valid_job_id(field(object, "id")) &&
           (component.empty() || field(object, "component") == component) &&
           (to_version.empty() || field(object, "to_version") == to_version);
}

/** @brief Ghost getStatus(): never rejects, maps any host failure to a status. */
[[nodiscard]] inline Output browse_status(blog::BlogContext& ctx)
{
    if (adapter_active(ctx)) {
        if (!error_code(ctx).empty()) {
            return result(200, wrap(kUnavailableStatus));
        }
        const auto configured = adapter_option(ctx, "status_json");
        if (!configured.empty()) {
            const auto parsed = core_serde::JsonCodec{}.decode<serde::Value>(configured);
            const auto* object = parsed ? std::get_if<Object>(&parsed->get()) : nullptr;
            if (object != nullptr) {
                const auto supported = field(object, "supported");
                if (supported == "false") {
                    const auto reason = field(object, "reason");
                    if (reason == "not-configured" || reason == "not-supported") {
                        return result(200, wrap(configured));
                    }
                } else if (supported == "true") {
                    const auto availability = field(object, "availability");
                    if (valid_availability(availability) &&
                        (availability != "ready" || has_key(object, "backup_required"))) {
                        // zod defaults oneClick/manual to [] for supported hosts.
                        auto normalized = inject_default(configured, "one_click", "[]");
                        normalized = inject_default(normalized, "manual", "[]");
                        return result(200, wrap(normalized));
                    }
                }
            }
            return result(200, wrap(kUnavailableStatus));
        }
    }
    // Default NoopUpgradeAdapter: {supported:false, reason:'not-configured'}.
    return result(200, wrap(kNoopStatus));
}

/** @brief Ghost createRequest(): SHA-256-scoped key + host acceptance (outbox). */
[[nodiscard]] inline Output create_request(
    blog::BlogContext& ctx,
    std::string_view component,
    std::string_view to_version,
    std::string_view scoped_key,
    std::string_view user_id
)
{
    const auto configured_error = error_code(ctx);
    if (!configured_error.empty()) {
        return adapter_error(ctx, configured_error);
    }
    if (!adapter_active(ctx)) {
        // NoopUpgradeAdapter.createRequest throws UpgradeAdapterError('unsupported').
        return adapter_error(ctx, "unsupported");
    }
    const auto job = adapter_option(ctx, "job_json");
    if (job.empty() || !valid_job(job, component, to_version)) {
        // Invalid adapter output or an id/target mismatch is logged and returned as 502.
        return adapter_error(ctx, "unavailable");
    }

    const auto job_id = field_from_json(job, "id");
    const auto payload = std::format(
        R"({{"idempotency_key":{},"user_id":{},"component":{},"to_version":{},"job":{}}})",
        json_quote(scoped_key),
        json_quote(user_id),
        json_quote(component),
        json_quote(to_version),
        job
    );
    blog::handlers::outbox::global_outbox().emit(
        blog::handlers::outbox::Event{"upgrade.create_request", payload, job_id}
    );

    return result(202, wrap(job));
}

/** @brief Ghost getJob(): full job | unknown | expired; adapter failures are 502. */
[[nodiscard]] inline Output read_job(blog::BlogContext& ctx, std::string_view id)
{
    const auto configured_error = error_code(ctx);
    if (!configured_error.empty()) {
        return adapter_error(ctx, configured_error);
    }
    if (adapter_active(ctx)) {
        const auto expired = adapter_option(ctx, "expired_job_id");
        if (!expired.empty() && expired == id) {
            return result(
                200,
                wrap(std::format(R"({{"id":{},"state":"expired"}})", json_quote(id)))
            );
        }
        const auto job = adapter_option(ctx, "job_json");
        if (!job.empty()) {
            if (!valid_job(job, {}, {})) {
                return adapter_error(ctx, "unavailable");
            }
            if (field_from_json(job, "id") != id) {
                // UpgradeService.getJob rejects a lookup that returns a different id.
                return adapter_error(ctx, "unavailable");
            }
            return result(200, wrap(job));
        }
    }
    // NoopUpgradeAdapter.getJob resolves every id as unknown.
    return result(200, wrap(std::format(R"({{"id":{},"state":"unknown"}})", json_quote(id))));
}

/** @brief Default ADMIN_API / Administrator headers for a bearer request. */
[[nodiscard]] inline void prepare_principal(interfaces::io::IRequest& request)
{
    using blog::handlers::middleware::kGhostContextHeader;
    using blog::handlers::middleware::kGhostPermissionsHeader;
    using blog::handlers::middleware::kGhostRoleHeader;
    using blog::handlers::middleware::kGhostStaffHeader;

    if (request.get_authorization().empty()) {
        return;
    }
    if (request.find_header(kGhostContextHeader).empty()) {
        request.set_header(kGhostContextHeader, "ADMIN_API");
    }
    if (request.find_header(kGhostRoleHeader).empty()) {
        request.set_header(kGhostRoleHeader, "Administrator");
        request.set_header(kGhostPermissionsHeader, "*");
        request.set_header(kGhostStaffHeader, "true");
    }
}

[[nodiscard]] inline core::auth::ghost::Principal principal_for(interfaces::io::IRequest& request)
{
    using blog::handlers::middleware::kGhostContextHeader;
    using blog::handlers::middleware::kGhostPermissionsHeader;
    using blog::handlers::middleware::kGhostRoleHeader;
    using blog::handlers::middleware::kGhostStaffHeader;
    using core::auth::ghost::ApiContext;

    core::auth::ghost::Principal principal;
    const auto context = request.find_header(kGhostContextHeader);
    if (context == "CONTENT_API") {
        principal.context = ApiContext::CONTENT_API;
    } else if (context == "MEMBERS_APP") {
        principal.context = ApiContext::MEMBERS_APP;
    } else if (context == "INTERNAL") {
        principal.context = ApiContext::INTERNAL;
    } else if (context == "SYSTEM") {
        principal.context = ApiContext::SYSTEM;
    } else if (context == "ADMIN_API") {
        principal.context = ApiContext::ADMIN_API;
    } else {
        principal.context =
            request.get_authorization().empty() ? ApiContext::ANONYMOUS : ApiContext::ADMIN_API;
    }
    principal.role = std::string{request.find_header(kGhostRoleHeader)};
    if (principal.role.empty() && principal.context == ApiContext::ADMIN_API) {
        principal.role = "Administrator";
    }
    for (auto part:
         std::views::split(std::string_view{request.find_header(kGhostPermissionsHeader)}, ',')) {
        const std::string scope{std::ranges::begin(part), std::ranges::end(part)};
        if (!scope.empty()) {
            principal.permissions.insert(scope);
        }
    }
    const auto staff = request.find_header(kGhostStaffHeader);
    principal.staff = staff == "true" || staff == "1";
    if (!request.get_authorization().empty()) {
        principal.id = "bearer";
    }
    return principal;
}

/** @brief Ghost controller `permissions:true`, docName `upgrades`. */
[[nodiscard]] inline core::auth::ghost::AuthResult
authorize(interfaces::io::IRequest& request, std::string_view method)
{
    core::auth::ghost::GhostAuthorizer authorizer;
    return authorizer.authorize(principal_for(request), true, kDocName, method);
}

/** @brief frame.options.context.user; the harness carries it as x-blog-user-id. */
[[nodiscard]] inline std::string staff_user_id(interfaces::io::IRequest& request)
{
    const auto id = request.find_header("x-blog-user-id");
    if (!id.empty()) {
        return std::string{id};
    }
    return request.get_authorization().empty() ? std::string{} : std::string{"bearer"};
}

} // namespace blog::handlers::upgrades::support

export namespace blog::handlers::upgrades::browse::workflow {

class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = support::Output;
    static constexpr std::string_view name{"upgrades_browse_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input&, core::workflow::WorkflowContext&) const
    {
        auto& ctx = *s_ctx;
        // Ghost GET /upgrades/: no options, no validation, no rate-limit or event
        // side effect in core. getStatus() maps host failures into a status body.
        return core::workflow::StepResponse<Output>{support::browse_status(ctx)};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};

} // namespace blog::handlers::upgrades::browse::workflow
