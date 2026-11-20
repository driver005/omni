module;
export module app_handlers:client_secret;

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
import app_keystore;
import app_locking;
import app_services;

export namespace app::handlers::client_secret {

/**
 * @brief C++ port of the upstream v2 client-secret resource.
 *
 * The two route functions are `routes/client_secret.rs:12-39` (create) and `:43-63`
 * (delete). Their core logic lives in `core/payments/helpers.rs:4694-4782`
 * (`make_client_secret`, `create_client_secret`, `delete_client_secret`) and the Redis
 * hash store in `db/client_secret.rs:49-151`. Every upstream function is ported below as
 * a local helper using the in-process facilities (cache = Redis hash, key store =
 * platform key store, repository = DB).
 */
class ClientSecretRouteHandlers
{
public:
    explicit ClientSecretRouteHandlers(app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

    void handle_client_secret_create(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // api::server_wrap(flow=ClientSecretCreate, auth=V2ApiKeyAuth{false,false},
        // LockAction::NotApplicable) — routes/client_secret.rs:17-37.
        const auto rule = m_context->auth_rules().rule_for("client_secret_create");
        auto auth_ctx = app::auth::AuthContextBuilder{}(req);
        if (auto denied = m_context->authorizer().authorize_route(auth_ctx, rule)) {
            send_error(res, send, *denied);
            return;
        }
        if (auto denied = enforce_v2_api_key_flags(auth_ctx)) {
            send_error(res, send, *denied);
            return;
        }

        m_context->metrics().record_flow("ClientSecretCreate");
        m_context->metrics().counter("client_secret_create").increment();
        // No API lock: upstream declares LockAction::NotApplicable.

        using Input = app::dto::client_secret::ClientSecretCreateRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", req.body_as_string());
        if (!parsed || !parsed->get_resource_id().is_customer()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        // make_client_secret(state, payload.resource_id, auth.platform, req.headers())
        // helpers.rs:4694-4732. Merchant id is the authenticated platform account id
        // (platform.get_processor().get_account().get_id()); header is a local fallback.
        std::string merchant_id = auth_ctx.merchant_id();
        if (merchant_id.empty()) {
            merchant_id = app::http::merchant_id_from_header{}(req);
        }

        using Response = app::dto::client_secret::ClientSecretResponse;
        Response response{};
        if (auto err = make_client_secret(merchant_id, parsed->get_resource_id(), response)) {
            send_error(res, send, *err);
            return;
        }
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

    void handle_client_secret_delete(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        // api::server_wrap(flow=ClientSecretDelete, auth=V2ApiKeyAuth{false,false},
        // LockAction::NotApplicable) — routes/client_secret.rs:48-60.
        const auto rule = m_context->auth_rules().rule_for("client_secret_delete");
        auto auth_ctx = app::auth::AuthContextBuilder{}(req);
        if (auto denied = m_context->authorizer().authorize_route(auth_ctx, rule)) {
            send_error(res, send, *denied);
            return;
        }
        if (auto denied = enforce_v2_api_key_flags(auth_ctx)) {
            send_error(res, send, *denied);
            return;
        }

        m_context->metrics().record_flow("ClientSecretDelete");
        m_context->metrics().counter("client_secret_delete").increment();

        // web::Path<String> id — routes/client_secret.rs:46,49.
        const std::string client_secret_id = client_secret_id_from_request(req);
        if (client_secret_id.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(
                core::error::AppErrorKind::INVALID_REQUEST, "missing client secret id"));
            return;
        }

        // delete_client_secret(state, req) — helpers.rs:4761-4782.
        using Response = app::dto::client_secret::ClientSecretResponse;
        Response response{};
        if (auto err = delete_client_secret(client_secret_id, response)) {
            send_error(res, send, *err);
            return;
        }
        send_json(res, send, serde::Ser::encode_json(response), true);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/v2/client-secret"}
            .add_route(
                Route{""}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_client_secret_create(req, res, std::move(send));
                    })
                    .summary("Hyperswitch client_secret_create")
            )
            .add_route(
                Route{"/:id"}
                    .delt([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_client_secret_delete(req, res, std::move(send));
                    })
                    .summary("Hyperswitch client_secret_delete")
            );
    }

private:
    using ResourceId = app::dto::client_secret::ResourceId;
    using Response = app::dto::client_secret::ClientSecretResponse;
    using ClientSecretType = app::dto::client_secret::ClientSecretType;
    using AppError = core::error::AppError;

