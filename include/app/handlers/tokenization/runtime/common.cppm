export module app_handlers_tokenization:runtime_common;

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
import app_http;
import app_crypto;
import app_metrics;
import app_store;
import app_connector;
import app_events;
import app_workflow;
import app_keystore;

export namespace app::handlers::tokenization {

/** Local mirror of pm_types::AddVaultRequest (core/tokenization.rs:44-49). */
class AddVaultRequestWire
{
public:
    std::string entity_id{};
    std::string vault_id{};
    std::string data{}; // Secret<serde_json::Value>: opaque JSON text on the wire
    std::int64_t ttl{};
};

/** Local mirror of pm_types::AddVaultResponse (core/tokenization.rs:64-67). */
class AddVaultResponseWire
{
public:
    std::string vault_id{};
};

/** Local mirror of pm_types::VaultRetrieveRequest (core/tokenization.rs:188-193). */
class VaultRetrieveRequestWire
{
public:
    std::string entity_id{};
    std::string vault_id{};
};

/**
 * @brief Shared business support for the tokenization sub-runtimes.
 *
 * Ported verbatim from the original facade's in-process substitutes for pm_vault / KMS,
 * plus the id / crypto / time / error helpers used by the create and delete cores and
 * by the un-routed get_token_vault_core primitive.
 */
class TokenizationRuntimeSupport
{
public:
    explicit TokenizationRuntimeSupport(app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    [[nodiscard]] core::error::AppError
    make_error(core::error::AppErrorKind kind, std::string message) const
    {
        return core::error::AppErrorBuilder{}.operator()(kind, std::move(message));
    }

    [[nodiscard]] core::error::AppError
    upstream_error(core::error::AppErrorKind kind, std::string_view name, std::string message) const
    {
        // AppError serializes its kind_name into the wire "error" field; use the exact
        // upstream ApiErrorResponse variant name (currency.cppm:49-56 precedent).
        auto result = make_error(kind, std::move(message));
        result.set_kind_name(std::string{name});
        return result;
    }

    // ─── Local in-process substitutes for external pm_vault / KMS ───

    /** Local pm_vault::call_to_vault::<AddVault> (core/tokenization.rs:57-61). */
    [[nodiscard]] std::expected<AddVaultResponseWire, core::error::AppError>
    call_to_vault_add(const AddVaultRequestWire& request) const
    {
        const auto payload = serde::Ser::encode_json(request);
        const std::string wrapped = encrypt_vault_data(request.vault_id, payload);
        m_context.cache().put(
            vault_cache_key(request.vault_id),
            core::cache::to_cache_value(wrapped),
            core::cache::Ttl{}.seconds(request.ttl)
        );
        // The local locker echoes the caller-supplied vault id (AddVaultResponse).
        AddVaultResponseWire response{};
        response.vault_id = request.vault_id;
        return response;
    }

    /** Local pm_vault::retrieve_value_from_vault (core/tokenization.rs:195-198). */
    [[nodiscard]] std::expected<std::string, core::error::AppError>
    retrieve_value_from_vault(const VaultRetrieveRequestWire& request) const
    {
        const auto entry = m_context.cache().get(vault_cache_key(request.vault_id));
        if (!entry.hit) {
            return std::unexpected{upstream_error(
                core::error::AppErrorKind::INTERNAL_ERROR,
                "InternalServerError",
                "Failed to retrieve vault data"
            )};
        }
        return decrypt_vault_data(request.vault_id, entry.value_or<std::string>({}));
    }

    /** Local pm_vault::delete_payment_method_data_from_vault_internal (core:135-143). */
    void delete_payment_method_data_from_vault_internal(
        std::string_view vault_id,
        std::string_view /*entity_id*/
    ) const
    {
        m_context.cache().erase(vault_cache_key(vault_id));
    }

    /**
     * Port of core::tokenization::get_token_vault_core (core/tokenization.rs:167-207).
     * No route calls this upstream; kept as a local helper so the retrieve primitive is
     * present verbatim for future use.
     */
    [[nodiscard]] std::expected<std::string, core::error::AppError>
    get_token_vault_core(std::string_view token_id) const
    {
        // core:175-179 — get_entity_id_vault_id_by_token_id -> InternalServerError.
        const auto record = m_context.repository().find_tokenization_by_string(token_id);
        if (!record) {
            return std::unexpected{upstream_error(
                core::error::AppErrorKind::INTERNAL_ERROR,
                "InternalServerError",
                "Failed to get tokenization record"
            )};
        }

        // core:181-186 — Disabled -> GenericNotFoundError.
        if (record->get_status() == disabled_flag()) {
            return std::unexpected{upstream_error(
                core::error::AppErrorKind::NOT_FOUND,
                "GenericNotFoundError",
                "Tokenization is disabled for the id"
            )};
        }

        // core:188-193 — VaultRetrieveRequest { entity_id, VaultId::generate(locker_id) }.
        VaultRetrieveRequestWire request{};
        request.entity_id = record->get_customer_id();
        request.vault_id = record->get_vault_id();

        // core:195-198 — retrieve_value_from_vault -> InternalServerError.
        auto vault_data = retrieve_value_from_vault(request);
        if (!vault_data) {
            return std::unexpected{std::move(vault_data.error())};
        }

        // core:200-203 — data = vault_data.get("data").cloned().unwrap_or(Null).
        auto data = serde::Ser::decode_generic("application/json", *vault_data);
        if (!data) {
            return std::string{"null"};
        }
        const auto* object = std::get_if<serde::Value::Object>(&data->get());
        if (object == nullptr) {
            return std::string{"null"};
        }
        const auto field = object->get("data");
        if (!field) {
            return std::string{"null"};
        }
        return serde::Ser::encode_json(field.value());
    }

    // ─── Ids, crypto, time, flags ───

    [[nodiscard]] std::string cell_id(std::string_view merchant_id) const
    {
        // conf.cell_information.id is a 2-char CellId (e.g. "0a"). Local cell is a keyed
        // digest of the merchant id because the deployment config is external.
        return app::crypto::KeyedHasher{"cell-id"}.hash(merchant_id).substr(0, 2);
    }

    [[nodiscard]] std::string global_token_id(std::string_view cell) const
    {
        // core/tokenization.rs:71 + global_id.rs:149-153 + token.rs:38-40:
        // GlobalTokenId::generate(cell) == "{cell}_tok_{uuid_v7().as_simple()}".
        return std::string{cell} + "_tok_" + uuid_v7_simple();
    }

    [[nodiscard]] std::string uuid_v7() const
    {
        // Local time-ordered UUIDv7 stand-in over IdGenerator::uuid_v4.
        const auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(
                                std::chrono::system_clock::now().time_since_epoch()
        )
                                .count();
        const std::string stamp = std::format(
            "{:012x}",
            static_cast<unsigned long long>(millis) & 0xFF'FF'FF'FF'FF'FFULL
        );
        std::string out = m_context.ids().uuid_v4();
        out.replace(0, 8, stamp, 0, 8);
        out.replace(9, 4, stamp, 8, 4);
        out[14] = '7';
        return out;
    }

    [[nodiscard]] std::string uuid_v7_simple() const
    {
        // Uuid::as_simple() — the 32 hex chars used by generate_time_ordered_id
        // (common_utils/src/lib.rs:717-719).
        std::string id = uuid_v7();
        std::erase(id, '-');
        return id;
    }

    [[nodiscard]] std::string vault_cache_key(std::string_view vault_id) const
    {
        return std::string{"vault.token."} +
               app::crypto::KeyedHasher{"vault-cache-key"}.hash(vault_id);
    }

    [[nodiscard]] std::int64_t epoch_seconds() const
    {
        return std::chrono::duration_cast<std::chrono::seconds>(
                   std::chrono::system_clock::now().time_since_epoch()
        )
            .count();
    }

    [[nodiscard]] std::string vault_wrap_key() const
    {
        // Local stand-in for the processor key store. The wrap key is resolved from
        // KeyStore; a missing key is seeded once rather than embedded in the cipher step.
        if (const auto key = m_context.key_store().get("vault", "wrap")) {
            return key->value();
        }
        const std::string seed{vault_key()};
        m_context.key_store().put(
            "vault",
            "wrap",
            "AES-GCM-STANDIN",
            app::crypto::SecretString{seed},
            epoch_seconds()
        );
        return seed;
    }

    [[nodiscard]] std::string
    encrypt_vault_data(std::string_view vault_id, std::string_view plaintext) const
    {
        app::crypto::AesWrap wrap{};
        return wrap.encrypt(plaintext, vault_wrap_key() + std::string{vault_id});
    }

    [[nodiscard]] std::string
    decrypt_vault_data(std::string_view vault_id, std::string_view ciphertext) const
    {
        app::crypto::AesWrap wrap{};
        return wrap.decrypt(ciphertext, vault_wrap_key() + std::string{vault_id});
    }

    [[nodiscard]] std::string primitive_date_time_now() const
    {
        // time::PrimitiveDateTime serde shape: "YYYY-MM-DD HH:MM:SS.nnnnnnnnn".
        // common_utils::date_time::now() builds it from OffsetDateTime::now_utc()
        // (common_utils/src/lib.rs:93-97); no offset is serialized.
        const auto now = std::chrono::system_clock::now();
        const std::int64_t seconds =
            std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
        const std::int64_t nanos =
            std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count() %
            1'000'000'000;

        const std::int64_t days = seconds / 86'400;
        const std::int64_t sod = seconds % 86'400;
        const std::int64_t z = days + 719'468;
        const std::int64_t era = (z >= 0 ? z : z - 146'096) / 146'097;
        const unsigned doe = static_cast<unsigned>(z - era * 146'097);
        const unsigned yoe = (doe - doe / 1'460 + doe / 36'524 - doe / 146'096) / 365;
        const std::int64_t year = static_cast<std::int64_t>(yoe) + era * 400;
        const unsigned doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
        const unsigned mp = (5 * doy + 2) / 153;
        const unsigned day = doy - (153 * mp + 2) / 5 + 1;
        const unsigned month = mp < 10 ? mp + 3 : mp - 9;
        const std::int64_t y = year + (month <= 2 ? 1 : 0);
        const unsigned hour = static_cast<unsigned>(sod / 3'600);
        const unsigned minute = static_cast<unsigned>((sod % 3'600) / 60);
        const unsigned sec = static_cast<unsigned>(sod % 60);
        return std::format(
            "{:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}.{:09d}",
            y,
            month,
            day,
            hour,
            minute,
            sec,
            nanos
        );
    }

    [[nodiscard]] std::string_view vault_key() const noexcept
    {
        return "hyperswitch-vault-wrap-key";
    }

    [[nodiscard]] std::int64_t vault_ttl_seconds() const noexcept
    {
        // Stand-in for conf.locker.ttl_for_storage_in_secs (upstream default: one day).
        return 86'400;
    }

    [[nodiscard]] std::string enabled_flag() const noexcept
    {
        // TokenizationFlag is #[serde(rename_all = "snake_case")] at
        // common_enums/src/enums.rs:12123.
        return "enabled";
    }

    [[nodiscard]] std::string disabled_flag() const noexcept
    {
        return "disabled";
    }

private:
    app::context::AppContext& m_context;
};

} // namespace app::handlers::tokenization
