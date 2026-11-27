module;
export module app_dto:errors;

import core_serde;
import core_behavior;

import std;
import core_serde;
import core_behavior;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::dto::errors {

class ErrorsCatalog : public core::behavior::Serializer<ErrorsCatalog>
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

    std::string name{};
};

} // namespace app::dto::errors

template<>
struct serde::Serializable<app::dto::errors::ErrorsCatalog>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "name",
            &app::dto::errors::ErrorsCatalog::get_name,
            &app::dto::errors::ErrorsCatalog::set_name,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "errors_catalog";
    }
};

#ifdef CONGELADO_TEST
namespace app::dto::errors::tests {
using namespace boost::ut;
suite<"app_dto_errors"> app_dto_errors_suite = []
{
    "reflects converted types"_test = []
    {
        expect(
            std::tuple_size_v<
                decltype(serde::Serializable<app::dto::errors::ErrorsCatalog>::fields())> > 0
        );
    };
};
} // namespace app::dto::errors::tests
#endif
