export module core_workflow:value;

import std;

export namespace core::workflow {

/**
 * @brief Serializable value carried through a workflow execution — compensate inputs,
 * context entries and step outputs all reduce to this shape.
 *
 * Mirrors Medusa's JSON-safe execution payload: primitives, string lists (ids) and
 * flat string maps. Structured outputs can be added as further alternatives later.
 */
using WorkflowValue = std::variant<
    std::monostate,
    bool,
    std::int64_t,
    std::uint64_t,
    double,
    std::string,
    std::vector<std::string>,
    std::map<std::string, std::string>>;

/** @brief Instance functor: reduces any supported C++ value to a WorkflowValue. */
class ToWorkflowValue
{
public:
    template<typename T>
    [[nodiscard]] WorkflowValue operator()(T&& value) const
    {
        using U = std::decay_t<T>;
        if constexpr (std::same_as<U, WorkflowValue>) {
            return std::forward<T>(value);
        } else if constexpr (std::same_as<U, std::monostate>) {
            return std::monostate{};
        } else if constexpr (std::same_as<U, bool>) {
            return static_cast<bool>(value);
        } else if constexpr (std::is_integral_v<U>) {
            if constexpr (std::is_signed_v<U>) {
                return static_cast<std::int64_t>(value);
            } else {
                return static_cast<std::uint64_t>(value);
            }
        } else if constexpr (std::is_floating_point_v<U>) {
            return static_cast<double>(value);
        } else if constexpr (std::constructible_from<std::string, U>) {
            return std::string{std::forward<T>(value)};
        } else {
            return WorkflowValue{U{std::forward<T>(value)}};
        }
    }
};

/** @brief Instance functor: renders a WorkflowValue as text (diagnostics, ids). */
class WorkflowValueText
{
public:
    [[nodiscard]] std::string operator()(const WorkflowValue& value) const
    {
        return std::visit(
            [](const auto& v) -> std::string {
                using T = std::decay_t<decltype(v)>;
                if constexpr (std::same_as<T, std::monostate>) {
                    return "";
                } else if constexpr (std::same_as<T, bool>) {
                    return v ? "true" : "false";
                } else if constexpr (std::same_as<T, std::string>) {
                    return v;
                } else if constexpr (std::same_as<T, std::vector<std::string>>) {
                    return std::ranges::fold_left(v, std::string{}, [](std::string acc, const std::string& s) {
                        if (!acc.empty()) {
                            acc += ',';
                        }
                        acc += s;
                        return acc;
                    });
                } else if constexpr (std::same_as<T, std::map<std::string, std::string>>) {
                    return std::ranges::fold_left(v, std::string{}, [](std::string acc, const auto& kv) {
                        if (!acc.empty()) {
                            acc += ',';
                        }
                        acc += kv.first;
                        acc += '=';
                        acc += kv.second;
                        return acc;
                    });
                } else {
                    return std::format("{}", v);
                }
            },
            value
        );
    }
};

/** @brief Instance functor: decodes a WorkflowValue into a requested C++ type. */
class WorkflowValueAs
{
public:
    template<typename T>
    [[nodiscard]] T operator()(const WorkflowValue& value) const
    {
        if constexpr (std::same_as<T, WorkflowValue>) {
            return value;
        } else if constexpr (std::same_as<T, std::string>) {
            if (const auto* p = std::get_if<std::string>(&value)) {
                return *p;
            }
            return WorkflowValueText{}(value);
        } else if constexpr (std::same_as<T, bool>) {
            if (const auto* p = std::get_if<bool>(&value)) {
                return *p;
            }
            return false;
        } else if constexpr (std::is_integral_v<T> || std::is_floating_point_v<T>) {
            return std::visit(
                [](const auto& v) -> T {
                    using V = std::decay_t<decltype(v)>;
                    if constexpr (std::is_arithmetic_v<V> && !std::same_as<V, bool>) {
                        return static_cast<T>(v);
                    } else {
                        return T{};
                    }
                },
                value
            );
        } else if constexpr (std::same_as<T, std::vector<std::string>>) {
            if (const auto* p = std::get_if<std::vector<std::string>>(&value)) {
                return *p;
            }
            if (const auto* s = std::get_if<std::string>(&value)) {
                return std::vector<std::string>{*s};
            }
            return {};
        } else {
            return std::get<T>(value);
        }
    }
};

} // namespace core::workflow

#ifdef CONGELADO_TEST
namespace core::workflow::tests {
using namespace boost::ut;

suite<"core_workflow_value"> core_workflow_value_suite = []
{
    "round trips primitives through WorkflowValue"_test = []
    {
        expect(std::get<std::string>(ToWorkflowValue{}(std::string{"prom_01"})) == "prom_01");
        expect(WorkflowValueAs{}.operator()<std::string>(WorkflowValue{std::string{"prom_01"}}) == "prom_01");
        expect(WorkflowValueAs{}.operator()<std::int64_t>(WorkflowValue{std::int64_t{7}}) == 7);
    };

    "renders value text"_test = []
    {
        expect(WorkflowValueText{}(WorkflowValue{std::vector<std::string>{"a", "b"}}) == "a,b");
    };
};

} // namespace core::workflow::tests
#endif
