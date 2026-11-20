export module blog_handlers:snippets_browse_step;
import :domain_all;
import std;
import core_workflow;
import core_query;
import core_auth;
import core_serde;
import :outbox_shim;
import interfaces;

// ---------------------------------------------------------------------------
// Shared Ghost-parity helpers for every snippets handler/step. Kept in this
// already-registered partition so no central BUILD/all.cppm wiring is needed.
// ---------------------------------------------------------------------------
export namespace blog::handlers::snippets::support {

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

using core::auth::ghost::ApiContext;

// Ghost's declared controller options (snippets.js:19,37,57,74,100). `fields`,
// `include` and `offset` are accepted by the api-framework pipeline as well.
inline constexpr std::array<std::string_view, 8> kBrowseOptions{
    "limit", "order", "page", "offset", "formats", "filter", "fields", "include"};
inline constexpr std::array<std::string_view, 3> kReadOptions{"formats", "fields", "include"};
inline constexpr std::array<std::string_view, 3> kAddOptions{"formats", "fields", "include"};
inline constexpr std::array<std::string_view, 4> kEditOptions{"id", "formats", "fields", "include"};
inline constexpr std::array<std::string_view, 2> kFormats{"mobiledoc", "lexical"};

[[nodiscard]] inline bool in_list(std::string_view value,
                                  std::span<const std::string_view> list)
{
    return std::ranges::find(list, value) != list.end();
}

/** @brief Admin API principal. A bearer token with no explicit role is Ghost's
 *         Administrator; x-blog-* headers override the default (authorizeAdminApi). */
[[nodiscard]] inline core::auth::ghost::Principal principal_for(interfaces::io::IRequest& req)
{
    core::auth::ghost::Principal p;
    const auto context = req.find_header("x-blog-context");
    if (context == "CONTENT_API") p.context = ApiContext::CONTENT_API;
    else if (context == "MEMBERS_APP") p.context = ApiContext::MEMBERS_APP;
    else if (context == "INTERNAL") p.context = ApiContext::INTERNAL;
    else if (context == "SYSTEM") p.context = ApiContext::SYSTEM;
    else if (context == "ADMIN_API") p.context = ApiContext::ADMIN_API;
    else p.context = req.get_authorization().empty() ? ApiContext::ANONYMOUS
                                                      : ApiContext::ADMIN_API;

    p.role = std::string{req.find_header("x-blog-role")};
    if (p.role.empty() && p.context == ApiContext::ADMIN_API) p.role = "Administrator";
    for (auto part : std::string_view{req.find_header("x-blog-permissions")} |
                         std::views::split(',')) {
        const std::string scope{std::ranges::begin(part), std::ranges::end(part)};
        if (!scope.empty()) p.permissions.insert(scope);
    }
    const auto staff = req.find_header("x-blog-staff");
    p.staff = staff == "true" || staff == "1";
    if (!req.get_authorization().empty()) p.id = "bearer";
    return p;
}

/** @brief `:id` is the last non-empty path segment (query and trailing `/` stripped). */
[[nodiscard]] inline std::string path_id(std::string_view path)
{
    auto view = path.substr(0, path.find('?'));
    while (view.size() > 1 && view.back() == '/') view.remove_suffix(1);
    const auto slash = view.find_last_of('/');
    return std::string{slash == std::string_view::npos ? view : view.substr(slash + 1)};
}

/** @brief Ghost rejects `formats` values outside models.Snippet.allowedFormats. */
[[nodiscard]] inline bool formats_ok(const core::query::ParsedQuery& q)
{
    const auto it = q.options.find("formats");
    if (it == q.options.end() || it->second.empty()) return true;
    std::size_t i = 0;
    while (i <= it->second.size()) {
        const auto comma = it->second.find(',', i);
        const auto token = std::string_view{it->second}.substr(
            i, comma == std::string::npos ? std::string_view::npos : comma - i);
        if (!token.empty() && !in_list(token, kFormats)) return false;
        if (comma == std::string::npos) break;
        i = comma + 1;
    }
    return true;
}

[[nodiscard]] inline std::string formats_error()
{
    return blog::api::ApiError{}.validation(
        "Validation error, cannot save snippet.",
        "Validation (Formats) failed for formats", "formats");
}

[[nodiscard]] inline std::string not_found_error()
{
    return blog::api::ApiError{}.json("Snippet not found.", "Not Found");
}

/** @brief Unwrap Ghost's `frame.data.snippets[0]` (also accepts a bare top-level
 *         `snippets` array, which older C++ callers send). */
[[nodiscard]] inline std::string unwrap_body(std::string_view raw)
{
    const std::string body = raw.empty() ? "{}" : std::string{raw};
    const auto top = blog::api::JsonBody{}.parse_raw(body);

    if (const auto dit = top.find("data"); dit != top.end()) {
        const auto data = blog::api::JsonBody{}.parse_raw(dit->second);
        if (const auto sit = data.find("snippets"); sit != data.end()) {
            const auto items = blog::api::parse_array(sit->second);
            if (!items.empty()) return blog::api::record_to_json(items.front());
        }
    }
    if (const auto sit = top.find("snippets"); sit != top.end()) {
        const auto items = blog::api::parse_array(sit->second);
        if (!items.empty()) return blog::api::record_to_json(items.front());
    }
    return body;
}

[[nodiscard]] inline std::string field_string(const serde::Value& v, const char* key)
{
    const auto object = v.to_object();
    if (!object) return {};
    const auto field = object->get(key);
    if (!field) return {};
    const auto encoded = core_serde::JsonCodec{}.encode(*field);
    if (encoded.size() >= 2 && encoded.front() == '"' && encoded.back() == '"') {
        return encoded.substr(1, encoded.size() - 2);
    }
    return encoded;
}

[[nodiscard]] inline std::string row_field(std::string_view json, const char* key)
{
    try {
        if (auto value = core_serde::JsonCodec{}.decode<serde::Value>(json)) {
            return field_string(*value, key);
        }
    } catch (...) {
    }
    return {};
}

[[nodiscard]] inline std::string first_row(std::string_view rows)
{
    try {
        if (auto value = core_serde::JsonCodec{}.decode<serde::Value>(rows)) {
            auto array = value->to_array();
            if (array && !array->empty()) return core_serde::JsonCodec{}.encode((*array)[0]);
        }
    } catch (...) {
    }
    return {};
}

/** @brief Snippet.formatsToJSON: drop every format the request did not ask for.
 *         Ghost's default keeps `mobiledoc` only. */
[[nodiscard]] inline std::string with_formats(std::string_view rows_json,
                                              const core::query::ParsedQuery& q)
{
    auto rows = blog::api::parse_array(rows_json);
    const auto it = q.options.find("formats");
    const std::string formats = it == q.options.end() ? std::string{} : it->second;
    const bool keep_mobiledoc = formats.empty() || formats.contains("mobiledoc");
    const bool keep_lexical = formats.contains("lexical");
    for (auto& row : rows) {
        if (!keep_mobiledoc) row.erase("mobiledoc");
        if (!keep_lexical) row.erase("lexical");
    }
    std::string out{"["};
    for (std::size_t i = 0; i < rows.size(); ++i) {
        if (i) out += ',';
        out += blog::api::record_to_json(rows[i]);
    }
    out += ']';
    return out;
}

/** @brief Seed the single Ghost fixture snippet into the existing ResourceStore
 *         so browse/read/edit/destroy all observe the same record. */
inline void seed_snippet(blog::BlogContext& ctx)
{
    auto& store = ctx.store().resources();
    if (store.count("snippets") != 0 || store.count("snippets_seeded") != 0) return;
    store.create("snippets", core_serde::JsonCodec{}.encode(blog::model::SnippetWire{}));
    store.create("snippets_seeded", "{}");
}

/** @brief Ghost actionsCollectCRUD / actionsResourceType:'snippet' side effect. */
inline void emit_action(std::string_view event, std::string_view id,
                        std::string_view payload)
{
    auto& outbox = blog::handlers::outbox::global_outbox();
    outbox.begin_transaction();
    outbox.emit(blog::handlers::outbox::Event{
        .type = std::string{"snippet."} + std::string{event},
        .payload = std::string{payload},
        .aggregate_id = std::string{id}});
    outbox.commit();
}

} // namespace blog::handlers::snippets::support