    [[nodiscard]] static AppError error(core::error::AppErrorKind kind, std::string message)
    {
        return core::error::AppErrorBuilder{}.operator()(kind, std::move(message));
    }

    // Both routes declare V2ApiKeyAuth { allow_connected_scope_operation: false,
    // allow_platform_self_operation: false } (routes/client_secret.rs:32-35, :56-59).
    // The generic authorizer resolves scheme/permission but does not consume these flags
    // for API_KEY, so enforce them against auth-derived claims (revenue_recovery pattern).
    [[nodiscard]] std::optional<AppError>
    enforce_v2_api_key_flags(const app::auth::AuthContext& auth_ctx) const
    {
        const auto connected = auth_ctx.claim(app::auth::kClaimConnectedMerchantId);
        if (connected.has_value() && !connected->empty()) {
            return error(core::error::AppErrorKind::FORBIDDEN,
                         "connected-scope operations are not allowed");
        }
        const auto internal_key = auth_ctx.claim(app::auth::kClaimInternalApiKey);
        if (internal_key.has_value() && !internal_key->empty()) {
            return error(core::error::AppErrorKind::FORBIDDEN,
                         "platform self-operations are not allowed");
        }
        return std::nullopt;
    }

    // make_client_secret — helpers.rs:4694-4732.
    [[nodiscard]] std::optional<AppError>
    make_client_secret(const std::string& merchant_id, const ResourceId& resource_id, Response& out) const
    {
        // match &resource_id { Customer(global_customer_id) =>
        //   db.find_customer_by_global_id(global_customer_id, platform key store,
        //     storage_scheme).to_not_found_response(CustomerNotFound)? }  — helpers.rs:4702-4711.
        // Local substitute: resolve the global id through the key store, then the customer
        // repository. Upstream message is "Customer does not exist in our records"
        // (core/errors/transformers.rs:31-36).
        std::string resolved = resource_id.get_customer_id();
        if (const auto mapped = m_context->key_store().get("customer-global-id", resolved)) {
            resolved = mapped->value();
        }
        if (!m_context->repository().find_customer_by_string(resolved)) {
            return error(core::error::AppErrorKind::NOT_FOUND,
                         "Customer does not exist in our records");
        }

        // create_client_secret(state, account_id, Customer(global_customer_id))
        // helpers.rs:4720-4727 — any store failure is wrapped as InternalServerError.
        ClientSecretType record{};
        if (auto err = create_client_secret(merchant_id, resource_id, record)) {
            return err;
        }

        // ClientSecretResponse::foreign_try_from(record) — helpers.rs:4729-4731,
        // types/storage/client_secret.rs:11-31. The typed ResourceId only models the
        // Customer variant, so this conversion is infallible here.
        out = foreign_try_from(record);
        return std::nullopt;
    }

    // create_client_secret — helpers.rs:4735-4758.
    [[nodiscard]] std::optional<AppError>
    create_client_secret(const std::string& merchant_id, const ResourceId& resource_id, ClientSecretType& out) const
    {
        // ClientSecretId::generate() -> "csi_" + 20 alphanumeric chars
        // (impl_generate_id_id_type!(ClientSecretId,"csi"), consts::ID_LENGTH=20).
        const std::string id = m_context->ids().payment_id("csi_");
        // Secret::new(generate_time_ordered_id("cs")) -> "cs_" + uuid v7 simple (32 hex)
        // (common_utils/src/lib.rs:717-719).
        const std::string secret = generate_time_ordered_id("cs");

        // store.create_client_secret(new, state.conf.eph_key.validity)
        //   .change_context(InternalServerError).attach_printable("Unable to create client secret")
        // helpers.rs:4752-4756. Every store failure (incl. DuplicateValue) is a 500.
        if (auto err = store_create_client_secret(
                id, merchant_id, secret, resource_id, eph_validity_hours(), out)) {
            return error(core::error::AppErrorKind::INTERNAL_ERROR, "Unable to create client secret");
        }
        return std::nullopt;
    }

