module;
export module core_error:error;

import std;
import core_serde;
import :kind;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::error {

/** @brief Typed operation error. Serializes to {"error":KIND,"message":TEXT}. */
class AppError
{
public:
    void set_kind(AppErrorKind value)
    {
        kind = std::move(value);
    }

    [[nodiscard]] const AppErrorKind& get_kind() const noexcept
    {
        return kind;
    }

    void set_kind_name(std::string value)
    {
        kind_name = std::move(value);
    }

    [[nodiscard]] const std::string& get_kind_name() const noexcept
    {
        return kind_name;
    }

    void set_message(std::string value)
    {
        message = std::move(value);
    }

    [[nodiscard]] const std::string& get_message() const noexcept
    {
        return message;
    }

    [[nodiscard]] interfaces::io::types::Status get_status() const noexcept
    {
        return AppErrorStatus{}(kind);
    }

    AppErrorKind kind{AppErrorKind::INTERNAL};
    std::string kind_name{};
    std::string message{};
};

/** @brief Builds AppError instances without static helpers. */
class AppErrorBuilder
{
public:
    [[nodiscard]] AppError operator()(AppErrorKind kind, std::string message) const
    {
        AppError error;
        error.set_kind(kind);
        error.set_kind_name(std::string{AppErrorKindName{}(kind)});
        error.set_message(std::move(message));
        return error;
    }
};

} // namespace core::error

template<>
struct serde::Serializable<core::error::AppError>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "error",
                &core::error::AppError::get_kind_name,
                &core::error::AppError::set_kind_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "message",
                &core::error::AppError::get_message,
                &core::error::AppError::set_message,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "app_error";
    }
};

#ifdef CONGELADO_TEST
namespace core::error::tests {
using namespace boost::ut;

suite<"core_error"> error_suite = []
{
    "builder sets kind name and message"_test = []
    {
        const auto error = AppErrorBuilder{}.operator()(AppErrorKind::INVALID_JSON, "invalid JSON");
        expect(error.get_kind_name() == "INVALID_JSON");
        expect(error.get_message() == "invalid JSON");
    };
};

} // namespace core::error::tests
#endif
