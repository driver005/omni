export module app_handlers_dummy_connector:result;

import std;
import core_serde;

export namespace app::handlers::dummy_connector {

/**
 * @brief HTML payload for the authorize page. The handler encodes it through
 * Ser::serialize; the wire body bytes change from the original raw HTML.
 */
class DummyConnectorAuthorizePaymentResult
{
public:
    DummyConnectorAuthorizePaymentResult() = default;

    explicit DummyConnectorAuthorizePaymentResult(std::string html) noexcept :
        m_html{std::move(html)}
    {
    }

    [[nodiscard]] const std::string& get_html() const noexcept
    {
        return m_html;
    }

    void set_html(std::string value)
    {
        m_html = std::move(value);
    }

private:
    std::string m_html{};
};

/**
 * @brief 302 redirection body (return_url / params / return_url_with_query_params
 * / http_method / headers) built as a serde::Value document; Location is kept
 * as a separate typed header value.
 */
class DummyConnectorCompletePaymentResult
{
public:
    DummyConnectorCompletePaymentResult() = default;

    DummyConnectorCompletePaymentResult(serde::Value document, std::string location) noexcept :
        m_document{std::move(document)},
        m_location{std::move(location)}
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

    [[nodiscard]] const std::string& get_location() const noexcept
    {
        return m_location;
    }

    void set_location(std::string value)
    {
        m_location = std::move(value);
    }

private:
    serde::Value m_document{};
    std::string m_location{};
};

/**
 * @brief Payment response document. Externally-tagged payment_method_type and
 * optional next_action make a serde::Value the natural typed carrier.
 */
class DummyConnectorPaymentResult
{
public:
    DummyConnectorPaymentResult() = default;

    explicit DummyConnectorPaymentResult(serde::Value document) noexcept :
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

/** @brief Same typed payment-response document for the retrieve route. */
class DummyConnectorPaymentDataResult
{
public:
    DummyConnectorPaymentDataResult() = default;

    explicit DummyConnectorPaymentDataResult(serde::Value document) noexcept :
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

/** @brief Flat refund response DTO (status / id / currency / created / amounts). */
class DummyConnectorRefundResult
{
public:
    DummyConnectorRefundResult() = default;

    DummyConnectorRefundResult(
        std::string status,
        std::string id,
        std::string currency,
        std::string created,
        std::int64_t payment_amount,
        std::int64_t refund_amount
    ) noexcept :
        m_status{std::move(status)},
        m_id{std::move(id)},
        m_currency{std::move(currency)},
        m_created{std::move(created)},
        m_payment_amount{payment_amount},
        m_refund_amount{refund_amount}
    {
    }

    [[nodiscard]] const std::string& get_status() const noexcept
    {
        return m_status;
    }

    void set_status(std::string value)
    {
        m_status = std::move(value);
    }

    [[nodiscard]] const std::string& get_id() const noexcept
    {
        return m_id;
    }

    void set_id(std::string value)
    {
        m_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_currency() const noexcept
    {
        return m_currency;
    }

    void set_currency(std::string value)
    {
        m_currency = std::move(value);
    }

    [[nodiscard]] const std::string& get_created() const noexcept
    {
        return m_created;
    }

    void set_created(std::string value)
    {
        m_created = std::move(value);
    }

    [[nodiscard]] std::int64_t get_payment_amount() const noexcept
    {
        return m_payment_amount;
    }

    void set_payment_amount(std::int64_t value)
    {
        m_payment_amount = value;
    }

    [[nodiscard]] std::int64_t get_refund_amount() const noexcept
    {
        return m_refund_amount;
    }

    void set_refund_amount(std::int64_t value)
    {
        m_refund_amount = value;
    }

private:
    std::string m_status{};
    std::string m_id{};
    std::string m_currency{};
    std::string m_created{};
    std::int64_t m_payment_amount{0};
    std::int64_t m_refund_amount{0};
};

/** @brief Same flat refund response DTO for the retrieve route. */
class DummyConnectorRefundDataResult
{
public:
    DummyConnectorRefundDataResult() = default;

    DummyConnectorRefundDataResult(
        std::string status,
        std::string id,
        std::string currency,
        std::string created,
        std::int64_t payment_amount,
        std::int64_t refund_amount
    ) noexcept :
        m_status{std::move(status)},
        m_id{std::move(id)},
        m_currency{std::move(currency)},
        m_created{std::move(created)},
        m_payment_amount{payment_amount},
        m_refund_amount{refund_amount}
    {
    }

    [[nodiscard]] const std::string& get_status() const noexcept
    {
        return m_status;
    }

    void set_status(std::string value)
    {
        m_status = std::move(value);
    }

    [[nodiscard]] const std::string& get_id() const noexcept
    {
        return m_id;
    }

    void set_id(std::string value)
    {
        m_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_currency() const noexcept
    {
        return m_currency;
    }

    void set_currency(std::string value)
    {
        m_currency = std::move(value);
    }

    [[nodiscard]] const std::string& get_created() const noexcept
    {
        return m_created;
    }

    void set_created(std::string value)
    {
        m_created = std::move(value);
    }

    [[nodiscard]] std::int64_t get_payment_amount() const noexcept
    {
        return m_payment_amount;
    }

    void set_payment_amount(std::int64_t value)
    {
        m_payment_amount = value;
    }

    [[nodiscard]] std::int64_t get_refund_amount() const noexcept
    {
        return m_refund_amount;
    }

    void set_refund_amount(std::int64_t value)
    {
        m_refund_amount = value;
    }

private:
    std::string m_status{};
    std::string m_id{};
    std::string m_currency{};
    std::string m_created{};
    std::int64_t m_payment_amount{0};
    std::int64_t m_refund_amount{0};
};

} // namespace app::handlers::dummy_connector

template<>
struct serde::Serializable<::app::handlers::dummy_connector::DummyConnectorRefundResult>
{
    static constexpr auto fields()
    {
        using T = ::app::handlers::dummy_connector::DummyConnectorRefundResult;
        return std::tuple{
            serde::FieldDesc<"status", &T::get_status, &T::set_status, serde::FieldOptions{}>{},
            serde::FieldDesc<"id", &T::get_id, &T::set_id, serde::FieldOptions{}>{},
            serde::
                FieldDesc<"currency", &T::get_currency, &T::set_currency, serde::FieldOptions{}>{},
            serde::FieldDesc<"created", &T::get_created, &T::set_created, serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_amount",
                &T::get_payment_amount,
                &T::set_payment_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "refund_amount",
                &T::get_refund_amount,
                &T::set_refund_amount,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "dummy_connector_refund_result";
    }
};

template<>
struct serde::Serializable<::app::handlers::dummy_connector::DummyConnectorRefundDataResult>
{
    static constexpr auto fields()
    {
        using T = ::app::handlers::dummy_connector::DummyConnectorRefundDataResult;
        return std::tuple{
            serde::FieldDesc<"status", &T::get_status, &T::set_status, serde::FieldOptions{}>{},
            serde::FieldDesc<"id", &T::get_id, &T::set_id, serde::FieldOptions{}>{},
            serde::
                FieldDesc<"currency", &T::get_currency, &T::set_currency, serde::FieldOptions{}>{},
            serde::FieldDesc<"created", &T::get_created, &T::set_created, serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_amount",
                &T::get_payment_amount,
                &T::set_payment_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "refund_amount",
                &T::get_refund_amount,
                &T::set_refund_amount,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "dummy_connector_refund_data_result";
    }
};
