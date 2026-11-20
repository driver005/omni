export module blog_handlers:app_sessions_sessions_create_step;
import :domain_all;
import std;
import core_workflow;
import core_query;
import core_serde;
import interfaces;
import blog_handlers_middleware;
import :outbox_shim;

// ---------------------------------------------------------------------------
// Strict Ghost parity for `appSessions.start`:
//   app-sessions.ts:6-23 -> service.startSession(id) -> recheck(id) + read(id).
//
// Shared abstractions used (all in already-registered partitions/modules):
//   * core::query          - allowlisted NQL filtering/order over the store rows
//   * core_serde           - JSON decode/encode
//   * core_auth::ghost + blog_handlers_middleware - permission gate (handler)
//   * blog::handlers::outbox -
//       transactional side-effect log for manifest challenge/fetch, icon copy,
//       manifest row writes and activation/suspension (CONTRACTS.md section 4).
//
// No session state is stored: Ghost pins the session to installation.revision
// and returns { installation } only (service.ts:281-294).
// ---------------------------------------------------------------------------
export namespace blog::handlers::app_sessions::sessions_create::support {

using Array = serde::Value::Array;
using Object = serde::Value::Object;

inline constexpr std::string_view kInstallation{"app_installations"};
inline constexpr std::string_view kManifests{"app_installation_manifests"};
inline constexpr std::string_view kDefaultTime{"1970-01-01T00:00:00.000Z"};

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

/** @brief Scalar string for a JSON object member; null/absent is the empty string. */
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

/** @brief revision is an int in Ghost; fixtures may omit it. */
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

/** @brief Raw JSON value of a member (quotes/braces kept), e.g. the manifest object. */
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

/** @brief core::query over the real ResourceStore rows (one filtered, ordered pass). */
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

/** @brief repository.find(id): uninstalled rows still exist but are not "current". */
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

/** @brief Full Ghost error envelope, including the app failure code. */
[[nodiscard]] inline std::string ghost_error(
    std::string_view message, std::string_view type, std::string_view code,
    std::string_view context = {})
{
    static std::uint64_t counter{0};
    return std::format(
        R"({{"errors":[{{"id":"{:032x}","code":{},"context":{},"details":null,"ghostErrorCode":null,"help":null,"message":{},"property":null,"type":{}}}]}})",
        ++counter,
        code.empty() ? std::string{"null"} : quote(code),
        context.empty() ? std::string{"null"} : quote(context),
        quote(message), quote(type));
}

/** @brief service.ts:76-82 - notInstalled() = 404 with code APP_NOT_INSTALLED. */
[[nodiscard]] inline std::string not_installed()
{
    return ghost_error("This app is not installed.", "NotFoundError", "APP_NOT_INSTALLED");
}

/** @brief service.ts:72-74 - notFound() from read(). */
[[nodiscard]] inline std::string not_found()
{
    return ghost_error("App installation not found.", "NotFoundError", {});
}

/** @brief service.ts:98-104 - unavailable() = 409 ConflictError. */
[[nodiscard]] inline std::string unavailable(
    std::string_view message, std::string_view code, std::string_view context)
{
    return ghost_error(message, "ConflictError", code, context);
}

/** @brief AppInstallationResource (serializers.ts:7-19); no history on read(). */
[[nodiscard]] inline std::string installation_resource(
    blog::BlogContext& ctx, std::string_view row)
{
    const auto id = scalar_raw(row, "id");
    const auto app_id = scalar_raw(row, "app_id");
    auto status = scalar_raw(row, "status");
    if (status.empty()) status = "active";
    auto revision = scalar_raw(row, "revision");
    if (revision.empty()) revision = "0";
    auto created = scalar_raw(row, "created_at");
    if (created.empty()) created = std::string{kDefaultTime};
    const auto updated = scalar_raw(row, "updated_at");

    auto manifest_url = scalar_raw(row, "manifest_url");
    std::string manifest{"null"};
    std::string icon{"null"};
    const auto manifest_id = scalar_raw(row, "manifest_id");
    if (!manifest_id.empty()) {
        if (const auto stored = find_one(ctx, kManifests, "id", manifest_id)) {
            if (manifest_url.empty()) manifest_url = scalar_raw(*stored, "manifest_url");
            const auto value = raw_member(*stored, "manifest");
            if (!value.empty()) manifest = value;
            const auto icon_value = raw_member(*stored, "icon_data_url");
            if (!icon_value.empty() && icon_value != "null") icon = icon_value;
        }
    }

    return "{\"id\":" + quote(id)
         + ",\"app_id\":" + quote(app_id)
         + ",\"status\":" + quote(status)
         + ",\"revision\":" + revision
         + ",\"manifest_url\":" + quote(manifest_url)
         + ",\"manifest\":" + manifest
         + ",\"icon_data_url\":" + icon
         + ",\"created_at\":" + quote(created)
         + ",\"updated_at\":" + (updated.empty() ? std::string{"null"} : quote(updated))
         + "}";
}

/** @brief Replace one member of a flat JSON object (raw JSON value). */
[[nodiscard]] inline std::string set_member(std::string_view json, std::string_view key,
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

/** @brief One in-flight recheck per installation id (service.ts:145-146,525-533). */
struct RecheckSlot
{
    std::mutex mutex;
    std::condition_variable cv;
    bool done{false};
    int status{200};
    std::string error;
};

[[nodiscard]] inline std::mutex& recheck_mutex() noexcept
{
    static std::mutex mutex;
    return mutex;
}

[[nodiscard]] inline std::map<std::string, std::shared_ptr<RecheckSlot>>& rechecks()
{
    static std::map<std::string, std::shared_ptr<RecheckSlot>> slots;
    return slots;
}

struct RecheckResult
{
    int status{200};
    std::string error;
};

/** @brief ValidationError from recheckOnce -> unavailableUnlessChanged (service.ts:577-584). */
[[nodiscard]] inline RecheckResult unavailable_unless_changed(
    blog::BlogContext& ctx, std::string_view before, std::string_view code,
    std::string_view message, std::string_view context)
{
    const auto current = current_installation(ctx, scalar_raw(before, "id"));
    if (!current) return {404, not_installed()};
    if (scalar_raw(*current, "revision") != scalar_raw(before, "revision")) {
        return {}; // The installation changed while the manifest was loading: proceed.
    }
    return {409, unavailable(message, code, context)};
}

/**
 * @brief service.ts:553-590. The approved manifest URL is re-fetched (outbox-recorded
 *        stand-in for the network call), the identity challenge is started in parallel,
 *        the gone app / different app / failed identity cases map to Ghost failures, and
 *        silent changes are applied while approval-requiring changes suspend.
 */
[[nodiscard]] inline RecheckResult recheck_once(
    blog::BlogContext& ctx, std::string_view id,
    blog::handlers::outbox::Outbox& outbox)
{
    const auto before = current_installation(ctx, id);
    if (!before) return {404, not_installed()};

    const auto url = scalar_raw(*before, "manifest_url");
    // load(): identity.challenge(url) + loadManifest(url) run together (service.ts:345-350).
    outbox.emit({"identity.challenge", "{\"manifest_url\":" + quote(url) + "}", std::string{id}});
    outbox.emit({"manifest.fetch", "{\"manifest_url\":" + quote(url) + "}", std::string{id}});

    const auto manifests = rows_for(ctx, kManifests, {{"installation_id", std::string{id}}},
                                    "created_at desc");
    if (manifests.empty()) {
        return unavailable_unless_changed(ctx, *before, "APP_UNAVAILABLE",
            "App manifest could not be loaded.", "manifest_url");
    }
    const auto loaded = core_serde::JsonCodec{}.encode(manifests.front());

    // APP_MANIFEST_OTHER_APP (service.ts:567-572): the URL now serves a different app.
    const auto loaded_manifest = raw_member(loaded, "manifest");
    const auto served_app = scalar_raw(loaded_manifest, "id");
    const auto app_id = scalar_raw(*before, "app_id");
    if (!served_app.empty() && !app_id.empty() && served_app != app_id) {
        return unavailable_unless_changed(ctx, *before, "APP_MANIFEST_OTHER_APP",
            url + " now serves " + served_app + ".", "manifest_url");
    }

    // identity.check(loaded, pinned, started) (service.ts:573). A stored thumbprint
    // mismatch fails the session; absent thumbprints are the fixture stand-in.
    const auto loaded_key = scalar_raw(loaded, "backend_key_thumbprint");
    const auto pinned_key = scalar_raw(*before, "identity_key_thumbprint");
    if (!loaded_key.empty() && !pinned_key.empty() && loaded_key != pinned_key) {
        return unavailable_unless_changed(ctx, *before, "APP_UNAVAILABLE",
            "App manifest failed the identity check.", "manifest");
    }

    const auto approved_id = scalar_raw(*before, "manifest_id");
    const auto pending_id = scalar_raw(*before, "pending_manifest_id");
    const auto loaded_digest = scalar_raw(loaded, "digest");
    const auto requires_approval = scalar_raw(loaded, "requires_approval");
    const bool needs_approval = requires_approval == "true" || requires_approval == "1";

    std::string approved_digest;
    if (!approved_id.empty()) {
        if (const auto approved = find_one(ctx, kManifests, "id", approved_id)) {
            approved_digest = scalar_raw(*approved, "digest");
        }
    }
    std::string pending_digest;
    if (!pending_id.empty()) {
        if (const auto pending = find_one(ctx, kManifests, "id", pending_id)) {
            pending_digest = scalar_raw(*pending, "digest");
        }
    }

    // decideOnManifest/recheckWrite (decide-on-manifest.ts:133-168).
    enum class Write { None, Resume, Approve, Suspend } write{Write::None};
    if (!loaded_digest.empty() && loaded_digest == approved_digest) {
        if (!pending_id.empty()) write = Write::Resume; // app reverted pending changes
    } else if (!loaded_digest.empty() && loaded_digest == pending_digest) {
        write = Write::None; // use_pending
    } else if (needs_approval) {
        write = Write::Suspend;
    } else if (!loaded_digest.empty()) {
        write = Write::Approve;
    }

    if (write == Write::None) return {};

    // withIconCopy(): only writes that add a row fetch the icon.
    outbox.emit({"icon.fetch", "{\"manifest_url\":" + quote(url) + "}", std::string{id}});

    // whileUnchanged(): re-read before writing, so an uninstall/change during the
    // manifest load wins and this recheck does nothing.
    const auto current = current_installation(ctx, id);
    if (!current) return {404, not_installed()};
    if (scalar_raw(*current, "revision") != scalar_raw(*before, "revision")) return {};

    outbox.begin_transaction();
    auto copy = loaded;
    const auto strip = [](std::string_view json, std::string_view key) {
        // Rows are flat JSON objects; remove a member without a full serde rewrite.
        const auto parsed = core_serde::JsonCodec{}.decode<serde::Value>(json);
        if (!parsed) return std::string{json};
        const auto* object = std::get_if<Object>(&parsed->get());
        if (object == nullptr) return std::string{json};
        std::string out{"{"};
        bool first{true};
        for (const auto& [name, member] : *object) {
            if (name == key || name == "id") continue;
            if (!first) out.push_back(',');
            first = false;
            out += quote(name) + ':' + core_serde::JsonCodec{}.encode(member);
        }
        out.push_back('}');
        return out;
    };
    copy = strip(loaded, "created_at");
    // decide-on-manifest.ts:154-168 - the new row records why it was added.
    copy = set_member(copy, "source",
                      quote(write == Write::Resume ? "resume" : "update"));
    copy = set_member(copy, "requires_approval",
                      write == Write::Suspend ? "true" : "false");
    const std::string new_manifest_id = ctx.store().resources().create(kManifests, copy);
    outbox.emit({"manifest.added", "{\"id\":\"" + new_manifest_id + "\",\"installation_id\":\"" +
        std::string{id} + "\"}", std::string{id}});

    std::string patch;
    const auto next_revision = std::to_string(revision_number(scalar_raw(*current, "revision")) + 1);
    if (write == Write::Suspend) {
        patch = "{\"status\":\"suspended\",\"pending_manifest_id\":\"" + new_manifest_id +
                "\",\"revision\":" + next_revision + "}";
        outbox.emit({"installation.suspended", "{\"id\":\"" + std::string{id} + "\"}", std::string{id}});
    } else {
        patch = "{\"status\":\"active\",\"manifest_id\":\"" + new_manifest_id +
                "\",\"pending_manifest_id\":null,\"revision\":" + next_revision + "}";
        outbox.emit({"installation.activated", "{\"id\":\"" + std::string{id} + "\"}", std::string{id}});
    }
    ctx.store().resources().edit(kInstallation, id, patch);
    outbox.commit();

    const auto stored = current_installation(ctx, id);
    if (!stored) return {404, not_installed()};
    return {};
}

[[nodiscard]] inline RecheckResult recheck(
    blog::BlogContext& ctx, std::string_view id, blog::handlers::outbox::Outbox& outbox)
{
    std::shared_ptr<RecheckSlot> slot;
    bool owner{false};
    {
        std::lock_guard lock{recheck_mutex()};
        auto& slots = rechecks();
        const auto it = slots.find(std::string{id});
        if (it != slots.end()) {
            slot = it->second;
        } else {
            slot = std::make_shared<RecheckSlot>();
            slots.emplace(std::string{id}, slot);
            owner = true;
        }
    }
    if (!owner) {
        std::unique_lock lock{slot->mutex};
        slot->cv.wait(lock, [&slot] { return slot->done; });
        return {slot->status, slot->error};
    }

    RecheckResult result;
    try {
        result = recheck_once(ctx, id, outbox);
    } catch (...) {
        result = {500, ghost_error("App recheck failed.", "InternalServerError", {})};
    }
    {
        std::lock_guard lock{slot->mutex};
        slot->status = result.status;
        slot->error = result.error;
        slot->done = true;
    }
    slot->cv.notify_all();
    {
        std::lock_guard lock{recheck_mutex()};
        rechecks().erase(std::string{id});
    }
    return result;
}

/** @brief service.ts:286-294 then read(id): recheck first, then load the installation. */
[[nodiscard]] inline RecheckResult start_session(
    blog::BlogContext& ctx, std::string_view id, blog::handlers::outbox::Outbox& outbox)
{
    const auto checked = recheck(ctx, id, outbox);
    if (checked.status != 200) return checked;

    const auto row = find_installation(ctx, id);
    if (!row) return {404, not_found()};
    if (scalar_raw(*row, "status") == "uninstalled") return {404, not_installed()};

    // serializers.ts:27-29 -> {"app_sessions":[{"installation": AppInstallationResource}]}
    const std::string body = "{\"app_sessions\":[{\"installation\":" +
                             installation_resource(ctx, *row) + "}]}";
    return {200, body};
}

} // namespace blog::handlers::app_sessions::sessions_create::support

export namespace blog::handlers::app_sessions::sessions_create::workflow {

class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"app_sessions_sessions_create_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output> invoke(
        const Input& in, core::workflow::WorkflowContext&) const
    {
        using namespace support;

        // options:['id'], validation.options.id.required=true (app-sessions.ts:16-17).
        const std::string raw{in.get_path()};
        auto path = std::string_view{raw}.substr(0, raw.find('?'));
        while (path.size() > 1 && path.back() == '/') path.remove_suffix(1);
        const auto slash = path.find_last_of('/');
        const std::string id{slash == std::string_view::npos ? std::string{path}
                                                            : std::string{path.substr(slash + 1)}};
        if (id.empty()) return invalid();

        auto& ctx = *s_ctx;
        auto& outbox = blog::handlers::outbox::global_outbox();
        const auto result = start_session(ctx, id, outbox);
        return core::workflow::StepResponse<Output>{{{"status", std::to_string(result.status)},
                                                     {"body", result.error}}};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;

private:
    [[nodiscard]] static core::workflow::StepResponse<Output> invalid()
    {
        return core::workflow::StepResponse<Output>{{{"status", "422"},
            {"body", blog::api::ApiError{}.validation("id is required", {}, "id")}}};
    }
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};

} // namespace blog::handlers::app_sessions::sessions_create::workflow
