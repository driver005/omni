export module blog_handlers:session_create_step;

import :domain_all;
import std;
import core_workflow;
import core_auth;
import blog_handlers_middleware;
import interfaces;
import :outbox_shim;

export namespace blog::handlers::session::support {

/**
 * @file session_create_step.cppm
 * @brief Strict Ghost session behaviour shared by all four session routes.
 *
 * The session controller in Ghost (session.ts + auth/session/*) is not a CRUD
 * controller: every action is an Express middleware that authenticates a user,
 * regenerates a server session, binds verification to a user, rotates a TOTP
 * challenge and emits a sign-in mail.  These helpers implement that behaviour
 * on top of the shared abstractions that are already registered:
 *   - core_auth::PasswordHasher / PasswordHash / GhostAuthorizer
 *   - blog_handlers_middleware (principal_from_request, write_error)
 *   - blog_handlers:outbox_shim (observable mail side effects)
 *   - core::cache-backed ResourceStore (cross-request session state)
 *
 * No new partition is added: a new module partition would have to be wired into
 * the shared BUILD.bazel, which is reserved for the central pass.  This file is
 * an already registered partition, so the helpers live here and are imported by
 * the other three steps with `import :session_create_step;`.
 */

using Fields = std::map<std::string, std::string>;

/** Full server-side session shape (Ghost `auth/session/session-service.ts:Session`). */
struct SessionState
{
    std::string id{};
    std::string user_id{};
    std::string origin{};
    std::string user_agent{};
    std::string ip{};
    bool verified{false};
    std::string verified_user_id{};
    std::string auth_code_challenge{};
    long long auth_code_generated_at{0};
};

// ---------------------------------------------------------------------------
// Small JSON / string helpers (no third-party dependency)
// ---------------------------------------------------------------------------

[[nodiscard]] inline std::string quote(std::string_view text)
{
    std::string out{"\""};
    for (const char c : text) {
        switch (c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default: out += c; break;
        }
    }
    out += '"';
    return out;
}

[[nodiscard]] inline long long now_ms() noexcept
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::system_clock::now().time_since_epoch())
        .count();
}

[[nodiscard]] inline std::string random_hex(std::size_t bytes)
{
    static std::atomic<unsigned long long> seed{0x9e3779b97f4a7c15ULL};
    std::mt19937_64 rng{static_cast<std::mt19937_64::result_type>(
        std::random_device{}() ^ seed.fetch_add(1, std::memory_order_relaxed))};
    std::string out;
    out.reserve(bytes * 2);
    for (std::size_t i = 0; i < bytes; ++i) {
        out += std::format("{:02x}", static_cast<unsigned char>(rng() & 0xffu));
    }
    return out;
}

[[nodiscard]] inline std::atomic<unsigned long long>& error_counter() noexcept
{
    static std::atomic<unsigned long long> counter{0};
    return counter;
}

[[nodiscard]] inline std::string error_id()
{
    return std::format("{:032x}", error_counter().fetch_add(1, std::memory_order_relaxed) + 1);
}

[[nodiscard]] inline std::string envelope(std::string_view message,
                                          std::string_view type,
                                          std::string_view code,
                                          std::string_view context)
{
    return std::format(
        R"({{"errors":[{{"id":{},"code":{},"context":{},"details":null,"ghostErrorCode":null,"help":null,"message":{},"property":null,"type":{}}}]}})",
        quote(error_id()),
        code.empty() ? std::string{"null"} : quote(code),
        context.empty() ? std::string{"null"} : quote(context),
        quote(message),
        quote(type));
}

// ---------------------------------------------------------------------------
// Request helpers
// ---------------------------------------------------------------------------

[[nodiscard]] inline std::string header(interfaces::io::IRequest& request,
                                        std::string_view name)
{
    return std::string{request.find_header(name)};
}

