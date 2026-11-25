module;
#include <rfl/to_generic.hpp>

export module app_handlers_pm_auth:result;

import std;
import core_serde;
import app_dto;

export namespace app::handlers::pm_auth {

/** @brief Builds typed dynamic response documents; no hand-written JSON strings. */
class ResponseDocument
{
public:
    [[nodiscard]] static serde::Value empty() noexcept
    {
        return serde::Value{serde::Value::Object{}};
    }

    template<typename T>
    [[nodiscard]] static serde::Value from(const T& value)
    {
        return rfl::to_generic(value);
    }
};

// Link token response inherits its wire fields from the exact DTO the old route
// built and encoded, so field names, order and wrapper types stay byte-identical.
class LinkTokenCreateResult : public app::dto::pm_auth::LinkTokenCreateResponse
{
public:
    LinkTokenCreateResult() = default;

    explicit LinkTokenCreateResult(app::dto::pm_auth::LinkTokenCreateResponse value) noexcept :
        LinkTokenCreateResponse{std::move(value)}
    {
    }
};

// Upstream exchange_token_core returns ApplicationResponse::StatusOk: HTTP 200
// with an empty body. That empty body is carried as a typed empty serde::Value
// document, serialized by the handler with Ser::serialize.
class ExchangeTokenResult
{
public:
    ExchangeTokenResult() = default;

    explicit ExchangeTokenResult(serde::Value document) noexcept :
        m_document{std::move(document)}
    {
    }

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

} // namespace app::handlers::pm_auth

template<>
struct serde::Serializable<app::handlers::pm_auth::LinkTokenCreateResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<app::dto::pm_auth::LinkTokenCreateResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "link_token_create_result";
    }
};
