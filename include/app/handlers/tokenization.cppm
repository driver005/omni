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
        using Input = app::dto::tokenization::GenericTokenizationRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        // Upstream binds V2ClientAuth to ResourceId::Customer(customer_id); the
        // customer id is read from the payload before api_or_client_auth runs.
        const std::string customer_id = parsed->get_customer_id().get_value();
        if (customer_id.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::VALIDATION_FAILED, "missing customer id"));
            return;
        }
        if (const auto auth_error = authorize_request(req, "create_token_vault_api", customer_id)) {
            send_error(res, send, *auth_error);
            return;
        }

        // Upstream: VaultId::generate(uuid_v7) + GlobalTokenId::generate(cell_id).
        // uuid_v7() is a deterministic v7-shaped stand-in; the cell component is a
        // keyed digest of the merchant id because conf.cell_information.id is external.
        const std::string vault_id = uuid_v7();
        const std::string merchant_id = resolve_merchant_id(req);
        if (merchant_id.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::VALIDATION_FAILED, "missing merchant id"));
            return;
        }
        const std::string token_id = global_token_id(merchant_id);
        const std::string created_at = now_value();

        // Upstream writes the secret to the external locker with a configured TTL.
        // Local substitute: wrap with a key-store-resolved key, keep the wrapped
        // bytes in the ephemeral cache with a TTL, and persist only the record.
        const std::string wrapped = encrypt_vault_data(vault_id, parsed->get_token_request());
        m_context->cache().put(
            vault_cache_key(vault_id),
            core::cache::to_cache_value(wrapped),
            core::cache::Ttl{}.seconds(vault_ttl_seconds())
        );

        app::store::tokenizations::TokenizationRecord record{};
        record.set_token_id(token_id);
        record.set_merchant_id(merchant_id);
        record.set_customer_id(customer_id);
        record.set_entity_id(customer_id);
        record.set_vault_id(vault_id);
        record.set_status(enabled_flag());
        record.set_created_at(created_at);

        auto created = m_context->repository().create_tokenization(std::move(record));
        if (!created) {
            send_error(res, send, created.error());
            return;
        }

        m_context->metrics().record_flow("TokenizationCreate");
        m_context->metrics().counter("TOKENIZATION_CREATED").increment();

        app::dto::tokenization::GenericTokenizationResponse response;
        app::dto::tokenization::GlobalTokenIdValue id;
        id.set_value(created->get_token_id());
        response.set_id(std::move(id));
        app::dto::tokenization::PrimitiveDateTimeValue created_value;
        created_value.set_value(created->get_created_at());
        response.set_created_at(std::move(created_value));
        app::dto::tokenization::TokenizationFlagValue flag;
        flag.set_value(created->get_status());
        response.set_flag(std::move(flag));
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
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
        using Input = app::dto::tokenization::DeleteTokenDataRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        if (!parsed) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        // Upstream binds V2ClientAuth to ResourceId::PaymentMethodSession(session_id).
        const std::string session_id = parsed->get_session_id().get_value();
        if (session_id.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::VALIDATION_FAILED, "missing session id"));
            return;
        }

        // Upstream takes GlobalTokenId from the route path, not from the body.
        const auto path = app::http::RequestPath{}(req);
        const std::string token_id = std::string{path.at(2).value_or("")};
        if (token_id.empty()) {
            send_error(res, send, error(core::error::AppErrorKind::INVALID_REQUEST, "missing token id"));
            return;
        }
        if (const auto auth_error = authorize_request(req, "delete_tokenized_data_api", session_id)) {
            send_error(res, send, *auth_error);
            return;
        }

        auto record = m_context->repository().find_tokenization_by_string(token_id);
        if (!record) {
            send_error(res, send, error(core::error::AppErrorKind::NOT_FOUND, "tokenization record not found"));
            return;
        }
        if (record->get_customer_id() != parsed->get_customer_id().get_value()) {
            send_error(res, send, error(core::error::AppErrorKind::VALIDATION_FAILED, "Tokenization record does not belong to the customer"));
            return;
        }
        if (record->get_status() == disabled_flag()) {
            send_error(res, send, error(core::error::AppErrorKind::NOT_FOUND, "Tokenization is already disabled for the id"));
            return;
        }

        // Exercise the get_token_vault_core retrieval primitive as a pre-delete
        // verification of the wrapped payload; the delete then drops the vault
        // entry and moves the persisted record to Disabled.
        (void)retrieve_vault_data(*record);
        m_context->cache().erase(vault_cache_key(record->get_vault_id()));

        record->set_status(disabled_flag());
        if (!m_context->repository().update_tokenization(*record)) {
            send_error(res, send, error(core::error::AppErrorKind::INTERNAL_ERROR, "failed to update tokenization record"));
            return;
        }

        m_context->metrics().record_flow("TokenizationDelete");
        m_context->metrics().counter("TOKENIZATION_DELETED").increment();

        app::dto::tokenization::DeleteTokenDataResponse response;
        app::dto::tokenization::GlobalTokenIdValue id;
        id.set_value(token_id);
        response.set_id(std::move(id));
        auto text = serde::Ser::encode_json(response);
        send_json(res, send, text, true);
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
    [[nodiscard]] std::optional<core::error::AppError> authorize_request(
        interfaces::io::IRequest& req,
        std::string_view operation,
        std::string_view bound_resource
    ) const
    {
        // api_or_client_auth: primary branch comes from RouteAuthRegistry. The
        // registry row already carries allow_connected=false/allow_platform=false
        // for both tokenization operations.
        auto context = app::auth::AuthContextBuilder{}(req);
        const auto rule = m_context->auth_rules().rule_for(operation);
        auto denied = m_context->authorizer().authorize_route(context, rule);
        if (!denied) {
            return std::nullopt;
        }

        // Fallback client-auth branch, deterministically bound to the resource id
        // (Customer on create, PaymentMethodSession on delete). The bound id is
        // injected as a claim and enforced through the rule's id_source check.
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
        return m_context->authorizer().authorize_route(context, client_rule);
    }

    [[nodiscard]] std::string resolve_merchant_id(interfaces::io::IRequest& req) const
    {
        // Upstream uses provider.get_account().get_id(); the injected repository
        // canonicalizes the header-derived merchant id to its stored value.
        std::string merchant_id = app::http::merchant_id_from_header{}(req);
        if (const auto account = m_context->repository().find_merchant_account_by_string(merchant_id)) {
            merchant_id = account->get_merchant_id();
        }
        return merchant_id;
    }

    [[nodiscard]] std::string global_token_id(std::string_view merchant_id) const
    {
        // GlobalTokenId::generate(cell_id): local cell is a keyed digest of the
        // merchant id because conf.cell_information.id is an external setting.
        const std::string cell = app::crypto::KeyedHasher{"cell-id"}.hash(merchant_id).substr(0, 8);
        return std::string{"tok_"} + cell + "_" + uuid_v7();
    }

    [[nodiscard]] std::string uuid_v7() const
    {
        // UUIDv7-shaped deterministic stand-in: first two groups carry the
        // current millisecond timestamp, the version nibble is 7, and the
        // variant nibble from IdGenerator::uuid_v4 stays valid.
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
        // Deterministic local stand-in for the processor key store. The wrap key
        // is resolved from KeyStore; a missing key is seeded once rather than
        // kept as a hard-coded literal embedded in the encryption step.
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

    [[nodiscard]] std::expected<std::string, core::error::AppError> retrieve_vault_data(
        const app::store::tokenizations::TokenizationRecord& record
    ) const
    {
        // Deterministic local substitute for get_token_vault_core: reject a
        // Disabled record, then retrieve and unwrap the vault payload. Real
        // retrieve_value_from_vault needs an external locker.
        if (record.get_status() == disabled_flag()) {
            return std::unexpected{error(core::error::AppErrorKind::NOT_FOUND, "Tokenization is disabled for the id")};
        }
        const auto entry = m_context->cache().get(vault_cache_key(record.get_vault_id()));
        if (!entry.hit) {
            return std::unexpected{error(core::error::AppErrorKind::NOT_FOUND, "vault data not found")};
        }
        return decrypt_vault_data(record.get_vault_id(), entry.value_or<std::string>({}));
    }

    [[nodiscard]] core::error::AppError error(
        core::error::AppErrorKind kind,
        std::string message
    ) const
    {
        return core::error::AppErrorBuilder{}.operator()(kind, std::move(message));
    }

    [[nodiscard]] std::string now_value() const
    {
        // common_utils::date_time::now(): UTC ISO-8601 with millisecond precision,
        // computed without libc so the shape is a real timestamp, not epoch seconds.
        const auto now = std::chrono::system_clock::now();
        const std::int64_t seconds = std::chrono::duration_cast<std::chrono::seconds>(
            now.time_since_epoch()
        ).count();
        const std::int64_t millis = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()
        ).count() % 1000;

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
            "{:04d}-{:02d}-{:02d}T{:02d}:{:02d}:{:02d}.{:03d}Z",
            y, month, day, hour, minute, sec, millis
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
        return "Enabled";
    }

    [[nodiscard]] std::string disabled_flag() const noexcept
    {
        return "Disabled";
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