    // Store::create_client_secret — db/client_secret.rs:49-104.
    [[nodiscard]] std::optional<AppError> store_create_client_secret(
        const std::string& id,
        const std::string& merchant_id,
        const std::string& secret,
        const ResourceId& resource_id,
        std::int64_t validity_hours,
        ClientSecretType& out
    ) const
    {
        // created_at = date_time::now(); expires = created_at + validity.hours()
        // (db/client_secret.rs:54-55). Real nanoseconds keep the 9-digit wire subsecond.
        const auto created = std::chrono::system_clock::now();
        const auto expires = created + std::chrono::hours{validity_hours};

        ClientSecretType record{};
        record.set_id(id);
        record.set_merchant_id(merchant_id);
        record.set_resource_id(resource_id);
        record.set_created_at(primitive_datetime(created));
        record.set_expires(primitive_datetime(expires));
        record.set_secret(secret);

        // id_key = new.id.generate_redis_key() -> "cs_<id>" (id_type/client_secret.rs:31-33)
        // secret_key = created.generate_secret_key() -> "cs_<secret>" (diesel_models/client_secret.rs:25-27)
        const std::string id_key = record_key(id);
        const std::string secret_key = secret_key_from_secret(secret); // raw secret, verbatim
        const auto encoded = serde::Ser::encode_json(record);

        // serialize_and_set_multiple_hash_field_if_not_exist([...],"csh",None): a HSETNX
        // duplicate (KeyNotSet) becomes StorageError::DuplicateValue (db:81-87). A keyed
        // lock makes the local get-then-set deterministic; a hit maps to the same duplicate.
        const std::string lock_key = "client_secret.create:" + merchant_id + ":" + id;
        const auto lock = m_context->locks().acquire(lock_key, std::int64_t{30});
        if (!lock) {
            return error(core::error::AppErrorKind::INTERNAL_ERROR, "client secret storage lock failed");
        }
        if (m_context->cache().get(id_key).hit || m_context->cache().get(secret_key).hit) {
            m_context->locks().release(lock_key);
            return error(core::error::AppErrorKind::INTERNAL_ERROR, "client secret already exists");
        }
        // set_expire_at(key, expires.unix_timestamp()) (db:89-99) is the cache TTL.
        const auto ttl = core::cache::Ttl{}.hours(validity_hours);
        m_context->cache().put(id_key, core::cache::to_cache_value(encoded), ttl);
        m_context->cache().put(secret_key, core::cache::to_cache_value(encoded), ttl);
        m_context->locks().release(lock_key);

        out = std::move(record);
        return std::nullopt;
    }

    // delete_client_secret — helpers.rs:4761-4782.
    [[nodiscard]] std::optional<AppError>
    delete_client_secret(const std::string& client_secret_id, Response& out) const
    {
        // db.delete_client_secret(&id): ValueNotFound -> GenericNotFoundError{
        //   message:"Client secret not found" }, anything else -> InternalServerError
        // (helpers.rs:4766-4777).
        auto record = get_client_secret(client_secret_id);
        if (!record) {
            return error(core::error::AppErrorKind::NOT_FOUND, "Client secret not found");
        }

        // redis_id_key = record.id.generate_redis_key(); secret_key = record.generate_secret_key()
        // delete both (db/client_secret.rs:126-149).
        m_context->cache().erase(record_key(record->get_id()));
        m_context->cache().erase(secret_key_from_secret(record->get_secret()));

        out = foreign_try_from(*record);
        return std::nullopt;
    }

    // Store::get_client_secret — db/client_secret.rs:108-118. Prefixed "cs_<key>" lookup
    // in hash "csh"; the cache record is the local hash-field value.
    [[nodiscard]] std::optional<ClientSecretType> get_client_secret(std::string_view key) const
    {
        const auto cached = m_context->cache().get(record_key(key));
        if (!cached.hit) {
            return std::nullopt;
        }
        return serde::Ser::deserialize<ClientSecretType>(
            "application/json", cached.value_or<std::string>(std::string{}));
    }

    // ClientSecretResponse::foreign_try_from — types/storage/client_secret.rs:11-31.
    // Non-Customer variants (Payment, PaymentMethodSession) map to InternalServerError;
    // the local typed ResourceId models only Customer, so the Customer arm always applies.
    [[nodiscard]] Response foreign_try_from(const ClientSecretType& record) const
    {
        Response response{};
        response.set_id(record.get_id());
        response.set_resource_id(record.get_resource_id());
        response.set_created_at(record.get_created_at());
        response.set_expires(record.get_expires());
        response.set_secret(record.get_secret());
        return response;
    }

