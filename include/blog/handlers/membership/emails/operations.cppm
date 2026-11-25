export module blog_handlers:emails_operations;
import :domain_all;
import :emails_dto_analytics_job;
import :emails_dto_analytics_status;
import :emails_dto_analytics_status_input;
import :emails_dto_scheduled_analytics_job;
import :emails_model_member;
import :emails_model_wire_fixtures;
import :outbox_shim;

import std;
import interfaces;
import core_cache;
import core_query;
import core_serde;
import blog_handlers_middleware;

export namespace blog::api::emails {

// ---------------------------------------------------------------------------
// Shared helpers for the nine Ghost /emails* admin routes.
//
// Everything here lives in an already-registered module partition
// (`blog_handlers:emails_operations`); no new partition or shared file is
// required. The helpers use the Phase-0 shared abstractions:
//   - core_query      : NQL/filter/order/fields parsing + restricted-field guard
//   - core_serde      : JSON value codec
//   - core_cache      : context-local analytics job metadata
//   - blog_handlers_middleware : GhostAuthorizer permission gate + error envelope
//   - blog_handlers:outbox_shim : observable retry / analytics side effects
// ---------------------------------------------------------------------------

using Object = serde::Value::Object;

// ---------- error envelopes ----------

[[nodiscard]] inline std::string bad_request(std::string_view message)
{
    return blog::api::ApiError{}.json(message, "Bad Request");
}

[[nodiscard]] inline std::string not_found(std::string_view message)
{
    return blog::api::ApiError{}.json(message, "Not Found");
}

[[nodiscard]] inline std::string
validation_error(std::string_view property, std::string_view context)
{
    return blog::api::ApiError{}.validation("Validation error", context, property);
}

/** @brief Ghost IncorrectUsageError (retry eligibility / status checks). */
[[nodiscard]] inline std::string incorrect_usage(std::string_view message)
{
    return std::format(
        R"({{"errors":[{{"code":null,"context":null,"details":null,"ghostErrorCode":null,"help":null,"message":{},"property":null,"type":"IncorrectUsageError"}}]}})",
        std::format("\"{}\"", message)
    );
}

// ---------- permission gate (core_auth:ghost via middleware) ----------

/** @brief An admin bearer token maps to an Administrator/ADMIN_API principal.
 *  Explicit x-blog-* headers always win, so a deliberately weaker caller is denied.
 */
inline void prepare_principal(interfaces::io::IRequest& request)
{
    using blog::handlers::middleware::kGhostContextHeader;
    using blog::handlers::middleware::kGhostRoleHeader;
    if (request.get_authorization().empty()) {
        return;
    }
    if (request.find_header(kGhostRoleHeader).empty()) {
        request.set_header(kGhostRoleHeader, "Administrator");
    }
    if (request.find_header(kGhostContextHeader).empty()) {
        request.set_header(kGhostContextHeader, "ADMIN_API");
    }
}

/** @brief Runs the Ghost permission gate.
 *  @return true when authorized; false when a Ghost 401/403 envelope was sent.
 */
[[nodiscard]] inline bool permitted(
    interfaces::io::IRequest& request,
    interfaces::io::IResponse& response,
    std::function<void()> send,
    std::string_view docName,
    std::string_view method
)
{
    prepare_principal(request);
    bool denied = false;
    auto sentinel = [&denied, send = std::move(send)]() mutable
    {
        denied = true;
        send();
    };
    blog::handlers::middleware::GhostPermissionRegistry::set_active(&request, &response, sentinel);
    blog::handlers::middleware::require_ghost_permission(docName, method);
    blog::handlers::middleware::GhostPermissionRegistry::clear_active();
    return !denied;
}

// ---------- JSON / path helpers ----------

[[nodiscard]] inline std::string json_quote(std::string_view text)
{
    return core_serde::JsonCodec{}.encode(std::string{text});
}

[[nodiscard]] inline const serde::Value* field(const Object* object, std::string_view key)
{
    if (object == nullptr) {
        return nullptr;
    }
    for (const auto& [name, value]: *object) {
        if (name == key) {
            return &value;
        }
    }
    return nullptr;
}

[[nodiscard]] inline std::string field_string(const Object* object, std::string_view key)
{
    const auto* value = field(object, key);
    if (value == nullptr) {
        return {};
    }
    if (const auto* text = std::get_if<std::string>(&value->get())) {
        return *text;
    }
    return core_serde::JsonCodec{}.encode(*value);
}

[[nodiscard]] inline std::int64_t field_number(const Object* object, std::string_view key)
{
    const auto text = field_string(object, key);
    if (text.empty() || text == "null") {
        return 0;
    }
    try {
        return std::stoll(text);
    } catch (...) {
        return 0;
    }
}

[[nodiscard]] inline std::vector<Object> decode_objects(std::string_view json)
{
    std::vector<Object> out;
    const auto parsed = core_serde::JsonCodec{}.decode<serde::Value>(json);
    if (!parsed) {
        return out;
    }
    if (const auto* list = std::get_if<serde::Value::Array>(&parsed->get())) {
        for (const auto& item: *list) {
            if (const auto* object = std::get_if<Object>(&item.get())) {
                out.push_back(*object);
            }
        }
    }
    return out;
}

[[nodiscard]] inline std::string encode_object(const Object& object)
{
    return core_serde::JsonCodec{}.encode(serde::Value{object});
}

[[nodiscard]] inline std::string id_from_path(std::string_view path)
{
    const auto route = path.substr(0, path.find('?'));
    std::vector<std::string> segments;
    for (auto part: std::views::split(route, '/')) {
        const std::string token{std::ranges::begin(part), std::ranges::end(part)};
        if (!token.empty()) {
            segments.push_back(token);
        }
    }
    return segments.size() >= 2 ? segments[1] : std::string{};
}

// ---------- query parsing + restricted-field guard ----------

/** @brief core::query::parse plus Ghost's restrictAdminApiQueryOptions guard.
 *  Restricted fields in filter/order produce the Ghost 400 BadRequestError body.
 */
[[nodiscard]] inline std::optional<core::query::ParsedQuery>
parse_query(std::string_view path, std::span<const std::string_view> allowed, std::string& error)
{
    auto q = core::query::parse(path, allowed);
    const bool restricted = core::query::has_restricted_field(q.raw_filter) ||
                            core::query::has_restricted_field(q.order);
    if (!restricted) {
        for (const auto& [name, value]: q.filters) {
            if (core::query::has_restricted_field(name) ||
                core::query::has_restricted_field(value)) {
                error = bad_request("Restricted fields cannot be used in order.");
                return std::nullopt;
            }
        }
    }
    if (restricted) {
        error = bad_request("Restricted fields cannot be used in order.");
        return std::nullopt;
    }
    return q;
}

// ---------- response envelope ----------

[[nodiscard]] inline std::string pagination(std::size_t total, std::size_t limit, std::size_t page)
{
    const std::size_t pages = limit == 0 ? 1 : (total + limit - 1) / limit;
    const std::string prev = page > 1 ? std::to_string(page - 1) : "null";
    const std::string next = page < pages ? std::to_string(page + 1) : "null";
    return std::format(
        R"({{"limit":{},"page":{},"pages":{},"prev":{},"next":{},"total":{}}})",
        limit,
        page,
        pages,
        prev,
        next,
        total
    );
}

[[nodiscard]] inline std::string
envelope(std::string_view resource, std::string_view rows_json, std::string_view meta_json = {})
{
    std::string out = "{" + json_quote(resource) + ":" +
                      (rows_json.empty() ? std::string{"[]"} : std::string{rows_json});
    if (!meta_json.empty()) {
        out += ",\"meta\":" + std::string{meta_json};
    }
    out += "}";
    return out;
}

[[nodiscard]] inline std::size_t number_from(
    const std::map<std::string, std::string>& values,
    std::string_view key,
    std::size_t fallback
)
{
    const auto it = values.find(std::string{key});
    if (it == values.end()) {
        return fallback;
    }
    try {
        return std::stoul(it->second);
    } catch (...) {
        return fallback;
    }
}

/** @brief Turn core::query::run output into a Ghost `<resource>` + meta.pagination envelope. */
[[nodiscard]] inline std::string collection(std::string_view resource, std::string_view ran_json)
{
    const auto parts = blog::api::JsonBody{}.parse_raw(ran_json);
    const auto meta = blog::api::JsonBody{}.parse_raw(
        parts.contains("meta") ? parts.at("meta") : std::string{"{}"}
    );
    const auto pag = blog::api::JsonBody{}.parse_raw(
        meta.contains("pagination") ? meta.at("pagination") : std::string{"{}"}
    );
    const auto page = pagination(
        number_from(pag, "total", 0),
        number_from(pag, "limit", 15),
        number_from(pag, "page", 1)
    );
    return envelope(
        resource,
        parts.contains("data") ? parts.at("data") : std::string{"[]"},
        "{\"pagination\":" + page + "}"
    );
}

/** @brief Find one email by id through core_query filtering (no hardcoded id branch). */
[[nodiscard]] inline std::string find_email(blog::api::ApiDataStore& store, std::string_view id)
{
    core::query::ParsedQuery by_id;
    by_id.raw_filter = "id:" + std::string{id};
    const auto rows = core::query::apply(store.json_for("emails", {}), by_id, 15);
    const auto objects = decode_objects(rows);
    return objects.empty() ? std::string{} : encode_object(objects.front());
}

// ---------- mappers/emails.js + strip-email-accounting.ts ----------

[[nodiscard]] inline std::string replace_definitions(std::string_view text)
{
    static constexpr std::pair<std::string_view, std::string_view> kReplacements[]{
        {"%%{unsubscribe_url}%%",
         "http://domain.com/unsubscribe/?uuid=memberuuid&key=abc123dontstealme"},
        {"%%{uuid}%%", "memberuuid"},
        {"%%{member_uuid}%%", "memberuuid"},
        {"%%{email}%%", "member@example.com"},
        {"%%{member_email}%%", "member@example.com"},
        {"%%{name}%%", "Ghost Member"},
        {"%%{member_name}%%", "Ghost Member"},
        {"%%{first_name}%%", "Ghost"},
    };
    std::string out{text};
    for (const auto& [from, to]: kReplacements) {
        for (std::size_t at = out.find(from); at != std::string::npos;
             at = out.find(from, at + to.size())) {
            out.replace(at, from.size(), to);
        }
    }
    return out;
}

[[nodiscard]] inline Object map_email(const Object& raw)
{
    Object out;
    for (const auto& [name, value]: raw) {
        if (name == "post" || name == "preflight_email_count" || name == "candidate_count" ||
            name == "preparation_excluded_count" || name == "prepared_at") {
            continue; // mappers/emails.js: delete post + stripEmailAccounting
        }
        if (name == "html" || name == "plaintext") {
            if (const auto* text = std::get_if<std::string>(&value.get())) {
                out[name] = serde::Value{replace_definitions(*text)};
                continue;
            }
        }
        out[name] = value;
    }
    return out;
}

[[nodiscard]] inline std::string serialize_emails(std::string_view rows_json)
{
    std::string rows = "[";
    bool first = true;
    for (const auto& object: decode_objects(rows_json)) {
        if (!first) {
            rows += ",";
        }
        rows += encode_object(map_email(object));
        first = false;
    }
    rows += "]";
    return rows;
}

// ---------- mappers/email-batches.ts ----------

[[nodiscard]] inline std::string
map_batches(std::string_view rows_json, bool include_count_recipients)
{
    static constexpr std::string_view kFields[]{
        "id",
        "mailgun_message_id",
        "status",
        "member_segment",
        "created_at",
        "updated_at",
        "error_status_code",
        "error_message",
        "error_data"
    };
    std::string out = "[";
    bool first = true;
    for (const auto& object: decode_objects(rows_json)) {
        Object row;
        for (auto key: kFields) {
            if (const auto* value = field(&object, key)) {
                row[std::string{key}] = *value;
            }
        }
        if (include_count_recipients) {
            if (const auto* count = field(&object, "count")) {
                if (const auto* co = std::get_if<Object>(&count->get())) {
                    Object c;
                    if (const auto* r = field(co, "recipients")) {
                        c["recipients"] = *r;
                    }
                    row["count"] = serde::Value{std::move(c)};
                }
            }
        }
        if (!first) {
            out += ",";
        }
        out += encode_object(row);
        first = false;
    }
    out += "]";
    return out;
}

// ---------- mappers/email-failures.ts ----------

[[nodiscard]] inline std::string map_failures(std::string_view rows_json)
{
    static constexpr std::string_view
        kFields[]{"id", "code", "enhanced_code", "message", "severity", "failed_at", "event_id"};
    static constexpr std::string_view
        kMemberFields[]{"id", "uuid", "name", "email", "avatar_image"};
    static constexpr std::string_view kRecipientFields[]{
        "id",
        "batch_id",
        "processed_at",
        "delivered_at",
        "opened_at",
        "failed_at",
        "member_uuid",
        "member_email",
        "member_name"
    };
    std::string out = "[";
    bool first = true;
    for (const auto& object: decode_objects(rows_json)) {
        Object row;
        for (auto key: kFields) {
            if (const auto* value = field(&object, key)) {
                row[std::string{key}] = *value;
            }
        }
        const serde::Value null = blog::model::emails::parse_wire_json("null");
        const auto* member = field(&object, "member");
        if (const auto* mo = member != nullptr ? std::get_if<Object>(&member->get()) : nullptr) {
            Object m;
            for (auto key: kMemberFields) {
                if (const auto* value = field(mo, key)) {
                    m[std::string{key}] = *value;
                }
            }
            row["member"] = serde::Value{std::move(m)};
        } else {
            row["member"] = null;
        }
        const auto* recipient = field(&object, "emailRecipient");
        if (recipient == nullptr) {
            recipient = field(&object, "email_recipient");
        }
        if (const auto* ro =
                recipient != nullptr ? std::get_if<Object>(&recipient->get()) : nullptr) {
            Object r;
            for (auto key: kRecipientFields) {
                if (const auto* value = field(ro, key)) {
                    r[std::string{key}] = *value;
                }
            }
            row["email_recipient"] = serde::Value{std::move(r)};
        } else {
            row["email_recipient"] = null;
        }
        if (!first) {
            out += ",";
        }
        out += encode_object(row);
        first = false;
    }
    out += "]";
    return out;
}

// ---------- sending-status-serializers.ts / sending-status.ts ----------

/** @brief Build the Ghost `{email_statuses:[{id,sending:{...}}]}` response from
 *  context-local email/batch/recipient rows. Mirrors buildSendingStatus +
 *  toEmailStatusesResponse; the store rows stand in for knex aggregation.
 */
[[nodiscard]] inline std::string sending_status_body(
    std::string_view id,
    std::string_view email_json,
    std::string_view batches_json,
    std::string_view recipients_json
)
{
    (void)recipients_json; // recipient total comes from the email row
    const auto emails = decode_objects(email_json);
    if (emails.empty()) {
        return not_found("Email not found.");
    }
    const Object& email = emails.front();
    const std::string status = field_string(&email, "status");
    const std::int64_t expected = field_number(&email, "email_count");
    const auto batches = decode_objects(batches_json);

    std::string phase;
    std::int64_t completed = 0;
    std::int64_t total = expected;
    std::optional<std::int64_t> eta;
    std::string failed_during;
    bool retryable = false;

    if (status == "submitted") {
        phase = "submitted";
        completed = expected;
        total = expected;
        eta = 0;
    } else {
        const bool submitting = std::ranges::any_of(
            batches,
            [](const Object& b)
            {
                const auto value = field_string(&b, "status");
                return !value.empty() && value != "pending";
            }
        );
        phase = submitting ? "submitting" : "preparing";
        std::int64_t prepared = 0;
        for (const auto& b: batches) {
            const auto rc = field_number(&b, "recipient_count");
            prepared += rc;
            const auto bs = field_string(&b, "status");
            if (phase == "preparing" || bs == "submitted") {
                std::int64_t done = rc;
                if (phase == "submitting") {
                    const auto accounted = field_number(&b, "accounted_recipient_count");
                    if (accounted != 0) {
                        done = accounted;
                    }
                }
                completed += done;
            }
        }
        total = phase == "preparing" ? std::max(expected, prepared) : prepared;

        if (status == "failed") {
            phase = "failed";
            failed_during = submitting ? "submitting" : "preparing";
            retryable = std::ranges::none_of(
                batches,
                [](const Object& b)
                {
                    return field_string(&b, "status") == "submitting";
                }
            );
        } else {
            eta = total - completed <= 0
                      ? std::optional<std::int64_t>{0}
                      : std::nullopt; // no timing samples in the context-local store
        }
    }

    std::string out = "{\"email_statuses\":[{\"id\":" + json_quote(id) +
                      ",\"sending\":{\"status\":" + json_quote(phase) +
                      ",\"progress\":{\"completed\":" + std::to_string(completed) +
                      ",\"total\":" + std::to_string(total) + ",\"estimated_seconds_remaining\":" +
                      (eta ? std::to_string(*eta) : std::string{"null"}) + "}";
    if (phase == "failed") {
        out += ",\"failed_during\":" + json_quote(failed_during);
        out += ",\"retryable\":" + std::string{retryable ? "true" : "false"};
    }
    out += "}}]}";
    return out;
}

// ---------- analytics helpers ----------

[[nodiscard]] inline std::int64_t iso_to_seconds(std::string_view s)
{
    if (s.size() < 19) {
        return 0;
    }
    const auto digits = [&](std::size_t at, std::size_t n)
    {
        int v = 0;
        for (std::size_t i = 0; i < n && at + i < s.size(); ++i) {
            if (s[at + i] < '0' || s[at + i] > '9') {
                return 0;
            }
            v = v * 10 + (s[at + i] - '0');
        }
        return v;
    };
    const int y = digits(0, 4), mo = digits(5, 2), d = digits(8, 2);
    const int h = digits(11, 2), mi = digits(14, 2), se = digits(17, 2);
    const int yy = y - (mo <= 2 ? 1 : 0);
    const int era = (yy >= 0 ? yy : yy - 399) / 400;
    const unsigned yoe = static_cast<unsigned>(yy - era * 400);
    const unsigned doy = (153u * static_cast<unsigned>(mo + (mo > 2 ? -3 : 9)) + 2) / 5 +
                         static_cast<unsigned>(d) - 1;
    const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    const std::int64_t days =
        static_cast<std::int64_t>(era) * 146'097 + static_cast<std::int64_t>(doe) - 719'468;
    return days * 86'400 + h * 3'600 + mi * 60 + se;
}

/** @brief EmailAnalyticsService.getStatus(): lagSeconds for the three lag-bearing jobs. */
[[nodiscard]] inline std::string patch_lag(std::string_view analytics_json)
{
    const auto now = std::chrono::duration_cast<std::chrono::seconds>(
                         std::chrono::system_clock::now().time_since_epoch()
    )
                         .count();
    const auto parsed = core_serde::JsonCodec{}.decode<serde::Value>(analytics_json);
    if (!parsed) {
        return std::string{analytics_json};
    }
    const auto* root = std::get_if<Object>(&parsed->get());
    if (root == nullptr) {
        return std::string{analytics_json};
    }

    Object patched;
    for (const auto& [name, value]: *root) {
        if (name != "latest" && name != "latestOpened" && name != "missing") {
            patched[name] = value;
            continue;
        }
        const auto* job = std::get_if<Object>(&value.get());
        if (job == nullptr) {
            patched[name] = value;
            continue;
        }
        Object new_job;
        std::string fetched_through;
        for (const auto& [key, field_value]: *job) {
            if (key == "fetchedThrough") {
                if (const auto* text = std::get_if<std::string>(&field_value.get())) {
                    fetched_through = *text;
                }
            }
            new_job[key] = field_value;
        }
        if (!fetched_through.empty() && fetched_through != "null") {
            const auto ts = iso_to_seconds(fetched_through);
            const auto lag = ts == 0 ? std::int64_t{0} : std::max<std::int64_t>(0, now - ts);
            new_job["lagSeconds"] = serde::Value{lag};
        }
        patched[name] = serde::Value{std::move(new_job)};
    }
    return core_serde::JsonCodec{}.encode(serde::Value{std::move(patched)});
}

// ---------------------------------------------------------------------------
// EmailsOperations — context-local analytics job metadata (cache + outbox).
// ---------------------------------------------------------------------------

class EmailsOperations
{
public:
    explicit EmailsOperations(core::cache::Cache& cache) noexcept :
        m_cache{cache}
    {
    }

