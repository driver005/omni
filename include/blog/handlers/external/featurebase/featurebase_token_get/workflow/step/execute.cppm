export module blog_handlers:featurebase_featurebase_token_get_step;
import :domain_all;
import std;
import core_workflow;
import core_serde;
import interfaces;

namespace {

// std-only HS256 JWT primitives. Ghost signs the Featurebase SSO token with
// `jsonwebtoken` (HS256, expiresIn '7d'); no third-party JWT lib is available
// in this module, so SHA-256 / HMAC / base64url are implemented locally over
// `import std`. A secret-taking signer in `core_auth` is recorded as a shared
// abstraction need; wire shape and signature match jsonwebtoken exactly.

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
            w[i] = w[i - 16] + s0 + w[i - 7] + s1;
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

[[nodiscard]] std::string unquote(std::string_view value)
{
    if (value.size() >= 2u && value.front() == '"' && value.back() == '"') {
        return std::string{value.substr(1u, value.size() - 2u)};
    }
    return std::string{value};
}

[[nodiscard]] std::string clean_field(std::string_view raw)
{
    const auto value = unquote(raw);
    if (value.empty() || value == "null" || value == "undefined") {
        return {};
    }
    return value;
}

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

/** @brief First non-empty environment variable. Ghost's nconf reads `featurebase:*`
 *  keys from env with a `__` separator and `parseValues:true` (shared/config/loader.ts:35-42).
 *  `core_adapters::detail::env_first` would be the shared equivalent; that target is
 *  not a dependency of //include/blog/handlers:blog_handlers, so std::getenv is used
 *  here with the same key spellings (recorded as a new-dependency need).
 */
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

[[nodiscard]] bool truthy(std::string_view value)
{
    return !value.empty() && value != "false" && value != "0" && value != "no";
}

} // namespace

export namespace blog::handlers::featurebase::featurebase_token_get::workflow {

class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"featurebase_featurebase_token_get_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(Input in, core::workflow::WorkflowContext&) const
    {
        auto& ctx = *s_ctx;

        // Ghost reads featurebase:enabled / featurebase:jwtSecret from config (nconf;
        // env keys FEATUREBASE__ENABLED / FEATUREBASE__JWTSECRET). The context-local
        // `config`/`settings` namespaces remain as harness overrides, matching how the
        // other handlers in this module simulate config without a new global.
        const auto settings = blog::api::parse_array(ctx.store().json_for("settings", {}));
        const auto config = blog::api::parse_array(ctx.store().resources().list("config"));

        auto enabled = env_first({"FEATUREBASE__ENABLED", "FEATUREBASE_ENABLED"});
        if (enabled.empty()) {
            enabled = find_setting(config, "featurebase:enabled");
        }
        if (enabled.empty()) {
            enabled = find_setting(settings, "featurebase:enabled");
        }
        auto secret = env_first(
            {"FEATUREBASE__JWTSECRET", "FEATUREBASE_JWT_SECRET", "FEATUREBASE_JWTSECRET"}
        );
        if (secret.empty()) {
            secret = find_setting(config, "featurebase:jwtSecret");
        }
        if (secret.empty()) {
            secret = find_setting(settings, "featurebase:jwtSecret");
        }

        // Ghost: `if (!enabled || !jwtSecret) throw new ValidationError({message:
        // 'Featurebase is not configured'})` (featurebase.js:41-45). nconf's
        // parseValues turns "false"/"0" into boolean false.
        if (!truthy(enabled) || secret.empty() || secret == "false" || secret == "0") {
            return core::workflow::StepResponse<Output>{
                {{"status", "422"},
                 {"body",
                  blog::api::ApiError{}.validation("Featurebase is not configured", "", "")}}
            };
        }

