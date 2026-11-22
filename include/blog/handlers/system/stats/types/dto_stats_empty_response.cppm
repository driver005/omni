module;
#include <rfl.hpp>
export module blog_handlers:stats_dto_stats_empty_response;

import std;
import core_serde;
import core_query;
import core_auth;
import :domain_all;
import :outbox_shim;
import blog_handlers_middleware;

export namespace blog::dto::stats {

namespace local_mapping {
[[nodiscard]] inline std::string wrap(std::string_view resource, std::string_view rows,
                                      std::string_view meta = {})
{
    return "{\"" + std::string{resource} + "\":" + std::string{rows} +
           (meta.empty() ? std::string{} : ",\"meta\":" + std::string{meta}) + "}";
}
[[nodiscard]] inline std::string clean(std::string_view json, std::string_view) { return std::string{json}; }
[[nodiscard]] inline std::string pagination(std::size_t total, std::size_t limit, std::size_t page,
                                            std::size_t pages, std::string_view prev,
                                            std::string_view next)
{
    return std::format(R"("total":{},"limit":{},"page":{},"pages":{},"prev":{},"next":{})",
                       total, limit, page, pages,
                       prev.empty() ? std::string{"null"} : std::string{prev},
                       next.empty() ? std::string{"null"} : std::string{next});
}
} // namespace local_mapping

/** @brief Ghost API DTO: StatsEmptyResponse. */
class StatsEmptyResponse
{
public:
    void set_stats(std::vector<rfl::Generic> value) { stats = std::move(value); }
    [[nodiscard]] const std::vector<rfl::Generic>& get_stats() const noexcept { return stats; }

    std::vector<rfl::Generic> stats{};
};

} // namespace blog::dto::stats

// ---------------------------------------------------------------------------
// Shared stats helpers. Kept in this already-registered partition so every one
// of the 17 stats steps/handlers uses the same Ghost-shaped primitives:
//   core::query           - allowlisted option parsing / validation
//   blog::mapping         - output clean + `{"stats":[...],"meta":...}` envelope
//   core_auth + middleware- Ghost permission gate (members/browse | posts/browse)
//   outbox_shim           - observable record of Tinybird outbound calls
// No shared file outside include/blog/handlers/system/stats/ is modified.
// ---------------------------------------------------------------------------
export namespace blog::handlers::stats::support {

inline constexpr std::string_view kDocName{"stats"};

[[nodiscard]] inline std::string quote(std::string_view text)
{
    return blog::api::JsonBody{}.quote(text);
}

[[nodiscard]] inline std::string unquote(std::string_view value)
{
    auto text = value;
    while (!text.empty() && std::isspace(static_cast<unsigned char>(text.front()))) text.remove_prefix(1);
    while (!text.empty() && std::isspace(static_cast<unsigned char>(text.back()))) text.remove_suffix(1);
    if (text.size() >= 2 && text.front() == '"' && text.back() == '"') {
        std::string out;
        for (std::size_t i = 1; i + 1 < text.size(); ++i) {
            if (text[i] == '\\' && i + 2 < text.size()) { out.push_back(text[i + 1]); ++i; }
            else out.push_back(text[i]);
        }
        return out;
    }
    return std::string{text};
}

[[nodiscard]] inline std::string plain_field(const blog::api::RecordDto& record, std::string_view key)
{
    const auto it = record.find(std::string{key});
    return it == record.end() ? std::string{} : unquote(it->second);
}

/** @brief default.all for `stats` has no mapper: expose every field except the
 *  restricted ones (the same guard `dto::stats::local_mapping::clean` applies to unknown kinds). */
[[nodiscard]] inline std::string clean(std::string_view rows)
{
    return dto::stats::local_mapping::clean(rows, "stats");
}

/** @brief Ghost output serializer: `frame.response.stats = data.map(...)`, `meta` copied. */
[[nodiscard]] inline std::string wrap(std::string_view rows, std::string_view meta = {})
{
    auto text = rows;
    while (!text.empty() && std::isspace(static_cast<unsigned char>(text.front()))) text.remove_prefix(1);
    while (!text.empty() && std::isspace(static_cast<unsigned char>(text.back()))) text.remove_suffix(1);
    // clean("") yields "{}" (no members); an empty store must stay an empty array.
    if (text.empty()) return dto::stats::local_mapping::wrap(kDocName, "[]", meta);
    return dto::stats::local_mapping::wrap(kDocName, clean(text), meta);
}

[[nodiscard]] inline std::string error(std::string_view message, std::string_view type,
                                       std::string_view property = {}, std::string_view context = {})
{
    return blog::api::ApiError{}.validation(message, context, property, type);
}

[[nodiscard]] inline std::string invalid(std::string_view message,
                                         std::string_view property = {}, std::string_view context = {})
{
    return error(message, "ValidationError", property, context);
}

[[nodiscard]] inline std::string bad_request(std::string_view message)
{
    return error(message, "BadRequestError");
}

[[nodiscard]] inline std::string internal_error(std::string_view message)
{
    return blog::api::ApiError{}.json(message, "", "");
}

[[nodiscard]] inline std::map<std::string, std::string> result(std::string_view status, std::string body)
{
    return {{"status", std::string{status}}, {"body", std::move(body)}};
}

[[nodiscard]] inline std::string join(const std::vector<std::string>& values, std::string_view sep)
{
    std::string out;
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (i != 0u) out += sep;
        out += values[i];
    }
    return out;
}

