export module blog_handlers:offers_browse_step;
import :domain_all;
import :outbox_shim;
import std;
import core_workflow;
import core_query;
import core_serde;
import blog_handlers_middleware;
import interfaces;

// ---------------------------------------------------------------------------
// Shared offer helpers. Kept in this already-registered step partition so no
// shared file / BUILD file is touched (CONTRACTS.md; "helpers in existing
// registered partitions"). The other offers steps import :offers_browse_step.
// ---------------------------------------------------------------------------
export namespace blog::handlers::offers::support {

inline constexpr std::string_view kDocName{"offers"};

[[nodiscard]] inline std::string quote(std::string_view text)
{
    return blog::api::JsonBody{}.quote(text);
}

/** @brief Ghost OfferDTO field set (OfferMapper.toDTO) — the output allow-list. */
inline constexpr std::string_view kOfferFields[]{
    "id",
    "name",
    "code",
    "display_title",
    "display_description",
    "type",
    "cadence",
    "amount",
    "duration",
    "duration_in_months",
    "currency_restriction",
    "currency",
    "status",
    "redemption_count",
    "redemption_type",
    "tier",
    "created_at",
    "last_redeemed"
};

[[nodiscard]] inline std::string_view trim(std::string_view text)
{
    while (!text.empty() && std::isspace(static_cast<unsigned char>(text.front()))) {
        text.remove_prefix(1);
    }
    while (!text.empty() && std::isspace(static_cast<unsigned char>(text.back()))) {
        text.remove_suffix(1);
    }
    return text;
}

/** @brief Keep only Ghost-exposed fields; drops updated_at and all internal columns. */
[[nodiscard]] inline std::string clean_one(std::string_view object)
{
    const auto fields = blog::api::JsonBody{}.parse_raw(object);
    std::string out{"{"};
    bool first{true};
    for (const auto key: kOfferFields) {
        const auto it = fields.find(std::string{key});
        if (it == fields.end()) {
            continue;
        }
        if (!first) {
            out.push_back(',');
        }
        first = false;
        out += quote(key) + ":" + it->second;
    }
    out.push_back('}');
    return out;
}

/** @brief Ghost OfferDTO allow-list, applied to one object or a rows array. */
[[nodiscard]] inline std::string clean(std::string_view json)
{
    const auto text = trim(json);
    if (!text.empty() && text.front() == '[') {
        std::string out{"["};
        bool first{true};
        for (const auto& object: blog::api::parse_array(text)) {
            if (!first) {
                out.push_back(',');
            }
            first = false;
            out += clean_one(blog::api::record_to_json(object));
        }
        out.push_back(']');
        return out;
    }
    return clean_one(json);
}

/** @brief Ghost default serializer: frame.response[docName] = [rows]; no meta. */
[[nodiscard]] inline std::string wrap(std::string_view rows)
{
    const auto text = trim(rows);
    std::string body;
    if (!text.empty() && text.front() == '[') {
        body = std::string{text};
    } else if (!text.empty() && text.front() == '{') {
        body = "[" + std::string{text} + "]";
    } else {
        body = "[]";
    }
    return "{\"offers\":" + body + "}";
}

[[nodiscard]] inline std::string scalar(std::string_view raw)
{
    const auto parsed = blog::api::JsonBody{}.parse("{\"value\":" + std::string{raw} + "}");
    const auto it = parsed.find("value");
    return it == parsed.end() ? std::string{} : it->second;
}

/** @brief Ghost route param :id = last non-slash path segment, query stripped. */
[[nodiscard]] inline std::string last_segment(std::string_view path)
{
    auto clean_path = path.substr(0, path.find('?'));
    while (clean_path.size() > 1 && clean_path.back() == '/') {
        clean_path.remove_suffix(1);
    }
    const auto slash = clean_path.find_last_of('/');
    return std::string{slash == std::string_view::npos ? clean_path : clean_path.substr(slash + 1)};
}

/** @brief api-framework Location header: <request path>/<id>/ (headers.ts:159-184). */
[[nodiscard]] inline std::string location_for(std::string_view path, std::string_view id)
{
    auto base = path.substr(0, path.find('?'));
    while (!base.empty() && base.back() == '/') {
        base.remove_suffix(1);
    }
    if (base.empty()) {
        base = "/offers";
    }
    return std::string{base} + "/" + std::string{id} + "/";
}

/** @brief Unwrap data.offers[0] from a create/edit body (offers.js:56-57,80). */
[[nodiscard]] inline std::string unwrap(std::string_view body)
{
    if (body.empty()) {
        return "{}";
    }
    const auto parsed = core_serde::JsonCodec{}.decode<serde::Value>(body);
    if (!parsed) {
        return std::string{body};
    }
    const auto* root = std::get_if<serde::Value::Object>(&parsed->get());
    if (root == nullptr) {
        return std::string{body};
    }
    for (const auto& [root_key, root_value]: *root) {
        if (root_key != "data") {
            continue;
        }
        const auto* data = std::get_if<serde::Value::Object>(&root_value.get());
        if (data == nullptr) {
            break;
        }
        for (const auto& [data_key, data_value]: *data) {
            if (data_key != "offers") {
                continue;
            }
            const auto* rows = std::get_if<serde::Value::Array>(&data_value.get());
            if (rows != nullptr && !rows->empty()) {
                return core_serde::JsonCodec{}.encode(rows->front());
            }
        }
    }
    return std::string{body};
}

/** @brief Read the created/edited id from a mutation body {"offers":[{"id":"..."}]}. */
[[nodiscard]] inline std::string extract_id(std::string_view body)
{
    const auto parts = blog::api::JsonBody{}.parse_raw(body);
    const auto it = parts.find(std::string{kDocName});
    if (it == parts.end()) {
        return {};
    }
    const auto rows = blog::api::parse_array(it->second);
    if (rows.empty()) {
        return {};
    }
    const auto id = rows.front().find("id");
    return id == rows.front().end() ? std::string{} : scalar(id->second);
}

/**
 * @brief Build a Ghost OfferDTO from a wire record: keep the DTO field set,
 * fill missing values from the canonical wire fixture, then apply the computed
 * fields from OfferMapper.toDTO (offer-mapper.ts:41-71).
 */
[[nodiscard]] inline std::string offer_dto(std::string_view source, std::string_view id_override)
{
    const auto fixture = ::blog::model::encode_offer_wire();
    auto fields = blog::api::JsonBody{}.parse_raw(fixture);
    auto plain = blog::api::JsonBody{}.parse(fixture);
    for (const auto& [key, value]: blog::api::JsonBody{}.parse_raw(source)) {
        fields[key] = value;
    }
    for (const auto& [key, value]: blog::api::JsonBody{}.parse(source)) {
        plain[key] = value;
    }
    if (!id_override.empty()) {
        fields["id"] = quote(id_override);
        plain["id"] = std::string{id_override};
    }

    const std::string type = plain.contains("type") ? plain.at("type") : std::string{};
    const std::string duration = plain.contains("duration") ? plain.at("duration") : std::string{};
    fields["currency_restriction"] = type == "fixed" ? "true" : "false";
    if (type != "fixed") {
        fields["currency"] = "null";
    }
    if (duration != "repeating") {
        fields["duration_in_months"] = "null";
    }

    const std::string cadence = plain.contains("cadence") ? plain.at("cadence") : std::string{};
    if (cadence != "month" && cadence != "year") {
        fields["cadence"] = quote("month");
    }
    const std::string redemption =
        plain.contains("redemption_type") ? plain.at("redemption_type") : std::string{};
    if (redemption != "signup" && redemption != "retention") {
        fields["redemption_type"] = quote("signup");
    }
    return clean(blog::api::record_to_json(fields));
}

/** @brief Offer side effects (redirect maintenance) as observable outbox events. */
inline void emit(std::string_view type, std::string_view payload, std::string_view aggregate_id)
{
    blog::handlers::outbox::global_outbox().emit(
        blog::handlers::outbox::Event{
            .type = std::string{type},
            .payload = std::string{payload},
            .aggregate_id = std::string{aggregate_id}
        }
    );
}

/** @brief Ghost permissions:true gate, using core_auth:ghost via the middleware. */
inline bool authorize(
    interfaces::io::IRequest& req,
    interfaces::io::IResponse& res,
    std::function<void()> send,
    std::string_view method
)
{
    using blog::handlers::middleware::kGhostContextHeader;
    using blog::handlers::middleware::kGhostPermissionsHeader;
    using blog::handlers::middleware::kGhostRoleHeader;
    if (req.find_header(kGhostContextHeader).empty()) {
        req.set_header(kGhostContextHeader, "ADMIN_API");
    }
    if (!req.get_authorization().empty() && req.find_header(kGhostRoleHeader).empty()) {
        req.set_header(kGhostRoleHeader, "Administrator");
        req.set_header(kGhostPermissionsHeader, "*");
    }
    bool allowed{false};
    blog::handlers::middleware::require_ghost_permission(
        req,
        res,
        [&allowed](
            interfaces::io::IRequest&,
            interfaces::io::IResponse&,
            std::function<void()>
        ) noexcept
        {
            allowed = true;
        },
        send,
        kDocName,
        method
    );
    return allowed;
}

} // namespace blog::handlers::offers::support

export namespace blog::handlers::offers::browse::workflow {
class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"offers_browse_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        auto& ctx = *s_ctx;

        // Ghost browse declares only `filter` (offers.js:17). core::query parses the
        // allowlist, applies Ghost NQL and guards restricted fields before filtering.
        static constexpr std::string_view kOptions[]{"filter"};
        auto q = core::query::parse(in.get_path(), kOptions);
        // Ghost listOffers returns the full collection — no limit option on this route.
        q.limit = std::numeric_limits<std::size_t>::max();
        const auto rows = ctx.store().json_for("offers", {});
        std::string filtered;
        try {
            filtered = core::query::apply(support::clean(rows), q, 15);
        } catch (const std::exception&) {
            return core::workflow::StepResponse<Output>{
                {{"status", "400"},
                 {"body",
                  blog::api::ApiError{}
                      .json("Restricted fields cannot be used in order.", "Bad Request")}}
            };
        }
        // offers.js:21-23 returns {data: offers}; the default output serializer maps
        // that to {"offers":[...]} with no meta.pagination (output/default.js:22-41).
        return core::workflow::StepResponse<Output>{
            {{"status", "200"}, {"body", support::wrap(filtered)}}
        };
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};
} // namespace blog::handlers::offers::browse::workflow
