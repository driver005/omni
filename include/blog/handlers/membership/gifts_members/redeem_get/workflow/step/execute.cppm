export module blog_handlers:gifts_members_redeem_get_step;
import :domain_all;
import std;
import core_workflow;
import interfaces;

// Strict-parity support shared by both members-app gift-redemption steps. It lives in
// this (already registered) partition so the POST step can import it without a new
// partition, a shared-file edit or a BUILD change. Wire values follow Ghost's
// `serializeRedemption` / `Gift.checkRedeemable` (gift-service.ts:282-300,768-814,1536-1573).
export namespace blog::handlers::gifts_members::support {

using Record = blog::api::RecordDto;

[[nodiscard]] inline std::string quote(std::string_view value)
{
    return blog::api::quote_fragment(value);
}

[[nodiscard]] inline std::string raw_field(const Record& record, std::string_view key)
{
    const auto it = record.find(std::string{key});
    return it == record.end() ? std::string{} : it->second;
}

/** Unquote a scalar field; `null`/absent collapse to the caller's fallback. */
[[nodiscard]] inline std::string field(const Record& record, std::string_view key,
                                       std::string_view fallback = {})
{
    const std::string raw = raw_field(record, key);
    if (raw.empty() || raw == "null") {
        return std::string{fallback};
    }
    if (raw.front() == '"' && raw.size() >= 2 && raw.back() == '"') {
        return raw.substr(1, raw.size() - 2);
    }
    return raw;
}

[[nodiscard]] inline bool present(const Record& record, std::string_view key)
{
    return !field(record, key).empty();
}

/** Ghost emits the model value verbatim; no fabricated defaults when a fixture omits it. */
[[nodiscard]] inline std::string wire_string(const Record& record, std::string_view key)
{
    return present(record, key) ? quote(field(record, key)) : std::string{"null"};
}

[[nodiscard]] inline std::string wire_number(const Record& record, std::string_view key)
{
    const std::string raw = raw_field(record, key);
    return (raw.empty() || raw == "null") ? std::string{"null"} : raw;
}

[[nodiscard]] inline std::string nullable(const Record& record, std::string_view key,
                                          std::string_view fallback = {})
{
    if (present(record, key)) {
        return quote(field(record, key));
    }
    // Fallback is an already-resolved value (e.g. a camelCase fixture alias).
    return fallback.empty() ? std::string{"null"} : quote(fallback);
}

/** `:token` is the second-to-last non-empty path segment ("/gifts/<token>/redeem"). */
[[nodiscard]] inline std::string token_from_path(std::string_view path)
{
    path = path.substr(0, path.find('?'));
    std::vector<std::string_view> seg;
    for (auto part : path | std::views::split('/')) {
        if (!part.empty()) {
            seg.emplace_back(part.begin(), part.end());
        }
    }
    return seg.size() >= 3 ? std::string{seg[seg.size() - 2]} : std::string{};
}

/** Full Ghost error envelope (id/code/context/details/ghostErrorCode/help/message/property/type). */
[[nodiscard]] inline std::string ghost_error(std::string_view code, std::string_view type,
                                             std::string_view message)
{
    static std::uint64_t counter{0};
    return std::format(
        R"({{"errors":[{{"id":"{:032x}","code":{},"context":null,"details":null,"ghostErrorCode":null,"help":null,"message":{},"property":null,"type":{}}}]}})",
        ++counter, code.empty() ? "null" : quote(code), quote(message), quote(type));
}

[[nodiscard]] inline std::string validation_error(std::string_view message,
                                                  std::string_view property)
{
    return blog::api::ApiError{}.validation("Validation error", message, property);
}

struct Rejection {
    std::string status;
    std::string code;
    std::string type;
    std::string message;
};

/** Howard Hinnant's civil-date helpers (UTC, proleptic Gregorian). */
[[nodiscard]] inline long long days_from_civil(long long y, unsigned m, unsigned d) noexcept
{
    y -= (m <= 2);
    const long long era = (y >= 0 ? y : y - 399) / 400;
    const unsigned yoe = static_cast<unsigned>(y - era * 400);
    const unsigned doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
    const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097 + static_cast<long long>(doe) - 719468;
}

struct IsoParts {
    long long year{0};
    unsigned month{0};
    unsigned day{0};
    unsigned hour{0};
    unsigned minute{0};
    unsigned second{0};
    unsigned millis{0};
};

/** Parse `YYYY-MM-DDTHH:MM:SS[.mmm][Z|+HH:MM]`; offsets are treated as UTC. */
[[nodiscard]] inline std::optional<IsoParts> iso_parts(std::string_view text)
{
    if (text.size() < 19) {
        return std::nullopt;
    }
    auto digits = [&](std::size_t pos, std::size_t len) -> std::optional<long long> {
        if (pos + len > text.size()) {
            return std::nullopt;
        }
        long long value = 0;
        for (std::size_t i = 0; i < len; ++i) {
            const char c = text[pos + i];
            if (c < '0' || c > '9') {
                return std::nullopt;
            }
            value = value * 10 + (c - '0');
        }
        return value;
    };
    if (text[4] != '-' || text[7] != '-' ||
        (text[10] != 'T' && text[10] != ' ') || text[13] != ':' || text[16] != ':') {
        return std::nullopt;
    }
    const auto year = digits(0, 4);
    const auto month = digits(5, 2);
    const auto day = digits(8, 2);
    const auto hour = digits(11, 2);
    const auto minute = digits(14, 2);
    const auto second = digits(17, 2);
    if (!year || !month || !day || !hour || !minute || !second) {
        return std::nullopt;
    }
    IsoParts out{*year, static_cast<unsigned>(*month), static_cast<unsigned>(*day),
                 static_cast<unsigned>(*hour), static_cast<unsigned>(*minute),
                 static_cast<unsigned>(*second), 0};
    if (text.size() > 20 && text[19] == '.') {
        std::size_t end = 20;
        while (end < text.size() && text[end] >= '0' && text[end] <= '9') {
            ++end;
        }
        const auto ms = digits(20, end - 20);
        if (ms) {
            out.millis = static_cast<unsigned>(*ms);
        }
    }
    return out;
}

[[nodiscard]] inline std::optional<long long> iso_ms(std::string_view text)
{
    const auto p = iso_parts(text);
    if (!p) {
        return std::nullopt;
    }
    return days_from_civil(p->year, p->month, p->day) * 86400000LL +
           static_cast<long long>((p->hour * 3600ULL + p->minute * 60ULL + p->second) * 1000ULL) +
           p->millis;
}

[[nodiscard]] inline long long now_ms() noexcept
{
    using namespace std::chrono;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

/** Ghost `new Date()` wire shape; real clock (no fabricated epoch constant). */
[[nodiscard]] inline std::string now_iso()
{
    const long long tick = now_ms();
    const long long seconds = tick / 1000;
    const long long millis = tick % 1000;
    long long z = seconds / 86400 + 719468;
    const long long era = (z >= 0 ? z : z - 146096) / 146097;
    const unsigned doe = static_cast<unsigned>(z - era * 146097);
    const unsigned yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    long long year = static_cast<long long>(yoe) + era * 400;
    const unsigned doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    const unsigned mp = (5 * doy + 2) / 153;
    const unsigned day = doy - (153 * mp + 2) / 5 + 1;
    const unsigned month = mp < 10 ? mp + 3 : mp - 9;
    year += (month <= 2) ? 1 : 0;
    const unsigned sod = static_cast<unsigned>(seconds % 86400);
    return std::format("{:04}-{:02}-{:02}T{:02}:{:02}:{:02}.{:03}Z",
                       year, month, day, sod / 3600, (sod % 3600) / 60, sod % 60, millis);
}

/** `Gift.redeem`: redeemedAt + duration months (or years for a yearly cadence). */
[[nodiscard]] inline std::string consumes_at(std::string_view redeemed_at,
                                             std::string_view cadence,
                                             std::string_view duration)
{
    const auto p = iso_parts(redeemed_at);
    long long years = 0;
    long long months = 0;
    try {
        if (!duration.empty()) {
            months = std::stoll(std::string{duration});
        }
    } catch (...) {
        months = 0;
    }
    if (!p) {
        return std::string{redeemed_at};
    }
    if (cadence == "year") {
        years = months; // duration is years for a yearly cadence
        months = 0;
    }
    const long long total = p->year * 12 + static_cast<long long>(p->month - 1) + months + years * 12;
    long long year = total / 12;
    unsigned month = static_cast<unsigned>(total % 12 + 1);
    if (month == 0) {
        --year;
        month = 12;
    }
    // JS setMonth/setFullYear roll day overflow into the following month.
    const long long days = days_from_civil(year, month, 1) + (p->day - 1);
    long long z = days + 719468;
    const long long era = (z >= 0 ? z : z - 146096) / 146097;
    const unsigned doe = static_cast<unsigned>(z - era * 146097);
    const unsigned yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    year = static_cast<long long>(yoe) + era * 400;
    const unsigned doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    const unsigned mp = (5 * doy + 2) / 153;
    const unsigned day = doy - (153 * mp + 2) / 5 + 1;
    month = mp < 10 ? mp + 3 : mp - 9;
    year += (month <= 2) ? 1 : 0;
    return std::format("{:04}-{:02}-{:02}T{:02}:{:02}:{:02}.{:03}Z",
                       year, month, day, p->hour, p->minute, p->second, p->millis);
}

/** `Gift.checkRedeemable` reason -> `assertRedeemable` error mapping, exact codes/messages. */
[[nodiscard]] inline std::optional<Rejection>
redeemability(const Record& gift, std::string_view memberStatus)
{
    if (field(gift, "status") == "payment_pending" ||
        field(gift, "payment_pending") == "true") {
        return Rejection{"404", "GIFT_NOT_FOUND", "NotFoundError", "This gift does not exist."};
    }
    // Ghost tests the timestamp fields, not the display status. Status strings remain as a
    // fixture-compatibility fallback only when the timestamp is absent.
    if (present(gift, "redeemed_at") || (!present(gift, "redeemed_at") && field(gift, "status") == "redeemed")) {
        return Rejection{"400", "GIFT_REDEEMED", "BadRequestError", "This gift has already been redeemed."};
    }
    if (present(gift, "consumed_at") || (!present(gift, "consumed_at") && field(gift, "status") == "consumed")) {
        return Rejection{"400", "GIFT_CONSUMED", "BadRequestError", "This gift has already been consumed."};
    }
    if (present(gift, "refunded_at") || (!present(gift, "refunded_at") && field(gift, "status") == "refunded")) {
        return Rejection{"400", "GIFT_REFUNDED", "BadRequestError", "This gift has been refunded."};
    }
    const bool expiredFlag = present(gift, "expired_at") ||
                             (!present(gift, "expired_at") && field(gift, "status") == "expired");
    const auto deadline = present(gift, "expires_at") ? iso_ms(field(gift, "expires_at")) : std::nullopt;
    const bool pastDeadline = deadline && *deadline < now_ms();
    if (expiredFlag || pastDeadline) {
        return Rejection{"400", "GIFT_EXPIRED", "BadRequestError", "This gift has expired."};
    }
    // Ghost: any truthy member status other than `free` is a `paid-member` rejection.
    if (!memberStatus.empty() && memberStatus != "free") {
        return Rejection{"400", "GIFT_PAID_MEMBER", "BadRequestError",
                         "You already have an active subscription."};
    }
    return std::nullopt;
}

/** `find_by_resource` has no "gifts" branch; `json_for("gifts")` lists the resource store. */
[[nodiscard]] inline std::optional<Record> find_gift(blog::BlogContext& ctx, const std::string& token)
{
    for (auto& record : blog::api::parse_array(ctx.store().json_for("gifts"))) {
        if (field(record, "token") == token) {
            return record;
        }
    }
    return std::nullopt;
}

/** `giftDeliveryService.getRecipientEmailForGift` -> first delivery's recipient_email. */
[[nodiscard]] inline std::string recipient_email(blog::BlogContext& ctx, const std::string& token)
{
    for (auto& delivery : blog::api::parse_array(ctx.store().json_for("gift_deliveries"))) {
        if (field(delivery, "gift_token") == token || field(delivery, "token") == token) {
            return field(delivery, "recipient_email", field(delivery, "recipientEmail"));
        }
    }
    return std::string{};
}

[[nodiscard]] inline std::optional<std::string> tier_object(blog::BlogContext& ctx,
                                                            const std::string& tierId)
{
    if (tierId.empty()) {
        return std::nullopt;
    }
    const auto tier = ctx.store().find_by_resource("tiers", tierId);
    if (!tier) {
        return std::nullopt;
    }
    const auto rows = blog::api::parse_array("[" + *tier + "]");
    if (rows.empty()) {
        return std::nullopt;
    }
    const auto& t = rows.front();
    const std::string benefits = raw_field(t, "benefits");
    return "{\"id\":" + quote(field(t, "id", tierId)) +
           ",\"name\":" + wire_string(t, "name") +
           ",\"description\":" + nullable(t, "description") +
           ",\"benefits\":" + (benefits.empty() || benefits == "null" ? std::string{"[]"} : benefits) + "}";
}

struct Serialization {
    std::string status;
    std::string body;
};

/** Bare snake_case GiftRedemption object (gift-service.ts:282-300,1536-1573). */
[[nodiscard]] inline Serialization serialize_redemption(blog::BlogContext& ctx,
                                                        const Record& gift,
                                                        std::string_view token)
{
    const std::string tierId = field(gift, "tier_id", field(gift, "tierId"));
    const auto tier = tier_object(ctx, tierId);
    if (!tier) {
        return {"500", ghost_error("", "InternalServerError",
            "Tier " + tierId + " not found for gift: " + std::string{token})};
    }
    const std::string email = recipient_email(ctx, std::string{token});
    const std::string payload =
        "{\"token\":" + wire_string(gift, "token") +
        ",\"cadence\":" + wire_string(gift, "cadence") +
        ",\"duration\":" + wire_number(gift, "duration") +
        ",\"currency\":" + wire_string(gift, "currency") +
        ",\"amount\":" + wire_number(gift, "amount") +
        ",\"buyer_name\":" + nullable(gift, "buyer_name", field(gift, "buyerName")) +
        ",\"recipient_name\":" + nullable(gift, "recipient_name", field(gift, "recipientName")) +
        ",\"recipient_email\":" + (email.empty() ? std::string{"null"} : quote(email)) +
        ",\"message\":" + nullable(gift, "personal_message", field(gift, "message")) +
        ",\"expires_at\":" + nullable(gift, "expires_at") +
        ",\"consumes_at\":" + nullable(gift, "consumes_at") +
        ",\"tier\":" + *tier + "}";
    return {"200", payload};
}

} // namespace blog::handlers::gifts_members::support

export namespace blog::handlers::gifts_members::redeem_get::workflow {

class ExecuteStep {
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"gifts_members_redeem_get_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        namespace support = blog::handlers::gifts_members::support;
        auto& ctx = *s_ctx;

        // Ghost: data:['token'], token validated string/required (gifts-members.js:11-19).
        const std::string token = support::token_from_path(in.get_path());
        if (token.empty()) {
            return result("400", support::validation_error("A token is required", "token"));
        }

        // Ghost: frame.options.context.member.status ?? null (gift-controller.ts:27). No
        // C++ member-session primitive exists; the optional status arrives as a header.
        const std::string memberStatus{in.find_header("X-Member-Status")};

        const auto gift = support::find_gift(ctx, token);
        if (!gift) {
            return result("404", support::ghost_error(
                "GIFT_NOT_FOUND", "NotFoundError", "This gift does not exist."));
        }

        if (const auto rejected = support::redeemability(*gift, memberStatus)) {
            return result(rejected->status,
                          support::ghost_error(rejected->code, rejected->type, rejected->message));
        }

        const auto out = support::serialize_redemption(ctx, *gift, token);
        return result(out.status, out.body);
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;

private:
    static core::workflow::StepResponse<Output> result(std::string status, std::string body)
    {
        return core::workflow::StepResponse<Output>{{{"status", std::move(status)}, {"body", std::move(body)}}};
    }
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};

} // namespace blog::handlers::gifts_members::redeem_get::workflow
