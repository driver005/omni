module;
export module app_dto:client_secret;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::dto::client_secret {

/**
 * @brief Typed `ResourceId` newtype carrying the global customer id.
 *
 * Upstream (api_models/client_secret.rs:9-12) is an externally-tagged enum:
 * `ResourceId::Customer(GlobalCustomerId)` rides the wire as `{"Customer":"cus_..."}`.
 * The single reflected field is the variant name, so the shared serde layer emits and
 * consumes exactly that shape (no raw-text scanning in the handler).
 */
class ResourceId
{
public:
    void set_customer_id(std::string v)
    {
        customer_id = std::move(v);
    }

    [[nodiscard]] const std::string& get_customer_id() const noexcept
    {
        return customer_id;
    }

    [[nodiscard]] bool is_customer() const noexcept
    {
        return !customer_id.empty();
    }

    std::string customer_id{};
};

/** @brief `ClientSecretCreateRequest`: `{"resource_id":{"Customer":"cus_..."}}`. */
class ClientSecretCreateRequest
{
public:
    void set_resource_id(ResourceId value)
    {
        resource_id = std::move(value);
    }

    [[nodiscard]] const ResourceId& get_resource_id() const noexcept
    {
        return resource_id;
    }

    ResourceId resource_id{};
};

/**
 * @brief `ClientSecretResponse` wire shape (api_models/client_secret.rs:24-39).
 * Field order mirrors the upstream struct: id, resource_id, created_at, expires, secret.
 * `id`/`secret` are plain strings (upstream `ClientSecretId`/`Secret<String>`);
 * `created_at`/`expires` carry the exact `time::PrimitiveDateTime` wire string.
 */
class ClientSecretResponse
{
public:
    void set_id(std::string value)
    {
        id = std::move(value);
    }

    [[nodiscard]] const std::string& get_id() const noexcept
    {
        return id;
    }

    void set_resource_id(ResourceId value)
    {
        resource_id = std::move(value);
    }

    [[nodiscard]] const ResourceId& get_resource_id() const noexcept
    {
        return resource_id;
    }

    void set_created_at(std::string value)
    {
        created_at = std::move(value);
    }

    [[nodiscard]] const std::string& get_created_at() const noexcept
    {
        return created_at;
    }

    void set_expires(std::string value)
    {
        expires = std::move(value);
    }

    [[nodiscard]] const std::string& get_expires() const noexcept
    {
        return expires;
    }

    void set_secret(std::string value)
    {
        secret = std::move(value);
    }

    [[nodiscard]] const std::string& get_secret() const noexcept
    {
        return secret;
    }

    std::string id{};
    ResourceId resource_id{};
    std::string created_at{};
    std::string expires{};
    std::string secret{};
};

/**
 * @brief Local stand-in for the Redis hash record `ClientSecretType`
 * (diesel_models/src/client_secret.rs:12-21). Field order is the upstream struct order.
 */
class ClientSecretType
{
public:
    void set_id(std::string value)
    {
        id = std::move(value);
    }

    [[nodiscard]] const std::string& get_id() const noexcept
    {
        return id;
    }

    void set_merchant_id(std::string value)
    {
        merchant_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_merchant_id() const noexcept
    {
        return merchant_id;
    }

    void set_resource_id(ResourceId value)
    {
        resource_id = std::move(value);
    }

    [[nodiscard]] const ResourceId& get_resource_id() const noexcept
    {
        return resource_id;
    }

    void set_created_at(std::string value)
    {
        created_at = std::move(value);
    }

    [[nodiscard]] const std::string& get_created_at() const noexcept
    {
        return created_at;
    }

    void set_expires(std::string value)
    {
        expires = std::move(value);
    }

    [[nodiscard]] const std::string& get_expires() const noexcept
    {
        return expires;
    }

    void set_secret(std::string value)
    {
        secret = std::move(value);
    }

    [[nodiscard]] const std::string& get_secret() const noexcept
    {
        return secret;
    }

    std::string id{};
    std::string merchant_id{};
    ResourceId resource_id{};
    std::string created_at{};
    std::string expires{};
    std::string secret{};
};

} // namespace app::dto::client_secret

template<>
struct serde::Serializable<app::dto::client_secret::ResourceId>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "Customer",
            &app::dto::client_secret::ResourceId::get_customer_id,
            &app::dto::client_secret::ResourceId::set_customer_id,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "client_secret_resource_id";
    }
};

template<>
struct serde::Serializable<app::dto::client_secret::ClientSecretCreateRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "resource_id",
            &app::dto::client_secret::ClientSecretCreateRequest::get_resource_id,
            &app::dto::client_secret::ClientSecretCreateRequest::set_resource_id,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "client_secret_create_request";
    }
};

template<>
struct serde::Serializable<app::dto::client_secret::ClientSecretResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &app::dto::client_secret::ClientSecretResponse::get_id,
                &app::dto::client_secret::ClientSecretResponse::set_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "resource_id",
                &app::dto::client_secret::ClientSecretResponse::get_resource_id,
                &app::dto::client_secret::ClientSecretResponse::set_resource_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created_at",
                &app::dto::client_secret::ClientSecretResponse::get_created_at,
                &app::dto::client_secret::ClientSecretResponse::set_created_at,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "expires",
                &app::dto::client_secret::ClientSecretResponse::get_expires,
                &app::dto::client_secret::ClientSecretResponse::set_expires,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "secret",
                &app::dto::client_secret::ClientSecretResponse::get_secret,
                &app::dto::client_secret::ClientSecretResponse::set_secret,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "client_secret_response";
    }
};

template<>
struct serde::Serializable<app::dto::client_secret::ClientSecretType>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &app::dto::client_secret::ClientSecretType::get_id,
                &app::dto::client_secret::ClientSecretType::set_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_id",
                &app::dto::client_secret::ClientSecretType::get_merchant_id,
                &app::dto::client_secret::ClientSecretType::set_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "resource_id",
                &app::dto::client_secret::ClientSecretType::get_resource_id,
                &app::dto::client_secret::ClientSecretType::set_resource_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created_at",
                &app::dto::client_secret::ClientSecretType::get_created_at,
                &app::dto::client_secret::ClientSecretType::set_created_at,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "expires",
                &app::dto::client_secret::ClientSecretType::get_expires,
                &app::dto::client_secret::ClientSecretType::set_expires,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "secret",
                &app::dto::client_secret::ClientSecretType::get_secret,
                &app::dto::client_secret::ClientSecretType::set_secret,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "client_secret_type";
    }
};

#ifdef CONGELADO_TEST
namespace app::dto::client_secret::tests {
using namespace boost::ut;
suite<"app_dto_client_secret"> app_dto_client_secret_suite = []
{
    "reflects converted types"_test = []
    {
        expect(
            std::tuple_size_v<
                decltype(serde::Serializable<
                         app::dto::client_secret::ClientSecretCreateRequest>::fields())> > 0
        );
        expect(
            std::tuple_size_v<
                decltype(serde::Serializable<app::dto::client_secret::ResourceId>::fields())> == 1
        );
    };
    "resource id carries the Customer newtype"_test = []
    {
        ResourceId id{};
        id.set_customer_id("cus_123");
        expect(id.is_customer());
        expect(id.get_customer_id() == "cus_123");
    };
};
} // namespace app::dto::client_secret::tests
#endif
