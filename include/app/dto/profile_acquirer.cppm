module;
export module app_dto:profile_acquirer;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::dto::profile_acquirer {

class CardNetworkValue
{
public:
    void set_value(std::string val)
    {
        value = std::move(val);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class CountryAlpha2Value
{
public:
    void set_value(std::string val)
    {
        value = std::move(val);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class ProfileAcquirerIdValue
{
public:
    void set_value(std::string val)
    {
        value = std::move(val);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class ProfileIdValue
{
public:
    void set_value(std::string val)
    {
        value = std::move(val);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class AcquirerBucketConfigResponse
{
public:
    void set_acquirer_assigned_merchant_id(std::optional<std::string> value)
    {
        acquirer_assigned_merchant_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>&
    get_acquirer_assigned_merchant_id() const noexcept
    {
        return acquirer_assigned_merchant_id;
    }

    void set_merchant_name(std::optional<std::string> value)
    {
        merchant_name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_merchant_name() const noexcept
    {
        return merchant_name;
    }

    void set_network(CardNetworkValue value)
    {
        network = std::move(value);
    }

    [[nodiscard]] const CardNetworkValue& get_network() const noexcept
    {
        return network;
    }

    void set_acquirer_bin(std::optional<std::string> value)
    {
        acquirer_bin = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_acquirer_bin() const noexcept
    {
        return acquirer_bin;
    }

    void set_acquirer_ica(std::optional<std::string> value)
    {
        acquirer_ica = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_acquirer_ica() const noexcept
    {
        return acquirer_ica;
    }

    void set_acquirer_fraud_rate(std::optional<double> value)
    {
        acquirer_fraud_rate = std::move(value);
    }

    [[nodiscard]] const std::optional<double>& get_acquirer_fraud_rate() const noexcept
    {
        return acquirer_fraud_rate;
    }

    void set_acquirer_country_code(std::optional<std::string> value)
    {
        acquirer_country_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_acquirer_country_code() const noexcept
    {
        return acquirer_country_code;
    }

    std::optional<std::string> acquirer_assigned_merchant_id{};
    std::optional<std::string> merchant_name{};
    CardNetworkValue network{};
    std::optional<std::string> acquirer_bin{};
    std::optional<std::string> acquirer_ica{};
    std::optional<double> acquirer_fraud_rate{};
    std::optional<std::string> acquirer_country_code{};
};

class ProfileAcquirerCreate
{
public:
    void set_acquirer_assigned_merchant_id(std::string value)
    {
        acquirer_assigned_merchant_id = std::move(value);
    }

    [[nodiscard]] const std::string& get_acquirer_assigned_merchant_id() const noexcept
    {
        return acquirer_assigned_merchant_id;
    }

    void set_merchant_name(std::string value)
    {
        merchant_name = std::move(value);
    }

    [[nodiscard]] const std::string& get_merchant_name() const noexcept
    {
        return merchant_name;
    }

    void set_network(CardNetworkValue value)
    {
        network = std::move(value);
    }

    [[nodiscard]] const CardNetworkValue& get_network() const noexcept
    {
        return network;
    }

    void set_acquirer_bin(std::string value)
    {
        acquirer_bin = std::move(value);
    }

    [[nodiscard]] const std::string& get_acquirer_bin() const noexcept
    {
        return acquirer_bin;
    }

    void set_acquirer_ica(std::optional<std::string> value)
    {
        acquirer_ica = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_acquirer_ica() const noexcept
    {
        return acquirer_ica;
    }

    void set_acquirer_fraud_rate(std::optional<double> value)
    {
        acquirer_fraud_rate = std::move(value);
    }

    [[nodiscard]] const std::optional<double>& get_acquirer_fraud_rate() const noexcept
    {
        return acquirer_fraud_rate;
    }

    void set_acquirer_country_code(std::optional<CountryAlpha2Value> value)
    {
        acquirer_country_code = std::move(value);
    }

    [[nodiscard]] const std::optional<CountryAlpha2Value>&
    get_acquirer_country_code() const noexcept
    {
        return acquirer_country_code;
    }

    void set_profile_id(ProfileIdValue value)
    {
        profile_id = std::move(value);
    }

    [[nodiscard]] const ProfileIdValue& get_profile_id() const noexcept
    {
        return profile_id;
    }

    void set_is_default(std::optional<bool> value)
    {
        is_default = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_is_default() const noexcept
    {
        return is_default;
    }

    std::string acquirer_assigned_merchant_id{};
    std::string merchant_name{};
    CardNetworkValue network{};
    std::string acquirer_bin{};
    std::optional<std::string> acquirer_ica{};
    std::optional<double> acquirer_fraud_rate{};
    std::optional<CountryAlpha2Value> acquirer_country_code{};
    ProfileIdValue profile_id{};
    std::optional<bool> is_default{};
};

class ProfileAcquirerResponse
{
public:
    void set_profile_acquirer_id(ProfileAcquirerIdValue value)
    {
        profile_acquirer_id = std::move(value);
    }

    [[nodiscard]] const ProfileAcquirerIdValue& get_profile_acquirer_id() const noexcept
    {
        return profile_acquirer_id;
    }

    void set_acquirer_assigned_merchant_id(std::optional<std::string> value)
    {
        acquirer_assigned_merchant_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>&
    get_acquirer_assigned_merchant_id() const noexcept
    {
        return acquirer_assigned_merchant_id;
    }

    void set_merchant_name(std::optional<std::string> value)
    {
        merchant_name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_merchant_name() const noexcept
    {
        return merchant_name;
    }

    void set_network(std::optional<CardNetworkValue> value)
    {
        network = std::move(value);
    }

    [[nodiscard]] const std::optional<CardNetworkValue>& get_network() const noexcept
    {
        return network;
    }

    void set_acquirer_bin(std::optional<std::string> value)
    {
        acquirer_bin = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_acquirer_bin() const noexcept
    {
        return acquirer_bin;
    }

    void set_acquirer_ica(std::optional<std::string> value)
    {
        acquirer_ica = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_acquirer_ica() const noexcept
    {
        return acquirer_ica;
    }

    void set_acquirer_fraud_rate(std::optional<double> value)
    {
        acquirer_fraud_rate = std::move(value);
    }

    [[nodiscard]] const std::optional<double>& get_acquirer_fraud_rate() const noexcept
    {
        return acquirer_fraud_rate;
    }

    void set_acquirer_country_code(std::optional<std::string> value)
    {
        acquirer_country_code = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_acquirer_country_code() const noexcept
    {
        return acquirer_country_code;
    }

    void set_profile_id(ProfileIdValue value)
    {
        profile_id = std::move(value);
    }

    [[nodiscard]] const ProfileIdValue& get_profile_id() const noexcept
    {
        return profile_id;
    }

    void set_is_default(bool value)
    {
        is_default = std::move(value);
    }

    [[nodiscard]] const bool& get_is_default() const noexcept
    {
        return is_default;
    }

    ProfileAcquirerIdValue profile_acquirer_id{};
    std::optional<std::string> acquirer_assigned_merchant_id{};
    std::optional<std::string> merchant_name{};
    std::optional<CardNetworkValue> network{};
    std::optional<std::string> acquirer_bin{};
    std::optional<std::string> acquirer_ica{};
    std::optional<double> acquirer_fraud_rate{};
    std::optional<std::string> acquirer_country_code{};
    ProfileIdValue profile_id{};
    bool is_default{};
};

class ProfileAcquirerUpdate
{
public:
    void set_acquirer_assigned_merchant_id(std::optional<std::string> value)
    {
        acquirer_assigned_merchant_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>&
    get_acquirer_assigned_merchant_id() const noexcept
    {
        return acquirer_assigned_merchant_id;
    }

    void set_merchant_name(std::optional<std::string> value)
    {
        merchant_name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_merchant_name() const noexcept
    {
        return merchant_name;
    }

    void set_network(std::optional<CardNetworkValue> value)
    {
        network = std::move(value);
    }

    [[nodiscard]] const std::optional<CardNetworkValue>& get_network() const noexcept
    {
        return network;
    }

    void set_acquirer_bin(std::optional<std::string> value)
    {
        acquirer_bin = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_acquirer_bin() const noexcept
    {
        return acquirer_bin;
    }

    void set_acquirer_ica(std::optional<std::string> value)
    {
        acquirer_ica = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_acquirer_ica() const noexcept
    {
        return acquirer_ica;
    }

    void set_acquirer_fraud_rate(std::optional<double> value)
    {
        acquirer_fraud_rate = std::move(value);
    }

    [[nodiscard]] const std::optional<double>& get_acquirer_fraud_rate() const noexcept
    {
        return acquirer_fraud_rate;
    }

    void set_acquirer_country_code(std::optional<CountryAlpha2Value> value)
    {
        acquirer_country_code = std::move(value);
    }

    [[nodiscard]] const std::optional<CountryAlpha2Value>&
    get_acquirer_country_code() const noexcept
    {
        return acquirer_country_code;
    }

    void set_is_default(std::optional<bool> value)
    {
        is_default = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_is_default() const noexcept
    {
        return is_default;
    }

    std::optional<std::string> acquirer_assigned_merchant_id{};
    std::optional<std::string> merchant_name{};
    std::optional<CardNetworkValue> network{};
    std::optional<std::string> acquirer_bin{};
    std::optional<std::string> acquirer_ica{};
    std::optional<double> acquirer_fraud_rate{};
    std::optional<CountryAlpha2Value> acquirer_country_code{};
    std::optional<bool> is_default{};
};

class ProfileAcquirerConfigsResponse
{
public:
    void set_default_acquirer_config(std::optional<ProfileAcquirerIdValue> value)
    {
        default_acquirer_config = std::move(value);
    }

    [[nodiscard]] const std::optional<ProfileAcquirerIdValue>&
    get_default_acquirer_config() const noexcept
    {
        return default_acquirer_config;
    }

    void set_configs(
        std::unordered_map<std::string, std::vector<AcquirerBucketConfigResponse>> value
    )
    {
        configs = std::move(value);
    }

    [[nodiscard]] const std::unordered_map<std::string, std::vector<AcquirerBucketConfigResponse>>&
    get_configs() const noexcept
    {
        return configs;
    }

    std::optional<ProfileAcquirerIdValue> default_acquirer_config{};
    std::unordered_map<std::string, std::vector<AcquirerBucketConfigResponse>> configs{};
};
} // namespace app::dto::profile_acquirer

template<>
struct serde::Serializable<app::dto::profile_acquirer::ProfileAcquirerCreate>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "acquirer_assigned_merchant_id",
                &app::dto::profile_acquirer::ProfileAcquirerCreate::
                    get_acquirer_assigned_merchant_id,
                &app::dto::profile_acquirer::ProfileAcquirerCreate::
                    set_acquirer_assigned_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_name",
                &app::dto::profile_acquirer::ProfileAcquirerCreate::get_merchant_name,
                &app::dto::profile_acquirer::ProfileAcquirerCreate::set_merchant_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "network",
                &app::dto::profile_acquirer::ProfileAcquirerCreate::get_network,
                &app::dto::profile_acquirer::ProfileAcquirerCreate::set_network,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "acquirer_bin",
                &app::dto::profile_acquirer::ProfileAcquirerCreate::get_acquirer_bin,
                &app::dto::profile_acquirer::ProfileAcquirerCreate::set_acquirer_bin,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "acquirer_ica",
                &app::dto::profile_acquirer::ProfileAcquirerCreate::get_acquirer_ica,
                &app::dto::profile_acquirer::ProfileAcquirerCreate::set_acquirer_ica,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "acquirer_fraud_rate",
                &app::dto::profile_acquirer::ProfileAcquirerCreate::get_acquirer_fraud_rate,
                &app::dto::profile_acquirer::ProfileAcquirerCreate::set_acquirer_fraud_rate,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "acquirer_country_code",
                &app::dto::profile_acquirer::ProfileAcquirerCreate::get_acquirer_country_code,
                &app::dto::profile_acquirer::ProfileAcquirerCreate::set_acquirer_country_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "profile_id",
                &app::dto::profile_acquirer::ProfileAcquirerCreate::get_profile_id,
                &app::dto::profile_acquirer::ProfileAcquirerCreate::set_profile_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "is_default",
                &app::dto::profile_acquirer::ProfileAcquirerCreate::get_is_default,
                &app::dto::profile_acquirer::ProfileAcquirerCreate::set_is_default,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "profile_acquirer_create";
    }
};

template<>
struct serde::Serializable<app::dto::profile_acquirer::ProfileAcquirerResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "profile_acquirer_id",
                &app::dto::profile_acquirer::ProfileAcquirerResponse::get_profile_acquirer_id,
                &app::dto::profile_acquirer::ProfileAcquirerResponse::set_profile_acquirer_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "acquirer_assigned_merchant_id",
                &app::dto::profile_acquirer::ProfileAcquirerResponse::
                    get_acquirer_assigned_merchant_id,
                &app::dto::profile_acquirer::ProfileAcquirerResponse::
                    set_acquirer_assigned_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_name",
                &app::dto::profile_acquirer::ProfileAcquirerResponse::get_merchant_name,
                &app::dto::profile_acquirer::ProfileAcquirerResponse::set_merchant_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "network",
                &app::dto::profile_acquirer::ProfileAcquirerResponse::get_network,
                &app::dto::profile_acquirer::ProfileAcquirerResponse::set_network,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "acquirer_bin",
                &app::dto::profile_acquirer::ProfileAcquirerResponse::get_acquirer_bin,
                &app::dto::profile_acquirer::ProfileAcquirerResponse::set_acquirer_bin,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "acquirer_ica",
                &app::dto::profile_acquirer::ProfileAcquirerResponse::get_acquirer_ica,
                &app::dto::profile_acquirer::ProfileAcquirerResponse::set_acquirer_ica,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "acquirer_fraud_rate",
                &app::dto::profile_acquirer::ProfileAcquirerResponse::get_acquirer_fraud_rate,
                &app::dto::profile_acquirer::ProfileAcquirerResponse::set_acquirer_fraud_rate,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "acquirer_country_code",
                &app::dto::profile_acquirer::ProfileAcquirerResponse::get_acquirer_country_code,
                &app::dto::profile_acquirer::ProfileAcquirerResponse::set_acquirer_country_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "profile_id",
                &app::dto::profile_acquirer::ProfileAcquirerResponse::get_profile_id,
                &app::dto::profile_acquirer::ProfileAcquirerResponse::set_profile_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "is_default",
                &app::dto::profile_acquirer::ProfileAcquirerResponse::get_is_default,
                &app::dto::profile_acquirer::ProfileAcquirerResponse::set_is_default,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "profile_acquirer_response";
    }
};

template<>
struct serde::Serializable<app::dto::profile_acquirer::AcquirerBucketConfigResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "acquirer_assigned_merchant_id",
                &app::dto::profile_acquirer::AcquirerBucketConfigResponse::
                    get_acquirer_assigned_merchant_id,
                &app::dto::profile_acquirer::AcquirerBucketConfigResponse::
                    set_acquirer_assigned_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_name",
                &app::dto::profile_acquirer::AcquirerBucketConfigResponse::get_merchant_name,
                &app::dto::profile_acquirer::AcquirerBucketConfigResponse::set_merchant_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "network",
                &app::dto::profile_acquirer::AcquirerBucketConfigResponse::get_network,
                &app::dto::profile_acquirer::AcquirerBucketConfigResponse::set_network,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "acquirer_bin",
                &app::dto::profile_acquirer::AcquirerBucketConfigResponse::get_acquirer_bin,
                &app::dto::profile_acquirer::AcquirerBucketConfigResponse::set_acquirer_bin,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "acquirer_ica",
                &app::dto::profile_acquirer::AcquirerBucketConfigResponse::get_acquirer_ica,
                &app::dto::profile_acquirer::AcquirerBucketConfigResponse::set_acquirer_ica,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "acquirer_fraud_rate",
                &app::dto::profile_acquirer::AcquirerBucketConfigResponse::get_acquirer_fraud_rate,
                &app::dto::profile_acquirer::AcquirerBucketConfigResponse::set_acquirer_fraud_rate,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "acquirer_country_code",
                &app::dto::profile_acquirer::AcquirerBucketConfigResponse::
                    get_acquirer_country_code,
                &app::dto::profile_acquirer::AcquirerBucketConfigResponse::
                    set_acquirer_country_code,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "acquirer_bucket_config_response";
    }
};

template<>
struct serde::Serializable<app::dto::profile_acquirer::ProfileAcquirerConfigsResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "default_acquirer_config",
                &app::dto::profile_acquirer::ProfileAcquirerConfigsResponse::
                    get_default_acquirer_config,
                &app::dto::profile_acquirer::ProfileAcquirerConfigsResponse::
                    set_default_acquirer_config,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "configs",
                &app::dto::profile_acquirer::ProfileAcquirerConfigsResponse::get_configs,
                &app::dto::profile_acquirer::ProfileAcquirerConfigsResponse::set_configs,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "profile_acquirer_configs_response";
    }
};

template<>
struct serde::Serializable<app::dto::profile_acquirer::ProfileAcquirerUpdate>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "acquirer_assigned_merchant_id",
                &app::dto::profile_acquirer::ProfileAcquirerUpdate::
                    get_acquirer_assigned_merchant_id,
                &app::dto::profile_acquirer::ProfileAcquirerUpdate::
                    set_acquirer_assigned_merchant_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_name",
                &app::dto::profile_acquirer::ProfileAcquirerUpdate::get_merchant_name,
                &app::dto::profile_acquirer::ProfileAcquirerUpdate::set_merchant_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "network",
                &app::dto::profile_acquirer::ProfileAcquirerUpdate::get_network,
                &app::dto::profile_acquirer::ProfileAcquirerUpdate::set_network,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "acquirer_bin",
                &app::dto::profile_acquirer::ProfileAcquirerUpdate::get_acquirer_bin,
                &app::dto::profile_acquirer::ProfileAcquirerUpdate::set_acquirer_bin,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "acquirer_ica",
                &app::dto::profile_acquirer::ProfileAcquirerUpdate::get_acquirer_ica,
                &app::dto::profile_acquirer::ProfileAcquirerUpdate::set_acquirer_ica,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "acquirer_fraud_rate",
                &app::dto::profile_acquirer::ProfileAcquirerUpdate::get_acquirer_fraud_rate,
                &app::dto::profile_acquirer::ProfileAcquirerUpdate::set_acquirer_fraud_rate,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "acquirer_country_code",
                &app::dto::profile_acquirer::ProfileAcquirerUpdate::get_acquirer_country_code,
                &app::dto::profile_acquirer::ProfileAcquirerUpdate::set_acquirer_country_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "is_default",
                &app::dto::profile_acquirer::ProfileAcquirerUpdate::get_is_default,
                &app::dto::profile_acquirer::ProfileAcquirerUpdate::set_is_default,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "profile_acquirer_update";
    }
};

#ifdef CONGELADO_TEST
namespace app::dto::profile_acquirer::tests {
using namespace boost::ut;
suite<"app_dto_profile_acquirer"> app_dto_profile_acquirer_suite = []
{
    "reflects converted types"_test = []
    {
        expect(
            std::tuple_size_v<
                decltype(serde::Serializable<
                         app::dto::profile_acquirer::ProfileAcquirerCreate>::fields())> > 0
        );
    };
};
} // namespace app::dto::profile_acquirer::tests
#endif
