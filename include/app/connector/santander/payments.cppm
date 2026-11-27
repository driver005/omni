module;
export module app_connector:model_santander_payments;

import std;
import core_serde;
import :model_shared_payments;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::model::payments {

enum class SantanderJourneyName
{
    Journey1,
    Journey2,
    Journey3,
    Journey4,
    AwaitingDefinition,
};

enum class SantanderMandatePeriodicity
{
    Weekly,
    Monthly,
    Quarterly,
    Semiannually,
    Annually,
};

class SantanderData : public serde::Serializer<SantanderData>
{
public:
    void set_end_to_end_id(std::optional<std::string> value)
    {
        end_to_end_id = std::move(value);
    }

    [[nodiscard]] const std::optional<std::string>& get_end_to_end_id() const noexcept
    {
        return end_to_end_id;
    }

    void set_journey_name(std::optional<SantanderJourneyName> value)
    {
        journey_name = std::move(value);
    }

    [[nodiscard]] const std::optional<SantanderJourneyName>& get_journey_name() const noexcept
    {
        return journey_name;
    }

    void set_paid_at(std::optional<PrimitiveDateTimeValue> value)
    {
        paid_at = std::move(value);
    }

    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>& get_paid_at() const noexcept
    {
        return paid_at;
    }

    std::optional<std::string> end_to_end_id{};
    std::optional<SantanderJourneyName> journey_name{};
    std::optional<PrimitiveDateTimeValue> paid_at{};
};

class SantanderMandateDetails : public serde::Serializer<SantanderMandateDetails>
{
public:
    void set_fixed_recurring_amount(std::optional<MinorUnitValue> value)
    {
        fixed_recurring_amount = std::move(value);
    }

    [[nodiscard]] const std::optional<MinorUnitValue>& get_fixed_recurring_amount() const noexcept
    {
        return fixed_recurring_amount;
    }

    void set_min_recurring_amount(std::optional<MinorUnitValue> value)
    {
        min_recurring_amount = std::move(value);
    }

    [[nodiscard]] const std::optional<MinorUnitValue>& get_min_recurring_amount() const noexcept
    {
        return min_recurring_amount;
    }

    void set_start_date(std::optional<PrimitiveDateTimeValue> value)
    {
        start_date = std::move(value);
    }

    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>& get_start_date() const noexcept
    {
        return start_date;
    }

    void set_end_date(std::optional<PrimitiveDateTimeValue> value)
    {
        end_date = std::move(value);
    }

    [[nodiscard]] const std::optional<PrimitiveDateTimeValue>& get_end_date() const noexcept
    {
        return end_date;
    }

    void set_periodicity(std::optional<SantanderMandatePeriodicity> value)
    {
        periodicity = std::move(value);
    }

    [[nodiscard]] const std::optional<SantanderMandatePeriodicity>& get_periodicity() const noexcept
    {
        return periodicity;
    }

    std::optional<MinorUnitValue> fixed_recurring_amount{};
    std::optional<MinorUnitValue> min_recurring_amount{};
    std::optional<PrimitiveDateTimeValue> start_date{};
    std::optional<PrimitiveDateTimeValue> end_date{};
    std::optional<SantanderMandatePeriodicity> periodicity{};
};

class SantanderPixAutomaticoReceiverDetails :
    public serde::Serializer<SantanderPixAutomaticoReceiverDetails>
{
public:
    void set_branch_code(std::optional<SecretValue> value)
    {
        branch_code = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_branch_code() const noexcept
    {
        return branch_code;
    }

    void set_account_number(std::optional<SecretValue> value)
    {
        account_number = std::move(value);
    }

    [[nodiscard]] const std::optional<SecretValue>& get_account_number() const noexcept
    {
        return account_number;
    }

    void set_account_type(std::optional<AccountType> value)
    {
        account_type = std::move(value);
    }

    [[nodiscard]] const std::optional<AccountType>& get_account_type() const noexcept
    {
        return account_type;
    }

    std::optional<SecretValue> branch_code{};
    std::optional<SecretValue> account_number{};
    std::optional<AccountType> account_type{};
};

} // namespace app::model::payments

template<>
struct serde::Serializable<app::model::payments::SantanderData>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "end_to_end_id",
                &app::model::payments::SantanderData::get_end_to_end_id,
                &app::model::payments::SantanderData::set_end_to_end_id,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "journey_name",
                &app::model::payments::SantanderData::get_journey_name,
                &app::model::payments::SantanderData::set_journey_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "paid_at",
                &app::model::payments::SantanderData::get_paid_at,
                &app::model::payments::SantanderData::set_paid_at,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "santander_data";
    }
};

template<>
struct serde::Serializable<app::model::payments::SantanderMandateDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "fixed_recurring_amount",
                &app::model::payments::SantanderMandateDetails::get_fixed_recurring_amount,
                &app::model::payments::SantanderMandateDetails::set_fixed_recurring_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "min_recurring_amount",
                &app::model::payments::SantanderMandateDetails::get_min_recurring_amount,
                &app::model::payments::SantanderMandateDetails::set_min_recurring_amount,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "start_date",
                &app::model::payments::SantanderMandateDetails::get_start_date,
                &app::model::payments::SantanderMandateDetails::set_start_date,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "end_date",
                &app::model::payments::SantanderMandateDetails::get_end_date,
                &app::model::payments::SantanderMandateDetails::set_end_date,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "periodicity",
                &app::model::payments::SantanderMandateDetails::get_periodicity,
                &app::model::payments::SantanderMandateDetails::set_periodicity,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "santander_mandate_details";
    }
};

template<>
struct serde::Serializable<app::model::payments::SantanderPixAutomaticoReceiverDetails>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "branch_code",
                &app::model::payments::SantanderPixAutomaticoReceiverDetails::get_branch_code,
                &app::model::payments::SantanderPixAutomaticoReceiverDetails::set_branch_code,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "account_number",
                &app::model::payments::SantanderPixAutomaticoReceiverDetails::get_account_number,
                &app::model::payments::SantanderPixAutomaticoReceiverDetails::set_account_number,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "account_type",
                &app::model::payments::SantanderPixAutomaticoReceiverDetails::get_account_type,
                &app::model::payments::SantanderPixAutomaticoReceiverDetails::set_account_type,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "santander_pix_automatico_receiver_details";
    }
};
