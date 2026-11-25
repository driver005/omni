module;
export module app_dto:surcharge_decision_configs;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::dto::surcharge_decision_configs {

class MerchantSurchargeConfigs
{
public:
    void set_show_surcharge_breakup_screen(std::optional<bool> value)
    {
        show_surcharge_breakup_screen = std::move(value);
    }

    [[nodiscard]] const std::optional<bool>& get_show_surcharge_breakup_screen() const noexcept
    {
        return show_surcharge_breakup_screen;
    }

    std::optional<bool> show_surcharge_breakup_screen{};
};

class MinorUnitValue
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

class PercentageValue
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

class ProgramValue
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

enum class SurchargeOutput
{
    Fixed,
    Rate,
};

class SurchargeDecisionConfigReq
{
public:
    void set_name(std::optional<std::string> value)
    {
        name = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_name() const noexcept
    {
        return name;
    }

    void set_merchant_surcharge_configs(MerchantSurchargeConfigs value)
    {
        merchant_surcharge_configs = std::move(value);
    }

    [[nodiscard]] const MerchantSurchargeConfigs& get_merchant_surcharge_configs() const noexcept
    {
        return merchant_surcharge_configs;
    }

    void set_algorithm(std::optional<ProgramValue> value)
    {
        algorithm = std::move(value);
    }

    [[nodiscard]] const std::optional<ProgramValue>& get_algorithm() const noexcept
    {
        return algorithm;
    }

    std::optional<std::string> name{};
    MerchantSurchargeConfigs merchant_surcharge_configs{};
    std::optional<ProgramValue> algorithm{};
};

class SurchargeDecisionManagerRecord
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

    void set_merchant_surcharge_configs(MerchantSurchargeConfigs value)
    {
        merchant_surcharge_configs = std::move(value);
    }

    [[nodiscard]] const MerchantSurchargeConfigs& get_merchant_surcharge_configs() const noexcept
    {
        return merchant_surcharge_configs;
    }

    void set_algorithm(ProgramValue value)
    {
        algorithm = std::move(value);
    }

    [[nodiscard]] const ProgramValue& get_algorithm() const noexcept
    {
        return algorithm;
    }

    void set_created_at(std::int64_t value)
    {
        created_at = std::move(value);
    }

    [[nodiscard]] const std::int64_t& get_created_at() const noexcept
    {
        return created_at;
    }

    void set_modified_at(std::int64_t value)
    {
        modified_at = std::move(value);
    }

    [[nodiscard]] const std::int64_t& get_modified_at() const noexcept
    {
        return modified_at;
    }

    std::string name{};
    MerchantSurchargeConfigs merchant_surcharge_configs{};
    ProgramValue algorithm{};
    std::int64_t created_at{};
    std::int64_t modified_at{};
};

class SurchargeDetailsOutput
{
public:
    void set_surcharge(SurchargeOutput value)
    {
        surcharge = std::move(value);
    }

    [[nodiscard]] const SurchargeOutput& get_surcharge() const noexcept
    {
        return surcharge;
    }

    void set_tax_on_surcharge(std::optional<PercentageValue> value)
    {
        tax_on_surcharge = std::move(value);
    }

    [[nodiscard]] const std::optional<PercentageValue>& get_tax_on_surcharge() const noexcept
    {
        return tax_on_surcharge;
    }

    SurchargeOutput surcharge{};
    std::optional<PercentageValue> tax_on_surcharge{};
};

class SurchargeDecisionConfigs
{
public:
    void set_surcharge_details(std::optional<SurchargeDetailsOutput> value)
    {
        surcharge_details = std::move(value);
    }

    [[nodiscard]] const std::optional<SurchargeDetailsOutput>&
    get_surcharge_details() const noexcept
    {
        return surcharge_details;
    }

    std::optional<SurchargeDetailsOutput> surcharge_details{};
};
} // namespace app::dto::surcharge_decision_configs

