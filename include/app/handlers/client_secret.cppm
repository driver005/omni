module;
#include <ctime>
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
        const auto rule = m_context->auth_rules().rule_for("client_secret_create");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const auto body = req.body_as_string();
        if (body.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::EMPTY_REQUEST, "empty request"));
            return;
        }

        using Input = app::dto::client_secret::ClientSecretCreateRequest;
        auto parsed = serde::Ser::deserialize<Input>("application/json", body);
        // Upstream wires ResourceId as {"Customer":"cus_..."}. The shared DTO flattens
        // it to a valueless enum, so read the variant from the parsed JSON tree instead
        // of scanning raw text — this keeps the input schema-driven.
        const auto customer_id = customer_id_from_body(body);
        if (!parsed && !customer_id) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_JSON, "invalid JSON"));
            return;
        }

        // Upstream resolves the global customer id through the platform key store and
        // maps a miss to CustomerNotFound. Local deterministic substitute: consult the
        // key-store mapping when present, then resolve via the customer repository.
        if (customer_id) {
            std::string resolved = *customer_id;
            if (const auto mapped = m_context->key_store().get("customer-global-id", *customer_id)) {
                resolved = mapped->value();
            }
            if (!m_context->repository().find_customer_by_string(resolved)) {
                send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "Customer not found"));
                return;
            }
        }

        const auto validity_hours = eph_validity_hours();
        const std::int64_t now = now_seconds();

        // Upstream uses ClientSecretId::generate() ("csi_" prefix) and a time-ordered
        // id ("cs_<uuidv7>") for the secret. IdGenerator has no v7 path, so build a
        // time-prefixed deterministic id from the millisecond clock plus the
        // process id counter.
        const std::string client_secret_id = m_context->ids().payment_id("csi_");
        const std::string merchant_id = app::http::merchant_id_from_header{}(req);
        const std::string secret = time_ordered_id("cs");

        using Response = app::dto::client_secret::ClientSecretResponse;
        Response response{};

        app::dto::client_secret::ClientSecretIdValue id;
        id.set_value(client_secret_id);
        response.set_id(std::move(id));
        response.set_resource_id(app::dto::client_secret::ResourceId::Customer);

        app::dto::client_secret::PrimitiveDateTimeValue created_at;
        created_at.set_value(primitive_datetime(now));
        response.set_created_at(std::move(created_at));
        app::dto::client_secret::PrimitiveDateTimeValue expires;
        expires.set_value(primitive_datetime(now + validity_hours * 3600));
        response.set_expires(std::move(expires));
        response.set_secret(secret);

        // Client secrets are ephemeral upstream (Redis hash "csh", keys "cs_<id>" and
        // "cs_<secret>", expire-at = expires). The cache is that ephemeral store here.
        // A single serialized record is written under both keys with the configured
        // validity; a keyed lock makes the HSETNX duplicate check deterministic and
        // a hit maps to StorageError::DuplicateValue (ALREADY_EXISTS / 409).
        const auto encoded = serde::Ser::encode_json(response);
        const auto ttl = core::cache::Ttl{}.hours(validity_hours);
        const std::string lock_key = "client_secret.create:" + merchant_id + ":" + client_secret_id;
        const auto lock = m_context->locks().acquire(lock_key, std::int64_t{30});
        if (!lock) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::ALREADY_EXISTS, "client secret already exists"));
            return;
        }
        if (m_context->cache().get(record_key(client_secret_id)).hit ||
            m_context->cache().get(secret_key(secret)).hit) {
            m_context->locks().release(lock_key);
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::ALREADY_EXISTS, "client secret already exists"));
            return;
        }
        m_context->cache().put(record_key(client_secret_id), core::cache::to_cache_value(encoded), ttl);
        m_context->cache().put(secret_key(secret), core::cache::to_cache_value(encoded), ttl);
        m_context->locks().release(lock_key);

        m_context->metrics().record_flow("ClientSecretCreate");
        m_context->metrics().counter("client_secret_create").increment();

        send_json(res, send, encoded, true);
    }

    void handle_client_secret_delete(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send
    )
    {
        const auto rule = m_context->auth_rules().rule_for("client_secret_delete");
        const auto auth = m_context->authorizer().authorize_route(app::auth::AuthContextBuilder{}(req), rule);
        if (auth) { send_error(res, send, *auth); return; }

        const std::string client_secret_id = client_secret_id_from_request(req);
        if (client_secret_id.empty()) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INVALID_REQUEST, "missing client secret id"));
            return;
        }

        // Prefixed key lookup ("cs_<id>") is the local get_client_secret equivalent.
        const auto cached = m_context->cache().get(record_key(client_secret_id));
        if (!cached.hit) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::NOT_FOUND, "Client secret not found"));
            return;
        }

        using Response = app::dto::client_secret::ClientSecretResponse;
        const std::string stored = cached.value_or<std::string>(std::string{});
        auto response = serde::Ser::deserialize<Response>("application/json", stored);
        if (!response) {
            send_error(res, send, core::error::AppErrorBuilder{}.operator()(core::error::AppErrorKind::INTERNAL_ERROR, "stored client secret was corrupt"));
            return;
        }

        // Upstream delete removes both the id key and the secret key, then returns the
        // deleted record.
        m_context->cache().erase(record_key(client_secret_id));
        m_context->cache().erase(secret_key(response->get_secret()));

        m_context->metrics().record_flow("ClientSecretDelete");
        m_context->metrics().counter("client_secret_delete").increment();

        auto text = serde::Ser::encode_json(*response);
        send_json(res, send, text, true);
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

    [[nodiscard]] std::optional<std::string> customer_id_from_body(std::string_view body) const
    {
        auto tree = serde::Ser::decode_generic("application/json", body);
        if (!tree) {
            return std::nullopt;
        }
        const auto* root = std::get_if<interfaces::Value::Object>(&tree->get());
        if (root == nullptr) {
            return std::nullopt;
        }
        const auto resource = root->get("resource_id");
        if (!resource) {
            return std::nullopt;
        }
        const auto* variant = std::get_if<interfaces::Value::Object>(&resource->get());
        if (variant == nullptr) {
            return std::nullopt;
        }
        const auto customer = variant->get("Customer");
        if (!customer) {
            return std::nullopt;
        }
        const auto* text = std::get_if<std::string>(&customer->get());
        if (text == nullptr || text->empty()) {
            return std::nullopt;
        }
        return std::string{*text};
    }

    [[nodiscard]] std::string time_ordered_id(std::string_view prefix) const
    {
        const auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();
        char buffer[32]{};
        const auto converted = std::to_chars(buffer, buffer + sizeof(buffer),
            static_cast<std::uint64_t>(millis), 16);
        std::string out{prefix};
        out.push_back('_');
        out.append(buffer, converted.ptr);
        out.push_back('_');
        out.append(m_context->ids().payment_id(""));
        return out;
    }

    [[nodiscard]] std::int64_t now_seconds() const
    {
        return std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();
    }

    [[nodiscard]] std::string primitive_datetime(std::int64_t seconds) const
    {
        const auto raw = static_cast<std::time_t>(seconds);
        std::tm parts{};
        gmtime_r(&raw, &parts);
        std::ostringstream out;
        out << std::put_time(&parts, "%Y-%m-%dT%H:%M:%S") << ".000";
        return out.str();
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

    [[nodiscard]] std::string record_key(std::string_view id) const
    {
        return std::string{"cs_"} + std::string{id};
    }

    [[nodiscard]] std::string secret_key(std::string_view secret) const
    {
        // Never embed the raw secret in a cache key; use domain-separated hash material.
        return std::string{"cs_"} + app::crypto::SecretString{std::string{secret}}.cache_key();
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