/** Ghost `getOriginOfRequest`: Origin, then Referer. */
[[nodiscard]] inline std::string origin_of_request(interfaces::io::IRequest& request)
{
    auto origin = header(request, "origin");
    if (!origin.empty()) return origin;
    auto referer = header(request, "referer");
    if (referer.empty()) return {};
    const auto scheme = referer.find("://");
    if (scheme == std::string::npos) return referer;
    const auto slash = referer.find('/', scheme + 3);
    return slash == std::string::npos ? referer : referer.substr(0, slash);
}

/** Login CSRF guard (session-service.ts `cookieCsrfProtection`). */
[[nodiscard]] inline bool csrf_ok(interfaces::io::IRequest& request,
                                  std::string_view bound_origin)
{
    const auto origin = origin_of_request(request);
    if (bound_origin.empty()) return true;  // new session: nothing to bind against
    if (origin.empty()) return false;
    return origin == bound_origin;
}

[[nodiscard]] inline std::string session_id(std::string_view cookie)
{
    constexpr std::string_view prefix{"ghost-admin-api-session="};
    const auto pos = cookie.find(prefix);
    if (pos == std::string_view::npos) return {};
    auto value = cookie.substr(pos + prefix.size());
    if (const auto end = value.find(';'); end != std::string_view::npos) {
        value = value.substr(0, end);
    }
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.front()))) {
        value.remove_prefix(1);
    }
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back()))) {
        value.remove_suffix(1);
    }
    return std::string{value};
}

[[nodiscard]] inline std::string cookie_session_id(interfaces::io::IRequest& request)
{
    return session_id(header(request, "cookie"));
}

// ---------------------------------------------------------------------------
// Configuration (best-effort: settings store, then Ghost defaults)
// ---------------------------------------------------------------------------

[[nodiscard]] inline std::optional<std::string> setting(blog::BlogContext& ctx,
                                                        std::string_view key)
{
    for (const auto& row : blog::api::parse_array(ctx.store().json_for("settings"))) {
        const auto kit = row.find("key");
        const auto vit = row.find("value");
        if (kit != row.end() && kit->second == key && vit != row.end()) {
            return vit->second;
        }
    }
    return std::nullopt;
}

[[nodiscard]] inline bool require_email_mfa(blog::BlogContext& ctx)
{
    return setting(ctx, "require_email_mfa").value_or("false") == "true";
}

[[nodiscard]] inline bool staff_device_verification(blog::BlogContext& ctx)
{
    auto value = setting(ctx, "staffDeviceVerification");
    if (!value) value = setting(ctx, "security:staffDeviceVerification");
    return value.value_or("false") == "true";
}

[[nodiscard]] inline std::string admin_session_secret(blog::BlogContext& ctx)
{
    return setting(ctx, "admin_session_secret").value_or("ghost-admin-session-secret");
}

[[nodiscard]] inline long long session_max_age_ms(blog::BlogContext& ctx)
{
    try {
        return std::stoll(setting(ctx, "admin_session_max_age_ms").value_or("604800000"));
    } catch (...) {
        return 604800000;
    }
}

/** express-session cookie flags (express-session.ts:24-31). */
[[nodiscard]] inline std::string session_cookie(blog::BlogContext& ctx,
                                                std::string_view sid,
                                                interfaces::io::IRequest& request)
{
    const bool ssl = header(request, "x-forwarded-proto") == "https";
    const auto subdir = header(request, "x-blog-subdir");
    return std::format("ghost-admin-api-session={}; Max-Age={}; Path={}/ghost; HttpOnly; SameSite={}{}",
                       sid,
                       session_max_age_ms(ctx) / 1000,
                       subdir,
                       ssl ? "None" : "Lax",
                       ssl ? "; Secure" : "");
}

// ---------------------------------------------------------------------------
// Password check (User.check -> security.password.compare)
// ---------------------------------------------------------------------------

[[nodiscard]] inline bool verify_password(std::string_view stored, std::string_view supplied)
{
    if (stored.empty() || supplied.empty()) return false;
    if (stored.starts_with('$')) {
        // Real Ghost hashes are bcrypt PHC strings; libsodium verifies Argon2id
        // PHC strings.  A non-PHC hash can never be accepted as plaintext.
        return core::auth::PasswordHash{std::string{stored}}.verify(supplied);
    }
    // Fixture-backed compatibility path: unhashed `password` field.
    return stored == supplied;
}