        // Ghost `frame.user` is the DB-authenticated user (featurebase.js:9-11).
        // The harness carries the authenticated subject via x-blog-user-id or the
        // admin bearer (the same mapping used by admin/users); resolve that user
        // from the store instead of trusting arbitrary client-supplied headers,
        // then fall back to the headers only when no user record exists.
        std::string subject = std::string{in.find_header("x-blog-user-id")};
        if (subject.empty() && !in.get_authorization().empty()) {
            subject = "1";
        }
        blog::api::RecordDto user;
        if (!subject.empty()) {
            for (const auto& row: blog::api::parse_array(ctx.store().json_for("users", {}))) {
                const auto id = row.find("id");
                if (id != row.end() && clean_field(id->second) == subject) {
                    user = row;
                    break;
                }
            }
        }
        std::string user_name = clean_field(user["name"]);
        std::string user_email = clean_field(user["email"]);
        std::string user_picture = clean_field(user["profile_image"]);
        if (user_name.empty()) {
            user_name = std::string{in.find_header("x-blog-user-name")};
        }
        if (user_email.empty()) {
            user_email = std::string{in.find_header("x-blog-user-email")};
        }
        if (user_picture.empty()) {
            user_picture = std::string{in.find_header("x-blog-user-profile-image")};
        }

        // Ghost builds companies[0] from settingsCache('site_uuid','title') and
        // urlUtils.urlFor('home', true) (featurebase.js:13-19).
        auto title = find_setting(settings, "title");
        if (title.empty()) {
            title = find_setting(config, "title");
        }
        if (title.empty()) {
            title = "Ghost";
        }
        auto site_uuid = find_setting(settings, "site_uuid");
        if (site_uuid.empty()) {
            site_uuid = find_setting(config, "site_uuid");
        }
        if (site_uuid.empty()) {
            site_uuid = "d5432f87-1d1a-4d2f-9f95-5c1a2f0a9e13";
        }

        auto home = env_first({"URL"});
        if (home.empty()) {
            home = find_setting(config, "url");
        }
        if (home.empty()) {
            home = "http://localhost:2368/";
            const auto scheme = in.get_scheme();
            const auto authority = in.get_authority();
            if (!scheme.empty() && !authority.empty()) {
                home = std::string{scheme} + "://" + std::string{authority} + "/";
            }
        }
        // urlUtils.urlFor('home', true) always returns an absolute URL with a
        // trailing slash; config `url` values need not carry one.
        if (!home.ends_with('/')) {
            home.push_back('/');
        }

        const std::int64_t issued_at = std::chrono::duration_cast<std::chrono::seconds>(
                                           std::chrono::system_clock::now().time_since_epoch()
        )
                                           .count();
        const std::int64_t expires_at = issued_at + 7LL * 24LL * 3'600LL; // expiresIn: '7d'

        const auto encode = [](const std::string& value)
        {
            return core_serde::JsonCodec{}.encode(value);
        };
        const auto header = std::format(R"({{"alg":{},"typ":{}}})", encode("HS256"), encode("JWT"));
        const auto payload = std::format(
            R"({{"name":{},"email":{},"profilePicture":{},"companies":[{{"id":{},"name":{},"website":{}}}],"iat":{},"exp":{}}})",
            encode(user_name),
            encode(user_email),
            encode(user_picture),
            encode(site_uuid),
            encode(title),
            encode(home),
            std::to_string(issued_at),
            std::to_string(expires_at)
        );

        const auto signing_input = base64url(header) + "." + base64url(payload);
        const auto signature = hmac_sha256(secret, signing_input);
        const auto token =
            signing_input + "." +
            base64url(
                std::string_view{reinterpret_cast<const char*>(signature.data()), signature.size()}
            );

        // Ghost's output serializer sets frame.response = {featurebase: data}
        // (utils/serializers/output/featurebase.js:7-9), so the wire body is
        // {"featurebase":{"token":"<jwt>"}} — not a bare {"token":...}.
        const auto body = std::format(R"({{"featurebase":{{"token":{}}}}})", encode(token));
        return core::workflow::StepResponse<Output>{{{"status", "200"}, {"body", body}}};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};
} // namespace blog::handlers::featurebase::featurebase_token_get::workflow
