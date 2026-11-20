export module blog_handlers:member_metafields_members_metafields_browse_step;
import :domain_all;
import std;
import core_workflow;
import core_query;
import core_serde;
import core_auth;
import interfaces;

// ---------------------------------------------------------------------------
// Shared strict-parity support for the single member-facing metafields route.
// Kept in this already-registered partition (CONTRACTS.md): helpers must live
// in registered partitions; a dedicated `member_metafields_members_support`
// partition and a metafield allow-list in `blog_handlers:mapping` need the
// central BUILD pass and are recorded in the parity reports.
//
// Primitives used:
//   core_auth:ghost  - unauthorized_error_json (Ghost UnauthorizedError envelope)
//   core_query       - publisher ordering over store rows (no paging: no meta)
//   core_serde       - JsonCodec decode/encode of store rows and wire values
//   ApiError         - Ghost 422 ValidationError envelope (core primitive)
//   ctx.store()      - members_metafields resource (DB stand-in; no SQL primitive)
//
// No blog_handlers:mapping (unregistered), core_adapters or core_outbox used.
// ---------------------------------------------------------------------------
export namespace blog::handlers::member_metafields_members::metafields_browse::workflow {

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

namespace metafields_detail {

using Value = serde::Value;
using Object = Value::Object;
using Array = Value::Array;
using ResultMap = std::map<std::string, std::string>;

inline constexpr std::string_view kNamespaceMessage{
    "A namespace can only contain lowercase letters, numbers and underscores."};
inline constexpr std::string_view kUnauthorizedMessage{"You must be signed in to do this."};
// Ghost inFieldOrder: namespace asc, sort_order asc, created_at asc, id asc
// (services/members-metafields/queries.ts:209-215).
inline constexpr std::string_view kOrder{"namespace asc, sort_order asc, created_at asc, id asc"};

[[nodiscard]] inline core::workflow::StepResponse<ResultMap> result(int status, std::string body)
{
    return core::workflow::StepResponse<ResultMap>{
        {{"status", std::to_string(status)}, {"body", std::move(body)}}};
}

[[nodiscard]] inline const Value* value_for(const Object* object, std::string_view key)
{
    if (object == nullptr) return nullptr;
    for (const auto& [name, value] : *object) {
        if (name == key) return &value;
    }
    return nullptr;
}

/** @brief Scalar as text; rfl::Generic keeps booleans/numbers as their own alternatives. */
[[nodiscard]] inline std::string scalar(const Object* object, std::string_view key)
{
    const auto* value = value_for(object, key);
    if (value == nullptr) return {};
    if (const auto* text = std::get_if<std::string>(&value->get())) return *text;
    if (const auto* flag = std::get_if<bool>(&value->get())) return *flag ? "true" : "false";
    if (const auto* number = std::get_if<std::int64_t>(&value->get())) return std::to_string(*number);
    if (const auto* number = std::get_if<double>(&value->get())) return std::to_string(*number);
    return {};
}

/** @brief Ghost re-emits dates as ISO strings (serializers.ts:25-34). Null stays null. */
[[nodiscard]] inline Value iso_date(const Value* value)
{
    if (value == nullptr || std::holds_alternative<std::nullopt_t>(value->get())) return Value{};
    if (const auto* text = std::get_if<std::string>(&value->get())) {
        std::string out = *text;
        if (out.size() == 19 && out[10] == ' ') {
            out[10] = 'T';
            out += ".000Z"; // SQLite-style datetime -> IS0 8601
        }
        return Value{std::move(out)};
    }
    return Value{*value};
}

/** @brief Typed settings decoding with Ghost defaults (metafieldCodecWithDefaults).
 *         Only `list` has settings on the wire; defaults are {options:[],multiple:false}
 *         (contracts/metafield-types/src/index.ts:313-343). */
[[nodiscard]] inline Value wire_settings(const Value* stored)
{
    Object result;
    result["options"] = Value{Array{}};
    result["multiple"] = Value{false};

    const Object* source = nullptr;
    std::optional<Value> decoded; // keeps a decoded JSON-string settings value alive
    if (stored != nullptr) {
        if (const auto* object = std::get_if<Object>(&stored->get())) {
            source = object;
        } else if (const auto* text = std::get_if<std::string>(&stored->get())) {
            decoded = core_serde::JsonCodec{}.decode<Value>(*text);
            if (decoded) {
                if (const auto* object = std::get_if<Object>(&decoded->get())) source = object;
            }
        }
    }
    if (source == nullptr) return Value{std::move(result)};

    if (const auto* options = value_for(source, "options")) {
        if (const auto* list = std::get_if<Array>(&options->get())) {
            Array values;
            for (const auto& item : *list) {
                if (std::holds_alternative<std::string>(item.get())) values.push_back(item);
            }
            result["options"] = Value{std::move(values)};
        }
    }
    if (const auto* multiple = value_for(source, "multiple")) {
        if (const auto* flag = std::get_if<bool>(&multiple->get())) {
            result["multiple"] = Value{*flag};
        }
    }
    return Value{std::move(result)};
}

/** @brief Store row -> Ghost `MetafieldDefinitionSchema` wire object
 *         (contracts/metafield-types/src/index.ts:382-393). Drops id, member_access,
 *         sort_order; nests access.member; emits ISO dates. */
[[nodiscard]] inline Value to_definition(const Object* raw)
{
    const std::string type = scalar(raw, "type");

    Object out;
    out["namespace"] = Value{scalar(raw, "namespace")};
    out["key"] = Value{scalar(raw, "key")};
    out["name"] = Value{scalar(raw, "name")};
    out["status"] = Value{scalar(raw, "status")};

    Object access;
    access["member"] = Value{scalar(raw, "member_access")};
    out["access"] = Value{std::move(access)};

    out["created_at"] = iso_date(value_for(raw, "created_at"));
    out["updated_at"] = iso_date(value_for(raw, "updated_at"));
    out["type"] = Value{type};
    if (type == "list") out["settings"] = wire_settings(value_for(raw, "settings"));
    return Value{std::move(out)};
}

[[nodiscard]] inline std::string namespace_from(std::string_view path)
{
    auto bare = path.substr(0, path.find('?'));
    while (bare.size() > 1 && bare.back() == '/') bare.remove_suffix(1);
    const auto slash = bare.find_last_of('/');
    return std::string{slash == std::string_view::npos ? bare : bare.substr(slash + 1)};
}

/** @brief Ghost NamespaceSchema /^[a-z0-9_]+$/ (identity.ts:30-38). */
[[nodiscard]] inline bool valid_namespace(std::string_view ns)
{
    if (ns.empty()) return false;
    for (const char c : ns) {
        if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_')) return false;
    }
    return true;
}

} // namespace metafields_detail

class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"member_metafields_members_metafields_browse_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output> invoke(
        const Input& in,
        core::workflow::WorkflowContext&
    ) const {
        using namespace metafields_detail;

        // Ghost: router.use(loadMemberIdentity) + rejectWhenAnonymous on this route
        // (web/members/account/routes.ts:24,59). No C++ member-session primitive;
        // identity is carried by X-Member-Id (codebase convention). The 401 body is
        // the shared core_auth:ghost UnauthorizedError envelope, not a hand-built one.
        const auto member = in.find_header("X-Member-Id");
        if (member.empty()) {
            return result(401, core::auth::ghost::unauthorized_error_json(kUnauthorizedMessage));
        }

        // Ghost: options:['namespace'] + NamespaceSchema parseRequest
        // (member-metafields-members.ts:32-33, serializers.ts:8,14-16). No filter,
        // body or multipart on this route.
        const std::string ns = namespace_from(in.get_path());
        if (!valid_namespace(ns)) {
            return result(422, blog::api::ApiError{}.validation(
                kNamespaceMessage, "namespace", "namespace"));
        }

        // definitions.browse({namespace}, MEMBERS) -> fields.definitions(...,
        // ACTIVE_ONLY, MEMBERS) (definitions-service.ts:168-194). The store is the
        // C++ DB stand-in; audience/status scoping is applied here in-step.
        auto& ctx = *s_ctx;
        const auto rows = ctx.store().json_for("members_metafields");
        const auto parsed = core_serde::JsonCodec{}.decode<Value>(rows);
        if (!parsed) {
            return result(500, blog::api::ApiError{}.json(
                "Definition read failed", "members_metafields"));
        }

        Array kept;
        if (const auto* list = std::get_if<Array>(&parsed->get())) {
            for (const auto& item : *list) {
                const auto* object = std::get_if<Object>(&item.get());
                if (object == nullptr) continue;
                if (scalar(object, "namespace") != ns) continue;
                if (scalar(object, "status") != "active") continue;
                const auto access = scalar(object, "member_access");
                if (access != "read" && access != "write") continue;
                kept.push_back(item);
            }
        }

        // core_query owns ordering. No q.filters/q.raw_filter (the audience scope is
        // already applied) and no paging: Ghost returns every active definition with
        // no meta.pagination (member-metafields-members.ts:35-40).
        core::query::ParsedQuery q;
        q.order = kOrder;
        q.limit = std::numeric_limits<std::size_t>::max(); // limit=all
        std::string ordered;
        try {
            ordered = core::query::apply(core_serde::JsonCodec{}.encode(Value{kept}), q);
        } catch (...) {
            return result(500, blog::api::ApiError{}.json(
                "Definition read failed", "members_metafields"));
        }
        const auto sorted = core_serde::JsonCodec{}.decode<Value>(ordered);

        // toMetafieldsResponse.parse(fields) -> snake_case definition objects with
        // typed settings + access.member + ISO dates (serializers.ts:23-34).
        Array definitions;
        if (sorted) {
            if (const auto* list = std::get_if<Array>(&sorted->get())) {
                for (const auto& item : *list) {
                    if (const auto* object = std::get_if<Object>(&item.get())) {
                        definitions.push_back(to_definition(object));
                    }
                }
            }
        }

        // Local equivalent of local_mapping::wrap (that partition is not registered
        // yet): no meta, status 200 on success.
        const auto body = "{\"members_metafields\":"
                        + core_serde::JsonCodec{}.encode(Value{definitions}) + "}";
        return result(200, body);
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};

} // namespace blog::handlers::member_metafields_members::metafields_browse::workflow
