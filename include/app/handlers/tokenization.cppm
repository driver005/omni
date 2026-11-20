module;
export module app_handlers:tokenization;

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
import app_keystore;

export namespace app::handlers::tokenization {

/**
 * Local wire mirrors of api_models/src/tokenization.rs:8-46. Upstream id newtypes,
 * time::PrimitiveDateTime and TokenizationFlag all serialize as plain scalars. The
 * converted app_dto::tokenization wrappers emit {"value": ...}, so these shapes keep
 * the upstream wire form while still going through serde encoding. The DTO file is
 * retained for consumers but is no longer the wire surface of these routes.
 */
class CreateResponseWire
{
public:
    std::string id{};
    std::string created_at{};
    std::string flag{};
};

class DeleteResponseWire
{
public:
    std::string id{};
};

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

/** Parsed GenericTokenizationRequest (api_models/src/tokenization.rs:21-28). */
struct CreateRequest
{
    std::string customer_id{};
    std::string token_request{}; // opaque Secret<Value> JSON text
};

/** Parsed DeleteTokenDataRequest (api_models/src/tokenization.rs:32-39). */
struct DeleteRequest
{
    std::string customer_id{};
    std::string session_id{};
};

class TokenizationRouteHandlers
{
public:
    void handle_create_token_vault_api(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        auto parsed = serde::Ser::decode_generic("application/json", body);
        if (!parsed) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }
        const auto request = parse_create_request(*parsed);
        if (!request) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        // routes/tokenization.rs:42 — customer_id is read before api_or_client_auth.
        const std::string customer_id = request->customer_id;
        if (customer_id.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::VALIDATION_FAILED, "missing customer id"));
            return;
        }

        // api_or_client_auth(V2ApiKeyAuth{false,false},
        //   V2ClientAuth(ResourceId::Customer(customer_id))) — routes/tokenization.rs:56-65.
        auto context = app::auth::AuthContextBuilder{}(req);
        if (const auto auth_error = authorize_request(context, "create_token_vault_api", customer_id)) {
            send_error(res, send, *auth_error);
            return;
        }

        // Flow::TokenizationCreate, LockAction::NotApplicable — routes/tokenization.rs:40,66.
        m_context->metrics().record_flow("TokenizationCreate");
        m_context->metrics().counter("TOKENIZATION_CREATED").increment();

        auto result = create_vault_token_core(*request, resolve_merchant_id(req, context));
        if (!result) {
            send_error(res, send, result.error());
            return;
        }
        send_json(res, send, serde::Ser::encode_json(*result), true);
    }

    void handle_delete_tokenized_data_api(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }
        auto parsed = serde::Ser::decode_generic("application/json", body);
        if (!parsed) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }
        const auto request = parse_delete_request(*parsed);
        if (!request) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        // routes/tokenization.rs:81 — session_id is read before api_or_client_auth.
        if (request->session_id.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::VALIDATION_FAILED, "missing session id"));
            return;
        }

        // routes/tokenization.rs:82 — GlobalTokenId comes from web::Path, not the body.
        const auto path = app::http::RequestPath{}(req);
        const std::string token_id = std::string{path.at(2).value_or("")};
        if (token_id.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_REQUEST, "missing token id"));
            return;
        }

        // api_or_client_auth(V2ApiKeyAuth{false,false},
        //   V2ClientAuth(ResourceId::PaymentMethodSession(session_id))) — routes:92-101.
        auto context = app::auth::AuthContextBuilder{}(req);
        if (const auto auth_error = authorize_request(context, "delete_tokenized_data_api", request->session_id)) {
            send_error(res, send, *auth_error);
            return;
        }

        // Flow::TokenizationDelete, LockAction::NotApplicable — routes/tokenization.rs:79,102.
        m_context->metrics().record_flow("TokenizationDelete");
        m_context->metrics().counter("TOKENIZATION_DELETED").increment();

        auto result = delete_tokenized_data_core(token_id, request->customer_id);
        if (!result) {
            send_error(res, send, result.error());
            return;
        }
        send_json(res, send, serde::Ser::encode_json(*result), true);
    }

    void install(core::router::RouterContext<>& router)
    {
        using Route = utils::openapi::ApiRoute;
        using Router = utils::openapi::ApiRouter;
        Router{router, "/v2/tokenize"}
            .add_route(
                Route{""}
                    .post([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_create_token_vault_api(req, res, std::move(send));
                    })
                    .summary("Hyperswitch create_token_vault_api")
            )
            .add_route(
                Route{"/:id"}
                    .delt([this](
                        interfaces::io::IRequest& req,
                        interfaces::io::IResponse& res,
                        std::function<void()> send
                    ) {
                        handle_delete_tokenized_data_api(req, res, std::move(send));
                    })
                    .summary("Hyperswitch delete_tokenized_data_api")
            );
    }

    explicit TokenizationRouteHandlers(app::context::AppContext& context) noexcept
        : m_context{&context}
    {
    }