// ---------------------------------------------------------------------------
// TOTP (RFC 6238, step 60s, 6 digits, window +/-10) — std-only SHA-256/HMAC,
// mirroring Ghost `auth/totp.ts` (otplib clone).  Key = secret + userId + context.
// ---------------------------------------------------------------------------

namespace detail {

inline constexpr std::array<std::uint32_t, 64> kSha256K{
    0x428a2f98u, 0x71374491u, 0xb5c0fbcfu, 0xe9b5dba5u, 0x3956c25bu, 0x59f111f1u, 0x923f82a4u, 0xab1c5ed5u,
    0xd807aa98u, 0x12835b01u, 0x243185beu, 0x550c7dc3u, 0x72be5d74u, 0x80deb1feu, 0x9bdc06a7u, 0xc19bf174u,
    0xe49b69c1u, 0xefbe4786u, 0x0fc19dc6u, 0x240ca1ccu, 0x2de92c6fu, 0x4a7484aau, 0x5cb0a9dcu, 0x76f988dau,
    0x983e5152u, 0xa831c66du, 0xb00327c8u, 0xbf597fc7u, 0xc6e00bf3u, 0xd5a79147u, 0x06ca6351u, 0x14292967u,
    0x27b70a85u, 0x2e1b2138u, 0x4d2c6dfcu, 0x53380d13u, 0x650a7354u, 0x766a0abbu, 0x81c2c92eu, 0x92722c85u,
    0xa2bfe8a1u, 0xa81a664bu, 0xc24b8b70u, 0xc76c51a3u, 0xd192e819u, 0xd6990624u, 0xf40e3585u, 0x106aa070u,
    0x19a4c116u, 0x1e376c08u, 0x2748774cu, 0x34b0bcb5u, 0x391c0cb3u, 0x4ed8aa4au, 0x5b9cca4fu, 0x682e6ff3u,
    0x748f82eeu, 0x78a5636fu, 0x84c87814u, 0x8cc70208u, 0x90befffau, 0xa4506cebu, 0xbef9a3f7u, 0xc67178f2u};

[[nodiscard]] inline std::uint32_t rotr(std::uint32_t x, unsigned n) noexcept
{
    return (x >> n) | (x << (32u - n));
}

[[nodiscard]] inline std::array<std::uint8_t, 32> sha256(std::string_view data)
{
    const std::uint64_t bit_length = static_cast<std::uint64_t>(data.size()) * 8u;
    std::vector<std::uint8_t> message(data.begin(), data.end());
    message.push_back(0x80u);
    while (message.size() % 64u != 56u) message.push_back(0u);
    for (int i = 7; i >= 0; --i) {
        message.push_back(static_cast<std::uint8_t>(bit_length >> (i * 8)));
    }
    std::uint32_t h[8]{0x6a09e667u, 0xbb67ae85u, 0x3c6ef372u, 0xa54ff53au,
                       0x510e527fu, 0x9b05688cu, 0x1f83d9abu, 0x5be0cd19u};
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
            hh = g; g = f; f = e; e = d + t1; d = c; c = b; b = a; a = t1 + t2;
        }
        h[0] += a; h[1] += b; h[2] += c; h[3] += d;
        h[4] += e; h[5] += f; h[6] += g; h[7] += hh;
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

[[nodiscard]] inline std::array<std::uint8_t, 32> hmac_sha256(std::string_view key,
                                                              std::string_view message)
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
    for (const std::uint8_t byte : block) {
        inner.push_back(static_cast<char>(byte ^ 0x36u));
        outer.push_back(static_cast<char>(byte ^ 0x5cu));
    }
    inner.append(message);
    const auto inner_digest = sha256(inner);
    outer.append(reinterpret_cast<const char*>(inner_digest.data()), inner_digest.size());
    return sha256(outer);
}