    [[nodiscard]] std::string client_secret_id_from_request(interfaces::io::IRequest& req) const
    {
        const app::http::PathSegments segments{app::http::RequestPath{}(req)};
        if (segments.count() == 0) {
            return {};
        }
        for (std::size_t i = 0; i + 1 < segments.count(); ++i) {
            const auto segment = segments.at(i);
            if (segment && *segment == "client-secret") {
                return std::string{segments.at(i + 1).value_or("")};
            }
        }
        return std::string{segments.at(segments.count() - 1).value_or("")};
    }

    // generate_time_ordered_id("cs") — common_utils/src/lib.rs:717-719:
    // "{prefix}_{uuid v7 as_simple()}" (32 hex chars, version nibble 7).
    [[nodiscard]] std::string generate_time_ordered_id(std::string_view prefix) const
    {
        std::string simple = m_context->ids().uuid_v4();
        std::erase(simple, '-'); // canonical 36 -> as_simple()'s 32 hex chars
        const auto millis = static_cast<unsigned long long>(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count());
        const std::string stamp = std::format("{:012x}", millis & 0xFFFFFFFFFFFFULL);
        if (simple.size() == 32) {
            simple.replace(0, 8, stamp, 0, 8);
            simple.replace(8, 4, stamp, 8, 4);
            simple[12] = '7'; // RFC 9562 version 7
        }
        return std::string{prefix} + "_" + simple;
    }

    [[nodiscard]] std::int64_t eph_validity_hours() const
    {
        constexpr std::int64_t fallback = 24;
        const auto value = m_context->config_store().get_value("eph_key", "validity");
        if (!value) {
            return fallback;
        }
        std::int64_t hours = 0;
        const auto* begin = value->data();
        const auto* end = begin + value->size();
        const auto [ptr, ec] = std::from_chars(begin, end, hours);
        if (ec != std::errc{} || ptr != end || hours <= 0) {
            return fallback;
        }
        return hours;
    }

    // time::PrimitiveDateTime wire shape "YYYY-MM-DD HH:MM:SS.nnnnnnnnn"
    // (hierarchical_resources.cppm:1264,1291-1297).
    [[nodiscard]] std::string primitive_datetime(std::chrono::system_clock::time_point tp) const
    {
        const auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
            tp.time_since_epoch()).count();
        std::int64_t seconds = static_cast<std::int64_t>(ns / 1'000'000'000);
        std::int64_t nanos = static_cast<std::int64_t>(ns % 1'000'000'000);
        if (nanos < 0) {
            nanos += 1'000'000'000;
            --seconds;
        }
        // Howard Hinnant's civil_from_days.
        std::int64_t days = seconds / 86'400;
        std::int64_t sod = seconds % 86'400;
        if (sod < 0) {
            sod += 86'400;
            --days;
        }
        days += 719'468;
        const std::int64_t era = (days >= 0 ? days : days - 146'096) / 146'097;
        const std::int64_t doe = days - era * 146'097;
        const std::int64_t yoe = (doe - doe / 1'460 + doe / 36'524 - doe / 146'096) / 365;
        std::int64_t year = yoe + era * 400;
        const std::int64_t doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
        const std::int64_t mp = (5 * doy + 2) / 153;
        const std::int64_t day = doy - (153 * mp + 2) / 5 + 1;
        const std::int64_t month = mp < 10 ? mp + 3 : mp - 9;
        year += (month <= 2) ? 1 : 0;
        return std::format("{:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}.{:09d}",
                           year, month, day,
                           sod / 3'600, (sod % 3'600) / 60, sod % 60, nanos);
    }

    // Redis id key: "cs_<id>" (id_type/client_secret.rs:31-33).
    [[nodiscard]] std::string record_key(std::string_view id) const
    {
        return std::string{"cs_"} + std::string{id};
    }

    // Redis secret key: "cs_<secret>" with the raw secret verbatim
    // (diesel_models/src/client_secret.rs:25-27).
    [[nodiscard]] std::string secret_key_from_secret(std::string_view secret) const
    {
        return std::string{"cs_"} + std::string{secret};
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

} // namespace app::handlers::client_secret
