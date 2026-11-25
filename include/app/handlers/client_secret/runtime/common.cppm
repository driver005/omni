export module app_handlers_client_secret:runtime_common;

import std;
import core_serde;
import core_cache;
import core_error;
import app_model;
import app_dto;
import app_context;
import app_store;
import app_crypto;
import app_keystore;
import app_locking;
import app_services;

export namespace app::handlers::client_secret {

/**
 * @brief Business helpers shared by the client_secret route runtimes.
 *
 * Ports the local stand-ins for helpers.rs:4694-4782 (make_client_secret,
 * create_client_secret, delete_client_secret) and db/client_secret.rs:49-151
 * (the Redis hash store). All state comes from AppContext; no HTTP types here.
 */
class ClientSecretRuntimeSupport
{
public:
    explicit ClientSecretRuntimeSupport(app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    [[nodiscard]] core::error::AppError
    make_error(core::error::AppErrorKind kind, std::string message) const
    {
        return core::error::AppErrorBuilder{}.operator()(kind, std::move(message));
    }

    // make_client_secret — helpers.rs:4694-4732.
    [[nodiscard]] std::optional<core::error::AppError> make_client_secret(
        const std::string& merchant_id,
        const app::dto::client_secret::ResourceId& resource_id,
        app::dto::client_secret::ClientSecretResponse& out
    ) const
    {
        // match &resource_id { Customer(global_customer_id) =>
        //   db.find_customer_by_global_id(global_customer_id, platform key store,
        //     storage_scheme).to_not_found_response(CustomerNotFound)? }  — helpers.rs:4702-4711.
        // Local substitute: resolve the global id through the key store, then the customer
        // repository. Upstream message is "Customer does not exist in our records"
        // (core/errors/transformers.rs:31-36).
        std::string resolved = resource_id.get_customer_id();
        if (const auto mapped = m_context.key_store().get("customer-global-id", resolved)) {
            resolved = mapped->value();
        }
        if (!m_context.repository().find_customer_by_string(resolved)) {
            return make_error(
                core::error::AppErrorKind::NOT_FOUND,
                "Customer does not exist in our records"
            );
        }

        // create_client_secret(state, account_id, Customer(global_customer_id))
        // helpers.rs:4720-4727 — any store failure is wrapped as InternalServerError.
        app::dto::client_secret::ClientSecretType record{};
        if (auto err = create_client_secret(merchant_id, resource_id, record)) {
            return err;
        }

        // ClientSecretResponse::foreign_try_from(record) — helpers.rs:4729-4731,
        // types/storage/client_secret.rs:11-31.
        out = foreign_try_from(record);
        return std::nullopt;
    }

    // create_client_secret — helpers.rs:4735-4758.
    [[nodiscard]] std::optional<core::error::AppError> create_client_secret(
        const std::string& merchant_id,
        const app::dto::client_secret::ResourceId& resource_id,
        app::dto::client_secret::ClientSecretType& out
    ) const
    {
        // ClientSecretId::generate() -> "csi_" + 20 alphanumeric chars.
        const std::string id = m_context.ids().payment_id("csi_");
        // Secret::new(generate_time_ordered_id("cs")).
        const std::string secret = generate_time_ordered_id("cs");

        // store.create_client_secret(new, state.conf.eph_key.validity)
        //   .change_context(InternalServerError).attach_printable("Unable to create client secret")
        // helpers.rs:4752-4756. Every store failure (incl. DuplicateValue) is a 500.
        if (auto err = store_create_client_secret(
                id,
                merchant_id,
                secret,
                resource_id,
                eph_validity_hours(),
                out
            )) {
            return make_error(
                core::error::AppErrorKind::INTERNAL_ERROR,
                "Unable to create client secret"
            );
        }
        return std::nullopt;
    }

    // Store::create_client_secret — db/client_secret.rs:49-104.
    [[nodiscard]] std::optional<core::error::AppError> store_create_client_secret(
        const std::string& id,
        const std::string& merchant_id,
        const std::string& secret,
        const app::dto::client_secret::ResourceId& resource_id,
        std::int64_t validity_hours,
        app::dto::client_secret::ClientSecretType& out
    ) const
    {
        // created_at = date_time::now(); expires = created_at + validity.hours()
        // (db/client_secret.rs:54-55). Real nanoseconds keep the 9-digit wire subsecond.
        const auto created = std::chrono::system_clock::now();
        const auto expires = created + std::chrono::hours{validity_hours};

        app::dto::client_secret::ClientSecretType record{};
        record.set_id(id);
        record.set_merchant_id(merchant_id);
        record.set_resource_id(resource_id);
        record.set_created_at(primitive_datetime(created));
        record.set_expires(primitive_datetime(expires));
        record.set_secret(secret);

        // id_key = new.id.generate_redis_key() -> "cs_<id>"
        // secret_key = created.generate_secret_key() -> "cs_<secret>" (raw secret, verbatim)
        const std::string id_key = record_key(id);
        const std::string secret_key = secret_key_from_secret(secret);
        const auto encoded = serde::Ser::encode_json(record);

        // serialize_and_set_multiple_hash_field_if_not_exist([...],"csh",None): a HSETNX
        // duplicate (KeyNotSet) becomes StorageError::DuplicateValue (db:81-87). A keyed
        // lock makes the local get-then-set deterministic; a hit maps to the same duplicate.
        const std::string lock_key = "client_secret.create:" + merchant_id + ":" + id;
        const auto lock = m_context.locks().acquire(lock_key, std::int64_t{30});
        if (!lock) {
            return make_error(
                core::error::AppErrorKind::INTERNAL_ERROR,
                "client secret storage lock failed"
            );
        }
        if (m_context.cache().get(id_key).hit || m_context.cache().get(secret_key).hit) {
            m_context.locks().release(lock_key);
            return make_error(
                core::error::AppErrorKind::INTERNAL_ERROR,
                "client secret already exists"
            );
        }
        // set_expire_at(key, expires.unix_timestamp()) (db:89-99) is the cache TTL.
        const auto ttl = core::cache::Ttl{}.hours(validity_hours);
        m_context.cache().put(id_key, core::cache::to_cache_value(encoded), ttl);
        m_context.cache().put(secret_key, core::cache::to_cache_value(encoded), ttl);
        m_context.locks().release(lock_key);

        out = std::move(record);
        return std::nullopt;
    }

    // delete_client_secret — helpers.rs:4761-4782.
    [[nodiscard]] std::optional<core::error::AppError> delete_client_secret(
        const std::string& client_secret_id,
        app::dto::client_secret::ClientSecretResponse& out
    ) const
    {
        // db.delete_client_secret(&id): ValueNotFound -> GenericNotFoundError{
        //   message:"Client secret not found" }, anything else -> InternalServerError
        // (helpers.rs:4766-4777).
        auto record = get_client_secret(client_secret_id);
        if (!record) {
            return make_error(core::error::AppErrorKind::NOT_FOUND, "Client secret not found");
        }

        // redis_id_key = record.id.generate_redis_key(); secret_key = record.generate_secret_key()
        // delete both (db/client_secret.rs:126-149).
        m_context.cache().erase(record_key(record->get_id()));
        m_context.cache().erase(secret_key_from_secret(record->get_secret()));

        out = foreign_try_from(*record);
        return std::nullopt;
    }

    // Store::get_client_secret — db/client_secret.rs:108-118.
    [[nodiscard]] std::optional<app::dto::client_secret::ClientSecretType>
    get_client_secret(std::string_view key) const
    {
        const auto cached = m_context.cache().get(record_key(key));
        if (!cached.hit) {
            return std::nullopt;
        }
        const auto decoded = serde::Ser::deserialize<app::dto::client_secret::ClientSecretType>(
            "application/json",
            cached.value_or<std::string>(std::string{})
        );
        return decoded ? std::optional<app::dto::client_secret::ClientSecretType>{*decoded}
                       : std::nullopt;
    }

    // ClientSecretResponse::foreign_try_from — types/storage/client_secret.rs:11-31.
    [[nodiscard]] app::dto::client_secret::ClientSecretResponse
    foreign_try_from(const app::dto::client_secret::ClientSecretType& record) const
    {
        app::dto::client_secret::ClientSecretResponse response{};
        response.set_id(record.get_id());
        response.set_resource_id(record.get_resource_id());
        response.set_created_at(record.get_created_at());
        response.set_expires(record.get_expires());
        response.set_secret(record.get_secret());
        return response;
    }

    // generate_time_ordered_id("cs") — common_utils/src/lib.rs:717-719:
    // "{prefix}_{uuid v7 as_simple()}" (32 hex chars, version nibble 7).
    [[nodiscard]] std::string generate_time_ordered_id(std::string_view prefix) const
    {
        std::string simple = m_context.ids().uuid_v4();
        std::erase(simple, '-'); // canonical 36 -> as_simple()'s 32 hex chars
        const auto millis =
            static_cast<unsigned long long>(std::chrono::duration_cast<std::chrono::milliseconds>(
                                                std::chrono::system_clock::now().time_since_epoch()
            )
                                                .count());
        const std::string stamp = std::format("{:012x}", millis & 0xFF'FF'FF'FF'FF'FFULL);
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
        const auto value = m_context.config_store().get_value("eph_key", "validity");
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

    // time::PrimitiveDateTime wire shape "YYYY-MM-DD HH:MM:SS.nnnnnnnnn".
    [[nodiscard]] std::string primitive_datetime(std::chrono::system_clock::time_point tp) const
    {
        const auto ns =
            std::chrono::duration_cast<std::chrono::nanoseconds>(tp.time_since_epoch()).count();
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
        return std::format(
            "{:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}.{:09d}",
            year,
            month,
            day,
            sod / 3'600,
            (sod % 3'600) / 60,
            sod % 60,
            nanos
        );
    }

    // Redis id key: "cs_<id>" (id_type/client_secret.rs:31-33).
    [[nodiscard]] std::string record_key(std::string_view id) const
    {
        return std::string{"cs_"} + std::string{id};
    }

    // Redis secret key: "cs_<secret>" with the raw secret verbatim.
    [[nodiscard]] std::string secret_key_from_secret(std::string_view secret) const
    {
        return std::string{"cs_"} + std::string{secret};
    }

private:
    app::context::AppContext& m_context;
};

} // namespace app::handlers::client_secret
