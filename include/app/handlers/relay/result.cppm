export module app_handlers_relay:result;

import std;
import core_serde;
import app_dto;

export namespace app::handlers::relay {

// The two DTO-shaped responses inherit their wire fields from the exact DTO the
// old core built and encoded, so field names, order and wrapper types stay
// byte-identical. The Serializable specialization reuses the DTO's FieldDesc
// tuple; those descriptors apply to the derived object just as they do to the
// base object (including RelayResponseData's externally-tagged FieldConverter).
class RelayResult : public app::dto::relay::RelayResponse
{
public:
    RelayResult() = default;

    explicit RelayResult(app::dto::relay::RelayResponse value) noexcept :
        RelayResponse{std::move(value)}
    {
    }
};

class RelayRetrieveResult : public app::dto::relay::RelayResponse
{
public:
    RelayRetrieveResult() = default;

    explicit RelayRetrieveResult(app::dto::relay::RelayResponse value) noexcept :
        RelayResponse{std::move(value)}
    {
    }
};

class UnreferencedRefundResult : public app::dto::unreferenced_refund::UnreferencedRefundResponse
{
public:
    UnreferencedRefundResult() = default;

    explicit UnreferencedRefundResult(
        app::dto::unreferenced_refund::UnreferencedRefundResponse value
    ) noexcept :
        UnreferencedRefundResponse{std::move(value)}
    {
    }
};

// Upstream WebhookResponse::StatusOk is HTTP 200; the typed result is empty and
// serializes to an empty JSON object.
struct RelayWebhookResult
{
    [[nodiscard]] const serde::Value& get_document() const noexcept
    {
        return m_document;
    }

    void set_document(serde::Value value)
    {
        m_document = std::move(value);
    }

private:
    serde::Value m_document{serde::Value::Object{}};
};

} // namespace app::handlers::relay

template<>
struct serde::Serializable<app::handlers::relay::RelayResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<app::dto::relay::RelayResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "relay_result";
    }
};

template<>
struct serde::Serializable<app::handlers::relay::RelayRetrieveResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<app::dto::relay::RelayResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "relay_retrieve_result";
    }
};

template<>
struct serde::Serializable<app::handlers::relay::UnreferencedRefundResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<
            app::dto::unreferenced_refund::UnreferencedRefundResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "unreferenced_refund_result";
    }
};