[[nodiscard]] inline std::string totp_for_counter(std::string_view key, long long counter)
{
    std::array<char, 8> message{};
    for (int i = 7; i >= 0; --i) {
        message[static_cast<std::size_t>(i)] = static_cast<char>(counter & 0xffu);
        counter >>= 8;
    }
    const auto mac = hmac_sha256(key, std::string_view{message.data(), message.size()});
    const unsigned offset = static_cast<unsigned>(mac.back()) & 0x0fu;
    const unsigned value =
        ((static_cast<unsigned>(mac[offset]) & 0x7fu) << 24u) |
        (static_cast<unsigned>(mac[offset + 1u]) << 16u) |
        (static_cast<unsigned>(mac[offset + 2u]) << 8u) |
        static_cast<unsigned>(mac[offset + 3u]);
    return std::format("{:06d}", value % 1000000u);
}

} // namespace detail

[[nodiscard]] inline std::string totp_key(std::string_view user_id,
                                          std::string_view secret,
                                          std::string_view context)
{
    return std::string{secret} + std::string{user_id} + std::string{context};
}

/** otplib.totp.generate with step 60 and 6 digits. */
[[nodiscard]] inline std::string totp_generate(std::string_view user_id,
                                               std::string_view secret,
                                               std::string_view context,
                                               long long at_ms = 0)
{
    const long long step = at_ms == 0 ? now_ms() / 60000 : at_ms / 60000;
    return detail::totp_for_counter(totp_key(user_id, secret, context), step);
}

/** otplib.totp.check with window [10, 10]. */
[[nodiscard]] inline bool totp_verify(std::string_view user_id,
                                      std::string_view token,
                                      std::string_view secret,
                                      std::string_view context,
                                      long long at_ms = 0)
{
    if (token.size() != 6) return false;
    const long long step = (at_ms == 0 ? now_ms() : at_ms) / 60000;
    for (long long delta = -10; delta <= 10; ++delta) {
        if (detail::totp_for_counter(totp_key(user_id, secret, context), step + delta) == token) {
            return true;
        }
    }
    return false;
}

// ---------------------------------------------------------------------------
// Session record persistence
// ---------------------------------------------------------------------------

[[nodiscard]] inline SessionState read_session(blog::BlogContext& ctx, std::string_view sid)
{
    SessionState state;
    if (sid.empty()) return state;
    const auto record = ctx.store().resources().get("session", sid);
    if (record.empty()) return state;
    const auto fields = blog::api::JsonBody{}.parse(record);
    state.id = sid;
    const auto get = [&fields](std::string_view key) -> std::string {
        const auto it = fields.find(std::string{key});
        return it == fields.end() ? std::string{} : it->second;
    };
    state.user_id = get("user_id");
    state.origin = get("origin");
    state.user_agent = get("user_agent");
    state.ip = get("ip");
    state.verified = get("verified") == "true";
    state.verified_user_id = get("verified_user_id");
    state.auth_code_challenge = get("auth_code_challenge");
    try {
        state.auth_code_generated_at = std::stoll(get("auth_code_generated_at"));
    } catch (...) {
        state.auth_code_generated_at = 0;
    }
    return state;
}

[[nodiscard]] inline std::string session_json(const SessionState& state)
{
    return std::format(
        R"({{"id":{},"user_id":{},"origin":{},"user_agent":{},"ip":{},"verified":{},"verified_user_id":{},"auth_code_challenge":{},"auth_code_generated_at":{}}})",
        quote(state.id), quote(state.user_id), quote(state.origin), quote(state.user_agent),
        quote(state.ip), state.verified ? "true" : "false", quote(state.verified_user_id),
        quote(state.auth_code_challenge), state.auth_code_generated_at);
}

inline void write_session(blog::BlogContext& ctx, const SessionState& state)
{
    if (state.id.empty()) return;
    ctx.store().resources().edit("session", state.id, session_json(state));
}

