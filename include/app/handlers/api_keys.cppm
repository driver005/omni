module;
export module app_handlers:api_keys;

import std;
import interfaces;
import core_router;
import core_serde;
import utils_openapi;
import app_model;
import app_dto;
import app_context;
import core_cache;
import core_error;
import app_auth;
import app_store;
import app_http;
import app_crypto;
import app_metrics;
import app_workflow;
import app_services;

export namespace app::handlers::api_keys {

class ApiKeysRouteHandlers
{
public:
    void handle_api_key_create(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize_operation(req, "api_key_create")) {
            send_error(res, send, *auth_error);
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, make_error(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::dto::api_keys::CreateApiKeyRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, make_error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }
        if (parsed->get_name().empty()) {
            send_error(res, send, make_error(core::error::AppErrorKind::VALIDATION_FAILED, "name is required"));
            return;
        }

        const std::string merchant_id = merchant_from_request(req);
        const std::string key_id = m_context->ids().api_key_id();
        const app::crypto::SecretString plaintext{
            std::string{"sk_"} + m_context->ids().uuid_v4()
        };
        const std::string hashed = app::crypto::KeyedHasher{configured_hash_key()}
                                       .hash_with_domain(
                                           app::crypto::HashDomain::ApiKey, plaintext.value());
        const std::string prefix = plaintext.value().substr(0, 12);
        const std::int64_t created_epoch = now_epoch();
        const std::string created_at = iso_utc(created_epoch);

        app::store::api_keys::ApiKeyRecord record;
        record.set_key_id(key_id);
        record.set_merchant_id(merchant_id);
        record.set_name(parsed->get_name());
        record.set_description(parsed->get_description().value_or(""));
        record.set_prefix(prefix);
        record.set_api_key(hashed);
        record.set_expiration(expiration_string(parsed->get_expiration()));
        record.set_created_at(created_at);
        record.set_revoked(false);
        auto inserted = m_context->repository().create_api_key(std::move(record));
        if (!inserted) {
            send_error(res, send, inserted.error());
            return;
        }
        const app::store::api_keys::ApiKeyRecord saved = std::move(*inserted);

        const std::string expires_at =
            compute_expires_at(parsed->get_expiration(), created_epoch);
        app::store::api_key_expiries::ApiKeyExpiryRecord expiry;
        expiry.set_key_id(saved.get_key_id());
        expiry.set_merchant_id(saved.get_merchant_id());
        expiry.set_expiration(saved.get_expiration());
        expiry.set_expires_at(expires_at);
        expiry.set_revoked(false);
        expiry.set_created_at(created_at);
        auto expiry_inserted = m_context->repository().create_api_key_expiry(std::move(expiry));
        if (!expiry_inserted) {
            send_error(res, send, expiry_inserted.error());
            return;
        }

        sync_decision_cache(saved.get_api_key(), true);
        if (!expires_at.empty()) {
            schedule_expiry_task(saved.get_key_id(), saved.get_merchant_id(), expires_at);
        }

        m_context->metrics().record_flow("ApiKeyCreate");
        m_context->metrics().counter("API_KEY_CREATED").increment();

        app::dto::api_keys::CreateApiKeyResponse response;
        app::dto::api_keys::ApiKeyIdValue response_key_id;
        response_key_id.set_value(saved.get_key_id());
        response.set_key_id(std::move(response_key_id));
        app::dto::api_keys::MerchantIdValue response_merchant_id;
        response_merchant_id.set_value(saved.get_merchant_id());
        response.set_merchant_id(std::move(response_merchant_id));
        response.set_name(saved.get_name());
        response.set_description(
            saved.get_description().empty()
                ? std::nullopt
                : std::optional<std::string>{saved.get_description()}
        );
        response.set_api_key(plaintext.value());
        app::dto::api_keys::PrimitiveDateTimeValue created;
        created.set_value(saved.get_created_at());
        response.set_created(std::move(created));
        response.set_expiration(parsed->get_expiration());
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void handle_api_key_retrieve(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize_operation(req, "api_key_retrieve")) {
            send_error(res, send, *auth_error);
            return;
        }

        const std::string merchant_id = merchant_from_request(req);
        const std::string key_id = key_id_from_request(req);
        if (key_id.empty()) {
            send_error(res, send, make_error(core::error::AppErrorKind::INVALID_REQUEST, "missing key id"));
            return;
        }
        const auto found = find_api_key(merchant_id, key_id);
        if (!found) {
            send_error(res, send, make_error(core::error::AppErrorKind::NOT_FOUND, "api key not found"));
            return;
        }

        m_context->metrics().record_flow("ApiKeyRetrieve");
        m_context->metrics().counter("API_KEY_RETRIEVED").increment();

        auto text = serde::Ser::encode_json(to_retrieve_response(*found));
        send_json(res, send, text, true);
    }

