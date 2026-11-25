module;
#include <rfl/to_generic.hpp>

export module app_handlers_payment_link:result;

import std;
import core_serde;
import app_model;

export namespace app::handlers::payment_link {

// The two typed responses inherit their wire fields from the exact models the
// old handlers built and encoded, so field names, order and wrapper types stay
// byte-identical. The Serializable specialization reuses the model's FieldDesc
// tuple; those descriptors apply to the derived object just as they do to the
// base object.

class PaymentLinkRetrieveResult : public app::model::payments::RetrievePaymentLinkResponse
{
public:
    PaymentLinkRetrieveResult() = default;

    explicit PaymentLinkRetrieveResult(
        app::model::payments::RetrievePaymentLinkResponse value
    ) noexcept :
        RetrievePaymentLinkResponse{std::move(value)}
    {
    }
};

class PaymentLinkListResult : public app::model::payments::PaymentLinkListResponse
{
public:
    PaymentLinkListResult() = default;

    explicit PaymentLinkListResult(app::model::payments::PaymentLinkListResponse value) noexcept :
        PaymentLinkListResponse{std::move(value)}
    {
    }
};

/** @brief Converts reflected DTOs into typed wire documents (no JSON text). */
class PaymentLinkDocument
{
public:
    template<typename T>
    [[nodiscard]] static serde::Value from(const T& value)
    {
        return rfl::to_generic(value);
    }
};

// Hand-built payment-link wire shapes (form data, status page data, GenericLinks
// wrapper) are carried as typed serde::Value documents built from reflected DTOs
// and typed field assignments. The handler writes each document with
// serde::Ser::serialize; no pre-encoded JSON text crosses the boundary.

class PaymentLinkInitiateResult
{
public:
    PaymentLinkInitiateResult() = default;

    explicit PaymentLinkInitiateResult(serde::Value document) noexcept :
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
    serde::Value m_document{};
};

class PaymentLinkInitiateSecureResult
{
public:
    PaymentLinkInitiateSecureResult() = default;

    explicit PaymentLinkInitiateSecureResult(serde::Value document) noexcept :
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
    serde::Value m_document{};
};

class PaymentLinkStatusResult
{
public:
    PaymentLinkStatusResult() = default;

    explicit PaymentLinkStatusResult(serde::Value document) noexcept :
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
    serde::Value m_document{};
};

} // namespace app::handlers::payment_link

template<>
struct serde::Serializable<app::handlers::payment_link::PaymentLinkRetrieveResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<app::model::payments::RetrievePaymentLinkResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "payment_link_retrieve_result";
    }
};

template<>
struct serde::Serializable<app::handlers::payment_link::PaymentLinkListResult>
{
    static constexpr auto fields()
    {
        return serde::Serializable<app::model::payments::PaymentLinkListResponse>::fields();
    }

    static constexpr std::string_view table_name()
    {
        return "payment_link_list_result";
    }
};