[[nodiscard]] inline bool has_valid_challenge(const SessionState& state)
{
    return !state.auth_code_challenge.empty() && state.auth_code_generated_at > 0 &&
           now_ms() - state.auth_code_generated_at <= 5 * 60 * 1000;
}

inline void invalidate_challenge(SessionState& state)
{
    state.auth_code_challenge.clear();
    state.auth_code_generated_at = 0;
}

inline void rotate_challenge(SessionState& state)
{
    state.auth_code_challenge = random_hex(16); // AUTH_CODE_CHALLENGE_BYTES
    state.auth_code_generated_at = now_ms();
}

[[nodiscard]] inline std::string user_email(blog::BlogContext& ctx, std::string_view user_id)
{
    if (user_id.empty()) return {};
    const auto record = ctx.store().find_by_resource("users", std::string{user_id});
    if (!record) return {};
    const auto fields = blog::api::JsonBody{}.parse(*record);
    const auto it = fields.find("email");
    return it == fields.end() ? std::string{} : it->second;
}

/** sendAuthCodeToUser: rotate challenge, generate TOTP and emit the sign-in mail. */
[[nodiscard]] inline std::string send_auth_code(blog::BlogContext& ctx,
                                                interfaces::io::IRequest& request,
                                                SessionState& state)
{
    if (state.user_id.empty()) return {};
    rotate_challenge(state);
    const auto token = totp_generate(state.user_id, admin_session_secret(ctx),
                                     state.auth_code_challenge);
    const auto email = user_email(ctx, state.user_id);
    const auto device = std::format("{}, {}", header(request, "user-agent"), state.ip);
    blog::handlers::outbox::global_outbox().emit(blog::handlers::outbox::Event{
        .type = "mail.signin_code",
        .payload = std::format(R"({{"to":{},"token":{},"device":{},"is_2fa_required":{}}})",
                               quote(email), quote(token), quote(device),
                               require_email_mfa(ctx) ? "true" : "false"),
        .aggregate_id = state.user_id});
    return token;
}

/** verifySession: bind verification to the user, drop the challenge, reset the mail budget. */
inline void verify_session(blog::BlogContext& ctx, SessionState& state)
{
    state.verified = true;
    state.verified_user_id = state.user_id;
    invalidate_challenge(state);
    blog::handlers::outbox::global_outbox().emit(blog::handlers::outbox::Event{
        .type = "verification_email_limit.reset",
        .payload = std::format(R"({{"user_id":{}}})", quote(state.user_id)),
        .aggregate_id = state.user_id});
    write_session(ctx, state);
}

[[nodiscard]] inline bool is_verified(const SessionState& state)
{
    return state.verified && !state.verified_user_id.empty() &&
           state.verified_user_id == state.user_id;
}

// ---------------------------------------------------------------------------
// Ghost-shaped errors
// ---------------------------------------------------------------------------

[[nodiscard]] inline std::string access_denied()
{
    return core::auth::ghost::unauthorized_error_json("Access Denied.");
}

[[nodiscard]] inline std::string unauthorized()
{
    return core::auth::ghost::unauthorized_error_json("Unauthorized");
}

[[nodiscard]] inline std::string needs_2fa(bool required)
{
    return envelope("User must verify session to login.", "Needs2FAError",
                    required ? "2FA_TOKEN_REQUIRED" : "2FA_NEW_DEVICE_DETECTED",
                    "A 6-digit sign-in verification code has been sent to your email to keep your account safe.");
}

[[nodiscard]] inline std::string bad_request(std::string_view message)
{
    return envelope(message, "BadRequestError", "", {});
}

} // namespace blog::handlers::session::support

// ---------------------------------------------------------------------------
// POST /session — controller.add -> auth.session.createSession
// ---------------------------------------------------------------------------