private:
    // ─── Ported upstream core functions ───────────────────────────────────────

    /** Port of core::tokenization::create_vault_token_core (core/tokenization.rs:34-96). */
    [[nodiscard]] std::expected<CreateResponseWire, core::error::AppError>
    create_vault_token_core(const CreateRequest& request, std::string_view merchant_id) const
    {
        // core:40 — VaultId::generate(generate_uuid_v7().to_string()).
        const std::string vault_id = uuid_v7();

        // core:44-49 — AddVaultRequest { entity_id, vault_id, data, ttl }.
        AddVaultRequestWire add_request{};
        add_request.entity_id = request.customer_id;
        add_request.vault_id = vault_id;
        add_request.data = request.token_request;
        add_request.ttl = vault_ttl_seconds();

        // core:54-61 — WriteMode::Insert + pm_vault::call_to_vault::<AddVault>.
        auto stored = call_to_vault_add(add_request);
        if (!stored) {
            return std::unexpected{stored.error()};
        }

        // core:70-79 — Tokenization { id, merchant_id, customer_id, locker_id,
        // created_at, updated_at, flag: Enabled, version: V2 }.
        app::store::tokenizations::TokenizationRecord record{};
        record.set_token_id(global_token_id(cell_id(merchant_id)));
        record.set_merchant_id(std::string{merchant_id});
        record.set_customer_id(request.customer_id);
        record.set_entity_id(request.customer_id);
        record.set_vault_id(stored->vault_id); // core:74 locker_id = stored_resp.vault_id
        record.set_status(enabled_flag());     // core:77 TokenizationFlag::Enabled
        record.set_created_at(primitive_date_time_now()); // core:75-76

        // core:82-86 — insert_tokenization(tokenization_new, provider.get_key_store()).
        auto inserted = m_context->repository().create_tokenization(std::move(record));
        if (!inserted) {
            return std::unexpected{inserted.error()};
        }

        // core:89-94 — GenericTokenizationResponse { id, created_at, flag }.
        CreateResponseWire response{};
        response.id = inserted->get_token_id();
        response.created_at = inserted->get_created_at();
        response.flag = inserted->get_status();
        return response;
    }

    /** Port of core::tokenization::delete_tokenized_data_core (core/tokenization.rs:100-163). */
    [[nodiscard]] std::expected<DeleteResponseWire, core::error::AppError>
    delete_tokenized_data_core(std::string_view token_id, std::string_view customer_id) const
    {
        // core:109-115 — get_entity_id_vault_id_by_token_id -> TokenizationRecordNotFound.
        auto record = m_context->repository().find_tokenization_by_string(token_id);
        if (!record) {
            return std::unexpected{upstream_error(
                core::error::AppErrorKind::NOT_FOUND,
                "TokenizationRecordNotFound",
                std::format("Tokenization record not found for the given token_id '{}' ", token_id)
            )};
        }

        // core:117-124 — when(customer mismatch) -> UnprocessableEntity.
        if (record->get_customer_id() != customer_id) {
            return std::unexpected{upstream_error(
                core::error::AppErrorKind::VALIDATION_FAILED,
                "UnprocessableEntity",
                "Tokenization record does not belong to the customer"
            )};
        }

        // core:126-130 — when(is_disabled) -> GenericNotFoundError.
        if (record->get_status() == disabled_flag()) {
            return std::unexpected{upstream_error(
                core::error::AppErrorKind::NOT_FOUND,
                "GenericNotFoundError",
                "Tokenization is already disabled for the id"
            )};
        }

        // core:133 — VaultId::generate(tokenization_record.locker_id).
        const std::string vault_id = record->get_vault_id();

        // core:135-143 — delete_payment_method_data_from_vault_internal.
        delete_payment_method_data_from_vault_internal(vault_id, record->get_customer_id());

        // core:146-156 — update_tokenization_record(DeleteTokenizationRecordUpdate{Disabled}).
        record->set_status(disabled_flag());
        if (!m_context->repository().update_tokenization(*record)) {
            return std::unexpected{upstream_error(
                core::error::AppErrorKind::INTERNAL_ERROR,
                "InternalServerError",
                "Failed to update tokenization record"
            )};
        }

        // core:158-161 — DeleteTokenDataResponse { id: token_id }.
        DeleteResponseWire response{};
        response.id = std::string{token_id};
        return response;
    }

    /**
     * Port of core::tokenization::get_token_vault_core (core/tokenization.rs:167-207).
     * No route calls this upstream (no route in routes/tokenization.rs); kept as a local
     * helper so the retrieve primitive is present verbatim for future use.
     */
    [[nodiscard]] std::expected<std::string, core::error::AppError>
    get_token_vault_core(std::string_view token_id) const
    {
        // core:175-179 — get_entity_id_vault_id_by_token_id -> InternalServerError.
        const auto record = m_context->repository().find_tokenization_by_string(token_id);
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

    // ─── Local in-process substitutes for external pm_vault / KMS ─────────────

    /** Local pm_vault::call_to_vault::<AddVault> (core/tokenization.rs:57-61). */
    [[nodiscard]] std::expected<AddVaultResponseWire, core::error::AppError>
    call_to_vault_add(const AddVaultRequestWire& request) const
    {
        const auto payload = serde::Ser::encode_json(request);
        const std::string wrapped = encrypt_vault_data(request.vault_id, payload);
        m_context->cache().put(
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
        const auto entry = m_context->cache().get(vault_cache_key(request.vault_id));
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
        m_context->cache().erase(vault_cache_key(vault_id));
    }

    // ─── Wire helpers ─────────────────────────────────────────────────────────

    [[nodiscard]] std::optional<CreateRequest>
    parse_create_request(const serde::Value& root) const
    {
        const auto* object = std::get_if<serde::Value::Object>(&root.get());
        if (object == nullptr) {
            return std::nullopt;
        }
        CreateRequest request{};
        if (const auto field = object->get("customer_id")) {
            if (const auto* text = std::get_if<std::string>(&field.value().get())) {
                request.customer_id = *text;
            }
        }
        // Secret<serde_json::Value> is transparent on the wire: keep the raw JSON text.
        if (const auto field = object->get("token_request")) {
            request.token_request = serde::Ser::encode_json(field.value());
        } else {
            request.token_request = "null";
        }
        return request;
    }

    [[nodiscard]] std::optional<DeleteRequest>
    parse_delete_request(const serde::Value& root) const
    {
        const auto* object = std::get_if<serde::Value::Object>(&root.get());
        if (object == nullptr) {
            return std::nullopt;
        }
        DeleteRequest request{};
        if (const auto field = object->get("customer_id")) {
            if (const auto* text = std::get_if<std::string>(&field.value().get())) {
                request.customer_id = *text;
            }
        }
        if (const auto field = object->get("session_id")) {
            if (const auto* text = std::get_if<std::string>(&field.value().get())) {
                request.session_id = *text;
            }
        }
        return request;
    }

    // ─── Auth, ids, crypto, time, errors ──────────────────────────────────────

    [[nodiscard]] std::optional<core::error::AppError> authorize_request(
        app::auth::AuthContext context,
        std::string_view operation,
        std::string_view bound_resource
    ) const
    {
        const auto rule = m_context->auth_rules().rule_for(operation);

        // Primary api_or_client_auth branch: V2ApiKeyAuth with both overrides false.
        auto denied = m_context->authorizer().authorize_route(context, rule);
        if (!denied) {
            return enforce_no_connected_no_platform(context, rule);
        }

        // Fallback branch: V2ClientAuth(ResourceId::Customer / PaymentMethodSession).
        if (bound_resource.empty()) {
            return denied;
        }
        context.add_claim("x-resource-id", std::string{bound_resource});
        const auto client_rule = app::auth::RouteAuthRuleBuilder{}
                                     .scheme(app::auth::AuthScheme::SDK)
                                     .permission(rule.permission())
                                     .allow_connected(false)
                                     .allow_platform(false)
                                     .id_source("x-resource-id")
                                     .build();
        denied = m_context->authorizer().authorize_route(context, client_rule);
        if (denied) {
            return denied;
        }
        return enforce_no_connected_no_platform(context, client_rule);
    }

    [[nodiscard]] std::optional<core::error::AppError> enforce_no_connected_no_platform(
        const app::auth::AuthContext& context,
        const app::auth::RouteAuthRule& rule
    ) const
    {
        // V2ApiKeyAuth { allow_connected_scope_operation: false,
        // allow_platform_self_operation: false } — routes/tokenization.rs:57-60,93-96.
        if (!rule.allow_connected()) {
            const auto connected = context.claim(app::auth::kClaimConnectedMerchantId);
            if (connected.has_value() && !connected->empty()) {
                return error(
                    core::error::AppErrorKind::FORBIDDEN,
                    "connected-scope operations are not allowed for tokenization"
                );
            }
        }
        if (!rule.allow_platform()) {
            const auto internal_key = context.claim(app::auth::kClaimInternalApiKey);
            if (internal_key.has_value() && !internal_key->empty()) {
                return error(
                    core::error::AppErrorKind::FORBIDDEN,
                    "platform self-operations are not allowed for tokenization"
                );
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] std::string resolve_merchant_id(
        interfaces::io::IRequest& req,
        const app::auth::AuthContext& auth
    ) const
    {
        // Upstream: provider.get_account().get_id() (core/tokenization.rs:72). The local
        // substitute uses the auth-derived merchant id (header-backed) canonicalized via
        // the repository to its stored provider-account value.
        std::string merchant_id = auth.merchant_id();
        if (merchant_id.empty()) {
            merchant_id = std::string{app::http::merchant_id_from_header{}(req)};
        }
        if (const auto account = m_context->repository().find_merchant_account_by_string(merchant_id)) {
            merchant_id = account->get_merchant_id();
        }
        return merchant_id;
    }

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
        ).count();
        const std::string stamp = std::format(
            "{:012x}", static_cast<unsigned long long>(millis) & 0xFFFFFFFFFFFFULL
        );
        std::string out = m_context->ids().uuid_v4();
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
        ).count();
    }

    [[nodiscard]] std::string vault_wrap_key() const
    {
        // Local stand-in for the processor key store. The wrap key is resolved from
        // KeyStore; a missing key is seeded once rather than embedded in the cipher step.
        if (const auto key = m_context->key_store().get("vault", "wrap")) {
            return key->value();
        }
        const std::string seed{vault_key()};
        m_context->key_store().put(
            "vault", "wrap", "AES-GCM-STANDIN",
            app::crypto::SecretString{seed}, epoch_seconds()
        );
        return seed;
    }

    [[nodiscard]] std::string encrypt_vault_data(std::string_view vault_id, std::string_view plaintext) const
    {
        app::crypto::AesWrap wrap;
        return wrap.encrypt(plaintext, vault_wrap_key() + std::string{vault_id});
    }

    [[nodiscard]] std::string decrypt_vault_data(std::string_view vault_id, std::string_view ciphertext) const
    {
        app::crypto::AesWrap wrap;
        return wrap.decrypt(ciphertext, vault_wrap_key() + std::string{vault_id});
    }

    [[nodiscard]] std::string primitive_date_time_now() const
    {
        // time::PrimitiveDateTime serde shape: "YYYY-MM-DD HH:MM:SS.nnnnnnnnn".
        // common_utils::date_time::now() builds it from OffsetDateTime::now_utc()
        // (common_utils/src/lib.rs:93-97); no offset is serialized.
        const auto now = std::chrono::system_clock::now();
        const std::int64_t seconds = std::chrono::duration_cast<std::chrono::seconds>(
            now.time_since_epoch()
        ).count();
        const std::int64_t nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(
            now.time_since_epoch()
        ).count() % 1'000'000'000;

        const std::int64_t days = seconds / 86400;
        const std::int64_t sod = seconds % 86400;
        const std::int64_t z = days + 719468;
        const std::int64_t era = (z >= 0 ? z : z - 146096) / 146097;
        const unsigned doe = static_cast<unsigned>(z - era * 146097);
        const unsigned yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
        const std::int64_t year = static_cast<std::int64_t>(yoe) + era * 400;
        const unsigned doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
        const unsigned mp = (5 * doy + 2) / 153;
        const unsigned day = doy - (153 * mp + 2) / 5 + 1;
        const unsigned month = mp < 10 ? mp + 3 : mp - 9;
        const std::int64_t y = year + (month <= 2 ? 1 : 0);
        const unsigned hour = static_cast<unsigned>(sod / 3600);
        const unsigned minute = static_cast<unsigned>((sod % 3600) / 60);
        const unsigned sec = static_cast<unsigned>(sod % 60);
        return std::format(
            "{:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}.{:09d}",
            y, month, day, hour, minute, sec, nanos
        );
    }

    [[nodiscard]] std::string_view vault_key() const noexcept
    {
        return "hyperswitch-vault-wrap-key";
    }

    [[nodiscard]] std::int64_t vault_ttl_seconds() const noexcept
    {
        // Stand-in for conf.locker.ttl_for_storage_in_secs (upstream default: one day).
        return 86400;
    }

    [[nodiscard]] std::string enabled_flag() const noexcept
    {
        // TokenizationFlag is #[serde(rename_all = "snake_case")] at common_enums/src/enums.rs:12123.
        return "enabled";
    }

    [[nodiscard]] std::string disabled_flag() const noexcept
    {
        return "disabled";
    }

    [[nodiscard]] core::error::AppError error(
        core::error::AppErrorKind kind,
        std::string message
    ) const
    {
        return core::error::AppErrorBuilder{}.operator()(kind, std::move(message));
    }

    [[nodiscard]] core::error::AppError upstream_error(
        core::error::AppErrorKind kind,
        std::string_view name,
        std::string message
    ) const
    {
        // AppError serializes its kind_name into the wire "error" field; use the exact
        // upstream ApiErrorResponse variant name (currency.cppm:49-56 precedent).
        auto result = error(kind, std::move(message));
        result.set_kind_name(std::string{name});
        return result;
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

} // namespace app::handlers::tokenization
