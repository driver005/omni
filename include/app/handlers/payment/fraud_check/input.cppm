export module app_payment:fraud_check_input;

import std;
import interfaces;
import core_serde;
import core_error;
import app_context;
import app_auth;
import app_http;

export namespace app::handlers::fraud_check {

/**
 * @brief Typed mirror of upstream `Address`
 * (hyperswitch_domain_models/src/router_request_types/fraud_check.rs:184-194).
 */
class FrmAddressWire
{
public:
    void set_street_address(std::string value)
    {
        street_address = std::move(value);
    }

    [[nodiscard]] const std::string& get_street_address() const noexcept
    {
        return street_address;
    }

    void set_unit(std::optional<std::string> value)
    {
        unit = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_unit() const noexcept
    {
        return unit;
    }

    void set_postal_code(std::string value)
    {
        postal_code = std::move(value);
    }

    [[nodiscard]] const std::string& get_postal_code() const noexcept
    {
        return postal_code;
    }

    void set_city(std::string value)
    {
        city = std::move(value);
    }

    [[nodiscard]] const std::string& get_city() const noexcept
    {
        return city;
    }

    void set_province_code(std::string value)
    {
        province_code = std::move(value);
    }

    [[nodiscard]] const std::string& get_province_code() const noexcept
    {
        return province_code;
    }

    void set_country_code(std::string value)
    {
        country_code = std::move(value);
    }

    [[nodiscard]] const std::string& get_country_code() const noexcept
    {
        return country_code;
    }

    std::string street_address{};
    std::optional<std::string> unit{};
    std::string postal_code{};
    std::string city{};
    std::string province_code{};
    std::string country_code{};
};

/**
 * @brief Typed mirror of upstream `Destination` (fraud_check.rs:174-182).
 */
class FrmDestinationWire
{
public:
    void set_full_name(std::string value)
    {
        full_name = std::move(value);
    }

    [[nodiscard]] const std::string& get_full_name() const noexcept
    {
        return full_name;
    }

