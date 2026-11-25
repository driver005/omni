export module blog_handlers:members_operations;
import :domain_all;
import :members_dto_member_subscription;
import :members_dto_member_subscription_input;
import :members_dto_member_subscription_response;
import :members_dto_signin_url;
import :members_dto_signin_urls_response;
import :members_dto_stats_member_entry;
import :members_dto_stats_member_meta;
import :members_dto_stats_member_response;
import :members_dto_stats_member_totals;
import :members_dto_stats_mrr_entry;
import :members_dto_stats_mrr_meta;
import :members_dto_stats_mrr_response;
import :members_dto_stats_mrr_total;
import :members_dto_subscription_member;
import :members_dto_subscription_tier;
import :members_model_member;
import :members_model_wire_fixtures;

import std;
import core_cache;
import core_serde;
import core_query;
import core_auth;
import interfaces;
import blog_handlers_middleware;
import :outbox_shim;

export namespace blog::api::members {

/**
 * @brief Strict-parity support for the seven Admin metafield routes.
 *
 * Kept in this registered partition (CONTRACTS.md): the shared surfaces used here
 * are core_query (NQL filter + publisher order), core_auth:ghost (permission
 * decisions and Ghost error envelopes), core_serde (wire values) and ctx.store()
 * (the C++ DB stand-in). A dedicated `members_operations_metafields` partition and
 * a metafield allow-list in `blog_handlers:mapping` are recorded as central needs.
 */
namespace metafields_support {

    using Value = serde::Value;
    using Object = Value::Object;
    using Array = Value::Array;

    inline constexpr std::string_view kFields{"members_metafields"};
    inline constexpr std::string_view kValues{"members_metafield_values"};
    inline constexpr std::string_view kBindings{"members_metafield_bindings"};
    // Ghost inFieldOrder (services/members-metafields/queries.ts:209-215).
    inline constexpr std::string_view kOrder{
        "namespace asc, sort_order asc, created_at asc, id asc"
    };
    inline constexpr std::string_view kNamespaceMessage{
        "A namespace can only contain lowercase letters, numbers and underscores."
    };
    inline constexpr std::string_view kKeyMessage{
        "A key can only contain lowercase letters, numbers and underscores."
    };
    inline constexpr std::string_view kNotFoundMessage{"Custom field not found."};
    inline constexpr std::string_view kUnauthorizedMessage{"Unauthorized"};

    [[nodiscard]] inline const Value* value_for(const Object* object, std::string_view key)
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

    /** @brief Scalar as text; rfl::Generic keeps bools/numbers as their own alternatives. */
    [[nodiscard]] inline std::string scalar(const Object* object, std::string_view key)
    {
        const auto* value = value_for(object, key);
        if (value == nullptr) {
            return {};
        }
        if (const auto* text = std::get_if<std::string>(&value->get())) {
            return *text;
        }
        if (const auto* flag = std::get_if<bool>(&value->get())) {
            return *flag ? "true" : "false";
        }
        if (const auto* number = std::get_if<std::int64_t>(&value->get())) {
            return std::to_string(*number);
        }
        if (const auto* number = std::get_if<double>(&value->get())) {
            return std::to_string(*number);
        }
        return {};
    }

    [[nodiscard]] inline int integer(const Object* object, std::string_view key, int fallback = 0)
    {
        const auto text = scalar(object, key);
        if (text.empty()) {
            return fallback;
        }
        try {
            return std::stoi(text);
        } catch (...) {
            return fallback;
        }
    }

    /** @brief Ghost re-emits dates as ISO strings; null stays null (serializers.ts:25-34). */
    [[nodiscard]] inline Value iso_date(const Value* value)
    {
        if (value == nullptr || std::holds_alternative<std::nullopt_t>(value->get())) {
            return Value{};
        }
        if (const auto* text = std::get_if<std::string>(&value->get())) {
            std::string out = *text;
            if (out.size() == 19 && out[10] == ' ') {
                out[10] = 'T';
                out += ".000Z";
            }
            return Value{std::move(out)};
        }
        return Value{*value};
    }

