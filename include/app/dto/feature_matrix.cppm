module;
export module app_dto:feature_matrix;

import core_serde;
import core_behavior;

import std;
import core_serde;
import core_behavior;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

// Wire types mirror api_models/src/feature_matrix.rs:6-95 exactly. Upstream enum fields are
// serialized by serde as snake_case strings (e.g. PaymentMethod::Card -> "card"), so the
// reflected members below are plain std::string / std::vector<std::string> rather than the
// single-field `{ value }` wrappers the first port used. rfl (reflect-cpp) reflects these public
// members directly, producing the same JSON scalar/array shapes as upstream.
export namespace app::dto::feature_matrix {

/** @brief api_models CardSpecificFeatures (feature_matrix.rs:13-24). */
class CardSpecificFeatures : public core::behavior::Serializer<CardSpecificFeatures>
{
public:
    void set_three_ds(std::string value)
    {
        three_ds = std::move(value);
    }

    [[nodiscard]] const std::string& get_three_ds() const noexcept
    {
        return three_ds;
    }

    void set_no_three_ds(std::string value)
    {
        no_three_ds = std::move(value);
    }

    [[nodiscard]] const std::string& get_no_three_ds() const noexcept
    {
        return no_three_ds;
    }

    void set_supported_card_networks(std::vector<std::string> value)
    {
        supported_card_networks = std::move(value);
    }

    [[nodiscard]] const std::vector<std::string>& get_supported_card_networks() const noexcept
    {
        return supported_card_networks;
    }

    std::string three_ds{};
    std::string no_three_ds{};
    std::vector<std::string> supported_card_networks{};
};

/** @brief api_models PaymentMethodSpecificFeatures::Card untagged variant
 * (feature_matrix.rs:26-31). */
using PaymentMethodSpecificFeatures = CardSpecificFeatures;

/** @brief api_models FeatureMatrixRequest (feature_matrix.rs:6-11). */
class FeatureMatrixRequest : public core::behavior::Serializer<FeatureMatrixRequest>
{
public:
    void set_connectors(std::optional<std::vector<std::string>> value)
    {
        connectors = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<std::string>>& get_connectors() const noexcept
    {
        return connectors;
    }

    std::optional<std::vector<std::string>> connectors{};
};

/** @brief api_models SupportedPaymentMethod (feature_matrix.rs:33-61). */
class SupportedPaymentMethod : public core::behavior::Serializer<SupportedPaymentMethod>
{
public:
    void set_payment_method(std::string value)
    {
        payment_method = std::move(value);
    }

    [[nodiscard]] const std::string& get_payment_method() const noexcept
    {
        return payment_method;
    }

    void set_payment_method_type(std::string value)
    {
        payment_method_type = std::move(value);
    }

    [[nodiscard]] const std::string& get_payment_method_type() const noexcept
    {
        return payment_method_type;
    }

    void set_payment_method_type_display_name(std::string value)
    {
        payment_method_type_display_name = std::move(value);
    }

    [[nodiscard]] const std::string& get_payment_method_type_display_name() const noexcept
    {
        return payment_method_type_display_name;
    }

    void set_mandates(std::string value)
    {
        mandates = std::move(value);
    }

    [[nodiscard]] const std::string& get_mandates() const noexcept
    {
        return mandates;
    }

    void set_refunds(std::string value)
    {
        refunds = std::move(value);
    }

    [[nodiscard]] const std::string& get_refunds() const noexcept
    {
        return refunds;
    }

    void set_supported_capture_methods(std::vector<std::string> value)
    {
        supported_capture_methods = std::move(value);
    }

    [[nodiscard]] const std::vector<std::string>& get_supported_capture_methods() const noexcept
    {
        return supported_capture_methods;
    }