    [[nodiscard]] std::pair<int, std::string> emails_analytics_get()
    {
        return {200, patch_lag(blog::handlers::middleware::encode(load_analytics(m_cache)))};
    }

    /** @brief scheduleAnalytics: default end = min(now-1h, created_at+7d);
     *  persist job metadata in the cache and emit an outbox event. Void response.
     */
    [[nodiscard]] std::pair<int, std::string> emails_analytics_put(
        std::string_view id,
        std::string_view begin,
        std::string_view end,
        std::string_view created_at
    )
    {
        const auto analytics_json = blog::handlers::middleware::encode(load_analytics(m_cache));
        const auto parsed = core_serde::JsonCodec{}.decode<serde::Value>(analytics_json);
        if (parsed) {
            if (const auto* root = std::get_if<Object>(&parsed->get())) {
                if (const auto* scheduled = field(root, "scheduled")) {
                    if (const auto* so = std::get_if<Object>(&scheduled->get())) {
                        if (const auto* running = field(so, "running")) {
                            if (const auto* flag = std::get_if<bool>(&running->get())) {
                                if (*flag) {
                                    return {
                                        400,
                                        blog::api::ApiError{}.json(
                                            "Already fetching scheduled events. Wait for it to "
                                            "finish before scheduling a new one.",
                                            "Validation Error"
                                        )
                                    };
                                }
                            }
                        }
                    }
                }
            }
        }

        const std::string resolved_begin =
            begin.empty() ? std::string{created_at} : std::string{begin};
        std::string resolved_end{end};
        if (resolved_end.empty()) {
            const auto begin_seconds = iso_to_seconds(resolved_begin);
            const auto now = std::chrono::duration_cast<std::chrono::seconds>(
                                 std::chrono::system_clock::now().time_since_epoch()
            )
                                 .count();
            const auto plus_7d = begin_seconds == 0 ? now : begin_seconds + 7 * 86'400;
            const auto default_end = std::min(now - 3'600, plus_7d);
            resolved_end = std::format(
                "{:%Y-%m-%dT%H:%M:%S.000Z}",
                std::chrono::system_clock::time_point{std::chrono::seconds{default_end}}
            );
        }

        m_cache.put(
            kScheduleKey,
            core::cache::to_cache_value(
                std::format(
                    R"({{"id":{},"begin":{},"end":{}}})",
                    json_quote(id),
                    json_quote(resolved_begin),
                    json_quote(resolved_end)
                )
            ),
            core::cache::Ttl{}.never(),
            {"emails"}
        );

        blog::handlers::outbox::global_outbox().emit(
            blog::handlers::outbox::Event{
                .type = "email-analytics.scheduled",
                .payload = std::format(
                    R"({{"id":{},"begin":{},"end":{}}})",
                    json_quote(id),
                    json_quote(resolved_begin),
                    json_quote(resolved_end)
                ),
                .aggregate_id = std::string{id}
            }
        );
        // Ghost returns void from scheduleAnalytics -> empty body, no email edit.
        return {200, ""};
    }

    /** @brief cancelScheduledAnalytics: clear job metadata + outbox event. Void response. */
    [[nodiscard]] std::pair<int, std::string> emails_analytics_delete()
    {
        m_cache.erase(kAnalyticsKey);
        m_cache.erase(kScheduleKey);
        blog::handlers::outbox::global_outbox().emit(
            blog::handlers::outbox::Event{
                .type = "email-analytics.cancelled",
                .payload = R"({"jobName":"email-analytics-scheduled"})",
                .aggregate_id = "email-analytics-scheduled"
            }
        );
        return {200, ""};
    }

    /** @brief Kept for callers that still use the fixture helpers; strict routes no
     *  longer call these (empty store now yields an empty collection, total 0). */
    [[nodiscard]] std::pair<int, std::string> emails_batches()
    {
        return {
            200,
            "{\"batches\":[" + core_serde::JsonCodec{}.encode(blog::model::emails::BatchWire{}) +
                "],\"meta\":{\"pagination\":" + pagination(1, 15, 1) + "}}"
        };
    }

    [[nodiscard]] std::pair<int, std::string> emails_failures()
    {
        return {
            200,
            "{\"failures\":[" + core_serde::JsonCodec{}.encode(blog::model::emails::FailureWire{}) +
                "],\"meta\":{\"pagination\":" + pagination(1, 15, 1) + "}}"
        };
    }

private:
    static constexpr std::string_view kAnalyticsKey{"emails:analytics"};
    static constexpr std::string_view kScheduleKey{"emails:analytics:schedule"};

    [[nodiscard]] static std::string quote(std::string_view text)
    {
        return json_quote(text);
    }

    [[nodiscard]] static blog::dto::emails::AnalyticsStatus
    load_analytics(core::cache::Cache& cache)
    {
        const auto stored = core::cache::value_as<std::string>(cache.get(kAnalyticsKey).value);
        if (auto analytics =
                blog::handlers::middleware::decode<blog::dto::emails::AnalyticsStatus>(stored)) {
            return *analytics;
        }
        blog::dto::emails::AnalyticsStatus analytics;
        analytics.latest.jobName = "email-analytics-latest-others";
        analytics.latestOpened.jobName = "email-analytics-latest-opened";
        analytics.missing.jobName = "email-analytics-missing";
        analytics.scheduled.jobName = "email-analytics-scheduled";
        return analytics;
    }

    static void
    store_analytics(core::cache::Cache& cache, const blog::dto::emails::AnalyticsStatus& analytics)
    {
        cache.put(
            kAnalyticsKey,
            core::cache::to_cache_value(blog::handlers::middleware::encode(analytics)),
            core::cache::Ttl{}.never(),
            {"emails"}
        );
    }

    core::cache::Cache& m_cache;
};

} // namespace blog::api::emails