    void handle_api_key_update(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize_operation(req, "api_key_update")) {
            send_error(res, send, *auth_error);
            return;
        }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, make_error(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        using Input = app::dto::api_keys::UpdateApiKeyRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, make_error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        const std::string merchant_id = merchant_from_request(req);
        const std::string key_id = key_id_from_request(req);
        if (key_id.empty()) {
            send_error(res, send, make_error(core::error::AppErrorKind::INVALID_REQUEST, "missing key id"));
            return;
        }
        const auto found = find_api_key(merchant_id, key_id);
        if (!found) {
            send_error(res, send, make_error(core::error::AppErrorKind::NOT_FOUND, "api key not found"));
            return;
        }

        app::store::api_keys::ApiKeyRecord record = *found;
        if (parsed->get_name()) {
            record.set_name(*parsed->get_name());
        }
        if (parsed->get_description()) {
            record.set_description(*parsed->get_description());
        }
        const bool expiration_changed = parsed->get_expiration().has_value();
        if (expiration_changed) {
            record.set_expiration(expiration_string(*parsed->get_expiration()));
        }
        record.set_key_id(key_id);
        record.set_merchant_id(merchant_id);
        if (!m_context->repository().update_api_key(record)) {
            send_error(res, send, make_error(core::error::AppErrorKind::INTERNAL_ERROR, "api key update failed"));
            return;
        }
        const app::store::api_keys::ApiKeyRecord saved = record;

        auto repository = m_context->repository();
        const std::string expires_at = compute_expires_at(
            expiration_changed ? *parsed->get_expiration() : expiration_enum(saved.get_expiration()),
            now_epoch());
        auto expiry = repository.find_api_key_expiry_by_string(key_id);
        if (!expiry) {
            app::store::api_key_expiries::ApiKeyExpiryRecord fresh;
            fresh.set_key_id(key_id);
            fresh.set_merchant_id(merchant_id);
            fresh.set_expiration(saved.get_expiration());
            fresh.set_expires_at(expires_at);
            fresh.set_revoked(false);
            fresh.set_created_at(saved.get_created_at());
            repository.save_api_key_expiry(std::move(fresh));
        } else {
            expiry->set_expiration(saved.get_expiration());
            expiry->set_expires_at(expires_at);
            repository.update_api_key_expiry(*expiry);
        }

        if (expiration_changed) {
            finalize_expiry_task(key_id, "superseded");
            if (!expires_at.empty()) {
                schedule_expiry_task(key_id, merchant_id, expires_at);
            }
        }

        sync_decision_cache(saved.get_api_key(), true);

        m_context->metrics().record_flow("ApiKeyUpdate");
        m_context->metrics().counter("API_KEY_UPDATED").increment();

        auto text = serde::Ser::encode_json(to_retrieve_response(saved));
        send_json(res, send, text, true);
    }

    void handle_api_key_revoke(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize_operation(req, "api_key_revoke")) {
            send_error(res, send, *auth_error);
            return;
        }

        const std::string merchant_id = merchant_from_request(req);
        const std::string key_id = key_id_from_request(req);
        if (key_id.empty()) {
            send_error(res, send, make_error(core::error::AppErrorKind::INVALID_REQUEST, "missing key id"));
            return;
        }
        const auto found = find_api_key(merchant_id, key_id);
        if (!found) {
            send_error(res, send, make_error(core::error::AppErrorKind::NOT_FOUND, "api key not found"));
            return;
        }

        app::store::api_keys::ApiKeyRecord record = *found;
        record.set_revoked(true);
        if (!m_context->repository().update_api_key(record)) {
            send_error(res, send, make_error(core::error::AppErrorKind::INTERNAL_ERROR, "api key revoke failed"));
            return;
        }
        const app::store::api_keys::ApiKeyRecord saved = record;

        auto repository = m_context->repository();
        if (auto expiry = repository.find_api_key_expiry_by_string(key_id)) {
            expiry->set_revoked(true);
            expiry->set_expires_at({});
            repository.update_api_key_expiry(*expiry);
        }

        finalize_expiry_task(key_id, "revoked");
        sync_decision_cache(saved.get_api_key(), false);

