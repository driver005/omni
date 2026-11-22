export module blog_handlers:apps_app_installations_browse_step;
import :domain_all;
import std;
import core_workflow;
import core_query;
import core_serde;
import interfaces;
import blog_handlers_middleware;
import :outbox_shim;

// ---------------------------------------------------------------------------
// Strict Ghost parity for the app-installations controller
// (app-installations.ts + app-installation-previews.ts).
//
// This registered partition hosts the shared `blog::handlers::apps::support`
// helpers used by all seven `/apps/installations*` steps. Keeping the helpers
// here avoids adding a new module partition (the central BUILD file is owned by
// the parent session). Shared abstractions used, all already registered:
//   core::query              - allowlisted NQL filter/order over store rows
//   core_serde               - JSON decode/encode
//   core_auth::ghost + blog_handlers_middleware - permission gate (handlers)
//   blog::handlers::outbox   - transactional side-effect log (:outbox_shim)
//
// Fixture model: the in-memory ResourceStore stands in for Knex. Installations
// are flat joined rows (installation columns + approved-manifest columns),
// manifests live in `app_installation_manifests`, staff actions in `actions`.
// Real HTTP/crypto work is outbox-recorded; no live credentials are needed.
// ---------------------------------------------------------------------------
export namespace blog::handlers::apps::support {

using Array = serde::Value::Array;
using Object = serde::Value::Object;

inline constexpr std::string_view kInstallation{"app_installations"};
inline constexpr std::string_view kManifests{"app_installation_manifests"};
inline constexpr std::string_view kActions{"actions"};
inline constexpr std::string_view kDefaultTime{"1970-01-01T00:00:00.000Z"};

struct Result
{
    int status{200};
    std::string body;
};

/** @brief JSON string escape for a scalar. */
[[nodiscard]] inline std::string quote(std::string_view value)
{
    std::string out{"\""};
    for (const unsigned char c : value) {
        switch (c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:
                if (c < 0x20) {
                    out += std::format("\\u{:04x}", static_cast<unsigned>(c));
                } else {
                    out.push_back(static_cast<char>(c));
                }
        }
    }
    out.push_back('"');
    return out;
}

[[nodiscard]] inline std::string base64(std::string_view input)
{
    static constexpr char kAlphabet[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string out;
    for (std::size_t i = 0; i < input.size(); i += 3) {
        const unsigned char a = static_cast<unsigned char>(input[i]);
        const unsigned char b = i + 1 < input.size() ? static_cast<unsigned char>(input[i + 1]) : 0;
        const unsigned char c = i + 2 < input.size() ? static_cast<unsigned char>(input[i + 2]) : 0;
        out.push_back(kAlphabet[(a >> 2) & 0x3F]);
        out.push_back(kAlphabet[((a & 0x03) << 4) | ((b >> 4) & 0x0F)]);
        out.push_back(i + 1 < input.size() ? kAlphabet[((b & 0x0F) << 2) | ((c >> 6) & 0x03)] : '=');
        out.push_back(i + 2 < input.size() ? kAlphabet[c & 0x3F] : '=');
    }
    return out;
}

[[nodiscard]] inline std::string object_of(std::initializer_list<std::pair<std::string_view, std::string>> fields)
{
    std::string out{"{"};
    bool first{true};
    for (const auto& [name, value] : fields) {
        if (!first) out.push_back(',');
        first = false;
        out += quote(name) + ':' + value;
    }
    out.push_back('}');
    return out;
}

[[nodiscard]] inline std::string null_if_empty(std::string_view value)
{
    return value.empty() ? std::string{"null"} : quote(value);
}

/** @brief Scalar string for a JSON member; null/absent is the empty string. */
[[nodiscard]] inline std::string scalar(const serde::Value& value, std::string_view key)
{
    const auto* object = std::get_if<Object>(&value.get());
    if (object == nullptr) return {};
    for (const auto& [name, member] : *object) {
        if (name != key) continue;
        if (const auto* text = std::get_if<std::string>(&member.get())) return *text;
        const auto encoded = core_serde::JsonCodec{}.encode(member);
        return encoded == "null" ? std::string{} : encoded;
    }
    return {};
}

[[nodiscard]] inline std::string scalar_raw(std::string_view json, std::string_view key)
{
    const auto parsed = core_serde::JsonCodec{}.decode<serde::Value>(json);
    return parsed ? scalar(*parsed, key) : std::string{};
}

/** @brief Raw JSON value of a member (quotes/braces kept). */
[[nodiscard]] inline std::string raw_member(std::string_view json, std::string_view key)
{
    const auto parsed = core_serde::JsonCodec{}.decode<serde::Value>(json);
    if (!parsed) return {};
    const auto* object = std::get_if<Object>(&parsed->get());
    if (object == nullptr) return {};
    for (const auto& [name, member] : *object) {
        if (name == key) return core_serde::JsonCodec{}.encode(member);
    }
    return {};
}

[[nodiscard]] inline long long revision_number(std::string_view raw)
{
    try {
        std::size_t used{0};
        const long long value = std::stoll(std::string{raw}, &used);
        if (used == raw.size()) return value;
    } catch (...) {
    }
    return 0;
}

/** @brief core::query over the ResourceStore rows (one filtered, ordered pass). */
[[nodiscard]] inline std::vector<serde::Value> rows_for(
    blog::BlogContext& ctx,
    std::string_view resource,
    std::vector<std::pair<std::string, std::string>> filters,
    std::string_view order = {})
{
    core::query::ParsedQuery q;
    q.filters = std::move(filters);
    q.order = std::string{order};
    q.limit = std::numeric_limits<std::size_t>::max();
    q.page = 1;
    const auto json = core::query::apply(ctx.store().json_for(resource, {}), q);
    const auto parsed = core_serde::JsonCodec{}.decode<serde::Value>(json);
    if (!parsed) return {};
    const auto* array = std::get_if<Array>(&parsed->get());
    return array == nullptr ? std::vector<serde::Value>{} : *array;
}

[[nodiscard]] inline std::optional<std::string> find_one(
    blog::BlogContext& ctx, std::string_view resource, std::string_view key,
    std::string_view value)
{
    const auto rows = rows_for(ctx, resource, {{std::string{key}, std::string{value}}});
    if (rows.empty()) return std::nullopt;
    return core_serde::JsonCodec{}.encode(rows.front());
}

[[nodiscard]] inline std::optional<std::string> find_installation(
    blog::BlogContext& ctx, std::string_view id)
{
    return find_one(ctx, kInstallation, "id", id);
}

[[nodiscard]] inline std::optional<std::string> current_installation(
    blog::BlogContext& ctx, std::string_view id)
{
    auto row = find_installation(ctx, id);
    if (row && scalar_raw(*row, "status") == "uninstalled") return std::nullopt;
    return row;
}

[[nodiscard]] inline std::optional<std::string> current_installation_of(
    blog::BlogContext& ctx, std::string_view app_id)
{
    if (app_id.empty()) return std::nullopt;
    const auto rows = rows_for(ctx, kInstallation, {{"app_id", std::string{app_id}}});
    for (const auto& row : rows) {
        const auto encoded = core_serde::JsonCodec{}.encode(row);
        if (scalar_raw(encoded, "status") != "uninstalled") return encoded;
    }
    return std::nullopt;
}

/** @brief Full Ghost error envelope, including optional code/details/errorDetails. */
[[nodiscard]] inline std::string ghost_error(
    std::string_view message, std::string_view type, std::string_view code = {},
    std::string_view context = {}, std::string_view details = {},
    std::string_view error_details = {})
{
    static std::uint64_t counter{0};
    return std::format(
        R"({{"errors":[{{"id":"{:032x}","code":{},"context":{},"details":{},"ghostErrorCode":null,"help":null,"message":{},"property":null,"type":{}{}}}]}})",
        ++counter,
        code.empty() ? std::string{"null"} : quote(code),
        context.empty() ? std::string{"null"} : quote(context),
        details.empty() ? std::string{"null"} : details,
        quote(message),
        quote(type),
        error_details.empty() ? std::string{} : ",\"errorDetails\":" + std::string{error_details});
}

[[nodiscard]] inline std::string validation_error(
    std::string_view message, std::string_view context, std::string_view property)
{
    return blog::api::ApiError{}.validation(message, context, property);
}

[[nodiscard]] inline Result invalid(std::string_view message, std::string_view property)
{
    return {422, validation_error("Validation error", message, property)};
}

[[nodiscard]] inline std::string not_found()
{
    return ghost_error("App installation not found.", "NotFoundError");
}

[[nodiscard]] inline std::string not_installed()
{
    return ghost_error("This app is not installed.", "NotFoundError", "APP_NOT_INSTALLED");
}

[[nodiscard]] inline std::string uninstalled_error()
{
    return ghost_error("This app has been uninstalled.", "ConflictError",
                       "APP_INSTALLATION_UNINSTALLED");
}

[[nodiscard]] inline std::string unavailable(
    std::string_view message, std::string_view code, std::string_view context)
{
    return ghost_error(message, "ConflictError", code, context);
}

[[nodiscard]] inline std::string manifest_invalid(std::string_view message, std::string_view context)
{
    return ghost_error("The app manifest is not valid.", "ValidationError",
                       "APP_MANIFEST_INVALID", context,
                       object_of({{"path", quote(context)},
                                  {"message", quote(message)}}));
}

/** @brief Replace/append one raw JSON member. */
[[nodiscard]] inline std::string set_member_impl(std::string_view json, std::string_view key,
                                                 std::string_view raw_value)
{
    const auto parsed = core_serde::JsonCodec{}.decode<serde::Value>(json);
    if (!parsed) return std::string{json};
    const auto* object = std::get_if<Object>(&parsed->get());
    if (object == nullptr) return std::string{json};
    std::string out{"{"};
    bool first{true};
    bool replaced{false};
    for (const auto& [name, member] : *object) {
        if (!first) out.push_back(',');
        first = false;
        out += quote(name) + ':';
        out += name == key ? std::string{raw_value} : core_serde::JsonCodec{}.encode(member);
        if (name == key) replaced = true;
    }
    if (!replaced) {
        if (!first) out.push_back(',');
        out += quote(key) + ':' + std::string{raw_value};
    }
    out.push_back('}');
    return out;
}

/** @brief Merge the approved manifest's joined columns into a stored row. */
[[nodiscard]] inline std::string join_manifest(blog::BlogContext& ctx, std::string_view row)
{
    std::string out{row};
    const auto manifest_id = scalar_raw(row, "manifest_id");
    if (manifest_id.empty() || !raw_member(row, "manifest").empty()) return out;
    const auto manifest = find_one(ctx, kManifests, "id", manifest_id);
    if (!manifest) return out;
    for (const auto key : {"manifest_url", "manifest", "icon_data_url"}) {
        if (!raw_member(out, key).empty()) continue;
        const auto value = raw_member(*manifest, key);
        if (value.empty()) continue;
        out = set_member_impl(out, key, value);
    }
    return out;
}

/** @brief AppInstallationResource (serializers.ts:7-19). */
[[nodiscard]] inline std::string installation_resource(blog::BlogContext& ctx,
                                                      std::string_view row,
                                                      std::string_view history = {})
{
    const auto joined = join_manifest(ctx, row);
    auto status = scalar_raw(joined, "status");
    if (status.empty()) status = "active";
    auto revision = scalar_raw(joined, "revision");
    if (revision.empty()) revision = "0";
    auto created = scalar_raw(joined, "created_at");
    if (created.empty()) created = std::string{kDefaultTime};
    auto manifest = raw_member(joined, "manifest");
    if (manifest.empty()) manifest = "null";
    auto icon = raw_member(joined, "icon_data_url");
    if (icon.empty() || icon == "null") icon = "null";
    const auto updated = raw_member(joined, "updated_at");

    std::string out = object_of({
        {"id", quote(scalar_raw(joined, "id"))},
        {"app_id", quote(scalar_raw(joined, "app_id"))},
        {"status", quote(status)},
        {"revision", revision},
        {"manifest_url", quote(scalar_raw(joined, "manifest_url"))},
        {"manifest", manifest},
        {"icon_data_url", icon},
        {"created_at", quote(created)},
        {"updated_at", updated.empty() || updated == "null" ? std::string{"null"} : updated},
    });
    if (!history.empty()) {
        out.pop_back();
        out += ",\"history\":" + std::string{history} + "}";
    }
    return out;
}

[[nodiscard]] inline std::string actor_value(std::string_view id, std::string_view name)
{
    if (id.empty() && name.empty()) return "null";
    return object_of({{"id", quote(id)}, {"name", null_if_empty(name)}});
}

/** @brief history.ts: buildHistory - merge manifest rows and staff actions, newest first. */
[[nodiscard]] inline std::string history_for(blog::BlogContext& ctx, std::string_view id)
{
    struct Entry
    {
        std::string id;
        std::string event;
        std::string actor;
        std::string created;
    };
    std::vector<Entry> entries;

    const auto manifests = rows_for(ctx, kManifests, {{"installation_id", std::string{id}}},
                                    "created_at asc");
    const auto actions = rows_for(ctx, kActions, {{"resource_id", std::string{id}}},
                                  "created_at asc");

    auto action_json = [](const serde::Value& value) { return core_serde::JsonCodec{}.encode(value); };
    auto is_event = [](const std::string& json, std::string_view event) {
        return scalar_raw(json, "event") == event;
    };
    auto actor_of = [](const std::string& json) {
        const auto aid = scalar_raw(json, "actor_id");
        const auto aname = scalar_raw(json, "actor_name");
        return actor_value(aid, aname);
    };

    // installOf: the install manifest row, with the recorded `installed` action if any.
    for (const auto& manifest : manifests) {
        const auto mjson = action_json(manifest);
        if (scalar_raw(mjson, "source") != "install") continue;
        std::string entry_id = scalar_raw(mjson, "id");
        std::string actor{"null"};
        for (const auto& action : actions) {
            const auto aj = action_json(action);
            if (!is_event(aj, "installed")) continue;
            entry_id = scalar_raw(aj, "id");
            actor = actor_of(aj);
            break;
        }
        auto created = scalar_raw(mjson, "created_at");
        if (created.empty()) created = std::string{kDefaultTime};
        entries.push_back({entry_id, "installed", actor, created});
        break;
    }

    // updates / suspensions / resumes from recheck manifest rows.
    for (const auto& manifest : manifests) {
        const auto mjson = action_json(manifest);
        const auto source = scalar_raw(mjson, "source");
        if (source != "update" && source != "resume") continue;
        const std::string event = source == "resume" ? "resumed"
            : (scalar_raw(mjson, "requires_approval") == "true" ? "suspended" : "updated");
        auto created = scalar_raw(mjson, "created_at");
        if (created.empty()) created = std::string{kDefaultTime};
        entries.push_back({scalar_raw(mjson, "id"), event, "null", created});
    }

    // approvals and uninstalls from staff actions.
    for (const auto& action : actions) {
        const auto aj = action_json(action);
        if (!is_event(aj, "changes_approved") && !is_event(aj, "uninstalled")) continue;
        auto created = scalar_raw(aj, "created_at");
        if (created.empty()) created = std::string{kDefaultTime};
        std::string moved_to;
        if (is_event(aj, "changes_approved")) {
            const auto context = raw_member(aj, "context");
            const auto to = scalar_raw(context, "to_manifest_id");
            if (!to.empty()) {
                if (const auto manifest = find_one(ctx, kManifests, "id", to)) {
                    moved_to = scalar_raw(*manifest, "manifest_url");
                }
            }
        }
        std::string json = object_of({
            {"id", quote(scalar_raw(aj, "id"))},
            {"event", quote(is_event(aj, "changes_approved") ? "changes_approved" : "uninstalled")},
            {"actor", actor_of(aj)},
            {"created_at", quote(created)},
        });
        if (!moved_to.empty()) {
            json.pop_back();
            json += ",\"moved_to\":" + quote(moved_to) + "}";
        }
        // Kept as a pre-serialized entry by storing it verbatim in `event`.
        entries.push_back({scalar_raw(aj, "id"), json, "null", created});
    }

    std::stable_sort(entries.begin(), entries.end(), [](const Entry& a, const Entry& b) {
        if (a.created != b.created) return a.created > b.created;
        return a.id > b.id;
    });

    std::string out{"["};
    bool first{true};
    for (const auto& entry : entries) {
        if (!first) out.push_back(',');
        first = false;
        if (entry.event.front() == '{') {
            out += entry.event;
        } else {
            out += object_of({
                {"id", quote(entry.id)},
                {"event", quote(entry.event)},
                {"actor", entry.actor},
                {"created_at", quote(entry.created)},
            });
        }
    }
    out.push_back(']');
    return out;
}

/** @brief browse()/read() read path; history is attached for include=history. */
[[nodiscard]] inline Result read_one(blog::BlogContext& ctx, std::string_view id,
                                     bool with_history)
{
    const auto row = find_installation(ctx, id);
    if (!row) return {404, not_found()};
    std::string history;
    if (with_history) history = history_for(ctx, id);
    return {200, object_of({{"app_installations", "[" + installation_resource(ctx, *row, history) + "]"}})};
}

/** @brief service.browse() -> repository.list(): uninstalled excluded, oldest first. */
[[nodiscard]] inline Result browse(blog::BlogContext& ctx)
{
    core::query::ParsedQuery q;
    q.raw_filter = "status:!uninstalled";
    q.order = "created_at asc,id asc";
    q.limit = std::numeric_limits<std::size_t>::max();
    const auto rows = core::query::apply(ctx.store().json_for(kInstallation, {}), q);
    const auto parsed = core_serde::JsonCodec{}.decode<serde::Value>(rows);
    std::string data{"["};
    bool first{true};
    if (parsed) {
        if (const auto* array = std::get_if<Array>(&parsed->get())) {
            for (const auto& row : *array) {
                const auto encoded = core_serde::JsonCodec{}.encode(row);
                if (!first) data.push_back(',');
                first = false;
                data += installation_resource(ctx, encoded);
            }
        }
    }
    data.push_back(']');
    return {200, object_of({{"app_installations", data}})};
}

// ------------------------ manifest / identity stand-ins --------------------

struct Loaded
{
    std::string manifest_url;
    std::string manifest{"null"};
    std::string digest;
    std::string icon{"null"};
    std::string app_id;
    std::string identity_key_thumbprint;
    std::string trusted_origins{"[]"};
    std::string row;
};

struct Reviewed
{
    Loaded loaded;
    std::string trusted_origins{"[]"};
    std::optional<std::string> installation;
};

[[nodiscard]] inline std::string digest_for(const Loaded& loaded)
{
    // No crypto primitive in the shared set: use the stored digest when present,
    // otherwise a deterministic stand-in covering the manifest and its URL.
    if (!loaded.digest.empty()) return loaded.digest;
    return "sha256-" + std::to_string(std::hash<std::string>{}(loaded.manifest_url + loaded.manifest));
}

[[nodiscard]] inline Result load(blog::BlogContext& ctx, blog::handlers::outbox::Outbox& outbox,
                                 std::string_view manifest_url)
{
    outbox.emit({"identity.challenge", object_of({{"manifest_url", quote(manifest_url)}}), {}});
    outbox.emit({"manifest.fetch", object_of({{"manifest_url", quote(manifest_url)}}), {}});

    const auto rows = rows_for(ctx, kManifests, {{"manifest_url", std::string{manifest_url}}},
                               "created_at desc");
    if (rows.empty()) {
        return {422, ghost_error("The app manifest could not be loaded.", "ValidationError",
                                 "APP_MANIFEST_UNREACHABLE", "manifest_url")};
    }
    Loaded loaded;
    loaded.row = core_serde::JsonCodec{}.encode(rows.front());
    loaded.manifest_url = std::string{manifest_url};
    auto manifest = raw_member(loaded.row, "manifest");
    if (manifest.empty() || manifest == "null") manifest = "{}";
    loaded.manifest = manifest;
    loaded.digest = scalar_raw(loaded.row, "digest");
    auto icon = raw_member(loaded.row, "icon");
    if (icon.empty() || icon == "null") {
        const auto stored_icon = raw_member(loaded.row, "icon_data_url");
        icon = stored_icon.empty() ? std::string{"null"} : stored_icon;
    }
    loaded.icon = icon;
    loaded.app_id = scalar_raw(manifest, "id");
    loaded.identity_key_thumbprint = scalar_raw(loaded.row, "backend_key_thumbprint");
    auto origins = raw_member(loaded.row, "trusted_origins");
    loaded.trusted_origins = origins.empty() ? std::string{"[]"} : origins;
    return {200, loaded.row};
}

[[nodiscard]] inline Result review(blog::BlogContext& ctx, blog::handlers::outbox::Outbox& outbox,
                                   const Loaded& loaded, std::optional<std::string> installation)
{
    (void)outbox;
    Reviewed reviewed;
    reviewed.loaded = loaded;
    reviewed.trusted_origins = loaded.trusted_origins;
    reviewed.installation = installation
        ? installation
        : current_installation_of(ctx, loaded.app_id);

    const auto pinned = reviewed.installation
        ? scalar_raw(*reviewed.installation, "identity_key_thumbprint") : std::string{};
    if (!pinned.empty() && !loaded.identity_key_thumbprint.empty() &&
        pinned != loaded.identity_key_thumbprint) {
        return {422, ghost_error("App manifest failed the identity check.", "ValidationError",
                                 "APP_IDENTITY_MISMATCH", "manifest")};
    }
    return {200, core_serde::JsonCodec{}.encode(loaded.row)};
}

/** @brief compareManifests stand-in: recursive path/requires_approval diff. */
[[nodiscard]] inline std::string manifest_changes(std::string_view approved, std::string_view loaded)
{
    auto flatten = [](std::string_view json, auto&& self, std::string prefix,
                      std::map<std::string, std::string>& out) -> void {
        const auto parsed = core_serde::JsonCodec{}.decode<serde::Value>(json);
        if (!parsed) return;
        if (const auto* object = std::get_if<Object>(&parsed->get())) {
            for (const auto& [name, value] : *object) {
                const auto path = prefix.empty() ? name : prefix + "." + name;
                const auto encoded = core_serde::JsonCodec{}.encode(value);
                if (std::holds_alternative<Object>(value.get()) || std::holds_alternative<Array>(value.get())) {
                    self(encoded, self, path, out);
                } else {
                    out[path] = encoded;
                }
            }
        } else if (const auto* array = std::get_if<Array>(&parsed->get())) {
            for (std::size_t i = 0; i < array->size(); ++i) {
                const auto path = prefix + "[" + std::to_string(i) + "]";
                const auto encoded = core_serde::JsonCodec{}.encode((*array)[i]);
                if (std::holds_alternative<Object>((*array)[i].get()) || std::holds_alternative<Array>((*array)[i].get())) {
                    self(encoded, self, path, out);
                } else {
                    out[path] = encoded;
                }
            }
        }
    };
    std::map<std::string, std::string> before;
    std::map<std::string, std::string> after;
    flatten(approved, flatten, {}, before);
    flatten(loaded, flatten, {}, after);

    std::string out{"["};
    bool first{true};
    auto emit = [&](const std::string& path) {
        if (!first) out.push_back(',');
        first = false;
        static constexpr std::string_view kApprovalKeys[]{"id", "name", "icon", "url"};
        bool needs_approval{false};
        for (const auto key : kApprovalKeys) {
            if (path == key || path.starts_with(std::string{key} + ".") ||
                path.starts_with(std::string{key} + "[")) {
                needs_approval = true;
                break;
            }
        }
        out += object_of({{"path", quote(path)},
                          {"requires_approval", needs_approval ? "true" : "false"}});
    };
    for (const auto& [path, value] : after) {
        if (before.contains(path) && before.at(path) == value) continue;
        emit(path);
    }
    for (const auto& [path, value] : before) {
        if (after.contains(path)) continue;
        (void)value;
        emit(path);
    }
    out.push_back(']');
    return out;
}

[[nodiscard]] inline Result fetch_icon(blog::BlogContext& ctx,
                                       blog::handlers::outbox::Outbox& outbox,
                                       std::string_view icon)
{
    (void)ctx;
    if (icon.empty() || icon == "null") return {200, "null"};
    const auto url = scalar_raw(icon, "url");
    if (url.empty()) return {200, "null"};
    outbox.emit({"icon.fetch", object_of({{"url", quote(url)}}), {}});
    // A named icon has no URL; a URL icon yields a data URL stand-in.
    return {200, quote("data:image/png;base64," + base64(url))};
}

[[nodiscard]] inline std::string icon_kept_by(std::string_view write_kind,
                                              std::string_view write_icon,
                                              std::string_view current_icon)
{
    if (write_kind == "add_approved" || write_kind == "add_pending") return std::string{write_icon};
    return current_icon.empty() ? std::string{"null"} : std::string{current_icon};
}

/** @brief service.preview() -> load -> review -> previewOf; stores nothing. */
[[nodiscard]] inline Result preview(blog::BlogContext& ctx,
                                    blog::handlers::outbox::Outbox& outbox,
                                    std::string_view manifest_url)
{
    const auto loaded_result = load(ctx, outbox, manifest_url);
    if (loaded_result.status != 200) {
        auto error = core_serde::JsonCodec{}.decode<serde::Value>(loaded_result.body);
        (void)error;
        return loaded_result;
    }
    Loaded loaded;
    loaded.row = loaded_result.body;
    loaded.manifest_url = std::string{manifest_url};
    auto manifest = raw_member(loaded.row, "manifest");
    if (manifest.empty() || manifest == "null") manifest = "{}";
    loaded.manifest = manifest;
    loaded.digest = scalar_raw(loaded.row, "digest");
    auto icon = raw_member(loaded.row, "icon");
    if (icon.empty() || icon == "null") icon = raw_member(loaded.row, "icon_data_url");
    loaded.icon = icon.empty() ? std::string{"null"} : icon;
    loaded.app_id = scalar_raw(manifest, "id");
    loaded.identity_key_thumbprint = scalar_raw(loaded.row, "backend_key_thumbprint");
    auto origins = raw_member(loaded.row, "trusted_origins");
    loaded.trusted_origins = origins.empty() ? std::string{"[]"} : origins;

    const auto reviewed_result = review(ctx, outbox, loaded, std::nullopt);
    if (reviewed_result.status != 200) return reviewed_result;
    Reviewed reviewed;
    reviewed.loaded = loaded;
    reviewed.trusted_origins = loaded.trusted_origins;
    reviewed.installation = current_installation_of(ctx, loaded.app_id);

    std::string icon_data_url;
    if (!reviewed.installation) {
        const auto icon_result = fetch_icon(ctx, outbox, loaded.icon);
        icon_data_url = icon_result.body;
    } else {
        auto current_icon = raw_member(*reviewed.installation, "icon_data_url");
        if (current_icon.empty()) current_icon = "null";
        const auto changes = manifest_changes(raw_member(*reviewed.installation, "manifest"), loaded.manifest);
        std::string installation = object_of({
            {"id", quote(scalar_raw(*reviewed.installation, "id"))},
            {"status", quote(scalar_raw(*reviewed.installation, "status"))},
            {"revision", scalar_raw(*reviewed.installation, "revision").empty()
                ? std::string{"0"} : scalar_raw(*reviewed.installation, "revision")},
            {"manifest_url", quote(scalar_raw(*reviewed.installation, "manifest_url"))},
            {"manifest", raw_member(*reviewed.installation, "manifest").empty()
                ? std::string{"null"} : raw_member(*reviewed.installation, "manifest")},
            {"icon_data_url", current_icon},
            {"changes", changes},
        });
        return {200, object_of({
            {"manifest_url", quote(loaded.manifest_url)},
            {"manifest", loaded.manifest},
            {"digest", quote(digest_for(loaded))},
            {"icon_data_url", current_icon},
            {"trusted_origins", reviewed.trusted_origins},
            {"installation", installation},
        })};
    }
    return {200, object_of({
        {"manifest_url", quote(loaded.manifest_url)},
        {"manifest", loaded.manifest},
        {"digest", quote(digest_for(loaded))},
        {"icon_data_url", icon_data_url},
        {"trusted_origins", reviewed.trusted_origins},
        {"installation", "null"},
    })};
}

struct CheckedManifest
{
    Loaded loaded;
    std::string trusted_origins{"[]"};
    std::optional<std::string> installation;
};

[[nodiscard]] inline Result fetch_reviewed(blog::BlogContext& ctx,
                                            blog::handlers::outbox::Outbox& outbox,
                                            std::string_view manifest_url,
                                            std::string_view digest,
                                            std::optional<std::string> installation = std::nullopt)
{
    const auto loaded_result = load(ctx, outbox, manifest_url);
    if (loaded_result.status != 200) return loaded_result;
    Loaded loaded;
    loaded.row = loaded_result.body;
    loaded.manifest_url = std::string{manifest_url};
    auto manifest = raw_member(loaded.row, "manifest");
    if (manifest.empty() || manifest == "null") manifest = "{}";
    loaded.manifest = manifest;
    loaded.digest = scalar_raw(loaded.row, "digest");
    auto icon = raw_member(loaded.row, "icon");
    if (icon.empty() || icon == "null") icon = raw_member(loaded.row, "icon_data_url");
    loaded.icon = icon.empty() ? std::string{"null"} : icon;
    loaded.app_id = scalar_raw(manifest, "id");
    loaded.identity_key_thumbprint = scalar_raw(loaded.row, "backend_key_thumbprint");

    if (installation && !loaded.app_id.empty() &&
        scalar_raw(*installation, "app_id") != loaded.app_id) {
        return {422, ghost_error("The reviewed manifest is for " + loaded.app_id +
            ", not the app that is installed.", "ValidationError", "APP_MANIFEST_OTHER_APP",
            "manifest_url")};
    }
    const auto review_result = review(ctx, outbox, loaded, installation);
    if (review_result.status != 200) return review_result;
    CheckedManifest checked;
    checked.loaded = loaded;
    checked.trusted_origins = loaded.trusted_origins;
    checked.installation = installation ? installation : current_installation_of(ctx, loaded.app_id);

    if (digest_for(loaded) != digest) {
        return {409, ghost_error("The app changed while it was being reviewed.", "ConflictError",
                                 "APP_MANIFEST_CHANGED", {}, {}, "null")};
    }
    return {200, core_serde::JsonCodec{}.encode(loaded.row)};
}

[[nodiscard]] inline std::string new_id()
{
    static std::uint64_t counter{0};
    return std::format("{:024x}", ++counter);
}

[[nodiscard]] inline std::string timestamp()
{
    return std::string{kDefaultTime};
}

/** @brief Best-effort staff Action write: store row + outbox record. */
[[nodiscard]] inline void record_action(blog::BlogContext& ctx,
                                        blog::handlers::outbox::Outbox& outbox,
                                        std::string_view event, std::string_view subject,
                                        std::string_view details, std::string_view actor_id)
{
    if (actor_id.empty()) return;
    const auto id = new_id();
    auto context = std::string{details};
    context.pop_back();
    context += ",\"actor_id\":" + quote(actor_id) + "}";
    const auto row = object_of({
        {"id", quote(id)},
        {"event", quote(event)},
        {"resource_type", quote("app_installation")},
        {"resource_id", quote(subject)},
        {"actor_id", quote(actor_id)},
        {"actor_name", "null"},
        {"context", quote(context)},
        {"created_at", quote(timestamp())},
    });
    ctx.store().resources().create(kActions, row);
    outbox.emit({"action.add", object_of({
        {"event", quote(event)}, {"resource_type", quote("app_installation")},
        {"resource_id", quote(subject)}, {"actor_id", quote(actor_id)}}), std::string{subject}});
}

[[nodiscard]] inline std::string actor_from_request(const interfaces::io::IRequest& req)
{
    if (req.get_authorization().empty()) return {};
    const auto header = req.find_header("x-blog-user");
    return header.empty() ? std::string{"bearer"} : std::string{header};
}

/** @brief service.create() -> repository.insert(); 201 Installation envelope. */
[[nodiscard]] inline Result install(blog::BlogContext& ctx,
                                    blog::handlers::outbox::Outbox& outbox,
                                    const CheckedManifest& checked,
                                    std::string_view actor_id)
{
    const auto& loaded = checked.loaded;
    if (current_installation_of(ctx, loaded.app_id)) {
        return {409, ghost_error("This app is already installed.", "ConflictError",
                                 "APP_ALREADY_INSTALLED", "app_id")};
    }
    const auto icon_result = fetch_icon(ctx, outbox, loaded.icon);
    if (icon_result.status != 200) return icon_result;

    outbox.begin_transaction();
    const auto manifest_id = new_id();
    const auto installation_id = new_id();
    const auto manifest_row = object_of({
        {"id", quote(manifest_id)},
        {"installation_id", quote(installation_id)},
        {"manifest_url", quote(loaded.manifest_url)},
        {"manifest", loaded.manifest},
        {"digest", quote(digest_for(loaded))},
        {"icon_data_url", icon_result.body},
        {"source", quote("install")},
        {"requires_approval", "false"},
        {"backend_key_thumbprint", quote(loaded.identity_key_thumbprint)},
        {"created_at", quote(timestamp())},
    });
    ctx.store().resources().create(kManifests, manifest_row);
    const auto row = object_of({
        {"id", quote(installation_id)},
        {"app_id", quote(loaded.app_id)},
        {"identity_key_thumbprint", quote(loaded.identity_key_thumbprint)},
        {"status", quote("active")},
        {"manifest_id", quote(manifest_id)},
        {"pending_manifest_id", "null"},
        {"revision", "0"},
        {"current_app_id", quote(loaded.app_id)},
        {"created_at", quote(timestamp())},
        {"updated_at", "null"},
        {"manifest_url", quote(loaded.manifest_url)},
        {"manifest", loaded.manifest},
        {"icon_data_url", icon_result.body},
    });
    ctx.store().resources().create(kInstallation, row);
    outbox.emit({"installation.installed", object_of({
        {"id", quote(installation_id)}, {"app_id", quote(loaded.app_id)}}), installation_id});
    outbox.commit();

    record_action(ctx, outbox, "installed", installation_id, object_of({
        {"primary_name", quote(scalar_raw(loaded.manifest, "name"))},
        {"app_id", quote(loaded.app_id)}}), actor_id);

    return {201, object_of({{"app_installations", "[" + installation_resource(ctx, row) + "]"}})};
}

[[nodiscard]] inline std::optional<std::string> pending_manifest_of(
    blog::BlogContext& ctx, std::string_view installation)
{
    const auto id = scalar_raw(installation, "pending_manifest_id");
    if (id.empty()) return std::nullopt;
    return find_one(ctx, kManifests, "id", id);
}

/** @brief service.approve() - revision-guarded approval write. */
[[nodiscard]] inline Result approve(blog::BlogContext& ctx,
                                    blog::handlers::outbox::Outbox& outbox,
                                    std::string_view id, std::string_view manifest_url,
                                    std::string_view digest, std::string_view revision,
                                    std::string_view actor_id)
{
    auto current = find_installation(ctx, id);
    if (!current) return {404, not_found()};
    if (scalar_raw(*current, "status") == "uninstalled") return {409, uninstalled_error()};

    const auto reviewed = fetch_reviewed(ctx, outbox, manifest_url, digest, current);
    if (reviewed.status != 200) return reviewed;
    CheckedManifest checked;
    checked.loaded.row = reviewed.body;
    checked.loaded.manifest_url = std::string{manifest_url};
    auto manifest = raw_member(reviewed.body, "manifest");
    if (manifest.empty() || manifest == "null") manifest = "{}";
    checked.loaded.manifest = manifest;
    checked.loaded.digest = scalar_raw(reviewed.body, "digest");
    auto icon = raw_member(reviewed.body, "icon");
    if (icon.empty() || icon == "null") icon = raw_member(reviewed.body, "icon_data_url");
    checked.loaded.icon = icon.empty() ? std::string{"null"} : icon;
    checked.loaded.app_id = scalar_raw(manifest, "id");
    checked.loaded.identity_key_thumbprint = scalar_raw(reviewed.body, "backend_key_thumbprint");
    checked.installation = current;

    const auto reviewed_revision = scalar_raw(*current, "revision");
    if (reviewed_revision != revision) {
        return {409, ghost_error("The installation changed while the app was being reviewed.",
                                 "ConflictError", "APP_INSTALLATION_CHANGED")};
    }

    const auto original_manifest_id = scalar_raw(*current, "manifest_id");
    const auto approved_digest = [&] {
        if (original_manifest_id.empty()) return std::string{};
        const auto row = find_one(ctx, kManifests, "id", original_manifest_id);
        return row ? scalar_raw(*row, "digest") : std::string{};
    }();
    const auto pending = pending_manifest_of(ctx, *current);
    const auto pending_digest = pending ? scalar_raw(*pending, "digest") : std::string{};
    const auto loaded_digest = digest_for(checked.loaded);
    const bool needs_approval = scalar_raw(checked.loaded.row, "requires_approval") == "true";

    enum class Write { None, ApproveExisting, AddApproved } write{Write::None};
    if (!loaded_digest.empty() && loaded_digest == approved_digest) {
        if (pending) write = Write::ApproveExisting;
    } else if (!loaded_digest.empty() && loaded_digest == pending_digest) {
        write = Write::ApproveExisting;
    } else if (!loaded_digest.empty()) {
        write = Write::AddApproved;
    }
    (void)needs_approval; // approval applies approval-requiring changes too.

    std::string write_icon;
    if (write == Write::AddApproved) {
        const auto icon_result = fetch_icon(ctx, outbox, checked.loaded.icon);
        if (icon_result.status != 200) return icon_result;
        write_icon = icon_result.body;
    }

    // whileUnchanged stand-in: re-read and compare the revision before writing.
    const auto locked = current_installation(ctx, id);
    if (!locked) return {409, uninstalled_error()};
    if (scalar_raw(*locked, "revision") != reviewed_revision) {
        return {409, ghost_error("The installation changed while the app was being reviewed.",
                                 "ConflictError", "APP_INSTALLATION_CHANGED")};
    }

    outbox.begin_transaction();
    std::string approved_id = original_manifest_id;
    if (write == Write::AddApproved) {
        approved_id = new_id();
        const auto manifest_row = object_of({
            {"id", quote(approved_id)},
            {"installation_id", quote(id)},
            {"manifest_url", quote(checked.loaded.manifest_url)},
            {"manifest", checked.loaded.manifest},
            {"digest", quote(loaded_digest)},
            {"icon_data_url", write_icon},
            {"source", quote("approval")},
            {"requires_approval", "false"},
            {"backend_key_thumbprint", quote(checked.loaded.identity_key_thumbprint)},
            {"created_at", quote(timestamp())},
        });
        ctx.store().resources().create(kManifests, manifest_row);
    }
    const auto next_revision = std::to_string(revision_number(reviewed_revision) + 1);
    auto kept_icon = raw_member(*current, "icon_data_url");
    if (kept_icon.empty()) kept_icon = "null";
    auto patch = object_of({
        {"status", quote("active")},
        {"manifest_id", quote(approved_id)},
        {"pending_manifest_id", "null"},
        {"revision", next_revision},
        {"updated_at", quote(timestamp())},
        {"manifest_url", quote(checked.loaded.manifest_url)},
        {"manifest", checked.loaded.manifest},
        {"icon_data_url", write == Write::AddApproved ? write_icon : kept_icon},
    });
    ctx.store().resources().edit(kInstallation, id, patch);
    outbox.emit({"installation.approved", object_of({
        {"id", quote(id)}, {"manifest_id", quote(approved_id)}}), std::string{id}});
    outbox.commit();

    if (write != Write::None) {
        record_action(ctx, outbox, "changes_approved", id, object_of({
            {"primary_name", quote(scalar_raw(checked.loaded.manifest, "name"))},
            {"app_id", quote(checked.loaded.app_id)},
            {"from_manifest_id", quote(original_manifest_id)},
            {"to_manifest_id", quote(approved_id)}}), actor_id);
    }
    return read_one(ctx, id, false);
}

/** @brief service.uninstall() - soft transition, idempotent, 204 empty. */
[[nodiscard]] inline Result uninstall(blog::BlogContext& ctx,
                                      blog::handlers::outbox::Outbox& outbox,
                                      std::string_view id, std::string_view actor_id)
{
    const auto row = find_installation(ctx, id);
    if (!row) return {404, not_found()};
    if (scalar_raw(*row, "status") == "uninstalled") return {204, {}};

    outbox.begin_transaction();
    const auto next = std::to_string(revision_number(scalar_raw(*row, "revision")) + 1);
    const auto patch = object_of({
        {"status", quote("uninstalled")},
        {"current_app_id", "null"},
        {"revision", next},
        {"updated_at", quote(timestamp())},
    });
    ctx.store().resources().edit(kInstallation, id, patch);
    outbox.emit({"installation.uninstalled", object_of({{"id", quote(id)}}), std::string{id}});
    outbox.commit();

    record_action(ctx, outbox, "uninstalled", id, object_of({
        {"primary_name", quote(scalar_raw(*row, "name"))},
        {"app_id", quote(scalar_raw(*row, "app_id"))}}), actor_id);
    return {204, {}};
}

/** @brief Read `id` from the last non-empty path segment (framework path param). */
[[nodiscard]] inline std::string path_id(std::string_view raw)
{
    auto path = raw.substr(0, raw.find('?'));
    while (path.size() > 1 && path.back() == '/') path.remove_suffix(1);
    const auto slash = path.find_last_of('/');
    return slash == std::string_view::npos ? std::string{path}
                                           : std::string{path.substr(slash + 1)};
}

/** @brief Parse query params from a request path. */
[[nodiscard]] inline std::map<std::string, std::string> query_params(std::string_view raw)
{
    std::map<std::string, std::string> params;
    const auto q = raw.find('?');
    if (q == std::string_view::npos) return params;
    for (auto part : std::std::views::split(string_view{raw}.substr(q + 1), '&')) {
        const std::string text{std::ranges::begin(part), std::ranges::end(part)};
        const auto eq = text.find('=');
        if (eq != std::string::npos) params[text.substr(0, eq)] = text.substr(eq + 1);
    }
    return params;
}

/** @brief Unwrap a Ghost write frame: data.<resource>[0]. */
[[nodiscard]] inline const serde::Value* first_data_item(const serde::Value& root,
                                                         std::string_view resource)
{
    const auto* data = std::get_if<Object>(&root.get());
    if (data == nullptr) return nullptr;
    const serde::Value* collection{nullptr};
    for (const auto& [name, value] : *data) {
        if (name == "data") { collection = &value; break; }
    }
    if (collection == nullptr) return nullptr;
    const auto* data_object = std::get_if<Object>(&collection->get());
    if (data_object == nullptr) return nullptr;
    for (const auto& [name, value] : *data_object) {
        if (name != resource) continue;
        const auto* array = std::get_if<Array>(&value.get());
        return array == nullptr || array->empty() ? nullptr : &array->front();
    }
    return nullptr;
}

[[nodiscard]] inline std::string scalar_of(const serde::Value& value, std::string_view key)
{
    return scalar(value, key);
}

} // namespace blog::handlers::apps::support

export namespace blog::handlers::apps::app_installations_browse::workflow {

class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"apps_app_installations_browse_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output> invoke(
        const Input&, core::workflow::WorkflowContext&) const
    {
        const auto result = support::browse(*s_ctx);
        return core::workflow::StepResponse<Output>{
            {{"status", std::to_string(result.status)}, {"body", result.body}}};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};

} // namespace blog::handlers::apps::app_installations_browse::workflow