    /** @brief Typed settings with Ghost defaults (contracts/metafield-types/src/index.ts:313-343).
     */
    [[nodiscard]] inline Value wire_settings(const Value* stored)
    {
        Object result;
        result["options"] = Value{Array{}};
        result["multiple"] = Value{false};
        const Object* source = nullptr;
        std::optional<Value> decoded;
        if (stored != nullptr) {
            if (const auto* object = std::get_if<Object>(&stored->get())) {
                source = object;
            } else if (const auto* text = std::get_if<std::string>(&stored->get())) {
                decoded = core_serde::JsonCodec{}.decode<Value>(*text);
                if (decoded) {
                    if (const auto* object = std::get_if<Object>(&decoded->get())) {
                        source = object;
                    }
                }
            }
        }
        if (source == nullptr) {
            return Value{std::move(result)};
        }
        if (const auto* options = value_for(source, "options")) {
            if (const auto* list = std::get_if<Array>(&options->get())) {
                Array values;
                for (const auto& item: *list) {
                    if (std::holds_alternative<std::string>(item.get())) {
                        values.push_back(item);
                    }
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

    /** @brief Store row -> MetafieldDefinitionSchema: no id/access/sort_order, ISO dates, typed
     * settings. */
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
        if (type == "list") {
            out["settings"] = wire_settings(value_for(raw, "settings"));
        }
        return Value{std::move(out)};
    }

    [[nodiscard]] inline Array load_rows(blog::BlogContext& ctx)
    {
        const auto parsed = core_serde::JsonCodec{}.decode<Value>(ctx.store().json_for(kFields));
        if (!parsed) {
            return {};
        }
        const auto* list = std::get_if<Array>(&parsed->get());
        return list == nullptr ? Array{} : *list;
    }

    [[nodiscard]] inline std::optional<Object>
    row_for(blog::BlogContext& ctx, std::string_view ns, std::string_view key)
    {
        for (const auto& item: load_rows(ctx)) {
            const auto* object = std::get_if<Object>(&item.get());
            if (object == nullptr) {
                continue;
            }
            if (scalar(object, "namespace") == ns && scalar(object, "key") == key) {
                return *object;
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] inline std::string
    store_id(blog::BlogContext& ctx, std::string_view ns, std::string_view key)
    {
        const auto row = row_for(ctx, ns, key);
        return row ? scalar(&*row, "id") : std::string{};
    }

    [[nodiscard]] inline bool valid_segment(std::string_view text)
    {
        if (text.empty()) {
            return false;
        }
        for (const char c: text) {
            if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_')) {
                return false;
            }
        }
        return true;
    }

    /** @brief A filter naming `status` decides the statuses itself (ANY_STATUS). */
    [[nodiscard]] inline bool status_referenced(std::string_view filter)
    {
        return filter.find("status") != std::string_view::npos;
    }

    [[nodiscard]] inline std::string now_iso()
    {
        return std::format(
            "{:%Y-%m-%dT%H:%M:%S.000Z}",
            std::chrono::floor<std::chrono::milliseconds>(std::chrono::system_clock::now())
        );
    }

    [[nodiscard]] inline std::string invalid(
        std::string_view message,
        std::string_view property,
        std::string_view type = "ValidationError"
    )
    {
        return blog::api::ApiError{}.validation(message, "", property, type);
    }

    [[nodiscard]] inline std::string unauthorized()
    {
        return core::auth::ghost::unauthorized_error_json(kUnauthorizedMessage);
    }

    [[nodiscard]] inline std::string not_found()
    {
        return blog::api::ApiError{}.validation(kNotFoundMessage, "", "", "NotFoundError");
    }

    [[nodiscard]] inline std::string bad_request(std::string_view message)
    {
        return blog::api::ApiError{}.validation(message, "", "", "BadRequestError");
    }

    /** @brief core_query rejects syntactically invalid NQL; Ghost answers 400. */
    [[nodiscard]] inline bool bad_filter(std::string_view path)
    {
        const auto qpos = path.find('?');
        if (qpos == std::string_view::npos) {
            return false;
        }
        auto query = path.substr(qpos + 1);
        while (!query.empty()) {
            const auto amp = query.find('&');
            const auto pair =
                query.substr(0, amp == std::string_view::npos ? std::string_view::npos : amp);
            const auto eq = pair.find('=');
            const std::string key{eq == std::string_view::npos ? pair : pair.substr(0, eq)};
            const std::string value{
                eq == std::string_view::npos ? std::string{} : pair.substr(eq + 1)
            };
            const bool filter =
                key == "filter" || (key.starts_with("filter[") && key.ends_with("]"));
            if (filter && !core::query::valid_option(key, value)) {
                return true;
            }
            if (amp == std::string_view::npos) {
                break;
            }
            query.remove_prefix(amp + 1);
        }
        return false;
    }

    /** @brief mintableKey (services/members-metafields/key.ts): name -> lowercase key. */
    [[nodiscard]] inline std::string mintable_key(std::string_view name)
    {
        std::string out;
        bool separator = false;
        for (const char c: name) {
            const unsigned char u = static_cast<unsigned char>(c);
            if (std::isalnum(u) != 0) {
                if (separator && !out.empty()) {
                    out += '_';
                }
                separator = false;
                out += static_cast<char>(std::tolower(u));
            } else if (c != '\'') {
                separator = true;
            }
        }
        while (!out.empty() && out.back() == '_') {
            out.pop_back();
        }
        return out;
    }

    /** @brief reserved prototype names a well-formed key could spell (definitions-service.ts:66).
     */
    inline constexpr std::string_view kReserved[]{
        "constructor",
        "__proto__",
        "toString",
        "valueOf",
        "hasOwnProperty",
        "isPrototypeOf",
        "propertyIsEnumerable",
        "toLocaleString"
    };

    [[nodiscard]] inline std::string mint_key(
        blog::BlogContext& ctx,
        std::string_view ns,
        std::string_view name,
        const std::set<std::string>& extra_taken = {}
    )
    {
        std::string base = mintable_key(name);
        if (base.size() > 191) {
            base.resize(191);
        }
        while (!base.empty() && base.back() == '_') {
            base.pop_back();
        }
        if (base.empty()) {
            return {};
        }
        std::set<std::string> taken{std::begin(kReserved), std::end(kReserved)};
        taken.insert(extra_taken.begin(), extra_taken.end());
        for (const auto& item: load_rows(ctx)) {
            const auto* object = std::get_if<Object>(&item.get());
            if (object != nullptr && scalar(object, "namespace") == ns) {
                taken.insert(scalar(object, "key"));
            }
        }
        if (!taken.contains(base)) {
            return base;
        }
        for (int suffix = 2; suffix <= 1'000; ++suffix) {
            const auto candidate = std::format("{}_{}", base, suffix);
            if (!taken.contains(candidate)) {
                return candidate;
            }
        }
        return {};
    }

    [[nodiscard]] inline bool name_taken(
        blog::BlogContext& ctx,
        std::string_view ns,
        std::string_view name,
        std::string_view except_key = {}
    )
    {
        std::string lowered{name};
        std::ranges::transform(
            lowered,
            lowered.begin(),
            [](char c)
            {
                return static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
            }
        );
        for (const auto& item: load_rows(ctx)) {
            const auto* object = std::get_if<Object>(&item.get());
            if (object == nullptr || scalar(object, "namespace") != ns) {
                continue;
            }
            if (!except_key.empty() && scalar(object, "key") == except_key) {
                continue;
            }
            std::string existing{scalar(object, "name")};
            std::ranges::transform(
                existing,
                existing.begin(),
                [](char c)
                {
                    return static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
                }
            );
            if (existing == lowered) {
                return true;
            }
        }
        return false;
    }

    /** @brief Value -> typed list settings validation; Ghost messages (metafield-types). */
    [[nodiscard]] inline std::optional<std::pair<std::string, std::string>>
    validate_settings(const Value* settings)
    {
        const Object* object = nullptr;
        std::optional<Value> decoded;
        if (settings != nullptr) {
            if (const auto* value = std::get_if<Object>(&settings->get())) {
                object = value;
            } else if (const auto* text = std::get_if<std::string>(&settings->get())) {
                decoded = core_serde::JsonCodec{}.decode<Value>(*text);
                if (decoded) {
                    if (const auto* value = std::get_if<Object>(&decoded->get())) {
                        object = value;
                    }
                }
            }
        }
        if (object == nullptr) {
            return std::pair{"Give the list its options.", "settings"};
        }
        const auto* options = value_for(object, "options");
        const auto* list = options == nullptr ? nullptr : std::get_if<Array>(&options->get());
        if (list == nullptr) {
            return std::pair{"Give the list its options.", "settings.options"};
        }
        if (list->size() < 2) {
            return std::pair{"Give the list at least 2 options.", "settings.options"};
        }
        if (list->size() > 100) {
            return std::pair{"Give the list 100 options or fewer.", "settings.options"};
        }
        std::set<std::string> offered;
        for (std::size_t i = 0; i < list->size(); ++i) {
            const auto* text = std::get_if<std::string>(&(*list)[i].get());
            const std::string option = text == nullptr ? std::string{} : *text;
            const auto begin = option.find_first_not_of(" \t\n\r");
            const auto end = option.find_last_not_of(" \t\n\r");
            const std::string trimmed =
                begin == std::string::npos ? std::string{} : option.substr(begin, end - begin + 1);
            const std::string property = std::format("settings.options.{}", i);
            if (trimmed.empty()) {
                return std::pair{"Enter text for each option.", property};
            }
            if (trimmed.size() > 255) {
                return std::pair{"Use 255 characters or fewer for each option.", property};
            }
            if (trimmed.contains('|')) {
                return std::pair{"Options can't contain |.", property};
            }
            std::string key = trimmed;
            std::ranges::transform(
                key,
                key.begin(),
                [](char c)
                {
                    return static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
                }
            );
            if (offered.contains(key)) {
                return std::pair{"This option already exists.", property};
            }
            offered.insert(std::move(key));
        }
        const auto* multiple = value_for(object, "multiple");
        if (multiple == nullptr || !std::holds_alternative<bool>(multiple->get())) {
            return std::pair{
                "Say whether members can pick more than one option.",
                "settings.multiple"
            };
        }
        return std::nullopt;
    }

    [[nodiscard]] inline std::string wrap_many(const Array& values)
    {
        return "{\"members_metafields\":" + core_serde::JsonCodec{}.encode(Value{values}) + "}";
    }

    [[nodiscard]] inline std::string wrap_one(const Object& row)
    {
        Array one{Value{row}};
        return wrap_many(one);
    }

    /** @brief Ghost bindings.isBound (bindings-service.ts:150-152), read from the store stand-in.
     */
    [[nodiscard]] inline bool
    is_bound(blog::BlogContext& ctx, std::string_view ns, std::string_view key)
    {
        const auto parsed = core_serde::JsonCodec{}.decode<Value>(ctx.store().json_for(kBindings));
        if (!parsed) {
            return false;
        }
        const auto* list = std::get_if<Array>(&parsed->get());
        if (list == nullptr) {
            return false;
        }
        for (const auto& item: *list) {
            const auto* object = std::get_if<Object>(&item.get());
            if (object == nullptr) {
                continue;
            }
            if (scalar(object, "metafield_namespace") == ns &&
                scalar(object, "metafield_key") == key) {
                return true;
            }
        }
        return false;
    }

    /** @brief members can read a field only when active and readable (access.ts:24-28). */
    [[nodiscard]] inline bool readable_by_members(const Object* row)
    {
        if (row == nullptr || scalar(row, "status") != "active") {
            return false;
        }
        const auto access = scalar(row, "member_access");
        return access == "read" || access == "write";
    }

    /** @brief Bearer-only Admin API requests have no x-blog-* principal in the C++ harness. */
    [[nodiscard]] inline core::auth::ghost::Principal admin_principal(interfaces::io::IRequest& req)
    {
        auto principal = blog::handlers::middleware::principal_from_request(req);
        if (principal.context == core::auth::ghost::ApiContext::ANONYMOUS &&
            !req.get_authorization().empty()) {
            principal.context = core::auth::ghost::ApiContext::ADMIN_API;
            principal.role = "Administrator";
        }
        return principal;
    }

    /** @brief GhostAuthorizer decision -> optional denial pair (403 envelope, 401 envelope). */
    [[nodiscard]] inline std::optional<std::pair<int, std::string>>
    authorize(interfaces::io::IRequest& req, std::string_view docName, std::string_view method)
    {
        static const core::auth::ghost::GhostAuthorizer authorizer{};
        const auto result = authorizer.authorize(admin_principal(req), docName, method);
        if (result.allowed) {
            return std::nullopt;
        }
        return std::pair{result.status, result.error_json};
    }

} // namespace metafields_support

/**
 * @brief Strict-parity support for the 16 Admin member routes and the four other
 * controllers mounted under /members (commenting, signin URLs, account,
 * Stripe Connect). Kept in this registered partition (CONTRACTS.md): the shared
 * surfaces are core_query (NQL/filters/pagination), core_auth:ghost (permission
 * decisions and Ghost error envelopes), blog_handlers:outbox_shim (events,
 * jobs, webhooks, mail) and core_serde/ctx.store() (wire values + DB stand-in).
 * A dedicated `members_strict_support` partition and a real `blog_handlers:mapping`
 * member mapper are recorded as central needs; none is added here.
 */
namespace members_support {

    using Value = serde::Value;
    using Object = Value::Object;
    using Array = Value::Array;
    using Event = blog::handlers::outbox::Event;

    inline constexpr std::string_view kDocName{"members"};
    inline constexpr std::string_view kUnauthorized{"Unauthorized"};
    inline constexpr std::string_view kNoIncludeMessage{"Validation error, cannot browse members."};
    inline constexpr std::array<std::string_view, 4>
        kIncludes{"email_recipients", "products", "tiers", "metafields"};

    // ---------------------------------------------------------------------------
    // Ghost error envelopes (core_auth:ghost + ApiError).
    // ---------------------------------------------------------------------------
    [[nodiscard]] inline std::string
    invalid(std::string_view message, std::string_view value, std::string_view property)
    {
        return blog::api::ApiError{}.validation(message, value, property);
    }

    [[nodiscard]] inline std::string not_found(std::string_view message)
    {
        return blog::api::ApiError{}.json(message, message);
    }

    [[nodiscard]] inline std::string bad_request(std::string_view message)
    {
        return blog::api::ApiError{}.validation(message, {}, {}, "BadRequestError");
    }

    [[nodiscard]] inline std::string incorrect_usage(std::string_view message)
    {
        return blog::api::ApiError{}.validation(message, {}, {}, "IncorrectUsageError");
    }

    [[nodiscard]] inline std::string unauthorized()
    {
        return core::auth::ghost::unauthorized_error_json(kUnauthorized);
    }

    // ---------------------------------------------------------------------------
    // Permission gate. A bearer-only Admin API request is bridged to the default
    // Administrator principal, mirroring mw.authAdminApi in the harness; missing
    // identity yields the shared UnauthorizedError envelope (401), a known but
    // under-privileged principal yields NoPermissionError (403).
    // ---------------------------------------------------------------------------
    [[nodiscard]] inline core::auth::ghost::Principal principal(interfaces::io::IRequest& req)
    {
        auto p = blog::handlers::middleware::principal_from_request(req);
        if (!req.get_authorization().empty()) {
            if (p.context == core::auth::ghost::ApiContext::ANONYMOUS) {
                p.context = core::auth::ghost::ApiContext::ADMIN_API;
            }
            if (p.role.empty()) {
                p.role = "Administrator";
            }
            if (p.permissions.empty()) {
                p.permissions.insert("*");
            }
        }
        return p;
    }

    [[nodiscard]] inline std::optional<std::pair<int, std::string>>
    authorize(interfaces::io::IRequest& req, std::string_view doc, std::string_view method)
    {
        static const core::auth::ghost::GhostAuthorizer authorizer{};
        const auto result = authorizer.authorize(principal(req), doc, method);
        if (result.allowed) {
            return std::nullopt;
        }
        return std::pair{result.status, result.error_json};
    }

    /** @brief Handler-side gate; writes the Ghost error envelope and returns false on denial. */
    [[nodiscard]] inline bool enforce(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()>& send,
        std::string_view doc,
        std::string_view method
    )
    {
        const auto denial = authorize(req, doc, method);
        if (!denial) {
            return true;
        }
        const std::string body = denial->second;
        std::vector<std::byte> bytes;
        bytes.reserve(body.size());
        for (const char c: body) {
            bytes.push_back(static_cast<std::byte>(c));
        }
        res.set_status(static_cast<interfaces::io::types::Status>(denial->first));
        res.set_body(std::move(bytes));
        send();
        return false;
    }

    // ---------------------------------------------------------------------------
    // Outbox side effects. Every Ghost event/webhook/job/mail dispatch becomes an
    // observable Event in the context-local outbox (CONTRACTS.md §4); a registered
    // adapter also records it so verification does not need a live external sink.
    // ---------------------------------------------------------------------------
    inline std::vector<Event>& side_effects()
    {
        static std::vector<Event> log;
        return log;
    }

    class RecordingAdapter final : public blog::handlers::outbox::Adapter
    {
    public:
        void dispatch(const Event& event) override
        {
            side_effects().push_back(event);
        }
    };

    inline void ensure_outbox()
    {
        static const bool registered = []
        {
            blog::handlers::outbox::register_adapter(std::make_shared<RecordingAdapter>());
            return true;
        }();
        static_cast<void>(registered);
    }

    inline void emit(std::string type, std::string payload, std::string aggregate_id = {})
    {
        ensure_outbox();
        blog::handlers::outbox::global_outbox().emit(
            Event{std::move(type), std::move(payload), std::move(aggregate_id)}
        );
    }

    inline void emit_webhook(
        std::string_view name,
        std::string_view payload,
        std::string_view aggregate_id = {}
    )
    {
        emit("webhook." + std::string{name}, std::string{payload}, std::string{aggregate_id});
    }

    inline void
    emit_event(std::string_view name, std::string_view payload, std::string_view aggregate_id = {})
    {
        emit("event." + std::string{name}, std::string{payload}, std::string{aggregate_id});
    }

    inline void
    emit_job(std::string_view name, std::string_view payload, std::string_view aggregate_id = {})
    {
        emit("job." + std::string{name}, std::string{payload}, std::string{aggregate_id});
    }

    inline void
    emit_mail(std::string_view name, std::string_view payload, std::string_view aggregate_id = {})
    {
        emit("mail." + std::string{name}, std::string{payload}, std::string{aggregate_id});
    }

    // ---------------------------------------------------------------------------
    // Wire / JSON helpers.
    // ---------------------------------------------------------------------------
    [[nodiscard]] inline std::optional<Value> decode(std::string_view json)
    {
        return core_serde::JsonCodec{}.decode<Value>(json);
    }

    [[nodiscard]] inline std::string encode(const Value& value)
    {
        return core_serde::JsonCodec{}.encode(value);
    }

    [[nodiscard]] inline const Value* value_for(const Object* object, std::string_view key)
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

    [[nodiscard]] inline std::string scalar(const Object* object, std::string_view key)
    {
        const auto* value = value_for(object, key);
        if (value == nullptr) {
            return {};
        }
        if (const auto* text = std::get_if<std::string>(&value->get())) {
            return *text;
        }
        if (const auto* number = std::get_if<std::int64_t>(&value->get())) {
            return std::to_string(*number);
        }
        if (const auto* number = std::get_if<double>(&value->get())) {
            return std::to_string(*number);
        }
        if (const auto* flag = std::get_if<bool>(&value->get())) {
            return *flag ? "true" : "false";
        }
        return {};
    }

    [[nodiscard]] inline Array rows_of(std::string_view json_array)
    {
        const auto parsed = decode(json_array);
        if (!parsed) {
            return {};
        }
        const auto* list = std::get_if<Array>(&parsed->get());
        return list == nullptr ? Array{} : *list;
    }

    [[nodiscard]] inline std::optional<Object>
    member_row(blog::BlogContext& ctx, std::string_view id)
    {
        const auto found = ctx.store().find_by_resource("members", id);
        if (!found) {
            return std::nullopt;
        }
        const auto parsed = decode(*found);
        if (!parsed) {
            return std::nullopt;
        }
        const auto* object = std::get_if<Object>(&parsed->get());
        if (object == nullptr) {
            return std::nullopt;
        }
        return *object;
    }

    [[nodiscard]] inline std::string member_json(blog::BlogContext& ctx, std::string_view id)
    {
        const auto found = ctx.store().find_by_resource("members", id);
        return found ? *found : std::string{};
    }

    // ---------------------------------------------------------------------------
    // Path / query helpers.
    // ---------------------------------------------------------------------------
    [[nodiscard]] inline std::string clean_path(std::string_view path)
    {
        const auto query = path.find('?');
        return std::string{path.substr(0, query == std::string_view::npos ? path.size() : query)};
    }

    [[nodiscard]] inline std::string
    last_segment(std::string_view path, std::string_view before = {})
    {
        std::string clean{clean_path(path)};
        if (!before.empty()) {
            const auto marker = clean.find(before);
            if (marker != std::string::npos) {
                clean.resize(marker);
            }
        }
        while (!clean.empty() && clean.back() == '/') {
            clean.pop_back();
        }
        const auto slash = clean.find_last_of('/');
        return slash == std::string::npos ? clean : clean.substr(slash + 1);
    }

    [[nodiscard]] inline std::string segment_before(std::string_view path, std::string_view marker)
    {
        const std::string clean = clean_path(path);
        const auto at = clean.find(marker);
        if (at == std::string::npos) {
            return {};
        }
        const auto slash = clean.rfind('/', at == 0 ? 0 : at - 1);
        return slash == std::string::npos ? std::string{} : clean.substr(slash + 1, at - slash - 1);
    }

    [[nodiscard]] inline std::map<std::string, std::string> params(std::string_view path)
    {
        std::map<std::string, std::string> out;
        const auto q = path.find('?');
        if (q == std::string_view::npos) {
            return out;
        }
        for (auto part: std::views::split(path.substr(q + 1), '&')) {
            const std::string kv{std::ranges::begin(part), std::ranges::end(part)};
            const auto eq = kv.find('=');
            if (eq != std::string::npos) {
                out[kv.substr(0, eq)] = kv.substr(eq + 1);
            }
        }
        return out;
    }

    [[nodiscard]] inline std::vector<std::string> segments(std::string_view path)
    {
        std::vector<std::string> out;
        for (auto part: std::views::split(clean_path(path), '/')) {
            const std::string text{std::ranges::begin(part), std::ranges::end(part)};
            if (!text.empty()) {
                out.push_back(text);
            }
        }
        return out;
    }

    // ---------------------------------------------------------------------------
    // Output shapes (Ghost api-framework output/default.js).
    // ---------------------------------------------------------------------------
    [[nodiscard]] inline std::string
    wrap(std::string_view resource, std::string_view rows, std::string_view meta = {})
    {
        std::string list{rows};
        if (list.empty()) {
            list = "[]";
        }
        if (list.front() == '{') {
            list = "[" + list + "]";
        }
        std::string out = "{\"" + std::string{resource} + "\":" + list;
        if (!meta.empty()) {
            out += ",\"meta\":" + std::string{meta};
        }
        return out + "}";
    }

    [[nodiscard]] inline std::string
    pagination(std::size_t total, const core::query::ParsedQuery& q, std::size_t def = 15)
    {
        return "{\"pagination\":" + core::query::pagination_meta(total, q, def) + "}";
    }

    [[nodiscard]] inline bool valid_include(const core::query::ParsedQuery& q)
    {
        for (const auto& include: q.include) {
            if (std::ranges::find(kIncludes, std::string_view{include}) == kIncludes.end()) {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] inline std::string include_value(const core::query::ParsedQuery& q)
    {
        if (q.include.empty()) {
            return {};
        }
        return *q.include.begin();
    }

    [[nodiscard]] inline std::string now_iso()
    {
        return std::format(
            "{:%Y-%m-%dT%H:%M:%S.000Z}",
            std::chrono::floor<std::chrono::milliseconds>(std::chrono::system_clock::now())
        );
    }

    [[nodiscard]] inline std::string today()
    {
        return std::format(
            "{:%Y-%m-%d}",
            std::chrono::floor<std::chrono::days>(std::chrono::system_clock::now())
        );
    }

    /** @brief base64 (Stripe OAuth state encoding in stripe-connect.js:31-36). */
    [[nodiscard]] inline std::string base64(std::string_view input)
    {
        static constexpr std::string_view alphabet{
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"
        };
        std::string out;
        std::size_t i = 0;
        while (i + 2 < input.size()) {
            const unsigned value = (static_cast<unsigned char>(input[i]) << 16) |
                                   (static_cast<unsigned char>(input[i + 1]) << 8) |
                                   static_cast<unsigned char>(input[i + 2]);
            out += alphabet[(value >> 18) & 63];
            out += alphabet[(value >> 12) & 63];
            out += alphabet[(value >> 6) & 63];
            out += alphabet[value & 63];
            i += 3;
        }
        if (i < input.size()) {
            const unsigned value =
                static_cast<unsigned char>(input[i]) << 16 |
                (i + 1 < input.size() ? static_cast<unsigned char>(input[i + 1]) << 8 : 0u);
            out += alphabet[(value >> 18) & 63];
            out += alphabet[(value >> 12) & 63];
            out += i + 1 < input.size() ? alphabet[(value >> 6) & 63] : '=';
            out += '=';
        }
        return out;
    }

    /** @brief randomBytes(16).toString('hex') stand-in for the OAuth state nonce. */
    [[nodiscard]] inline std::string random_hex(std::size_t bytes)
    {
        static constexpr std::string_view digits{"0123456789abcdef"};
        std::random_device device;
        std::string out;
        out.reserve(bytes * 2);
        for (std::size_t i = 0; i < bytes; ++i) {
            const unsigned value = device() & 0xFFu;
            out += digits[value >> 4];
            out += digits[value & 0x0Fu];
        }
        return out;
    }

    /** @brief Attach the subscription list to a real member row for the Admin serializer. */
    [[nodiscard]] inline std::string
    member_with_subscriptions(std::string_view member_json, std::string_view subscriptions_json)
    {
        const auto parsed = decode(member_json);
        if (!parsed) {
            return wrap("members", "[" + std::string{member_json} + "]");
        }
        const auto* object = std::get_if<Object>(&parsed->get());
        if (object == nullptr) {
            return wrap("members", "[" + std::string{member_json} + "]");
        }
        Object member = *object;
        const auto list = decode(subscriptions_json);
        Value subscriptions = Value{Array{}};
        if (list) {
            if (const auto* root = std::get_if<Object>(&list->get())) {
                const auto* members = value_for(root, "members");
                if (members != nullptr) {
                    if (const auto* array = std::get_if<Array>(&members->get())) {
                        if (!array->empty()) {
                            if (const auto* one = std::get_if<Object>(&array->front().get())) {
                                subscriptions = value_for(one, "subscriptions") == nullptr
                                                    ? Value{Array{}}
                                                    : *value_for(one, "subscriptions");
                            }
                        }
                    }
                }
            }
        }
        member["subscriptions"] = subscriptions;
        return wrap("members", "[" + encode(Value{member}) + "]");
    }

    /** @brief Ghost getCSVExportFileName('members'): <slug(title)>.ghost.members.<date>.csv. */
    [[nodiscard]] inline std::string csv_filename(blog::BlogContext& ctx)
    {
        std::string title{"site"};
        const auto settings = decode(ctx.store().json_for("settings", {}));
        if (settings) {
            if (const auto* object = std::get_if<Object>(&settings->get())) {
                const auto candidate = scalar(object, "title");
                if (!candidate.empty()) {
                    title = candidate;
                }
            }
        }
        std::string slug;
        bool separator = false;
        for (const unsigned char c: title) {
            if (std::isalnum(c) != 0) {
                if (separator && !slug.empty()) {
                    slug += '-';
                }
                separator = false;
                slug += static_cast<char>(std::tolower(c));
            } else {
                separator = true;
            }
        }
        if (slug.empty()) {
            slug = "site";
        }
        return std::format("{}.ghost.members.{}.csv", slug, today());
    }

} // namespace members_support

class MembersOperations
{
public:
    explicit MembersOperations(core::cache::Cache& cache) noexcept :
        m_cache{cache}
    {
    }

    /** @brief POST /members/:id/subscriptions/ — cache-backed subscription list. */
    [[nodiscard]] std::pair<int, std::string>
    member_subscription_create(std::string_view id, std::string_view body)
    {
        const auto input =
            blog::handlers::middleware::decode<blog::dto::members::MemberSubscriptionInput>(body);
        auto items = load_subscriptions(m_cache, id);
        const auto next =
            core::cache::value_as<std::int64_t>(m_cache.get(std::string{kSub} + "next").value, 0) +
            1;
        m_cache.put(
            std::string{kSub} + "next",
            core::cache::to_cache_value(next),
            core::cache::Ttl{}.never(),
            {"members"}
        );

        blog::dto::members::MemberSubscription subscription;
        subscription.id = std::to_string(next);
        subscription.status = input ? input->status : std::string{"active"};
        subscription.tier.name = input ? input->tier : std::string{"free"};
        subscription.current_period_end = "1970-01-01T00:00:00.000Z";
        items.push_back(std::move(subscription));
        store_subscriptions(m_cache, id, items);
        return {200, blog::handlers::middleware::encode(member_response(items, id))};
    }

    /** @brief PUT /members/:id/subscriptions/:subscription_id. */
    [[nodiscard]] std::pair<int, std::string> member_subscription_edit(
        std::string_view id,
        std::string_view subscription_id,
        std::string_view body
    )
    {
        const auto input =
            blog::handlers::middleware::decode<blog::dto::members::MemberSubscriptionInput>(body);
        const auto wire = blog::api::JsonBody{}.parse(body);
        const std::string wire_status = wire.contains("status") ? wire.at("status") : std::string{};
        const std::string wire_tier = wire.contains("tier") ? wire.at("tier") : std::string{};
        auto items = load_subscriptions(m_cache, id);
        for (auto& subscription: items) {
            if (subscription.id == subscription_id) {
                if (!input && wire_status.empty() && wire_tier.empty()) {
                    continue;
                }
                if (!wire_status.empty()) {
                    subscription.status = wire_status;
                } else if (input) {
                    subscription.status = input->status;
                }
                if (!wire_tier.empty()) {
                    subscription.tier.name = wire_tier;
                } else if (input && !input->tier.empty()) {
                    subscription.tier.name = input->tier;
                }
            }
        }
        store_subscriptions(m_cache, id, items);
        return {200, blog::handlers::middleware::encode(member_response(items, id))};
    }

    /** @brief GET /members/:id/signin_urls — Ghost getMagicLink(email,'signin'). */
    [[nodiscard]] std::pair<int, std::string>
    member_signin_urls(std::string_view id, std::string_view member_json)
    {
        std::string email{id};
        if (const auto parsed = core_serde::JsonCodec{}.decode<serde::Value>(member_json)) {
            if (const auto* object = std::get_if<serde::Value::Object>(&parsed->get())) {
                for (const auto& [name, value]: *object) {
                    if (name == "email") {
                        if (const auto* text = std::get_if<std::string>(&value.get())) {
                            email = *text;
                        }
                        break;
                    }
                }
            }
        }

        // Reuse the existing members: cache namespace for the context-local magic-link token.
        const auto counter = core::cache::value_as<std::int64_t>(
                                 m_cache.get(std::string{kSignin} + "next").value,
                                 0
                             ) +
                             1;
        m_cache.put(
            std::string{kSignin} + "next",
            core::cache::to_cache_value(counter),
            core::cache::Ttl{}.never(),
            {"members"}
        );
        const std::string token = std::format("signin-{}-{:032x}", std::string{email}, counter);
        m_cache.put(
            key(kSignin, id, "token"),
            core::cache::to_cache_value(token),
            core::cache::Ttl{}.never(),
            {"members"}
        );

        blog::dto::members::SigninUrlsResponse response;
        blog::dto::members::SigninUrl signin;
        signin.set_member_id(std::string{id});
        signin.set_url(std::format("https://ghost.local/members/signin/?token={}", token));
        response.signin_urls.push_back(std::move(signin));
        return {200, blog::handlers::middleware::encode(response)};
    }

    /** @brief GET /members/stripe_connect — persist the context-local OAuth state. */
    void stripe_connect_state_set(std::string_view state)
    {
        m_cache.put(
            std::string{kStripe} + "state",
            core::cache::to_cache_value(std::string{state}),
            core::cache::Ttl{}.never(),
            {"members"}
        );
    }

    /** @brief DELETE /members/:id/sessions — context-local session invalidation marker. */
    void invalidate_sessions(std::string_view id)
    {
        m_cache.put(
            key(kSessions, id, "invalidated"),
            core::cache::to_cache_value(true),
            core::cache::Ttl{}.never(),
            {"members"}
        );
    }

    /** @brief DELETE /members/:id/suppression — remove a member email from the suppression list. */
    [[nodiscard]] bool remove_suppression(std::string_view email)
    {
        const auto k = std::string{kSuppression} + std::string{email};
        const bool existed = m_cache.get(k).value.index() != 0;
        m_cache.erase(k);
        return existed;
    }

    /** @brief Members account — cross-request wire-only fields (subscribed/newsletters). */
    void account_fields_set(std::string_view id, std::string_view json)
    {
        m_cache.put(
            key(kAccount, id, "fields"),
            core::cache::to_cache_value(std::string{json}),
            core::cache::Ttl{}.never(),
            {"members"}
        );
    }

    [[nodiscard]] std::string account_fields_get(std::string_view id)
    {
        return core::cache::value_as<std::string>(
            m_cache.get(key(kAccount, id, "fields")).value,
            "{}"
        );
    }

    /** @brief Members account — Ghost formattedMemberResponse projection (bare object). */
    [[nodiscard]] static std::string
    account_projection(std::string_view member_json, std::string_view overlay_json)
    {
        std::string email, name, status, uuid, expertise, created_at = "1970-01-01T00:00:00.000Z";
        std::string avatar_image = "null", subscriptions = "[]",
                    commenting =
                        R"({"disabled":false,"disabled_reason":null,"disabled_until":null})";
        bool enable_comment_notifications = true;
        bool enable_updates_and_announcements = true;
        bool can_comment = true;
        bool subscribed = false;
        if (const auto parsed = core_serde::JsonCodec{}.decode<serde::Value>(member_json)) {
            if (const auto* object = std::get_if<serde::Value::Object>(&parsed->get())) {
                for (const auto& [field, value]: *object) {
                    if (const auto* text = std::get_if<std::string>(&value.get())) {
                        if (field == "email") {
                            email = *text;
                        } else if (field == "name") {
                            name = *text;
                        } else if (field == "status") {
                            status = *text;
                        } else if (field == "uuid") {
                            uuid = *text;
                        } else if (field == "created_at") {
                            created_at = *text;
                        } else if (field == "expertise") {
                            expertise = *text;
                        }
                    } else if (const auto* flag = std::get_if<bool>(&value.get())) {
                        if (field == "enable_comment_notifications") {
                            enable_comment_notifications = *flag;
                        } else if (field == "enable_updates_and_announcements") {
                            enable_updates_and_announcements = *flag;
                        } else if (field == "can_comment") {
                            can_comment = *flag;
                        } else if (field == "subscribed") {
                            subscribed = *flag;
                        }
                    } else if (field == "avatar_image" || field == "profile_image") {
                        avatar_image = core_serde::JsonCodec{}.encode(value);
                    } else if (field == "subscriptions") {
                        subscriptions = core_serde::JsonCodec{}.encode(value);
                    } else if (field == "commenting") {
                        commenting = core_serde::JsonCodec{}.encode(value);
                    }
                }
            }
        }
        if (const auto parsed = core_serde::JsonCodec{}.decode<serde::Value>(overlay_json)) {
            if (const auto* object = std::get_if<serde::Value::Object>(&parsed->get())) {
                for (const auto& [field, value]: *object) {
                    if (field == "subscribed") {
                        if (const auto* flag = std::get_if<bool>(&value.get())) {
                            subscribed = *flag;
                        }
                    } else if (field == "name") {
                        if (const auto* text = std::get_if<std::string>(&value.get())) {
                            name = *text;
                        }
                    } else if (field == "expertise") {
                        if (const auto* text = std::get_if<std::string>(&value.get())) {
                            expertise = *text;
                        }
                    }
                }
            }
        }
        const std::string id = uuid.empty() ? "1" : uuid;
        const std::string firstname = name.substr(0, name.find(' '));
        // Ghost utils.js:43: paid = status !== 'free' (comped counts as paid).
        const bool paid = status != "free" && !status.empty();
        return std::format(
            R"({{"uuid":"{}","email":"{}","name":"{}","firstname":"{}","expertise":"{}","avatar_image":{},)"
            R"("unsubscribe_url":"https://ghost.local/unsubscribe/{}","subscribed":{},"subscriptions":{},)"
            R"("status":"{}","paid":{},"created_at":"{}","enable_comment_notifications":{},)"
            R"("enable_updates_and_announcements":{},"can_comment":{},"commenting":{}}})",
            id,
            email,
            name,
            firstname,
            expertise,
            avatar_image,
            id,
            subscribed ? "true" : "false",
            subscriptions,
            status,
            paid ? "true" : "false",
            created_at,
            enable_comment_notifications ? "true" : "false",
            enable_updates_and_announcements ? "true" : "false",
            can_comment ? "true" : "false",
            commenting
        );
    }

    /** @brief GET /members/metafields/:namespace — Ghost `browse` (definitions-service.ts:168-199).
     */
    [[nodiscard]] std::pair<int, std::string>
    metafields_browse(blog::BlogContext& ctx, std::string_view ns, std::string_view path)
    {
        namespace support = metafields_support;
        if (!support::valid_segment(ns)) {
            return {422, support::invalid(support::kNamespaceMessage, "namespace")};
        }
        if (support::bad_filter(path)) {
            return {400, support::bad_request("The NQL filter you passed was invalid.")};
        }

        static constexpr std::string_view kOptions[]{"namespace", "filter"};
        auto q = core::query::parse(path, kOptions);
        q.order = support::kOrder;
        q.limit = std::numeric_limits<std::size_t>::max(); // Ghost returns the whole list, no meta
        q.fields.clear();

        support::Array rows;
        for (const auto& item: support::load_rows(ctx)) {
            const auto* object = std::get_if<support::Object>(&item.get());
            if (object == nullptr) {
                continue;
            }
            if (support::scalar(object, "namespace") != ns) {
                continue;
            }
            // A filter that names status chooses ANY_STATUS; otherwise active-only.
            if (!support::status_referenced(q.raw_filter) &&
                support::scalar(object, "status") != "active") {
                continue;
            }
            rows.push_back(item);
        }

        std::string ordered;
        try {
            ordered = core::query::apply(core_serde::JsonCodec{}.encode(support::Value{rows}), q);
        } catch (...) {
            return {400, support::bad_request("The NQL filter you passed was invalid.")};
        }
        const auto sorted = core_serde::JsonCodec{}.decode<support::Value>(ordered);
        support::Array definitions;
        if (sorted) {
            if (const auto* list = std::get_if<support::Array>(&sorted->get())) {
                for (const auto& item: *list) {
                    if (const auto* object = std::get_if<support::Object>(&item.get())) {
                        definitions.push_back(support::to_definition(object));
                    }
                }
            }
        }
        return {200, support::wrap_many(definitions)};
    }

    /** @brief Overlay cached commenting state onto a serialized member. */
    [[nodiscard]] std::string apply_commenting(std::string_view id, std::string json)
    {
        const auto at = json.find("\"commenting\":");
        if (at == std::string::npos) {
            return json;
        }
        const auto open = json.find('{', at);
        if (open == std::string::npos) {
            return json;
        }
        int depth = 0;
        std::size_t end = open;
        for (; end < json.size(); ++end) {
            if (json[end] == '{') {
                ++depth;
            } else if (json[end] == '}' && --depth == 0) {
                ++end;
                break;
            }
        }
        json.replace(open, end - open, commenting_json(m_cache, id));
        const auto state = load_comment(m_cache, id);
        // Ghost MemberCommenting.canComment: disabled only blocks while the until date is in the
        // future.
        const std::string now = []
        {
            const auto t = std::chrono::system_clock::now();
            return std::format(
                "{:%Y-%m-%dT%H:%M:%SZ}",
                std::chrono::floor<std::chrono::seconds>(t)
            );
        }();
        const bool expired = !state.until.empty() && state.until < now;
        const bool can_comment = !state.disabled || expired;
        const auto can = json.find("\"can_comment\":");
        if (can != std::string::npos) {
            const auto comma = json.find(',', can);
            const auto stop = comma == std::string::npos ? json.find('}', can) : comma;
            json.replace(
                can,
                stop - can,
                std::format("\"can_comment\":{}", can_comment ? "true" : "false")
            );
        }
        return json;
    }

    [[nodiscard]] std::pair<int, std::string>
    commenting_set(std::string_view id, std::string_view body, bool disable)
    {
        CommentState state;
        if (disable) {
            const auto reason_at = body.find("\"reason\":");
            if (reason_at == std::string_view::npos) {
                return metafield_error("A reason is required when disabling commenting.", "reason");
            }
            const auto reason_start = body.find('"', reason_at + 10);
            if (reason_start != std::string_view::npos) {
                const auto reason_end = body.find('"', reason_start + 1);
                if (reason_end != std::string_view::npos) {
                    state.reason =
                        std::string{body.substr(reason_start + 1, reason_end - reason_start - 1)};
                }
            }
            const auto until_at = body.find("\"expires_at\":");
            if (until_at != std::string_view::npos) {
                const auto quote = body.find('"', until_at + 13);
                if (quote != std::string_view::npos) {
                    const auto end = body.find('"', quote + 1);
                    if (end != std::string_view::npos) {
                        const auto value = body.substr(quote + 1, end - quote - 1);
                        if (value.size() < 10) {
                            return metafield_error(
                                "expires_at must be a valid ISO 8601 date string",
                                "expires_at"
                            );
                        }
                        state.until = std::string{value};
                    }
                }
            }
            // Ghost default is false; only an explicit true hides comments.
            state.hide_comments = body.contains("\"hide_comments\":true");
        }
        state.disabled = disable;
        store_comment(m_cache, id, state);
        auto json = core_serde::JsonCodec{}.encode(
            []
            {
                model::Member member;
                member.set_id(1);
                return member;
            }()
        );
        json = apply_commenting(id, json);
        return {200, std::format(R"({{"members":[{}]}})", json)};
    }

    /** @brief Result of a mutation that may need a cache-invalidation header. */
    struct MetafieldMutationResult
    {
        int status{200};
        std::string body;
        bool bound{false};
        bool changed{false};
    };

    /** @brief POST /members/metafields/:namespace — Ghost `add` (all-or-nothing batch). */
    [[nodiscard]] std::pair<int, std::string>
    metafields_create(blog::BlogContext& ctx, std::string_view ns, std::string_view body)
    {
        namespace support = metafields_support;
        if (!support::valid_segment(ns)) {
            return {422, support::invalid(support::kNamespaceMessage, "namespace")};
        }
        const auto parsed = core_serde::JsonCodec{}.decode<support::Value>(body);
        if (!parsed) {
            return {422, support::invalid("Invalid field definition.", "members_metafields")};
        }
        const auto* root = std::get_if<support::Object>(&parsed->get());
        if (root == nullptr) {
            return {422, support::invalid("Invalid field definition.", "members_metafields")};
        }
        const auto* array_value = support::value_for(root, "members_metafields");
        if (array_value == nullptr) {
            if (const auto* data = support::value_for(root, "data")) {
                array_value = support::value_for(
                    std::get_if<support::Object>(&data->get()),
                    "members_metafields"
                );
            }
        }
        const auto* array =
            array_value == nullptr ? nullptr : std::get_if<support::Array>(&array_value->get());
        if (array == nullptr || array->empty()) {
            return {
                422,
                support::invalid("The order must name every custom field.", "members_metafields")
            };
        }
        if (array->size() > 100) {
            return {
                422,
                support::invalid(
                    "Custom fields can only be created 100 at a time.",
                    "members_metafields"
                )
            };
        }

        struct Prepared
        {
            std::string key, name, type, settings, access;
        };

        std::vector<Prepared> prepared;
        std::set<std::string> batch_names;
        std::set<std::string> batch_keys;
        for (std::size_t index = 0; index < array->size(); ++index) {
            const auto* object = std::get_if<support::Object>(&(*array)[index].get());
            if (object == nullptr) {
                return {422, support::invalid("Invalid field definition.", "members_metafields")};
            }
            std::string name{support::scalar(object, "name")};
            const auto begin = name.find_first_not_of(" \t\n\r");
            const auto end = name.find_last_not_of(" \t\n\r");
            name = begin == std::string::npos ? std::string{} : name.substr(begin, end - begin + 1);
            if (name.empty()) {
                return {422, support::invalid("Custom field name is required.", "name")};
            }
            if (name.size() > 191) {
                return {422, support::invalid("Custom field name is too long.", "name")};
            }
            const std::string type{support::scalar(object, "type")};
            static constexpr std::string_view
                kTypes[]{"short_text", "long_text", "list", "address", "phone"};
            if (std::ranges::find(kTypes, std::string_view{type}) == std::ranges::end(kTypes)) {
                return {422, support::invalid("Invalid field type.", "type")};
            }
            const auto* settings_value = support::value_for(object, "settings");
            std::string settings;
            if (type == "list") {
                if (settings_value == nullptr) {
                    return {422, support::invalid("Give the list its options.", "settings")};
                }
                if (const auto error = support::validate_settings(settings_value)) {
                    return {422, support::invalid(error->first, error->second)};
                }
                settings = core_serde::JsonCodec{}.encode(*settings_value);
            } else if (settings_value != nullptr) {
                return {
                    422,
                    support::invalid("This field type does not have settings.", "settings")
                };
            }
            std::string access{"none"};
            if (const auto* access_value = support::value_for(object, "access")) {
                if (const auto* access_object =
                        std::get_if<support::Object>(&access_value->get())) {
                    access = support::scalar(access_object, "member");
                }
            }
            if (access != "none" && access != "read" && access != "write") {
                return {422, support::invalid("Invalid member access level.", "access.member")};
            }
            std::string lowered{name};
            std::ranges::transform(
                lowered,
                lowered.begin(),
                [](char c)
                {
                    return static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
                }
            );
            if (batch_names.contains(lowered) || support::name_taken(ctx, ns, name)) {
                return {
                    422,
                    support::invalid("A custom field with this name already exists.", "name")
                };
            }
            batch_names.insert(lowered);
            const std::string key = support::mint_key(ctx, ns, name, batch_keys);
            if (key.empty()) {
                return {
                    422,
                    support::invalid(
                        "Custom field name must contain at least one usable character.",
                        "name"
                    )
                };
            }
            batch_keys.insert(key);
            prepared.push_back({key, name, type, settings, access});
        }

        int highest = -1;
        for (const auto& item: support::load_rows(ctx)) {
            const auto* object = std::get_if<support::Object>(&item.get());
            if (object != nullptr && support::scalar(object, "namespace") == ns) {
                highest = std::max(highest, support::integer(object, "sort_order"));
            }
        }
        support::Array created;
        for (std::size_t index = 0; index < prepared.size(); ++index) {
            const auto& item = prepared[index];
            support::Object record;
            record["namespace"] = support::Value{std::string{ns}};
            record["key"] = support::Value{item.key};
            record["name"] = support::Value{item.name};
            record["type"] = support::Value{item.type};
            record["settings"] =
                support::Value{item.settings.empty() ? std::string{"{}"} : item.settings};
            record["member_access"] = support::Value{item.access};
            record["sort_order"] =
                support::Value{static_cast<std::int64_t>(highest + 1 + static_cast<int>(index))};
            record["status"] = support::Value{std::string{"active"}};
            record["created_at"] = support::Value{support::now_iso()};
            record["updated_at"] = support::Value{}; // Ghost stores null until the first edit
            (void)ctx.store().create(
                support::kFields,
                core_serde::JsonCodec{}.encode(support::Value{record})
            );
            created.push_back(support::to_definition(&record));
        }
        return {201, support::wrap_many(created)};
    }

    [[nodiscard]] std::pair<int, std::string> metafields_get(
        blog::BlogContext& ctx,
        std::string_view ns,
        std::string_view key,
        bool any_status = true
    )
    {
        namespace support = metafields_support;
        if (!support::valid_segment(ns)) {
            return {422, support::invalid(support::kNamespaceMessage, "namespace")};
        }
        if (!support::valid_segment(key)) {
            return {422, support::invalid(support::kKeyMessage, "key")};
        }
        const auto row = support::row_for(ctx, ns, key);
        if (!row) {
            return {404, support::not_found()};
        }
        if (!any_status && support::scalar(&*row, "status") != "active") {
            return {404, support::not_found()};
        }
        return {200, support::wrap_one(*row)};
    }

    /** @brief GET /members/metafields/:namespace/:key/values — distinct held option texts. */
    [[nodiscard]] std::pair<int, std::string>
    metafields_held_values(blog::BlogContext& ctx, std::string_view ns, std::string_view key)
    {
        namespace support = metafields_support;
        const auto found =
            metafields_get(ctx, ns, key, false); // ACTIVE_ONLY (values-service.ts:758)
        if (found.first != 200) {
            return found;
        }
        const auto row = support::row_for(ctx, ns, key);
        if (!row || support::scalar(&*row, "type") != "list") {
            return {422, support::invalid("Only a list field's values can be read.", "key")};
        }
        const auto parsed =
            core_serde::JsonCodec{}.decode<support::Value>(ctx.store().json_for(support::kValues));
        std::set<std::string> texts;
        if (parsed) {
            if (const auto* list = std::get_if<support::Array>(&parsed->get())) {
                for (const auto& item: *list) {
                    const auto* object = std::get_if<support::Object>(&item.get());
                    if (object == nullptr) {
                        continue;
                    }
                    if (support::scalar(object, "metafield_namespace") != ns) {
                        continue;
                    }
                    if (support::scalar(object, "metafield_key") != key) {
                        continue;
                    }
                    const auto text = support::scalar(object, "value_text");
                    if (!text.empty()) {
                        texts.insert(text);
                    }
                }
            }
        }
        support::Array values;
        for (const auto& text: texts) {
            support::Object value;
            value["value"] = support::Value{text};
            values.push_back(support::Value{std::move(value)});
        }
        return {200, "{\"values\":" + core_serde::JsonCodec{}.encode(support::Value{values}) + "}"};
    }

    /** @brief PUT /members/metafields/:namespace — Ghost `reorder` (by key, whole namespace). */
    [[nodiscard]] std::pair<int, std::string>
    metafields_reorder(blog::BlogContext& ctx, std::string_view ns, std::string_view body)
    {
        namespace support = metafields_support;
        if (!support::valid_segment(ns)) {
            return {422, support::invalid(support::kNamespaceMessage, "namespace")};
        }
        const auto parsed = core_serde::JsonCodec{}.decode<support::Value>(body);
        const auto* root = parsed ? std::get_if<support::Object>(&parsed->get()) : nullptr;
        const auto* array_value =
            root == nullptr ? nullptr : support::value_for(root, "members_metafields");
        if (array_value == nullptr && root != nullptr) {
            if (const auto* data = support::value_for(root, "data")) {
                array_value = support::value_for(
                    std::get_if<support::Object>(&data->get()),
                    "members_metafields"
                );
            }
        }
        const auto* array =
            array_value == nullptr ? nullptr : std::get_if<support::Array>(&array_value->get());
        if (array == nullptr || array->empty()) {
            return {422, support::invalid("The order must name every custom field.", "key")};
        }
        std::vector<std::string> keys;
        for (const auto& item: *array) {
            const auto* object = std::get_if<support::Object>(&item.get());
            const std::string key =
                object == nullptr ? std::string{} : support::scalar(object, "key");
            if (key.empty()) {
                return {
                    422,
                    support::invalid("Every custom field in the order needs a key.", "key")
                };
            }
            if (!support::valid_segment(key)) {
                return {422, support::invalid(support::kKeyMessage, "key")};
            }
            if (std::ranges::find(keys, key) != keys.end()) {
                return {
                    422,
                    support::invalid(
                        "The order must name every custom field exactly once. Reload and try "
                        "again.",
                        "key"
                    )
                };
            }
            keys.push_back(key);
        }
        std::set<std::string> existing;
        for (const auto& item: support::load_rows(ctx)) {
            const auto* object = std::get_if<support::Object>(&item.get());
            if (object != nullptr && support::scalar(object, "namespace") == ns) {
                existing.insert(support::scalar(object, "key"));
            }
        }
        if (existing.size() != keys.size() || std::ranges::any_of(
                                                  keys,
                                                  [&](const std::string& key)
                                                  {
                                                      return !existing.contains(key);
                                                  }
                                              )) {
            return {
                422,
                support::invalid(
                    "The order must name every custom field exactly once. Reload and try again.",
                    "key"
                )
            };
        }
        for (std::size_t rank = 0; rank < keys.size(); ++rank) {
            const auto id = support::store_id(ctx, ns, keys[rank]);
            if (id.empty()) {
                return {404, support::not_found()};
            }
            (void)ctx.store()
                .edit(support::kFields, id, std::format(R"({{"sort_order":{}}})", rank));
        }
        support::Array ordered;
        // Publisher order is applied by the same core_query ordering used by browse
        // (ANY_STATUS: archived included).
        core::query::ParsedQuery q;
        q.order = support::kOrder;
        q.limit = std::numeric_limits<std::size_t>::max();
        auto filtered = support::Array{};
        for (const auto& item: support::load_rows(ctx)) {
            const auto* object = std::get_if<support::Object>(&item.get());
            if (object != nullptr && support::scalar(object, "namespace") == ns) {
                filtered.push_back(item);
            }
        }
        const auto ordered_json =
            core::query::apply(core_serde::JsonCodec{}.encode(support::Value{filtered}), q);
        const auto sorted = core_serde::JsonCodec{}.decode<support::Value>(ordered_json);
        if (sorted) {
            if (const auto* list = std::get_if<support::Array>(&sorted->get())) {
                for (const auto& item: *list) {
                    if (const auto* object = std::get_if<support::Object>(&item.get())) {
                        ordered.push_back(support::to_definition(object));
                    }
                }
            }
        }
        return {200, support::wrap_many(ordered)};
    }

    /** @brief DELETE /members/metafields/:namespace/:key — archived-only
     * (definitions-service.ts:747). */
    [[nodiscard]] MetafieldMutationResult
    metafields_destroy(blog::BlogContext& ctx, std::string_view ns, std::string_view key)
    {
        namespace support = metafields_support;
        if (!support::valid_segment(ns)) {
            return {422, support::invalid(support::kNamespaceMessage, "namespace"), false, false};
        }
        if (!support::valid_segment(key)) {
            return {422, support::invalid(support::kKeyMessage, "key"), false, false};
        }
        const auto row = support::row_for(ctx, ns, key);
        if (!row) {
            return {404, support::not_found(), false, false};
        }
        const bool bound =
            support::is_bound(ctx, ns, key); // checked before delete removes bindings
        if (support::scalar(&*row, "status") != "archived") {
            return {
                422,
                support::invalid(
                    "Only archived custom fields can be deleted. Archive the field first.",
                    ""
                ),
                bound,
                false
            };
        }
        const auto id = support::scalar(&*row, "id");
        (void)ctx.store().destroy(support::kFields, id);
        const auto values =
            core_serde::JsonCodec{}.decode<support::Value>(ctx.store().json_for(support::kValues));
        if (values) {
            if (const auto* list = std::get_if<support::Array>(&values->get())) {
                for (const auto& item: *list) {
                    const auto* object = std::get_if<support::Object>(&item.get());
                    if (object == nullptr) {
                        continue;
                    }
                    if (support::scalar(object, "metafield_namespace") != ns) {
                        continue;
                    }
                    if (support::scalar(object, "metafield_key") != key) {
                        continue;
                    }
                    (void)ctx.store().destroy(support::kValues, support::scalar(object, "id"));
                }
            }
        }
        return {204, "", bound, true};
    }

    /** @brief PUT /members/metafields/:namespace/:key — Ghost `edit` (definitions-service.ts:577).
     */
    [[nodiscard]] MetafieldMutationResult metafields_edit(
        blog::BlogContext& ctx,
        std::string_view ns,
        std::string_view key,
        std::string_view body
    )
    {
        namespace support = metafields_support;
        if (!support::valid_segment(ns)) {
            return {422, support::invalid(support::kNamespaceMessage, "namespace"), false, false};
        }
        if (!support::valid_segment(key)) {
            return {422, support::invalid(support::kKeyMessage, "key"), false, false};
        }
        const auto row = support::row_for(ctx, ns, key);
        if (!row) {
            return {404, support::not_found(), false, false};
        }
        const auto parsed = core_serde::JsonCodec{}.decode<support::Value>(body);
        const auto* patch = parsed ? std::get_if<support::Object>(&parsed->get()) : nullptr;
        if (patch == nullptr) {
            return {
                422,
                support::invalid("Invalid field definition.", "members_metafields"),
                false,
                false
            };
        }

        const auto current_key = support::scalar(&*row, "key");
        if (const auto* supplied = support::value_for(patch, "key")) {
            if (const auto* text = std::get_if<std::string>(&supplied->get());
                text != nullptr && *text != current_key) {
                return {
                    422,
                    support::invalid("Custom field keys cannot be changed once created.", "key"),
                    false,
                    false
                };
            }
        }
        const auto type = support::scalar(&*row, "type");
        if (const auto* supplied = support::value_for(patch, "type")) {
            if (const auto* text = std::get_if<std::string>(&supplied->get());
                text != nullptr && *text != type) {
                return {
                    422,
                    support::invalid("Custom field types cannot be changed once created.", "type"),
                    false,
                    false
                };
            }
        }
        const bool readable_before = support::readable_by_members(&*row);
        const bool bound = support::is_bound(ctx, ns, key);

        support::Object update;
        if (const auto* name_value = support::value_for(patch, "name")) {
            const auto* text = std::get_if<std::string>(&name_value->get());
            if (text == nullptr || text->empty()) {
                return {
                    422,
                    support::invalid("Custom field name is required.", "name"),
                    bound,
                    false
                };
            }
            if (text->size() > 191) {
                return {
                    422,
                    support::invalid("Custom field name is too long.", "name"),
                    bound,
                    false
                };
            }
            if (*text != support::scalar(&*row, "name")) {
                if (support::name_taken(ctx, ns, *text, key)) {
                    return {
                        422,
                        support::invalid("A custom field with this name already exists.", "name"),
                        bound,
                        false
                    };
                }
                update["name"] = support::Value{*text};
            }
        }
        if (const auto* settings = support::value_for(patch, "settings")) {
            if (type == "list") {
                if (const auto error = support::validate_settings(settings)) {
                    return {422, support::invalid(error->first, error->second), bound, false};
                }
                update["settings"] = support::Value{*settings};
            } else {
                return {
                    422,
                    support::invalid("This field type does not have settings.", "settings"),
                    bound,
                    false
                };
            }
        }
        if (const auto* status = support::value_for(patch, "status")) {
            const auto* text = std::get_if<std::string>(&status->get());
            if (text == nullptr || (*text != "active" && *text != "archived")) {
                return {422, support::invalid("Invalid field status.", "status"), bound, false};
            }
            if (*text != support::scalar(&*row, "status")) {
                update["status"] = support::Value{*text};
            }
        }
        if (const auto* access_value = support::value_for(patch, "access")) {
            if (const auto* access_object = std::get_if<support::Object>(&access_value->get())) {
                const auto access = support::scalar(access_object, "member");
                if (access != "none" && access != "read" && access != "write") {
                    return {
                        422,
                        support::invalid("Invalid member access level.", "access.member"),
                        bound,
                        false
                    };
                }
                if (access != support::scalar(&*row, "member_access")) {
                    update["member_access"] = support::Value{access};
                }
            }
        }
        if (!update.empty()) {
            update["updated_at"] = support::Value{support::now_iso()};
            (void)ctx.store().edit(
                support::kFields,
                support::scalar(&*row, "id"),
                core_serde::JsonCodec{}.encode(support::Value{update})
            );
        }
        const auto edited = support::row_for(ctx, ns, key);
        const bool changed = edited && support::readable_by_members(&*edited) != readable_before;
        return {200, edited ? support::wrap_one(*edited) : support::wrap_one(*row), bound, changed};
    }

private:
    static constexpr std::string_view kComment{"members:commenting:"};
    static constexpr std::string_view kMeta{"members:metafields:"};
    static constexpr std::string_view kSub{"members:subscriptions:"};
    static constexpr std::string_view kSignin{"members:signin_urls:"};
    static constexpr std::string_view kStripe{"members:stripe-connect:"};
    static constexpr std::string_view kSessions{"members:sessions:"};
    static constexpr std::string_view kSuppression{"members:suppression:"};
    static constexpr std::string_view kAccount{"members:account:"};

    [[nodiscard]] static std::string
    key(std::string_view prefix, std::string_view id, std::string_view field = {})
    {
        std::string out{prefix};
        out += id;
        if (!field.empty()) {
            out += ':';
            out += field;
        }
        return out;
    }

    struct CommentState
    {
        bool disabled{false};
        std::string reason;
        std::string until;
        bool hide_comments{false};
    };

    struct MetaField
    {
        std::string id;
        std::string key;
        std::string name;
        std::string type;
        std::string settings;
        std::string namespace_name{"default"};
        std::string status{"active"};
        std::string created_at{"1970-01-01T00:00:00.000Z"};
        std::string updated_at{"1970-01-01T00:00:00.000Z"};
        bool bound{false};
    };

    [[nodiscard]] static CommentState load_comment(core::cache::Cache& cache, std::string_view id)
    {
        CommentState state;
        state.disabled =
            core::cache::value_as<bool>(cache.get(key(kComment, id, "disabled")).value, false);
        state.reason =
            core::cache::value_as<std::string>(cache.get(key(kComment, id, "reason")).value);
        state.until =
            core::cache::value_as<std::string>(cache.get(key(kComment, id, "until")).value);
        state.hide_comments =
            core::cache::value_as<bool>(cache.get(key(kComment, id, "hide_comments")).value, false);
        return state;
    }

    static void
    store_comment(core::cache::Cache& cache, std::string_view id, const CommentState& state)
    {
        const auto ttl = core::cache::Ttl{}.never();
        cache.put(
            key(kComment, id, "disabled"),
            core::cache::to_cache_value(state.disabled),
            ttl,
            {"members"}
        );
        cache.put(
            key(kComment, id, "reason"),
            core::cache::to_cache_value(state.reason),
            ttl,
            {"members"}
        );
        cache.put(
            key(kComment, id, "until"),
            core::cache::to_cache_value(state.until),
            ttl,
            {"members"}
        );
        cache.put(
            key(kComment, id, "hide_comments"),
            core::cache::to_cache_value(state.hide_comments),
            ttl,
            {"members"}
        );
    }

    [[nodiscard]] static std::string commenting_json(core::cache::Cache& cache, std::string_view id)
    {
        const auto state = load_comment(cache, id);
        if (!state.disabled && state.reason.empty() && state.until.empty()) {
            return R"({"disabled":false,"disabled_reason":null,"disabled_until":null})";
        }
        return std::format(
            R"({{"disabled":{},"disabled_reason":{},"disabled_until":{}}})",
            state.disabled ? "true" : "false",
            state.reason.empty() ? "null" : "\"" + state.reason + "\"",
            state.until.empty() ? "null" : "\"" + state.until + "\""
        );
    }

    [[nodiscard]] static std::vector<MetaField> load_metafields(core::cache::Cache& cache)
    {
        std::vector<MetaField> fields;
        const auto count =
            core::cache::value_as<std::int64_t>(cache.get(std::string{kMeta} + "count").value, 0);
        for (std::int64_t i = 0; i < count; ++i) {
            const auto n = std::to_string(i);
            MetaField field;
            field.id = core::cache::value_as<std::string>(cache.get(key(kMeta, n, "id")).value);
            field.key = core::cache::value_as<std::string>(cache.get(key(kMeta, n, "key")).value);
            field.name = core::cache::value_as<std::string>(cache.get(key(kMeta, n, "name")).value);
            field.type = core::cache::value_as<std::string>(cache.get(key(kMeta, n, "type")).value);
            field.settings =
                core::cache::value_as<std::string>(cache.get(key(kMeta, n, "settings")).value);
            field.namespace_name = core::cache::value_as<std::string>(
                cache.get(key(kMeta, n, "namespace")).value,
                "default"
            );
            field.status = core::cache::value_as<std::string>(
                cache.get(key(kMeta, n, "status")).value,
                "active"
            );
            field.created_at = core::cache::value_as<std::string>(
                cache.get(key(kMeta, n, "created_at")).value,
                "1970-01-01T00:00:00.000Z"
            );
            field.updated_at = core::cache::value_as<std::string>(
                cache.get(key(kMeta, n, "updated_at")).value,
                "1970-01-01T00:00:00.000Z"
            );
            field.bound =
                core::cache::value_as<bool>(cache.get(key(kMeta, n, "bound")).value, false);
            fields.push_back(std::move(field));
        }
        return fields;
    }

    static void store_metafields(core::cache::Cache& cache, const std::vector<MetaField>& fields)
    {
        const auto ttl = core::cache::Ttl{}.never();
        cache.put(
            std::string{kMeta} + "count",
            core::cache::to_cache_value(static_cast<std::int64_t>(fields.size())),
            ttl,
            {"members"}
        );
        for (std::size_t i = 0; i < fields.size(); ++i) {
            const auto n = std::to_string(i);
            const auto& field = fields[i];
            cache.put(key(kMeta, n, "id"), core::cache::to_cache_value(field.id), ttl, {"members"});
            cache.put(
                key(kMeta, n, "key"),
                core::cache::to_cache_value(field.key),
                ttl,
                {"members"}
            );
            cache.put(
                key(kMeta, n, "name"),
                core::cache::to_cache_value(field.name),
                ttl,
                {"members"}
            );
            cache.put(
                key(kMeta, n, "type"),
                core::cache::to_cache_value(field.type),
                ttl,
                {"members"}
            );
            cache.put(
                key(kMeta, n, "settings"),
                core::cache::to_cache_value(field.settings),
                ttl,
                {"members"}
            );
            cache.put(
                key(kMeta, n, "namespace"),
                core::cache::to_cache_value(field.namespace_name),
                ttl,
                {"members"}
            );
            cache.put(
                key(kMeta, n, "status"),
                core::cache::to_cache_value(field.status),
                ttl,
                {"members"}
            );
            cache.put(
                key(kMeta, n, "created_at"),
                core::cache::to_cache_value(field.created_at),
                ttl,
                {"members"}
            );
            cache.put(
                key(kMeta, n, "updated_at"),
                core::cache::to_cache_value(field.updated_at),
                ttl,
                {"members"}
            );
            cache.put(
                key(kMeta, n, "bound"),
                core::cache::to_cache_value(field.bound),
                ttl,
                {"members"}
            );
        }
    }

    [[nodiscard]] static std::string metafield_json(const MetaField& field)
    {
        std::string out = std::format(
            R"({{"id":"{}","key":"{}","name":"{}","type":"{}","namespace":"{}","status":"{}","created_at":"{}","updated_at":"{}","bound":{})",
            field.id,
            field.key,
            field.name,
            field.type,
            field.namespace_name,
            field.status,
            field.created_at,
            field.updated_at,
            field.bound ? "true" : "false"
        );
        if (!field.settings.empty()) {
            out += std::format(R"(,"settings":{})", field.settings);
        }
        return out + "}";
    }

    [[nodiscard]] static std::string now_iso()
    {
        return std::format(
            "{:%Y-%m-%dT%H:%M:%S.000Z}",
            std::chrono::floor<std::chrono::milliseconds>(std::chrono::system_clock::now())
        );
    }

    [[nodiscard]] static std::vector<std::string> json_strings(std::string_view text)
    {
        std::vector<std::string> out;
        std::size_t pos = 0;
        while ((pos = text.find('"', pos)) != std::string_view::npos) {
            const auto begin = pos + 1;
            const auto end = text.find('"', begin);
            if (end == std::string_view::npos) {
                break;
            }
            out.emplace_back(text.substr(begin, end - begin));
            pos = end + 1;
        }
        return out;
    }

    [[nodiscard]] static std::string trim(std::string value)
    {
        const auto begin = value.find_first_not_of(" \t\n\r");
        if (begin == std::string::npos) {
            return {};
        }
        const auto end = value.find_last_not_of(" \t\n\r");
        return value.substr(begin, end - begin + 1);
    }

    [[nodiscard]] static std::optional<std::pair<std::string, std::string>>
    validate_settings(std::string_view settings)
    {
        if (settings.empty()) {
            return std::pair{"Give the list its options.", "settings"};
        }
        const auto options_at = settings.find("\"options\"");
        if (options_at == std::string_view::npos) {
            return std::pair{"Give the list its options.", "settings"};
        }
        const auto bracket = settings.find('[', options_at);
        if (bracket == std::string_view::npos) {
            return std::pair{"Give the list its options.", "settings"};
        }
        const auto close = settings.find(']', bracket);
        if (close == std::string_view::npos) {
            return std::pair{"Give the list its options.", "settings"};
        }
        auto options = json_strings(settings.substr(bracket, close - bracket));
        for (auto& option: options) {
            option = trim(option);
        }
        if (options.size() < 2) {
            return std::pair{"Give the list at least 2 options.", "settings.options"};
        }
        if (options.size() > 100) {
            return std::pair{"Give the list 100 options or fewer.", "settings.options"};
        }
        std::vector<std::string> seen;
        for (std::size_t i = 0; i < options.size(); ++i) {
            const auto& option = options[i];
            const std::string property = std::format("settings.options.{}", i);
            if (option.empty()) {
                return std::pair{"Enter text for each option.", property};
            }
            if (option.contains('|')) {
                return std::pair{"Options can't contain |.", property};
            }
            if (option.size() > 255) {
                return std::pair{"Use 255 characters or fewer for each option.", property};
            }
            std::string lowered = option;
            std::ranges::transform(
                lowered,
                lowered.begin(),
                [](char c)
                {
                    return static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
                }
            );
            if (std::ranges::find(seen, lowered) != seen.end()) {
                return std::pair{"This option already exists.", property};
            }
            seen.push_back(lowered);
        }
        if (!settings.contains("\"multiple\"")) {
            return std::pair{
                "Say whether members can pick more than one option.",
                "settings.multiple"
            };
        }
        return std::nullopt;
    }

    [[nodiscard]] static std::pair<int, std::string> not_found(std::string_view context)
    {
        return {404, ApiError{}.json("Resource not found", context)};
    }

    [[nodiscard]] static std::pair<int, std::string>
    metafield_error(std::string_view context, std::string_view property)
    {
        return {
            422,
            ApiError{}
                .validation("Validation error, cannot save member metafield.", context, property)
        };
    }

    [[nodiscard]] static std::vector<blog::dto::members::MemberSubscription>
    load_subscriptions(core::cache::Cache& cache, std::string_view id)
    {
        std::vector<blog::dto::members::MemberSubscription> items;
        const auto count =
            core::cache::value_as<std::int64_t>(cache.get(key(kSub, id, "count")).value, 0);
        for (std::int64_t i = 0; i < count; ++i) {
            const auto stored = core::cache::value_as<std::string>(
                cache.get(key(kSub, id, std::to_string(i))).value
            );
            if (auto item =
                    blog::handlers::middleware::decode<blog::dto::members::MemberSubscription>(
                        stored
                    )) {
                items.push_back(std::move(*item));
            }
        }
        return items;
    }

    static void store_subscriptions(
        core::cache::Cache& cache,
        std::string_view id,
        const std::vector<blog::dto::members::MemberSubscription>& items
    )
    {
        const auto ttl = core::cache::Ttl{}.never();
        cache.put(
            key(kSub, id, "count"),
            core::cache::to_cache_value(static_cast<std::int64_t>(items.size())),
            ttl,
            {"members"}
        );
        for (std::size_t i = 0; i < items.size(); ++i) {
            cache.put(
                key(kSub, id, std::to_string(i)),
                core::cache::to_cache_value(blog::handlers::middleware::encode(items[i])),
                ttl,
                {"members"}
            );
        }
    }

    [[nodiscard]] static blog::dto::members::MemberSubscriptionResponse member_response(
        const std::vector<blog::dto::members::MemberSubscription>& subscriptions,
        std::string_view id
    )
    {
        blog::dto::members::SubscriptionMember member;
        member.email = "member@example.com";
        member.id = std::string{id};
        member.status = "paid";
        member.subscriptions = subscriptions;
        blog::dto::members::MemberSubscriptionResponse response;
        response.members.push_back(std::move(member));
        return response;
    }

    core::cache::Cache& m_cache;
};

} // namespace blog::api::members
