module;
export module app_dto:telephone_order;

import core_serde;
import core_behavior;

import std;
import core_serde;
import core_behavior;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::dto::telephone_order {

class TelephoneOrderCatalog : public core::behavior::Serializer<TelephoneOrderCatalog>
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

} // namespace app::dto::telephone_order

template<>
struct serde::Serializable<app::dto::telephone_order::TelephoneOrderCatalog>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "name",
            &app::dto::telephone_order::TelephoneOrderCatalog::get_name,
            &app::dto::telephone_order::TelephoneOrderCatalog::set_name,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "telephone_order_catalog";
    }
};

#ifdef CONGELADO_TEST
namespace app::dto::telephone_order::tests {
using namespace boost::ut;
suite<"app_dto_telephone_order"> app_dto_telephone_order_suite = []
{
    "reflects converted types"_test = []
    {
        expect(
            std::tuple_size_v<
                decltype(serde::Serializable<
                         app::dto::telephone_order::TelephoneOrderCatalog>::fields())> > 0
        );
    };
};
} // namespace app::dto::telephone_order::tests
#endif