/** @brief Segment after `/posts/` up to the next `/` or `?` (the Ghost `:id`). */
[[nodiscard]] inline std::string path_id(std::string_view path)
{
    const auto at = path.find("/posts/");
    if (at == std::string_view::npos) return {};
    const auto start = at + 7u;
    auto end = path.size();
    if (const auto slash = path.find('/', start); slash != std::string_view::npos) end = slash;
    if (const auto query = path.find('?', start); query != std::string_view::npos && query < end) end = query;
    return std::string{path.substr(start, end - start)};
}

/** @brief Reads `data.<key>` (framework frame.data) or a top-level `<key>` array. */
[[nodiscard]] inline std::optional<std::vector<std::string>> body_array(std::string_view body,
                                                                        std::string_view key)
{
    const auto decoded = core_serde::JsonCodec{}.decode<serde::Value>(body);
    if (!decoded) return std::nullopt;
    const auto* root = std::get_if<serde::Value::Object>(&decoded->get());
    if (root == nullptr) return std::nullopt;

    const serde::Value* array_value = nullptr;
    for (const auto& [name, value] : *root) {
        if (name == key) { array_value = &value; break; }
        if (name == "data") {
            const auto* data = std::get_if<serde::Value::Object>(&value.get());
            if (data == nullptr) continue;
            for (const auto& [data_name, data_value] : *data) {
                if (data_name == key) { array_value = &data_value; break; }
            }
        }
    }
    if (array_value == nullptr) return std::nullopt;
    const auto* array = std::get_if<serde::Value::Array>(&array_value->get());
    if (array == nullptr) return std::nullopt;

    std::vector<std::string> out;
    out.reserve(array->size());
    for (const auto& item : *array) {
        if (const auto* text = std::get_if<std::string>(&item.get())) out.push_back(*text);
        else out.push_back(core_serde::JsonCodec{}.encode(item));
    }
    return out;
}

/** @brief Only Ghost options that map to service filters go to the store stand-in. */
[[nodiscard]] inline std::map<std::string, std::string> store_params(const core::query::ParsedQuery& q)
{
    std::map<std::string, std::string> params;
    for (const auto& [key, value] : q.options) {
        if (key == "fail_on_error" || key == "timezone" || key == "order" ||
            key == "limit" || key == "page" || key == "after_id" || key == "after_date") continue;
        params["filter[" + key + "]"] = value;
    }
    return params;
}

