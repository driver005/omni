export module blog_handlers:tinybird_tinybird_token_get_step;
import :domain_all;
import std;
import core_workflow;
import core_serde;
import interfaces;

namespace {

// Ghost signs the Tinybird JWT with jsonwebtoken (HS256, noTimestamp:true).
// No secret-taking signer exists in the registered core modules (featurebase's
// strict-parity pass uses the same std-only block), so SHA-256 / HMAC / base64url
// are implemented here over `import std`. A shared `core_auth` signer is a
// recorded new-partition need; the wire shape matches jsonwebtoken exactly.

constexpr std::array<std::uint32_t, 64> kSha256K{
    0x42'8A'2F'98u, 0x71'37'44'91u, 0xB5'C0'FB'CFu, 0xE9'B5'DB'A5u, 0x39'56'C2'5Bu, 0x59'F1'11'F1u,
    0x92'3F'82'A4u, 0xAB'1C'5E'D5u, 0xD8'07'AA'98u, 0x12'83'5B'01u, 0x24'31'85'BEu, 0x55'0C'7D'C3u,
    0x72'BE'5D'74u, 0x80'DE'B1'FEu, 0x9B'DC'06'A7u, 0xC1'9B'F1'74u, 0xE4'9B'69'C1u, 0xEF'BE'47'86u,
    0x0F'C1'9D'C6u, 0x24'0C'A1'CCu, 0x2D'E9'2C'6Fu, 0x4A'74'84'AAu, 0x5C'B0'A9'DCu, 0x76'F9'88'DAu,
    0x98'3E'51'52u, 0xA8'31'C6'6Du, 0xB0'03'27'C8u, 0xBF'59'7F'C7u, 0xC6'E0'0B'F3u, 0xD5'A7'91'47u,
    0x06'CA'63'51u, 0x14'29'29'67u, 0x27'B7'0A'85u, 0x2E'1B'21'38u, 0x4D'2C'6D'FCu, 0x53'38'0D'13u,
    0x65'0A'73'54u, 0x76'6A'0A'BBu, 0x81'C2'C9'2Eu, 0x92'72'2C'85u, 0xA2'BF'E8'A1u, 0xA8'1A'66'4Bu,
    0xC2'4B'8B'70u, 0xC7'6C'51'A3u, 0xD1'92'E8'19u, 0xD6'99'06'24u, 0xF4'0E'35'85u, 0x10'6A'A0'70u,
    0x19'A4'C1'16u, 0x1E'37'6C'08u, 0x27'48'77'4Cu, 0x34'B0'BC'B5u, 0x39'1C'0C'B3u, 0x4E'D8'AA'4Au,
    0x5B'9C'CA'4Fu, 0x68'2E'6F'F3u, 0x74'8F'82'EEu, 0x78'A5'63'6Fu, 0x84'C8'78'14u, 0x8C'C7'02'08u,
    0x90'BE'FF'FAu, 0xA4'50'6C'EBu, 0xBE'F9'A3'F7u, 0xC6'71'78'F2u
};

[[nodiscard]] constexpr std::uint32_t rotr(std::uint32_t x, unsigned n) noexcept
{
    return (x >> n) | (x << (32u - n));
}

[[nodiscard]] std::array<std::uint8_t, 32> sha256(std::string_view data)
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
            const std::uint32_t s0 = rotr(w[i - 15], 7u) ^ rotr(w[i - 15], 18u) ^ (w[i - 15] >> 3u);
            const std::uint32_t s1 = rotr(w[i - 2], 17u) ^ rotr(w[i - 2], 19u) ^ (w[i - 2] >> 10u);
            w[i] = w[i - 16u] + s0 + w[i - 7u] + s1;
        }

        std::uint32_t a = h[0], b = h[1], c = h[2], d = h[3];
        std::uint32_t e = h[4], f = h[5], g = h[6], hh = h[7];
        for (int i = 0; i < 64; ++i) {
            const std::uint32_t s1 = rotr(e, 6u) ^ rotr(e, 11u) ^ rotr(e, 25u);
            const std::uint32_t ch = (e & f) ^ ((~e) & g);
            const std::uint32_t t1 = hh + s1 + ch + kSha256K[static_cast<std::size_t>(i)] + w[i];
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

[[nodiscard]] std::array<std::uint8_t, 32>
hmac_sha256(std::string_view key, std::string_view message)
{
    std::array<std::uint8_t, 64> block{};
    if (key.size() > block.size()) {
        const auto shortened = sha256(key);
        std::copy(shortened.begin(), shortened.end(), block.begin());
    } else {
        std::copy(key.begin(), key.end(), block.begin());
    }

    std::string inner;
    std::string outer;
    inner.reserve(block.size() + message.size());
    outer.reserve(block.size() + 32u);
    for (const std::uint8_t byte: block) {
        inner.push_back(static_cast<char>(byte ^ 0x36u));
        outer.push_back(static_cast<char>(byte ^ 0x5Cu));
    }
    inner.append(message);
    const auto inner_digest = sha256(inner);
    outer.append(reinterpret_cast<const char*>(inner_digest.data()), inner_digest.size());
    return sha256(outer);
}

[[nodiscard]] std::string base64url(std::string_view bytes)
{
    static constexpr std::string_view alphabet{
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_"
    };
    std::string out;
    out.reserve((bytes.size() + 2u) / 3u * 4u);
    for (std::size_t i = 0; i < bytes.size(); i += 3u) {
        std::uint32_t value = static_cast<std::uint32_t>(static_cast<std::uint8_t>(bytes[i]))
                              << 16u;
        if (i + 1u < bytes.size()) {
            value |= static_cast<std::uint32_t>(static_cast<std::uint8_t>(bytes[i + 1u])) << 8u;
        }
        if (i + 2u < bytes.size()) {
            value |= static_cast<std::uint32_t>(static_cast<std::uint8_t>(bytes[i + 2u]));
        }
        out.push_back(alphabet[(value >> 18u) & 63u]);
        out.push_back(alphabet[(value >> 12u) & 63u]);
        if (i + 1u < bytes.size()) {
            out.push_back(alphabet[(value >> 6u) & 63u]);
        }
        if (i + 2u < bytes.size()) {
            out.push_back(alphabet[value & 63u]);
        }
    }
    return out;
}

[[nodiscard]] std::string base64url_decode(std::string_view text)
{
    static constexpr std::string_view alphabet{
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_"
    };
    std::string out;
    out.reserve(text.size() * 3u / 4u + 1u);
    std::uint32_t accumulator{0};
    int bits{0};
    for (const char character: text) {
        if (character == '=' || character == '\0') {
            break;
        }
        const std::size_t index = alphabet.find(character);
        if (index == std::string_view::npos) {
            return {};
        }
        accumulator = (accumulator << 6u) | static_cast<std::uint32_t>(index);
        bits += 6;
        if (bits >= 8) {
            bits -= 8;
            out.push_back(static_cast<char>((accumulator >> bits) & 0xFFu));
        }
    }
    return out;
}

// Ghost's TinybirdService TINYBIRD_PIPES (tinybird-service.js:47-78). Order and
// spelling are part of the signed JWT payload.
constexpr std::array<std::string_view, 29> kTinybirdPipes{
    "api_kpis",
    "api_active_visitors",
    "api_post_visitor_counts",
    "api_top_locations",
    "api_top_pages",
    "api_top_sources",
    "api_top_utm_sources",
    "api_top_utm_mediums",
    "api_top_utm_campaigns",
    "api_top_utm_contents",
    "api_top_utm_terms",
    "api_top_devices",
    "api_gift_link_visits",
    "api_automation_browse_stats",
    "api_automation_performance_stats",
    "api_automation_runs",
    "api_automation_run_search",
    "api_kpis_v2",
    "api_active_visitors_v2",
    "api_post_visitor_counts_v2",
    "api_top_locations_v2",
    "api_top_pages_v2",
    "api_top_sources_v2",
    "api_top_utm_sources_v2",
    "api_top_utm_mediums_v2",
    "api_top_utm_campaigns_v2",
    "api_top_utm_contents_v2",
    "api_top_utm_terms_v2",
    "api_top_devices_v2",
};

[[nodiscard]] std::string unquote(std::string_view value)
{
    if (value.size() >= 2u && value.front() == '"' && value.back() == '"') {
        return std::string{value.substr(1u, value.size() - 2u)};
    }
    return std::string{value};
}

[[nodiscard]] bool truthy(std::string_view value)
{
    return !value.empty() && value != "false" && value != "0" && value != "no";
}

/** @brief First non-empty env var. core_adapters::detail::env_first is the shared
 *  equivalent; //include/core/adapters:core_adapters is not a dep of
 *  //include/blog/handlers:blog_handlers, so std::getenv is used with the same
 *  nconf (`tinybird__*`) and contract (`TINYBIRD_*`) spellings. New-dep need recorded. */
[[nodiscard]] std::string env_first(std::initializer_list<std::string_view> names)
{
    for (const auto name: names) {
        const char* value = std::getenv(std::string{name}.c_str()); // NOLINT(concurrency-mt-unsafe)
        if (value != nullptr && *value != '\0') {
            return std::string{value};
        }
    }
    return {};
}

/** @brief Reads a `[{"key":…,"value":…}]` array (or a direct-key object array). */
[[nodiscard]] std::string
find_setting(const std::vector<blog::api::RecordDto>& records, std::string_view key)
{
    for (const auto& record: records) {
        const auto key_it = record.find("key");
        const auto value_it = record.find("value");
        if (key_it != record.end() && value_it != record.end() && unquote(key_it->second) == key) {
            return unquote(value_it->second);
        }
        const auto direct = record.find(std::string{key});
        if (direct != record.end()) {
            return unquote(direct->second);
        }
    }
    return {};
}

[[nodiscard]] std::string pick_config(
    std::initializer_list<std::string_view> env_names,
    std::initializer_list<std::string_view> keys,
    const std::vector<blog::api::RecordDto>& config,
    const std::vector<blog::api::RecordDto>& settings
)
{
    if (const auto from_env = env_first(env_names); !from_env.empty()) {
        return from_env;
    }
    for (const auto key: keys) {
        if (const auto value = find_setting(config, key); !value.empty()) {
            return value;
        }
    }
    for (const auto key: keys) {
        if (const auto value = find_setting(settings, key); !value.empty()) {
            return value;
        }
    }
    return {};
}

/** @brief JS `new Date(exp*1000).toISOString()` (epoch seconds -> UTC ISO with .000Z). */
[[nodiscard]] std::string iso_from_epoch(std::int64_t seconds)
{
    // Howard Hinnant's civil_from_days algorithm (UTC, proleptic Gregorian).
    const std::int64_t z = seconds / 86'400 + 719'468;
    const std::int64_t era = (z >= 0 ? z : z - 146'096) / 146'097;
    const unsigned doe = static_cast<unsigned>(z - era * 146'097);
    const unsigned yoe = (doe - doe / 1'460 + doe / 36'524 - doe / 146'096) / 365;
    std::int64_t year = static_cast<std::int64_t>(yoe) + era * 400;
    const unsigned doy = doe - (365u * yoe + yoe / 4u - yoe / 100u);
    const unsigned mp = (5u * doy + 2u) / 153u;
    const unsigned day = doy - (153u * mp + 2u) / 5u + 1u;
    const unsigned month = mp < 10u ? mp + 3u : mp - 9u;
    year += (month <= 2u) ? 1 : 0;

    const unsigned sod = static_cast<unsigned>(seconds % 86'400);
    const unsigned hour = sod / 3'600u;
    const unsigned minute = (sod % 3'600u) / 60u;
    const unsigned second = sod % 60u;
    return std::format(
        "{:04}-{:02}-{:02}T{:02}:{:02}:{:02}.000Z",
        year,
        month,
        day,
        hour,
        minute,
        second
    );
}

[[nodiscard]] std::int64_t now_epoch_seconds()
{
    return std::chrono::duration_cast<std::chrono::seconds>(
               std::chrono::system_clock::now().time_since_epoch()
    )
        .count();
}

struct TinybirdConfig
{
    std::string workspace_id;
    std::string admin_token;
    std::string stats_id;
    std::string stats_token;
    bool stats_local_enabled{false};
    std::string stats_local_token;
};

struct TokenData
{
    std::string token;
    std::int64_t exp{0};
    bool has_exp{false};
    bool present{false}; // false == Ghost `null`
};

/** @brief C++ mirror of TinybirdService (tinybird-service.js:83-190). The wrapper
 *  re-creates the instance on every request (tinybird-service-wrapper.js:15-25),
 *  so the token cache below is instance-local exactly as in Ghost. */
class TinybirdService
{
public:
    TinybirdService(TinybirdConfig config, std::string site_uuid) :
        m_config{std::move(config)},
        m_site_uuid{std::move(site_uuid)}
    {
    }

    [[nodiscard]] TokenData get_token()
    {
        const bool jwt_enabled = !m_config.workspace_id.empty() && !m_config.admin_token.empty();
        if (jwt_enabled) {
            if (m_server_token.empty() || is_jwt_expired(m_server_token)) {
                const auto generated = generate_token();
                m_server_token = generated.token;
                m_server_token_exp = generated.exp;
            }
            return TokenData{m_server_token, m_server_token_exp, true, true};
        }
        if (m_config.stats_local_enabled) {
            return TokenData{m_config.stats_local_token, 0, false, true};
        }
        if (!m_config.stats_token.empty()) {
            return TokenData{m_config.stats_token, 0, false, true};
        }
        return TokenData{};
    }

private:
    [[nodiscard]] TokenData generate_token() const
    {
        // getToken default expiresInMinutes = 180 (tinybird-service.js:107).
        static constexpr std::int64_t kExpiresInMinutes = 180;
        const std::int64_t expires_at = now_epoch_seconds() + kExpiresInMinutes * 60;
        const std::string name = "tinybird-jwt-" + m_site_uuid;
        const auto encode = [](const std::string& value)
        {
            return core_serde::JsonCodec{}.encode(value);
        };

        std::string scopes = "[";
        for (std::size_t i = 0; i < kTinybirdPipes.size(); ++i) {
            if (i != 0u) {
                scopes += ',';
            }
            scopes += std::format(
                R"({{"type":{},"resource":{},"fixed_params":{{"site_uuid":{}}}}})",
                encode("PIPES:READ"),
                encode(std::string{kTinybirdPipes[i]}),
                encode(m_site_uuid)
            );
        }
        scopes += ']';

        const auto payload = std::format(
            R"({{"workspace_id":{},"name":{},"exp":{},"scopes":{}}})",
            encode(m_config.workspace_id),
            encode(name),
            std::to_string(expires_at),
            scopes
        );
        // jsonwebtoken.sign(..., {noTimestamp:true}) omits iat; header is alg/typ.
        const auto header = R"({"alg":"HS256","typ":"JWT"})";
        const auto signing_input = base64url(header) + "." + base64url(payload);
        const auto signature = hmac_sha256(m_config.admin_token, signing_input);
        const auto token =
            signing_input + "." +
            base64url(
                std::string_view{reinterpret_cast<const char*>(signature.data()), signature.size()}
            );
        return TokenData{token, expires_at, true, true};
    }

    /** @brief HS256 verify + 300s refresh buffer (tinybird-service.js:177-189). */
    [[nodiscard]] bool is_jwt_expired(std::string_view token) const
    {
        const auto first_dot = token.find('.');
        const auto second_dot = first_dot == std::string_view::npos
                                    ? std::string_view::npos
                                    : token.find('.', first_dot + 1u);
        if (first_dot == std::string_view::npos || second_dot == std::string_view::npos) {
            return true;
        }

        const auto signing_input = token.substr(0u, second_dot);
        const auto signature = hmac_sha256(m_config.admin_token, signing_input);
        const auto expected = base64url(
            std::string_view{reinterpret_cast<const char*>(signature.data()), signature.size()}
        );
        if (expected != token.substr(second_dot + 1u)) {
            return true;
        }

        const auto payload =
            base64url_decode(token.substr(first_dot + 1u, second_dot - first_dot - 1u));
        const auto exp_pos = payload.find("\"exp\":");
        if (exp_pos == std::string::npos) {
            return true;
        }
        const auto exp_start = payload.find_first_of("-0123456789", exp_pos + 6u);
        if (exp_start == std::string::npos) {
            return true;
        }
        const auto exp_end = payload.find_first_not_of("-0123456789", exp_start);
        std::int64_t exp{0};
        try {
            exp = std::stoll(payload.substr(
                exp_start,
                exp_end == std::string::npos ? std::string::npos : exp_end - exp_start
            ));
        } catch (...) {
            return true;
        }
        if (exp == 0) {
            return true;
        }
        return exp - now_epoch_seconds() < 300;
    }

    TinybirdConfig m_config;
    std::string m_site_uuid;
    std::string m_server_token;
    std::int64_t m_server_token_exp{0};
};

} // namespace

export namespace blog::handlers::tinybird::tinybird_token_get::workflow {

class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::string;
    static constexpr std::string_view name{"tinybird_tinybird_token_get_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input&, core::workflow::WorkflowContext&) const
    {
        auto& ctx = *s_ctx;

        // Ghost config.get('tinybird') + settingsCache.get('site_uuid')
        // (tinybird-service-wrapper.js:12-13). Env mirrors nconf's `tinybird__*`
        // keys first; the context-local config/settings namespaces are harness
        // overrides (same convention as the featurebase token step).
        const auto settings = blog::api::parse_array(ctx.store().json_for("settings", {}));
        const auto config = blog::api::parse_array(ctx.store().resources().list("config"));

        TinybirdConfig cfg;
        cfg.workspace_id = pick_config(
            {"tinybird__workspaceId", "TINYBIRD_WORKSPACE_ID", "TINYBIRD_WORKSPACEID"},
            {"tinybird:workspaceId", "tinybird_workspace_id"},
            config,
            settings
        );
        cfg.admin_token = pick_config(
            {"tinybird__adminToken", "TINYBIRD_ADMIN_TOKEN"},
            {"tinybird:adminToken", "tinybird_admin_token"},
            config,
            settings
        );
        cfg.stats_id = pick_config(
            {"tinybird__stats__id", "TINYBIRD_STATS_ID", "TINYBIRD_SITE_UUID"},
            {"tinybird:stats:id", "tinybird_stats_id"},
            config,
            settings
        );
        cfg.stats_token = pick_config(
            {"tinybird__stats__token",
             "TINYBIRD_STATS_TOKEN",
             "TINYBIRD_TOKEN",
             "TINYBIRD_API_KEY"},
            {"tinybird:stats:token", "tinybird_stats_token"},
            config,
            settings
        );
        cfg.stats_local_token = pick_config(
            {"tinybird__stats__local__token", "TINYBIRD_STATS_LOCAL_TOKEN"},
            {"tinybird:stats:local:token", "tinybird_stats_local_token"},
            config,
            settings
        );
        const auto local_enabled = pick_config(
            {"tinybird__stats__local__enabled", "TINYBIRD_STATS_LOCAL_ENABLED"},
            {"tinybird:stats:local:enabled", "tinybird_stats_local_enabled"},
            config,
            settings
        );
        cfg.stats_local_enabled = truthy(local_enabled);

        const auto site_uuid = pick_config(
            {"SITE_UUID", "site_uuid", "TINYBIRD_SITE_UUID"},
            {"site_uuid"},
            config,
            settings
        );
        const bool configured = !cfg.workspace_id.empty() || !cfg.admin_token.empty() ||
                                !cfg.stats_id.empty() || !cfg.stats_token.empty() ||
                                !cfg.stats_local_token.empty() || !local_enabled.empty();

        // Wrapper: `if (!tinybirdConfig || !siteUuid) { warn; instance = null; }`
        std::string token_json = "null";
        if (configured && !site_uuid.empty()) {
            // Service: `siteUuid = tinybirdConfig?.stats?.id || siteUuid`.
            const auto effective_site_uuid = cfg.stats_id.empty() ? site_uuid : cfg.stats_id;
            TinybirdService service{std::move(cfg), effective_site_uuid};
            const auto token_data = service.get_token();
            if (token_data.present && !token_data.token.empty()) {
                const auto encode = [](const std::string& value)
                {
                    return core_serde::JsonCodec{}.encode(value);
                };
                // tinybird.js:19-26 — exp seconds become an ISO string; static
                // tokens keep only `token`.
                token_json = "{\"token\":" + encode(token_data.token);
                if (token_data.has_exp && token_data.exp != 0) {
                    token_json += ",\"exp\":" + encode(iso_from_epoch(token_data.exp));
                }
                token_json += "}";
            }
        }

        // output/tinybird.js:3-10 — frame.response = {tinybird: data}, no meta.
        return core::workflow::StepResponse<Output>{"{\"tinybird\":" + token_json + "}"};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};
} // namespace blog::handlers::tinybird::tinybird_token_get::workflow
