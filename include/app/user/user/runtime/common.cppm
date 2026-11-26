export module app_user:user_runtime_common;

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
import app_locking;
import app_services;

import :user_input;
import :user_result;

export namespace app::handlers::user {

/** @brief Shared business helpers for the user route runtimes. */
class UserRuntimeSupport
{
public:
    explicit UserRuntimeSupport(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    // Upstream serialises ApiErrorResponse as
    // {"error":{"type":"...","message":"...","code":"..."}}. kind_name carries
    // "code" (or "api:code" for the "api" type); the handler renders the exact body.
    [[nodiscard]] core::error::AppError
    wire_error(std::string_view code, std::string message, core::error::AppErrorKind kind) const
    {
        auto error = core::error::AppErrorBuilder{}(kind, std::move(message));
        error.set_kind_name(std::string{code});
        return error;
    }

    std::string token_subject(std::string_view token) const
    {
        const auto first = token.find('.');
        if (first == std::string_view::npos) {
            return {};
        }
        const auto second = token.find('.', first + 1);
        if (second == std::string_view::npos) {
            return {};
        }

        static constexpr std::string_view alphabet =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
        std::string decoded;
        for (std::size_t i = first + 1; i < second;) {
            std::uint32_t bits{0};
            int count{0};
            for (; count < 4 && i < second; ++count, ++i) {
                const auto pos = alphabet.find(token[i]);
                if (pos == std::string_view::npos) {
                    return {};
                }
                bits = (bits << 6) | static_cast<std::uint32_t>(pos);
            }
            if (count >= 2) {
                // 4 chars -> 3 bytes, 3 chars -> 2 bytes, 2 chars -> 1 byte.
                for (int byte = count - 2; byte >= 0; --byte) {
                    decoded.push_back(static_cast<char>((bits >> (8 * byte)) & 0xFF));
                }
            }
        }

        const auto key = decoded.find("\"sub\"");
        if (key == std::string::npos) {
            return {};
        }
        const auto colon = decoded.find(':', key + 5);
        const auto open = decoded.find('"', colon == std::string::npos ? key : colon);
        if (open == std::string::npos) {
            return {};
        }
        const auto close = decoded.find('"', open + 1);
        if (close == std::string::npos) {
            return {};
        }
        return decoded.substr(open + 1, close - open - 1);
    }

    /** @brief Epoch seconds as a stable DB timestamp value. */
    std::string now_string() const
    {
        return std::to_string(now_seconds());
    }

    /** @brief Current epoch seconds (single source for TOTP steps and timestamps). */
    std::int64_t now_seconds() const
    {
        return std::chrono::duration_cast<std::chrono::seconds>(
                   std::chrono::system_clock::now().time_since_epoch()
        )
            .count();
    }

    /** @brief Decodes a string claim from the payload of a local JWT.
     *
     *  Ported stand-in for upstream `auth::decode_jwt` claim access: strips the
     *  base64url header/payload, decodes the JSON payload and reads a quoted
     *  string field. Used for purpose/permission-bound token checks.
     */
    std::string jwt_claim(std::string_view token, std::string_view key) const
    {
        const auto first = token.find('.');
        if (first == std::string_view::npos) {
            return {};
        }
        const auto second = token.find('.', first + 1);
        if (second == std::string_view::npos) {
            return {};
        }
        const std::string payload =
            ::app::crypto::Base64Url{}.decode(token.substr(first + 1, second - first - 1));
        const std::string needle = std::string{"\""} + std::string{key} + "\"";
        const auto key_pos = payload.find(needle);
        if (key_pos == std::string::npos) {
            return {};
        }
        auto pos = payload.find(':', key_pos + needle.size());
        if (pos == std::string::npos) {
            return {};
        }
        ++pos;
        while (pos < payload.size() && payload[pos] == ' ') {
            ++pos;
        }
        if (pos >= payload.size() || payload[pos] != '"') {
            return {};
        }
        ++pos;
        const auto end = payload.find('"', pos);
        if (end == std::string::npos) {
            return {};
        }
        return payload.substr(pos, end - pos);
    }

    /** @brief True when a token was blacklisted at signout (upstream BlackList check). */
    bool is_blacklisted(std::string_view token) const
    {
        if (token.empty()) {
            return false;
        }
        return m_context.cache().get("user.blacklist:" + std::string{token}).hit;
    }

    /** @brief Returns true when a comma-separated grant list contains a permission. */
    bool grant_contains(std::string_view grants, std::string_view required) const
    {
        while (!grants.empty()) {
            const auto comma = grants.find(',');
            const auto part = grants.substr(0, comma);
            if (part == required || part == "ADMIN") {
                return true;
            }
            if (comma == std::string_view::npos) {
                break;
            }
            grants = grants.substr(comma + 1);
        }
        return false;
    }

    /** @brief Key-store-backed signer with V2 key and deterministic V1 fallback.
     *
     *  Replaces hardcoded secrets with repository-backed key material. A missing
     *  configured key falls back to a documented deterministic local secret so
     *  local parity keeps a stable token shape without an external KMS.
     */
    ::app::crypto::JwtSigner user_signer() const
    {
        if (!m_context.key_store().get("jwt", "user-jwt-v2")) {
            m_context.key_store()
                .put("jwt", "user-jwt-v2", "HS256", ::app::crypto::SecretString{"user-jwt-secret"});
        }
        return ::app::crypto::JwtSigner{m_context.secret_resolver(), "user-jwt-v2", "jwt"};
    }

    /** @brief Persists a password hash as a key-store record, never in cache. */
    void store_password(std::string_view email, std::string_view hash) const
    {
        if (!m_context.key_store().put(
                "user-password",
                email,
                "FNV1a",
                ::app::crypto::SecretString{std::string{hash}}
            )) {
            m_context.key_store()
                .rotate("user-password", email, ::app::crypto::SecretString{std::string{hash}});
        }
    }

    /** @brief Creates the user, role and user-role rows for a newly provisioned identity. */
    void persist_new_user(
        std::string_view user_id,
        std::string_view email,
        std::string_view name,
        std::string_view status,
        std::string_view merchant_id,
        std::string_view role_name
    ) const
    {
        ::app::store::users::UserRecord user{};
        user.set_user_id(std::string{user_id});
        user.set_email(std::string{email});
        user.set_name(std::string{name});
        user.set_status(std::string{status});
        user.set_role(std::string{role_name});
        user.set_merchant_id(std::string{merchant_id});
        user.set_created_at(now_string());
        if (!m_context.repository().create_user(user).has_value()) {
            if (auto existing = m_context.repository().find_user_by_string(user_id)) {
                existing->set_status(std::string{status});
                existing->set_merchant_id(std::string{merchant_id});
                m_context.repository().update_user(*existing);
            }
        }

        const auto role_id = std::string{"role_"} + std::string{role_name};
        ::app::store::roles::RoleRecord role{};
        role.set_role_id(role_id);
        role.set_role_name(std::string{role_name});
        role.set_entity_type("merchant");
        role.set_merchant_id(std::string{merchant_id});
        role.set_status("active");
        role.set_created_at(now_string());
        (void)m_context.repository().create_role(role);

        ::app::store::user_roles::UserRoleRecord user_role{};
        user_role.set_user_role_id(std::string{"ur_"} + std::string{user_id} + "_" + role_id);
        user_role.set_user_id(std::string{user_id});
        user_role.set_role_id(role_id);
        user_role.set_merchant_id(std::string{merchant_id});
        user_role.set_status("active");
        user_role.set_created_at(now_string());
        (void)m_context.repository().create_user_role(user_role);

        // Email -> user id index used by signin to mint tokens whose `sub` claim
        // matches the persisted user id (role lookups depend on it).
        m_context.cache().put(
            "user.email_index:" + std::string{email},
            core::cache::to_cache_value(std::string{user_id})
        );
    }

    /** @brief Advances an invited user's lifecycle status in the user and role tables. */
    void set_user_status_by_email(std::string_view email, std::string_view status) const
    {
        const auto entry = m_context.cache().get("user.email_index:" + std::string{email});
        if (!entry.hit) {
            return;
        }
        const auto user_id = core::cache::value_as<std::string>(entry.value, std::string{});
        if (auto user = m_context.repository().find_user_by_string(user_id)) {
            user->set_status(std::string{status});
            m_context.repository().update_user(*user);
            for (auto& role: m_context.repository().list_user_roles()) {
                if (role.get_user_id() == user_id) {
                    role.set_status(std::string{status});
                    m_context.repository().update_user_role(role);
                }
            }
        }
    }

    /** @brief Kafka-shaped side-effect record for identity lifecycle transitions. */
    void publish_identity_event(std::string_view topic, std::string_view user_id) const
    {
        m_context.events().publish(
            std::string{topic},
            std::format(R"({{"user_id":"{}"}})", user_id)
        );
    }


private:
    ::app::context::AppContext& m_context;
};

} // namespace app::handlers::user