[[nodiscard]] inline bool order_valid(std::string_view order, std::span<const std::string_view> fields)
{
    if (order.empty()) return true;
    if (!core::query::valid_option("order", order)) return false;
    for (auto piece : std::views::split(order, ',')) {
        const std::string text{std::ranges::begin(piece), std::ranges::end(piece)};
        const auto field = text.substr(0, text.find_first_of(" \t"));
        bool found = false;
        for (const auto allowed : fields) {
            std::string a{field};
            std::string b{allowed};
            std::ranges::transform(a, a.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            std::ranges::transform(b, b.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            if (a == b) { found = true; break; }
        }
        if (!found) return false;
    }
    return true;
}

/** @brief Tinybird adapter stand-in. `core_adapters::TinybirdAdapter` is not a
 *  dep of `blog_handlers` (new-dep need recorded in docs/parity/stats.md); when
 *  a local Tinybird mock is configured we record the outbound pipe query on the
 *  registered outbox shim, exactly as the real adapter does for verification. */
inline void record_tinybird(std::string_view pipe, std::string_view params)
{
    const char* base = std::getenv("TINYBIRD_BASE_URL"); // NOLINT(concurrency-mt-unsafe)
    if (base == nullptr || *base == '\0') return;
    blog::handlers::outbox::global_outbox().emit(blog::handlers::outbox::Event{
        .type = "tinybird.stats.query",
        .payload = "{\"pipe\":" + quote(pipe) + ",\"params\":" + quote(params) + "}",
        .aggregate_id = "stats",
    });
}

[[nodiscard]] inline double number_field(const blog::api::RecordDto& record, std::string_view key)
{
    const auto text = plain_field(record, key);
    if (text.empty()) return 0.0;
    try {
        return std::stod(text);
    } catch (...) {
        return 0.0;
    }
}

/** @brief Ghost `meta.totals` for member_count: current counts by status. */
[[nodiscard]] inline std::string meta_member_totals(std::string_view rows)
{
    double paid = 0.0, free = 0.0, comped = 0.0, gift = 0.0;
    for (const auto& row : blog::api::parse_array(rows)) {
        paid += number_field(row, "paid");
        free += number_field(row, "free");
        comped += number_field(row, "comped");
        gift += number_field(row, "gift");
    }
    const auto whole = [](double value) { return std::to_string(static_cast<long long>(value)); };
    return "{\"totals\":{\"paid\":" + whole(paid) + ",\"free\":" + whole(free) +
           ",\"comped\":" + whole(comped) + ",\"gift\":" + whole(gift) + "}}";
}

/** @brief Ghost `meta.totals` for mrr: per-currency totals, USD placeholder when empty. */
[[nodiscard]] inline std::string meta_mrr_totals(std::string_view rows)
{
    std::string out;
    for (const auto& row : blog::api::parse_array(rows)) {
        const auto currency = plain_field(row, "currency");
        if (currency.empty()) continue;
        if (!out.empty()) out += ',';
        out += "{\"currency\":" + quote(currency) + ",\"mrr\":" +
               std::to_string(static_cast<long long>(number_field(row, "mrr"))) + "}";
    }
    if (out.empty()) out = R"({"currency":"usd","mrr":0})";
    return "{\"totals\":[" + out + "]}";
}

/** @brief Ghost `meta` for subscriptions: cadences / tiers / totals. */
[[nodiscard]] inline std::string meta_subscriptions(std::string_view rows)
{
    std::string cadences;
    std::string tiers;
    std::string totals;
    for (const auto& row : blog::api::parse_array(rows)) {
        const auto cadence = plain_field(row, "cadence");
        const auto tier = plain_field(row, "tier");
        if (!cadence.empty() && cadences.find(quote(cadence)) == std::string::npos) {
            if (!cadences.empty()) cadences += ',';
            cadences += quote(cadence);
        }
        if (!tier.empty() && tiers.find(quote(tier)) == std::string::npos) {
            if (!tiers.empty()) tiers += ',';
            tiers += quote(tier);
        }
    }
    for (const auto& row : blog::api::parse_array(rows)) {
        const auto tier = plain_field(row, "tier");
        const auto cadence = plain_field(row, "cadence");
        if (tier.empty() || cadence.empty()) continue;
        if (!totals.empty()) totals += ',';
        totals += "{\"tier\":" + quote(tier) + ",\"cadence\":" + quote(cadence) +
                  ",\"count\":" + std::to_string(static_cast<long long>(number_field(row, "count"))) + "}";
    }
    return "{\"cadences\":[" + cadences + "],\"tiers\":[" + tiers + "],\"totals\":[" + totals + "]}";
}

/** @brief newsletter-basic-stats `meta.pagination` (page / limit / next). */
[[nodiscard]] inline std::string meta_pagination(const core::query::ParsedQuery& q)
{
    const auto page = q.page == 0 ? std::size_t{1} : q.page;
    // Ghost's newsletter-basic service defaults limit to 20; core::query's
    // ParsedQuery default (15) is a generic pager default and must not leak here.
    const auto limit = q.options.contains("limit") ? q.limit : std::size_t{20};
    return "{\"pagination\":{\"page\":" + std::to_string(page) +
           ",\"limit\":" + std::to_string(limit) + ",\"next\":null}}";
}

} // namespace blog::handlers::stats::support

template<>
struct serde::Serializable<blog::dto::stats::StatsEmptyResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"stats", &blog::dto::stats::StatsEmptyResponse::get_stats, &blog::dto::stats::StatsEmptyResponse::set_stats, serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "stats_empty_response";
    }
};
