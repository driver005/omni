module;
export module app_dto:conditional_configs;

import core_serde;
import core_behavior;

import std;
import core_serde;
import core_behavior;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::dto::conditional_configs {

enum class DecisionManager
{
    DecisionManagerv0,
    DecisionManagerv1
};

class ProgramValue : public core::behavior::Serializer<ProgramValue>
{
public:
    void set_value(std::string value)
    {
        this->value = std::move(value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

    std::string value{};
};

class ConditionalConfigReq : public core::behavior::Serializer<ConditionalConfigReq>
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

    void set_algorithm(std::optional<ProgramValue> value)
    {
        algorithm = std::move(value);
    }

    [[nodiscard]] const std::optional<ProgramValue>& get_algorithm() const noexcept
    {
        return algorithm;
    }

    std::optional<std::string> name{};
    std::optional<ProgramValue> algorithm{};
};

class DecisionManagerRecord : public core::behavior::Serializer<DecisionManagerRecord>
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

    void set_program(ProgramValue value)
    {
        program = std::move(value);
    }

    [[nodiscard]] const ProgramValue& get_program() const noexcept
    {
        return program;
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
    ProgramValue program{};
    std::int64_t created_at{};
    std::int64_t modified_at{};
};

class DecisionManagerRequest : public core::behavior::Serializer<DecisionManagerRequest>
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

    void set_program(std::optional<ProgramValue> value)
    {
        program = std::move(value);
    }

    [[nodiscard]] const std::optional<ProgramValue>& get_program() const noexcept
    {
        return program;
    }

    std::optional<std::string> name{};
    std::optional<ProgramValue> program{};
};
} // namespace app::dto::conditional_configs

template<>
struct serde::Serializable<app::dto::conditional_configs::DecisionManagerRecord>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "name",
                &app::dto::conditional_configs::DecisionManagerRecord::get_name,
                &app::dto::conditional_configs::DecisionManagerRecord::set_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "program",
                &app::dto::conditional_configs::DecisionManagerRecord::get_program,
                &app::dto::conditional_configs::DecisionManagerRecord::set_program,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created_at",
                &app::dto::conditional_configs::DecisionManagerRecord::get_created_at,
                &app::dto::conditional_configs::DecisionManagerRecord::set_created_at,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "modified_at",
                &app::dto::conditional_configs::DecisionManagerRecord::get_modified_at,
                &app::dto::conditional_configs::DecisionManagerRecord::set_modified_at,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "decision_manager_record";
    }
};

template<>
struct serde::Serializable<app::dto::conditional_configs::ConditionalConfigReq>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "name",
                &app::dto::conditional_configs::ConditionalConfigReq::get_name,
                &app::dto::conditional_configs::ConditionalConfigReq::set_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "algorithm",
                &app::dto::conditional_configs::ConditionalConfigReq::get_algorithm,
                &app::dto::conditional_configs::ConditionalConfigReq::set_algorithm,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "conditional_config_req";
    }
};

template<>
struct serde::Serializable<app::dto::conditional_configs::DecisionManagerRequest>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "name",
                &app::dto::conditional_configs::DecisionManagerRequest::get_name,
                &app::dto::conditional_configs::DecisionManagerRequest::set_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "program",
                &app::dto::conditional_configs::DecisionManagerRequest::get_program,
                &app::dto::conditional_configs::DecisionManagerRequest::set_program,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "decision_manager_request";
    }
};

#ifdef CONGELADO_TEST
namespace app::dto::conditional_configs::tests {
using namespace boost::ut;
suite<app_dto_conditional_configs> app_dto_conditional_configs_suite = []
{
    "reflects converted types"_test = []
    {
        expect(
            std::tuple_size_v<
                decltype(serde::Serializable<
                         app::dto::conditional_configs::DecisionManagerRecord>::fields())> > 0
        );
    };
};
} // namespace app::dto::conditional_configs::tests
#endif