        m_context->metrics().record_flow("ApiKeyRevoke");
        m_context->metrics().counter("API_KEY_REVOKED").increment();

        app::dto::api_keys::RevokeApiKeyResponse response;
        app::dto::api_keys::MerchantIdValue response_merchant_id;
        response_merchant_id.set_value(saved.get_merchant_id());
        response.set_merchant_id(std::move(response_merchant_id));
        app::dto::api_keys::ApiKeyIdValue response_key_id;
        response_key_id.set_value(saved.get_key_id());
        response.set_key_id(std::move(response_key_id));
        response.set_revoked(saved.get_revoked());
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
    }

    void handle_api_key_list(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        if (const auto auth_error = authorize_operation(req, "api_key_list")) {
            send_error(res, send, *auth_error);
            return;
        }

        const std::string merchant_id = merchant_from_request(req);
        const auto query = app::http::RequestQuery{}(req);

        app::dto::api_keys::ListApiKeyConstraints constraints;
        if (const auto raw = query.get("limit")) {
            std::int64_t limit{};
            const auto converted = std::from_chars(raw->data(), raw->data() + raw->size(), limit);
            if (converted.ec == std::errc{} && converted.ptr == raw->data() + raw->size() && limit >= 0) {
                constraints.set_limit(limit);
            }
        }
        if (const auto raw = query.get("skip")) {
            std::int64_t skip{};
            const auto converted = std::from_chars(raw->data(), raw->data() + raw->size(), skip);
            if (converted.ec == std::errc{} && converted.ptr == raw->data() + raw->size() && skip >= 0) {
                constraints.set_skip(skip);
            }
        }

        // AppRepository has no merchant-scoped query with limit/offset, so the
        // merchant filter and pagination are applied deterministically here.
        std::vector<app::store::api_keys::ApiKeyRecord> scoped;
        for (const auto& record : m_context->repository().list_api_keys()) {
            if (record.get_merchant_id() == merchant_id) {
                scoped.push_back(record);
            }
        }

        const std::size_t offset =
            constraints.get_skip().has_value()
                ? static_cast<std::size_t>(*constraints.get_skip())
                : 0U;
        const std::size_t limit =
            constraints.get_limit().has_value()
                ? static_cast<std::size_t>(*constraints.get_limit())
                : scoped.size();
        const std::size_t stop = std::min(offset + limit, scoped.size());

        std::vector<app::dto::api_keys::RetrieveApiKeyResponse> responses;
        for (std::size_t i = offset; i < stop; ++i) {
            responses.push_back(to_retrieve_response(scoped[i]));
        }

        m_context->metrics().record_flow("ApiKeyList");
        m_context->metrics().counter("API_KEY_LISTED").increment();

        auto text = serde::Ser::encode_json(responses);
        send_json(res, send, text, true);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/api_keys"}
            .add_route(
                Route{"/:merchant_id"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_api_key_create(req, res, std::move(send));
                    })
                    .summary("Hyperswitch api_key_create")
            )
            .add_route(
                Route{"/:merchant_id/list"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_api_key_list(req, res, std::move(send));
                    })
                    .summary("Hyperswitch api_key_list")
            )
            .add_route(
                Route{"/:merchant_id/:key_id"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_api_key_retrieve(req, res, std::move(send));
                    })
                    .summary("Hyperswitch api_key_retrieve")
            )
            .add_route(
                Route{"/:merchant_id/:key_id"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_api_key_update(req, res, std::move(send));
                    })
                    .summary("Hyperswitch api_key_update")
            )
            .add_route(
                Route{"/:merchant_id/:key_id"}
                    .delt([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_api_key_revoke(req, res, std::move(send));
                    })
                    .summary("Hyperswitch api_key_revoke")
            );

        Router{router, "/v2/api-keys"}
            .add_route(
                Route{"/"}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_api_key_create(req, res, std::move(send));
                    })
                    .summary("Hyperswitch api_key_create (v2)")
            );
        Router{router, "/v2/api-keys"}
            .add_route(
                Route{"/list"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_api_key_list(req, res, std::move(send));
                    })
                    .summary("Hyperswitch api_key_list (v2)")
            );
        Router{router, "/v2/api-keys"}
            .add_route(
                Route{"/:key_id"}
                    .get([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_api_key_retrieve(req, res, std::move(send));
                    })
                    .summary("Hyperswitch api_key_retrieve (v2)")
            );
        Router{router, "/v2/api-keys"}
            .add_route(
                Route{"/:key_id"}
                    .put([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_api_key_update(req, res, std::move(send));
                    })
                    .summary("Hyperswitch api_key_update (v2)")
            );
        Router{router, "/v2/api-keys"}
            .add_route(
                Route{"/:key_id"}
                    .delt([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_api_key_revoke(req, res, std::move(send));
                    })
                    .summary("Hyperswitch api_key_revoke (v2)")
            );
    }

    explicit ApiKeysRouteHandlers(app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

private:
    [[nodiscard]] core::error::AppError make_error(
        core::error::AppErrorKind kind, std::string message
    ) const
    {
        return core::error::AppErrorBuilder{}(kind, std::move(message));
    }

    [[nodiscard]] bool is_v2(interfaces::io::IRequest& req) const
    {
        return app::http::RequestPath{}(req).at(0).value_or("") == "v2";
    }

    [[nodiscard]] std::string merchant_from_request(interfaces::io::IRequest& req) const
    {
        if (is_v2(req)) {
            return app::http::merchant_id_from_header{}(req);
        }
        return std::string{app::http::RequestPath{}(req).at(1).value_or("")};
    }

    [[nodiscard]] std::string key_id_from_request(interfaces::io::IRequest& req) const
    {
        const auto segments = app::http::RequestPath{}(req);
        if (segments.count() >= 3) {
            return std::string{segments.at(2).value_or("")};
        }
        return {};
    }

    /** @brief Auth rules come from RouteAuthRegistry, one auditable row per operation. */
    [[nodiscard]] std::optional<core::error::AppError> authorize_operation(
        interfaces::io::IRequest& req, std::string_view operation
    ) const
    {
        const auto rule = m_context->auth_rules().rule_for(operation);
        return m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
    }

    [[nodiscard]] std::int64_t now_epoch() const
    {
        return std::chrono::duration_cast<std::chrono::seconds>(
                   std::chrono::system_clock::now().time_since_epoch()
        ).count();
    }

    /** @brief Civil date from days since 1970-01-01; deterministic UTC formatting. */
    [[nodiscard]] std::string iso_utc(std::int64_t epoch_seconds) const
    {
        constexpr std::int64_t seconds_per_day{86400};
        std::int64_t days = epoch_seconds / seconds_per_day;
        std::int64_t seconds_of_day = epoch_seconds % seconds_per_day;
        if (seconds_of_day < 0) {
            seconds_of_day += seconds_per_day;
            --days;
        }
        days += 719468;
        const std::int64_t era = (days >= 0 ? days : days - 146096) / 146097;
        const std::int64_t doe = days - era * 146097;
        const std::int64_t yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
        std::int64_t year = yoe + era * 400;
        const std::int64_t doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
        const std::int64_t mp = (5 * doy + 2) / 153;
        const std::int64_t day = doy - (153 * mp + 2) / 5 + 1;
        const std::int64_t month = mp < 10 ? mp + 3 : mp - 9;
        year += (month <= 2) ? 1 : 0;
        return std::format(
            "{:04d}-{:02d}-{:02d}T{:02d}:{:02d}:{:02d}Z",
            year, month, day,
            seconds_of_day / 3600,
            (seconds_of_day % 3600) / 60,
            seconds_of_day % 60
        );
    }

    [[nodiscard]] std::string configured_hash_key() const
    {
        const auto resolved = m_context->secret_resolver().resolve("api_key_hash", "default");
        if (resolved && !resolved->empty()) {
            return *resolved;
        }
        return std::string{"hyperswitch-api-key-hash-key"};
    }

    [[nodiscard]] std::string expiration_string(
        app::dto::api_keys::ApiKeyExpiration expiration
    ) const
    {
        return expiration == app::dto::api_keys::ApiKeyExpiration::Never ? "never" : "datetime";
    }

    [[nodiscard]] app::dto::api_keys::ApiKeyExpiration expiration_enum(
        std::string_view value
    ) const
    {
        return value == "never"
            ? app::dto::api_keys::ApiKeyExpiration::Never
            : app::dto::api_keys::ApiKeyExpiration::DateTime;
    }

    /** @brief Concrete expiry timestamp from the configured reminder horizon. */
    [[nodiscard]] std::string compute_expires_at(
        app::dto::api_keys::ApiKeyExpiration expiration,
        std::int64_t created_epoch
    ) const
    {
        if (expiration == app::dto::api_keys::ApiKeyExpiration::Never) {
            return {};
        }
        std::int64_t days{90};
        const auto configured = m_context->config_store().get_value(
            "api_key_expiry", "reminder_days"
        );
        if (configured) {
            std::int64_t parsed{};
            const auto converted = std::from_chars(
                configured->data(), configured->data() + configured->size(), parsed
            );
            if (converted.ec == std::errc{} &&
                converted.ptr == configured->data() + configured->size() && parsed > 0) {
                days = parsed;
            }
        }
        return iso_utc(created_epoch + days * 86400);
    }

    /** @brief Deterministic substitute for the upstream email-expiry tracked job. */
    void schedule_expiry_task(
        std::string_view key_id,
        std::string_view merchant_id,
        std::string_view expires_at
    ) const
    {
        if (expires_at.empty()) {
            return;
        }
        const std::string payload = std::format(
            "{{\"key_id\":\"{}\",\"merchant_id\":\"{}\"}}", key_id, merchant_id
        );
        const std::int64_t tracker_id = m_context->process_trackers().schedule(
            "API_KEY_EXPIRY_REMINDER", payload, std::string{expires_at}
        );
        m_context->config_store().put("api_key_expiry", key_id, std::to_string(tracker_id));
        m_context->metrics().counter("TASKS_ADDED_COUNT").increment();
    }

    void finalize_expiry_task(std::string_view key_id, std::string_view result) const
    {
        const auto stored = m_context->config_store().get_value("api_key_expiry", key_id);
        if (!stored) {
            return;
        }
        std::int64_t tracker_id{};
        const auto converted = std::from_chars(
            stored->data(), stored->data() + stored->size(), tracker_id
        );
        if (converted.ec != std::errc{} ||
            converted.ptr != stored->data() + stored->size()) {
            return;
        }
        (void)m_context->process_trackers().mark_terminal(tracker_id, std::string{result});
        m_context->config_store().remove("api_key_expiry", key_id);
        m_context->metrics().counter("API_KEY_EXPIRY_TASKS_CLOSED").increment();
    }

    [[nodiscard]] std::optional<app::store::api_keys::ApiKeyRecord> find_api_key(
        const std::string& merchant_id,
        const std::string& key_id
    ) const
    {
        if (key_id.empty()) {
            return std::nullopt;
        }
        auto repository = m_context->repository();
        if (!merchant_id.empty()) {
            return repository.find_api_key_by_merchant_id_and_key_id(merchant_id, key_id);
        }
        return repository.find_api_key_by_string(key_id);
    }

    void sync_decision_cache(std::string_view hashed_api_key, bool present) const
    {
        const std::string key{std::string{"api_keys.decision."} + std::string{hashed_api_key}};
        if (present) {
            m_context->cache().put(key, core::cache::to_cache_value(std::string{"allowed"}));
        } else {
            (void)m_context->cache().erase(key);
        }
    }

    [[nodiscard]] app::dto::api_keys::RetrieveApiKeyResponse to_retrieve_response(
        const app::store::api_keys::ApiKeyRecord& record
    ) const
    {
        app::dto::api_keys::RetrieveApiKeyResponse response;
        app::dto::api_keys::ApiKeyIdValue key_id;
        key_id.set_value(record.get_key_id());
        response.set_key_id(std::move(key_id));
        app::dto::api_keys::MerchantIdValue merchant_id;
        merchant_id.set_value(record.get_merchant_id());
        response.set_merchant_id(std::move(merchant_id));
        response.set_name(record.get_name());
        response.set_description(
            record.get_description().empty()
                ? std::nullopt
                : std::optional<std::string>{record.get_description()}
        );
        response.set_prefix(record.get_prefix());
        app::dto::api_keys::PrimitiveDateTimeValue created;
        created.set_value(record.get_created_at());
        response.set_created(std::move(created));
        response.set_expiration(expiration_enum(record.get_expiration()));
        return response;
    }

    void send_error(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        core::error::AppError error
    ) const
    {
        auto text = serde::Ser::encode_json(error);
        auto bytes = serde::Ser::serialize_raw("application/json", text);
        res.set_status(core::error::AppErrorStatus{}(error.get_kind()));
        res.set_body(std::move(bytes));
        send();
    }

    void send_json(
        interfaces::io::IResponse& res,
        std::function<void()> send,
        std::string_view text,
        bool ok
    ) const
    {
        auto bytes = serde::Ser::serialize_raw("application/json", text);
        res.set_status(ok ? interfaces::io::types::Status::OK
                          : interfaces::io::types::Status::BAD_REQUEST);
        res.set_body(std::move(bytes));
        send();
    }

    app::context::AppContext* m_context{nullptr};
};

} // namespace app::handlers::api_keys