    // Upstream `#[serde(flatten)] payment_method_specific_features:
    // Option<PaymentMethodSpecificFeatures>` (feature_matrix.rs:52-54) emits the Card variant's
    // fields at the TOP level of the row when present and nothing when absent. rfl omits empty
    // std::optionals, so modelling the three fields as optional members reproduces that flatten
    // behavior on the wire.
    void set_three_ds(std::optional<std::string> value)
    {
        three_ds = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_three_ds() const noexcept
    {
        return three_ds;
    }

    void set_no_three_ds(std::optional<std::string> value)
    {
        no_three_ds = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_no_three_ds() const noexcept
    {
        return no_three_ds;
    }

    void set_supported_card_networks(std::optional<std::vector<std::string>> value)
    {
        supported_card_networks = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<std::string>>&
    get_supported_card_networks() const noexcept
    {
        return supported_card_networks;
    }

    void set_supported_countries(std::optional<std::vector<std::string>> value)
    {
        supported_countries = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<std::string>>&
    get_supported_countries() const noexcept
    {
        return supported_countries;
    }

    void set_supported_currencies(std::optional<std::vector<std::string>> value)
    {
        supported_currencies = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<std::string>>&
    get_supported_currencies() const noexcept
    {
        return supported_currencies;
    }

    std::string payment_method{};
    std::string payment_method_type{};
    std::string payment_method_type_display_name{};
    std::string mandates{};
    std::string refunds{};
    std::vector<std::string> supported_capture_methods{};
    std::optional<std::string> three_ds{};
    std::optional<std::string> no_three_ds{};
    std::optional<std::vector<std::string>> supported_card_networks{};
    std::optional<std::vector<std::string>> supported_countries{};
    std::optional<std::vector<std::string>> supported_currencies{};
};

/** @brief api_models ConnectorFeatureMatrixResponse (feature_matrix.rs:63-84). */
class ConnectorFeatureMatrixResponse :
    public core::behavior::Serializer<ConnectorFeatureMatrixResponse>
{
public:
    void set_name(std::string value)
    {
        name = std::move(value);
    }

    [[nodiscard]] const std::string& get_name() const noexcept
    {
        return name;
    }

    void set_display_name(std::string value)
    {
        display_name = std::move(value);
    }

    [[nodiscard]] const std::string& get_display_name() const noexcept
    {
        return display_name;
    }

    void set_description(std::string value)
    {
        description = std::move(value);
    }

    [[nodiscard]] const std::string& get_description() const noexcept
    {
        return description;
    }

    void set_base_url(std::optional<std::string> value)
    {
        base_url = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_base_url() const noexcept
    {
        return base_url;
    }

    void set_category(std::string value)
    {
        category = std::move(value);
    }

    [[nodiscard]] const std::string& get_category() const noexcept
    {
        return category;
    }

    void set_integration_status(std::string value)
    {
        integration_status = std::move(value);
    }

    [[nodiscard]] const std::string& get_integration_status() const noexcept
    {
        return integration_status;
    }

    void set_supported_payment_methods(std::optional<std::vector<SupportedPaymentMethod>> value)
    {
        supported_payment_methods = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<SupportedPaymentMethod>>&
    get_supported_payment_methods() const noexcept
    {
        return supported_payment_methods;
    }

    void set_supported_webhook_flows(std::optional<std::vector<std::string>> value)
    {
        supported_webhook_flows = std::move(value);
    }

    [[nodiscard]] const std::optional<std::vector<std::string>>&
    get_supported_webhook_flows() const noexcept
    {
        return supported_webhook_flows;
    }

    std::string name{};
    std::string display_name{};
    std::string description{};
    std::optional<std::string> base_url{};
    std::string category{};
    std::string integration_status{};
    std::optional<std::vector<SupportedPaymentMethod>> supported_payment_methods{};
    std::optional<std::vector<std::string>> supported_webhook_flows{};
};

/** @brief api_models FeatureMatrixListResponse (feature_matrix.rs:86-92). */
class FeatureMatrixListResponse : public core::behavior::Serializer<FeatureMatrixListResponse>
{
public:
    void set_connector_count(std::size_t value)
    {
        connector_count = std::move(value);
    }

    [[nodiscard]] const std::size_t& get_connector_count() const noexcept
    {
        return connector_count;
    }

    void set_connectors(std::vector<ConnectorFeatureMatrixResponse> value)
    {
        connectors = std::move(value);
    }

    [[nodiscard]] const std::vector<ConnectorFeatureMatrixResponse>& get_connectors() const noexcept
    {
        return connectors;
    }

    std::size_t connector_count{};
    std::vector<ConnectorFeatureMatrixResponse> connectors{};
};

} // namespace app::dto::feature_matrix

template<>
struct serde::Serializable<app::dto::feature_matrix::FeatureMatrixRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "connectors",
            &app::dto::feature_matrix::FeatureMatrixRequest::get_connectors,
            &app::dto::feature_matrix::FeatureMatrixRequest::set_connectors,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "feature_matrix_request";
    }
};

template<>
struct serde::Serializable<app::dto::feature_matrix::CardSpecificFeatures>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "three_ds",
                &app::dto::feature_matrix::CardSpecificFeatures::get_three_ds,
                &app::dto::feature_matrix::CardSpecificFeatures::set_three_ds,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "no_three_ds",
                &app::dto::feature_matrix::CardSpecificFeatures::get_no_three_ds,
                &app::dto::feature_matrix::CardSpecificFeatures::set_no_three_ds,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "supported_card_networks",
                &app::dto::feature_matrix::CardSpecificFeatures::get_supported_card_networks,
                &app::dto::feature_matrix::CardSpecificFeatures::set_supported_card_networks,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "card_specific_features";
    }
};

template<>
struct serde::Serializable<app::dto::feature_matrix::SupportedPaymentMethod>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "payment_method",
                &app::dto::feature_matrix::SupportedPaymentMethod::get_payment_method,
                &app::dto::feature_matrix::SupportedPaymentMethod::set_payment_method,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_type",
                &app::dto::feature_matrix::SupportedPaymentMethod::get_payment_method_type,
                &app::dto::feature_matrix::SupportedPaymentMethod::set_payment_method_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "payment_method_type_display_name",
                &app::dto::feature_matrix::SupportedPaymentMethod::
                    get_payment_method_type_display_name,
                &app::dto::feature_matrix::SupportedPaymentMethod::
                    set_payment_method_type_display_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "mandates",
                &app::dto::feature_matrix::SupportedPaymentMethod::get_mandates,
                &app::dto::feature_matrix::SupportedPaymentMethod::set_mandates,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "refunds",
                &app::dto::feature_matrix::SupportedPaymentMethod::get_refunds,
                &app::dto::feature_matrix::SupportedPaymentMethod::set_refunds,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "supported_capture_methods",
                &app::dto::feature_matrix::SupportedPaymentMethod::get_supported_capture_methods,
                &app::dto::feature_matrix::SupportedPaymentMethod::set_supported_capture_methods,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "three_ds",
                &app::dto::feature_matrix::SupportedPaymentMethod::get_three_ds,
                &app::dto::feature_matrix::SupportedPaymentMethod::set_three_ds,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "no_three_ds",
                &app::dto::feature_matrix::SupportedPaymentMethod::get_no_three_ds,
                &app::dto::feature_matrix::SupportedPaymentMethod::set_no_three_ds,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "supported_card_networks",
                &app::dto::feature_matrix::SupportedPaymentMethod::get_supported_card_networks,
                &app::dto::feature_matrix::SupportedPaymentMethod::set_supported_card_networks,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "supported_countries",
                &app::dto::feature_matrix::SupportedPaymentMethod::get_supported_countries,
                &app::dto::feature_matrix::SupportedPaymentMethod::set_supported_countries,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "supported_currencies",
                &app::dto::feature_matrix::SupportedPaymentMethod::get_supported_currencies,
                &app::dto::feature_matrix::SupportedPaymentMethod::set_supported_currencies,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "supported_payment_method";
    }
};

template<>
struct serde::Serializable<app::dto::feature_matrix::ConnectorFeatureMatrixResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "name",
                &app::dto::feature_matrix::ConnectorFeatureMatrixResponse::get_name,
                &app::dto::feature_matrix::ConnectorFeatureMatrixResponse::set_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "display_name",
                &app::dto::feature_matrix::ConnectorFeatureMatrixResponse::get_display_name,
                &app::dto::feature_matrix::ConnectorFeatureMatrixResponse::set_display_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "description",
                &app::dto::feature_matrix::ConnectorFeatureMatrixResponse::get_description,
                &app::dto::feature_matrix::ConnectorFeatureMatrixResponse::set_description,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "base_url",
                &app::dto::feature_matrix::ConnectorFeatureMatrixResponse::get_base_url,
                &app::dto::feature_matrix::ConnectorFeatureMatrixResponse::set_base_url,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "category",
                &app::dto::feature_matrix::ConnectorFeatureMatrixResponse::get_category,
                &app::dto::feature_matrix::ConnectorFeatureMatrixResponse::set_category,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "integration_status",
                &app::dto::feature_matrix::ConnectorFeatureMatrixResponse::get_integration_status,
                &app::dto::feature_matrix::ConnectorFeatureMatrixResponse::set_integration_status,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "supported_payment_methods",
                &app::dto::feature_matrix::ConnectorFeatureMatrixResponse::
                    get_supported_payment_methods,
                &app::dto::feature_matrix::ConnectorFeatureMatrixResponse::
                    set_supported_payment_methods,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "supported_webhook_flows",
                &app::dto::feature_matrix::ConnectorFeatureMatrixResponse::
                    get_supported_webhook_flows,
                &app::dto::feature_matrix::ConnectorFeatureMatrixResponse::
                    set_supported_webhook_flows,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "connector_feature_matrix_response";
    }
};

template<>
struct serde::Serializable<app::dto::feature_matrix::FeatureMatrixListResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "connector_count",
                &app::dto::feature_matrix::FeatureMatrixListResponse::get_connector_count,
                &app::dto::feature_matrix::FeatureMatrixListResponse::set_connector_count,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connectors",
                &app::dto::feature_matrix::FeatureMatrixListResponse::get_connectors,
                &app::dto::feature_matrix::FeatureMatrixListResponse::set_connectors,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "feature_matrix_list_response";
    }
};

#ifdef CONGELADO_TEST
namespace app::dto::feature_matrix::tests {
using namespace boost::ut;
suite<"app_dto_feature_matrix"> app_dto_feature_matrix_suite = []
{
    "reflects converted types"_test = []
    {
        expect(
            std::tuple_size_v<decltype(serde::Serializable<
                                       app::dto::feature_matrix::FeatureMatrixRequest>::fields())> >
            0
        );
    };
};
} // namespace app::dto::feature_matrix::tests
#endif