export namespace blog::handlers::session::create::workflow {

class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"session_create_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output> invoke(
        const Input& in, core::workflow::WorkflowContext&) const
    {
        using namespace support;
        auto& ctx = *s_ctx;
        auto& store = ctx.store();

        const auto fields = blog::api::JsonBody{}.parse(in.body_as_string());
        const auto username = fields.contains("username") ? fields.at("username") : std::string{};
        const auto password = fields.contains("password") ? fields.at("password") : std::string{};
        const auto token = fields.contains("token") ? fields.at("token") : std::string{};
        if (username.empty() || password.empty()) return denied();

        // models.User.getByEmail + models.User.check (session.ts:36-45).
        std::string user_id;
        bool has_logged_in = false;
        for (const auto& row : blog::api::parse_array(store.json_for("users", {{"filter", "email:" + username}}))) {
            const auto idit = row.find("id");
            if (idit == row.end()) continue;
            const auto record = store.find_by_resource("users", idit->second);
            if (!record) continue;
            const auto user = blog::api::JsonBody{}.parse(*record);
            const auto email = user.contains("email") ? user.at("email") : std::string{};
            const auto stored = user.contains("password") ? user.at("password") : std::string{};
            if (email != username || !verify_password(stored, password)) continue;
            user_id = idit->second;
            const auto seen = user.find("last_seen");
            has_logged_in = seen != user.end() && !seen->second.empty();
            break;
        }
        if (user_id.empty()) return denied();

        // User.check side effects: updateLastSeen + save(status:active).
        (void)store.edit("users", user_id,
                   std::format(R"({{"last_seen":{},"status":"active"}})",
                               quote(std::format("{:%Y-%m-%dT%H:%M:%SZ}",
                                                 std::chrono::floor<std::chrono::seconds>(
                                                     std::chrono::system_clock::now())))));

        // req.session.regenerate -> a fresh server session id, carrying the
        // prior session's verification (bound to this user) and auth challenge
        // (session-service.ts:242-289).
        const auto previous = read_session(ctx, cookie_session_id(in));
        const auto sid = store.resources().create("session", "{}");
        SessionState state;
        state.id = sid;
        state.user_id = user_id;
        state.origin = origin_of_request(in);
        state.user_agent = header(in, "user-agent");
        state.ip = header(in, "x-forwarded-for");
        state.auth_code_challenge = previous.auth_code_challenge;
        state.auth_code_generated_at = previous.auth_code_generated_at;
        if (previous.verified && previous.verified_user_id == user_id) {
            state.verified = true;
            state.verified_user_id = previous.verified_user_id;
        }

        bool verified = is_verified(state);
        if (!verified && !token.empty() && has_valid_challenge(state)) {
            if (totp_verify(user_id, token, admin_session_secret(ctx), state.auth_code_challenge)) {
                verify_session(ctx, state);
                verified = true;
            }
        }
        if (!verified && !has_logged_in) {
            // skipVerification: first-ever login is a verified session (session.ts:37-40).
            verify_session(ctx, state);
            verified = true;
        }
        if (!verified && !staff_device_verification(ctx)) {
            // isStaffDeviceVerificationDisabled() auto-verifies (session-service.ts:236-239).
            verify_session(ctx, state);
            verified = true;
        }
        write_session(ctx, state);
        const auto cookie = session_cookie(ctx, sid, in);

        if (!verified) {
            // createSession -> sendAuthCodeToUser -> Needs2FAError (middleware.ts:20-31).
            (void)send_auth_code(ctx, in, state);
            write_session(ctx, state);
            return core::workflow::StepResponse<Output>{{{"status", "403"},
                {"body", needs_2fa(require_email_mfa(ctx))}, {"set_cookie", cookie}}};
        }
        return core::workflow::StepResponse<Output>{{{"status", "201"}, {"body", R"({})"},
                                                       {"set_cookie", cookie}}};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
    static blog::BlogContext* s_ctx;

private:
    [[nodiscard]] static core::workflow::StepResponse<Output> denied()
    {
        return core::workflow::StepResponse<Output>{{{"status", "401"}, {"body", support::access_denied()}}};
    }
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};

} // namespace blog::handlers::session::create::workflow