template<>
struct serde::Serializable<app::dto::surcharge_decision_configs::MerchantSurchargeConfigs>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "show_surcharge_breakup_screen",
            &app::dto::surcharge_decision_configs::MerchantSurchargeConfigs::
                get_show_surcharge_breakup_screen,
            &app::dto::surcharge_decision_configs::MerchantSurchargeConfigs::
                set_show_surcharge_breakup_screen,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "merchant_surcharge_configs";
    }
};

template<>
struct serde::Serializable<app::dto::surcharge_decision_configs::SurchargeDetailsOutput>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "surcharge",
                &app::dto::surcharge_decision_configs::SurchargeDetailsOutput::get_surcharge,
                &app::dto::surcharge_decision_configs::SurchargeDetailsOutput::set_surcharge,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "tax_on_surcharge",
                &app::dto::surcharge_decision_configs::SurchargeDetailsOutput::get_tax_on_surcharge,
                &app::dto::surcharge_decision_configs::SurchargeDetailsOutput::set_tax_on_surcharge,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "surcharge_details_output";
    }
};

template<>
struct serde::Serializable<app::dto::surcharge_decision_configs::SurchargeDecisionConfigs>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "surcharge_details",
            &app::dto::surcharge_decision_configs::SurchargeDecisionConfigs::get_surcharge_details,
            &app::dto::surcharge_decision_configs::SurchargeDecisionConfigs::set_surcharge_details,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "surcharge_decision_configs";
    }
};

template<>
struct serde::Serializable<app::dto::surcharge_decision_configs::SurchargeDecisionManagerRecord>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "name",
                &app::dto::surcharge_decision_configs::SurchargeDecisionManagerRecord::get_name,
                &app::dto::surcharge_decision_configs::SurchargeDecisionManagerRecord::set_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_surcharge_configs",
                &app::dto::surcharge_decision_configs::SurchargeDecisionManagerRecord::
                    get_merchant_surcharge_configs,
                &app::dto::surcharge_decision_configs::SurchargeDecisionManagerRecord::
                    set_merchant_surcharge_configs,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "algorithm",
                &app::dto::surcharge_decision_configs::SurchargeDecisionManagerRecord::
                    get_algorithm,
                &app::dto::surcharge_decision_configs::SurchargeDecisionManagerRecord::
                    set_algorithm,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created_at",
                &app::dto::surcharge_decision_configs::SurchargeDecisionManagerRecord::
                    get_created_at,
                &app::dto::surcharge_decision_configs::SurchargeDecisionManagerRecord::
                    set_created_at,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "modified_at",
                &app::dto::surcharge_decision_configs::SurchargeDecisionManagerRecord::
                    get_modified_at,
                &app::dto::surcharge_decision_configs::SurchargeDecisionManagerRecord::
                    set_modified_at,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "surcharge_decision_manager_record";
    }
};

template<>
struct serde::Serializable<app::dto::surcharge_decision_configs::SurchargeDecisionConfigReq>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "name",
                &app::dto::surcharge_decision_configs::SurchargeDecisionConfigReq::get_name,
                &app::dto::surcharge_decision_configs::SurchargeDecisionConfigReq::set_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "merchant_surcharge_configs",
                &app::dto::surcharge_decision_configs::SurchargeDecisionConfigReq::
                    get_merchant_surcharge_configs,
                &app::dto::surcharge_decision_configs::SurchargeDecisionConfigReq::
                    set_merchant_surcharge_configs,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "algorithm",
                &app::dto::surcharge_decision_configs::SurchargeDecisionConfigReq::get_algorithm,
                &app::dto::surcharge_decision_configs::SurchargeDecisionConfigReq::set_algorithm,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "surcharge_decision_config_req";
    }
};

#ifdef CONGELADO_TEST
namespace app::dto::surcharge_decision_configs::tests {
using namespace boost::ut;
suite<"app_dto_surcharge_decision_configs"> app_dto_surcharge_decision_configs_suite = []
{
    "reflects converted types"_test = []
    {
        expect(
            std::tuple_size_v<
                decltype(serde::Serializable<
                         app::dto::surcharge_decision_configs::SurchargeDetailsOutput>::fields())> >
            0
        );
    };
};
} // namespace app::dto::surcharge_decision_configs::tests
#endif
