module;
export module app_dto:connector_onboarding;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::dto::connector_onboarding {

enum class ActionUrlResponse
{
    PayPal,
};

class ConnectorValue
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class MerchantConnectorAccountIdValue
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class MerchantIdValue
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

enum class OnboardingStatus
{
    PayPal,
};

class PayPalActionUrlResponse
{
public:
    void set_action_url(std::string value)
    {
        action_url = std::move(value);
    }

    [[nodiscard]] const std::string& get_action_url() const noexcept
    {
        return action_url;
    }

    std::string action_url{};
};

class PayPalIntegrationDone
{
public:
    void set_connector_id(std::string value)
    {
        connector_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_connector_id() const noexcept
    {
        return connector_id;
    }

    std::string connector_id{};
};

enum class PayPalOnboardingStatus
{
    AccountNotFound,
    PaymentsNotReceivable,
    PpcpCustomDenied,
    MorePermissionsNeeded,
    EmailNotVerified,
    Success,
    ConnectorIntegrated,
};

class ProfileIdValue
{
public:
    void set_value(std::string v)
    {
        value = std::move(v);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

// Upstream id_type and Connector newtypes are serde-transparent: they serialize as
// plain strings on the wire (common_utils/src/id_type.rs:88-104,
// common_enums/src/connector_enums.rs:29-32). Mirror that wire shape directly so
// serde::Ser::deserialize parses the upstream JSON without a {"value": ...} wrapper.
class ActionUrlRequest
{
public:
    void set_connector(std::string value)
    {
        connector = std::move(value);
    }

    [[nodiscard]] const std::string& get_connector() const noexcept
    {
        return connector;
    }

    void set_connector_id(std::string value)
    {
        connector_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_connector_id() const noexcept
    {
        return connector_id;
    }

    void set_return_url(std::string value)
    {
        return_url = std::move(value);
    }

    [[nodiscard]] const std::string& get_return_url() const noexcept
    {
        return return_url;
    }

    std::string connector{};
    std::string connector_id{};
    std::string return_url{};
};

class OnboardingSyncRequest
{
public:
    void set_profile_id(std::string value)
    {
        profile_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_profile_id() const noexcept
    {
        return profile_id;
    }

    void set_connector_id(std::string value)
    {
        connector_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_connector_id() const noexcept
    {
        return connector_id;
    }

    void set_connector(std::string value)
    {
        connector = std::move(value);
    }

    [[nodiscard]] const std::string& get_connector() const noexcept
    {
        return connector;
    }

    std::string profile_id{};
    std::string connector_id{};
    std::string connector{};
};

class PayPalOnboardingDone
{
public:
    void set_payer_id(std::string value)
    {
        payer_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_payer_id() const noexcept
    {
        return payer_id;
    }

    std::string payer_id{};
};

class ResetTrackingIdRequest
{
public:
    void set_connector_id(std::string value)
    {
        connector_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_connector_id() const noexcept
    {
        return connector_id;
    }

    void set_connector(std::string value)
    {
        connector = std::move(value);
    }

    [[nodiscard]] const std::string& get_connector() const noexcept
    {
        return connector;
    }

    std::string connector_id{};
    std::string connector{};
};
} // namespace app::dto::connector_onboarding

template<>
struct serde::Serializable<app::dto::connector_onboarding::ActionUrlRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "connector",
                &app::dto::connector_onboarding::ActionUrlRequest::get_connector,
                &app::dto::connector_onboarding::ActionUrlRequest::set_connector,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector_id",
                &app::dto::connector_onboarding::ActionUrlRequest::get_connector_id,
                &app::dto::connector_onboarding::ActionUrlRequest::set_connector_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "return_url",
                &app::dto::connector_onboarding::ActionUrlRequest::get_return_url,
                &app::dto::connector_onboarding::ActionUrlRequest::set_return_url,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "action_url_request";
    }
};

template<>
struct serde::Serializable<app::dto::connector_onboarding::OnboardingSyncRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "profile_id",
                &app::dto::connector_onboarding::OnboardingSyncRequest::get_profile_id,
                &app::dto::connector_onboarding::OnboardingSyncRequest::set_profile_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector_id",
                &app::dto::connector_onboarding::OnboardingSyncRequest::get_connector_id,
                &app::dto::connector_onboarding::OnboardingSyncRequest::set_connector_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector",
                &app::dto::connector_onboarding::OnboardingSyncRequest::get_connector,
                &app::dto::connector_onboarding::OnboardingSyncRequest::set_connector,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "onboarding_sync_request";
    }
};

template<>
struct serde::Serializable<app::dto::connector_onboarding::PayPalActionUrlResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "action_url",
            &app::dto::connector_onboarding::PayPalActionUrlResponse::get_action_url,
            &app::dto::connector_onboarding::PayPalActionUrlResponse::set_action_url,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "pay_pal_action_url_response";
    }
};

template<>
struct serde::Serializable<app::dto::connector_onboarding::PayPalOnboardingDone>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "payer_id",
            &app::dto::connector_onboarding::PayPalOnboardingDone::get_payer_id,
            &app::dto::connector_onboarding::PayPalOnboardingDone::set_payer_id,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "pay_pal_onboarding_done";
    }
};

template<>
struct serde::Serializable<app::dto::connector_onboarding::PayPalIntegrationDone>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "connector_id",
            &app::dto::connector_onboarding::PayPalIntegrationDone::get_connector_id,
            &app::dto::connector_onboarding::PayPalIntegrationDone::set_connector_id,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "pay_pal_integration_done";
    }
};

template<>
struct serde::Serializable<app::dto::connector_onboarding::ResetTrackingIdRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "connector_id",
                &app::dto::connector_onboarding::ResetTrackingIdRequest::get_connector_id,
                &app::dto::connector_onboarding::ResetTrackingIdRequest::set_connector_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "connector",
                &app::dto::connector_onboarding::ResetTrackingIdRequest::get_connector,
                &app::dto::connector_onboarding::ResetTrackingIdRequest::set_connector,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "reset_tracking_id_request";
    }
};

#ifdef CONGELADO_TEST
namespace app::dto::connector_onboarding::tests {
using namespace boost::ut;
suite<"app_dto_connector_onboarding"> app_dto_connector_onboarding_suite = []
{
    "reflects converted types"_test = []
    {
        expect(
            std::tuple_size_v<
                decltype(serde::Serializable<
                         app::dto::connector_onboarding::ActionUrlRequest>::fields())> > 0
        );
    };
};
} // namespace app::dto::connector_onboarding::tests
#endif