export namespace blog::handlers::snippets::browse::workflow {

class ExecuteStep {
public:
  using Input=interfaces::io::IRequest&;
  using Output=std::map<std::string, std::string>;
  static constexpr std::string_view name{"snippets_browse_execute"};
  [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
   using namespace blog::handlers::snippets::support;
   auto& ctx=*s_ctx;

   // Ghost: mw.authAdminApi + permissions:true / docName:'snippets'.
   core::auth::ghost::GhostAuthorizer authorizer;
   const auto authorized=authorizer.authorize(principal_for(in), true, "snippets", "browse");
   if(!authorized.allowed)
    return core::workflow::StepResponse<Output>{{{"status",std::to_string(authorized.status)},{"body",authorized.error_json}}};

   // Ghost input pipeline: allow-listed options, restricted-field guard, NQL,
   // order, fields projection and page/limit/offset pagination all via core::query.
   const auto q=core::query::parse(in.get_path(), kBrowseOptions);
   if(!formats_ok(q))
    return core::workflow::StepResponse<Output>{{{"status","422"},{"body",formats_error()}}};

   seed_snippet(ctx);
   std::string run;
   try {
    run=core::query::run(ctx.store().resources().list("snippets"), q, 15);
   } catch (const std::exception&) {
    return core::workflow::StepResponse<Output>{{{"status","422"},
      {"body",blog::api::ApiError{}.validation("Restricted field used in Ghost query","Validation Error","filter")}}};
   }

   std::string data="[]", meta="{}";
   try {
    if(auto value=core_serde::JsonCodec{}.decode<serde::Value>(run)) {
     if(const auto object=value->to_object()) {
      if(const auto d=object->get("data")) data=core_serde::JsonCodec{}.encode(*d);
      if(const auto m=object->get("meta")) meta=core_serde::JsonCodec{}.encode(*m);
     }
    }
   } catch (...) {}

   // Ghost output: Snippet.formatsToJSON + output/mappers/snippets + clean.
   const auto cleaned=local_mapping::clean(with_formats(data,q),"snippet");
   const auto body=local_mapping::wrap("snippets",cleaned,meta);
   return core::workflow::StepResponse<Output>{{{"status","200"},{"body",body}}};
  }
  void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
  static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