    void set_organization(std::optional<std::string> value)
    {
        organization = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_organization() const noexcept
    {
        return organization;
    }

    void set_email(std::optional<std::string> value)
    {
        email = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_email() const noexcept
    {
        return email;
    }

    void set_address(FrmAddressWire value)
    {
        address = std::move(value);
    }

    [[nodiscard]] const FrmAddressWire& get_address() const noexcept
    {
        return address;
    }

    std::string full_name{};
    std::optional<std::string> organization{};
    std::optional<std::string> email{};
    FrmAddressWire address{};
};

/**
 * @brief Typed mirror of upstream `Product` (fraud_check.rs:165-172).
 */
class FrmProductWire
{
public:
    void set_item_name(std::string value)
    {
        item_name = std::move(value);
    }

    [[nodiscard]] const std::string& get_item_name() const noexcept
    {
        return item_name;
    }

    void set_item_quantity(std::int64_t value)
    {
        item_quantity = value;
    }

    [[nodiscard]] std::int64_t get_item_quantity() const noexcept
    {
        return item_quantity;
    }

    void set_item_id(std::string value)
    {
        item_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_item_id() const noexcept
    {
        return item_id;
    }

    std::string item_name{};
    std::int64_t item_quantity{};
    std::string item_id{};
};

/**
 * @brief Typed mirror of upstream `Fulfillments` (fraud_check.rs:139-152).
 */
class FrmFulfillmentWire
{
public:
    void set_shipment_id(std::string value)
    {
        shipment_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_shipment_id() const noexcept
    {
        return shipment_id;
    }

    void set_products(std::optional<std::vector<FrmProductWire>> value)
    {
        products = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<FrmProductWire>>& get_products() const noexcept
    {
        return products;
    }

    void set_destination(FrmDestinationWire value)
    {
        destination = std::move(value);
    }

    [[nodiscard]] const FrmDestinationWire& get_destination() const noexcept
    {
        return destination;
    }

    std::string shipment_id{};
    std::optional<std::vector<FrmProductWire>> products{};
    FrmDestinationWire destination{};
};

/**
 * @brief Typed mirror of upstream `FrmFulfillmentRequest`
 * (hyperswitch_domain_models/src/router_request_types/fraud_check.rs:104-137).
 * `#[serde(deny_unknown_fields)]` is enforced locally by @ref parse_and_validate_request.
 */
class FrmFulfillmentRequestWire
{
public:
    void set_payment_id(std::string value)
    {
        payment_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_payment_id() const noexcept
    {
        return payment_id;
    }

    void set_order_id(std::string value)
    {
        order_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_order_id() const noexcept
    {
        return order_id;
    }

    void set_fulfillment_status(std::optional<std::string> value)
    {
        fulfillment_status = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_fulfillment_status() const noexcept
    {
        return fulfillment_status;
    }

    void set_fulfillments(std::vector<FrmFulfillmentWire> value)
    {
        fulfillments = std::move(value);
    }

    [[nodiscard]] const std::vector<FrmFulfillmentWire>& get_fulfillments() const noexcept
    {
        return fulfillments;
    }

    void set_tracking_company(std::optional<std::string> value)
    {
        tracking_company = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_tracking_company() const noexcept
    {
        return tracking_company;
    }

    void set_tracking_numbers(std::optional<std::vector<std::string>> value)
    {
        tracking_numbers = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<std::string>>&
    get_tracking_numbers() const noexcept
    {
        return tracking_numbers;
    }

    void set_tracking_urls(std::optional<std::vector<std::string>> value)
    {
        tracking_urls = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<std::string>>& get_tracking_urls() const noexcept
    {
        return tracking_urls;
    }

    void set_carrier(std::optional<std::string> value)
    {
        carrier = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_carrier() const noexcept
    {
        return carrier;
    }

    void set_fulfillment_method(std::optional<std::string> value)
    {
        fulfillment_method = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_fulfillment_method() const noexcept
    {
        return fulfillment_method;
    }

    void set_shipment_status(std::optional<std::string> value)
    {
        shipment_status = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_shipment_status() const noexcept
    {
        return shipment_status;
    }

    void set_shipped_at(std::optional<std::string> value)
    {
        shipped_at = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_shipped_at() const noexcept
    {
        return shipped_at;
    }

    std::string payment_id{};
    std::string order_id{};
    std::optional<std::string> fulfillment_status{};
    std::vector<FrmFulfillmentWire> fulfillments{};
    std::optional<std::string> tracking_company{};
    std::optional<std::vector<std::string>> tracking_numbers{};
    std::optional<std::vector<std::string>> tracking_urls{};
    std::optional<std::string> carrier{};
    std::optional<std::string> fulfillment_method{};
    std::optional<std::string> shipment_status{};
    std::optional<std::string> shipped_at{};
};

/**
 * @brief Parse/auth support for the fraud-check handler.
 *
 * Turns request data into the plain Input consumed by the domain runtime; no business
 * logic lives here.
 */
class FrmInputSupport
{
public:
    explicit FrmInputSupport(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    // routes/fraud_check.rs:11-32 -- server_wrap(Flow::FrmFulfillment, ApiKeyAuth{false,false},
    // LockAction::NotApplicable). The declarative rule is the local stand-in for the scheme;
    // the two override flags are enforced here exactly as upstream declares them.
    [[nodiscard]] std::optional<core::error::AppError>
    authorize_frm_fulfillment(interfaces::io::IRequest& req) const
    {
        const auto rule = m_context.auth_rules().rule_for("frm_fulfillment");
        const auto auth_ctx = ::app::auth::AuthContextBuilder{}(req);
        if (const auto auth = m_context.authorizer().authorize_route(auth_ctx, rule)) {
            return *auth;
        }
        if (const auto connected = auth_ctx.claim(::app::auth::kClaimConnectedMerchantId);
            connected.has_value() && !connected->empty()) {
            return core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::FORBIDDEN,
                "The provided API key does not support connected scope operations for this flow"
            );
        }
        if (const auto internal = auth_ctx.claim(::app::auth::kClaimInternalApiKey);
            internal.has_value() && !internal->empty()) {
            return core::error::AppErrorBuilder{}(
                core::error::AppErrorKind::FORBIDDEN,
                "The provided API key does not support platform self-operations for this flow"
            );
        }
        return std::nullopt;
    }

    // Rejects unknown root fields (serde deny_unknown_fields, fraud_check.rs:105) and enforces
    // the required fields of FrmFulfillmentRequest / Fulfillments / Destination / Address.
    [[nodiscard]] std::optional<FrmFulfillmentRequestWire>
    parse_and_validate_request(std::string_view body) const
    {
        auto generic = serde::Ser::decode_generic("application/json", body);
        if (!generic) {
            return std::nullopt;
        }
        const auto* root = std::get_if<interfaces::Value::Object>(&generic->get());
        if (root == nullptr) {
            return std::nullopt;
        }
        static constexpr std::array<std::string_view, 11> kAllowed{
            "payment_id",
            "order_id",
            "fulfillment_status",
            "fulfillments",
            "tracking_company",
            "tracking_numbers",
            "tracking_urls",
            "carrier",
            "fulfillment_method",
            "shipment_status",
            "shipped_at"
        };
        for (const auto& [key, value]: *root) {
            (void)value;
            if (std::find(kAllowed.begin(), kAllowed.end(), std::string_view{key}) ==
                kAllowed.end()) {
                return std::nullopt;
            }
        }
        if (!has_string_field(*root, "payment_id") || !has_string_field(*root, "order_id")) {
            return std::nullopt;
        }
        const auto fulfillments = root->get("fulfillments");
        if (!fulfillments) {
            return std::nullopt;
        }
        const auto* array = std::get_if<interfaces::Value::Array>(&fulfillments->get());
        if (array == nullptr) {
            return std::nullopt;
        }
        for (const auto& element: *array) {
            const auto* item = std::get_if<interfaces::Value::Object>(&element.get());
            if (item == nullptr || !has_string_field(*item, "shipment_id")) {
                return std::nullopt;
            }
            const auto destination = item->get("destination");
            if (!destination) {
                return std::nullopt;
            }
            const auto* dest = std::get_if<interfaces::Value::Object>(&destination->get());
            if (dest == nullptr || !has_string_field(*dest, "full_name")) {
                return std::nullopt;
            }
            const auto address = dest->get("address");
            if (!address) {
                return std::nullopt;
            }
            const auto* addr = std::get_if<interfaces::Value::Object>(&address->get());
            if (addr == nullptr || !has_string_field(*addr, "street_address") ||
                !has_string_field(*addr, "postal_code") || !has_string_field(*addr, "city") ||
                !has_string_field(*addr, "province_code") ||
                !has_string_field(*addr, "country_code")) {
                return std::nullopt;
            }
        }
        if (const auto status = root->get("fulfillment_status")) {
            if (const auto* text = std::get_if<std::string>(&status->get())) {
                static constexpr std::array<std::string_view, 4>
                    kStatuses{"PARTIAL", "COMPLETE", "REPLACEMENT", "CANCELED"};
                if (std::find(kStatuses.begin(), kStatuses.end(), std::string_view{*text}) ==
                    kStatuses.end()) {
                    return std::nullopt;
                }
            }
        }
        const auto decoded =
            serde::Ser::deserialize<FrmFulfillmentRequestWire>("application/json", body);
        return decoded ? std::optional<FrmFulfillmentRequestWire>{*decoded} : std::nullopt;
    }

private:
    [[nodiscard]] bool
    has_string_field(const interfaces::Value::Object& object, std::string_view key) const
    {
        const auto field = object.get(std::string{key});
        return field.has_value() && std::holds_alternative<std::string>(field->get());
    }

    ::app::context::AppContext& m_context;
};

// Each struct carries exactly what the old frm_fulfillment_core call received.
struct FrmFulfillmentInput
{
    FrmFulfillmentRequestWire request{};
    std::string raw_payload{};
    std::string header_merchant_id{};
    std::string org_id{};
};

} // namespace app::handlers::fraud_check

template<>
struct serde::Serializable<::app::handlers::fraud_check::FrmAddressWire>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "street_address",
                &::app::handlers::fraud_check::FrmAddressWire::get_street_address,
                &::app::handlers::fraud_check::FrmAddressWire::set_street_address,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "unit",
                &::app::handlers::fraud_check::FrmAddressWire::get_unit,
                &::app::handlers::fraud_check::FrmAddressWire::set_unit,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "postal_code",
                &::app::handlers::fraud_check::FrmAddressWire::get_postal_code,
                &::app::handlers::fraud_check::FrmAddressWire::set_postal_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "city",
                &::app::handlers::fraud_check::FrmAddressWire::get_city,
                &::app::handlers::fraud_check::FrmAddressWire::set_city,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "province_code",
                &::app::handlers::fraud_check::FrmAddressWire::get_province_code,
                &::app::handlers::fraud_check::FrmAddressWire::set_province_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "country_code",
                &::app::handlers::fraud_check::FrmAddressWire::get_country_code,
                &::app::handlers::fraud_check::FrmAddressWire::set_country_code,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "frm_address_wire";
    }
};

template<>
struct serde::Serializable<::app::handlers::fraud_check::FrmDestinationWire>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "full_name",
                &::app::handlers::fraud_check::FrmDestinationWire::get_full_name,
                &::app::handlers::fraud_check::FrmDestinationWire::set_full_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "organization",
                &::app::handlers::fraud_check::FrmDestinationWire::get_organization,
                &::app::handlers::fraud_check::FrmDestinationWire::set_organization,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "email",
                &::app::handlers::fraud_check::FrmDestinationWire::get_email,
                &::app::handlers::fraud_check::FrmDestinationWire::set_email,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "address",
                &::app::handlers::fraud_check::FrmDestinationWire::get_address,
                &::app::handlers::fraud_check::FrmDestinationWire::set_address,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "frm_destination_wire";
    }
};

template<>
struct serde::Serializable<::app::handlers::fraud_check::FrmProductWire>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "item_name",
                &::app::handlers::fraud_check::FrmProductWire::get_item_name,
                &::app::handlers::fraud_check::FrmProductWire::set_item_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "item_quantity",
                &::app::handlers::fraud_check::FrmProductWire::get_item_quantity,
                &::app::handlers::fraud_check::FrmProductWire::set_item_quantity,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "item_id",
                &::app::handlers::fraud_check::FrmProductWire::get_item_id,
                &::app::handlers::fraud_check::FrmProductWire::set_item_id,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "frm_product_wire";
    }
};

template<>
struct serde::Serializable<::app::handlers::fraud_check::FrmFulfillmentWire>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "shipment_id",
                &::app::handlers::fraud_check::FrmFulfillmentWire::get_shipment_id,
                &::app::handlers::fraud_check::FrmFulfillmentWire::set_shipment_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "products",
                &::app::handlers::fraud_check::FrmFulfillmentWire::get_products,
                &::app::handlers::fraud_check::FrmFulfillmentWire::set_products,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "destination",
                &::app::handlers::fraud_check::FrmFulfillmentWire::get_destination,
                &::app::handlers::fraud_check::FrmFulfillmentWire::set_destination,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "frm_fulfillment_wire";
    }
};

template<>
struct serde::Serializable<::app::handlers::fraud_check::FrmFulfillmentRequestWire>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_id",
                &::app::handlers::fraud_check::FrmFulfillmentRequestWire::get_payment_id,
                &::app::handlers::fraud_check::FrmFulfillmentRequestWire::set_payment_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "order_id",
                &::app::handlers::fraud_check::FrmFulfillmentRequestWire::get_order_id,
                &::app::handlers::fraud_check::FrmFulfillmentRequestWire::set_order_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "fulfillment_status",
                &::app::handlers::fraud_check::FrmFulfillmentRequestWire::get_fulfillment_status,
                &::app::handlers::fraud_check::FrmFulfillmentRequestWire::set_fulfillment_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "fulfillments",
                &::app::handlers::fraud_check::FrmFulfillmentRequestWire::get_fulfillments,
                &::app::handlers::fraud_check::FrmFulfillmentRequestWire::set_fulfillments,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "tracking_company",
                &::app::handlers::fraud_check::FrmFulfillmentRequestWire::get_tracking_company,
                &::app::handlers::fraud_check::FrmFulfillmentRequestWire::set_tracking_company,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "tracking_numbers",
                &::app::handlers::fraud_check::FrmFulfillmentRequestWire::get_tracking_numbers,
                &::app::handlers::fraud_check::FrmFulfillmentRequestWire::set_tracking_numbers,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "tracking_urls",
                &::app::handlers::fraud_check::FrmFulfillmentRequestWire::get_tracking_urls,
                &::app::handlers::fraud_check::FrmFulfillmentRequestWire::set_tracking_urls,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "carrier",
                &::app::handlers::fraud_check::FrmFulfillmentRequestWire::get_carrier,
                &::app::handlers::fraud_check::FrmFulfillmentRequestWire::set_carrier,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "fulfillment_method",
                &::app::handlers::fraud_check::FrmFulfillmentRequestWire::get_fulfillment_method,
                &::app::handlers::fraud_check::FrmFulfillmentRequestWire::set_fulfillment_method,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "shipment_status",
                &::app::handlers::fraud_check::FrmFulfillmentRequestWire::get_shipment_status,
                &::app::handlers::fraud_check::FrmFulfillmentRequestWire::set_shipment_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "shipped_at",
                &::app::handlers::fraud_check::FrmFulfillmentRequestWire::get_shipped_at,
                &::app::handlers::fraud_check::FrmFulfillmentRequestWire::set_shipped_at,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "frm_fulfillment_request_wire";
    }
};
