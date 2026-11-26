module;
export module app_dto:connector_enums;

import core_serde;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::dto::connector_enums {

class ConnectorEnumsCatalog : public serde::Serializer<ConnectorEnumsCatalog>
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

} // namespace app::dto::connector_enums

template<>
struct serde::Serializable<app::dto::connector_enums::ConnectorEnumsCatalog>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "name",
            &app::dto::connector_enums::ConnectorEnumsCatalog::get_name,
            &app::dto::connector_enums::ConnectorEnumsCatalog::set_name,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "connector_enums_catalog";
    }
};

#ifdef CONGELADO_TEST
namespace app::dto::connector_enums::tests {
using namespace boost::ut;
suite<"app_dto_connector_enums"> app_dto_connector_enums_suite = []
{
    "reflects converted types"_test = []
    {
        expect(
            std::tuple_size_v<
                decltype(serde::Serializable<
                         app::dto::connector_enums::ConnectorEnumsCatalog>::fields())> > 0
        );
    };
};
} // namespace app::dto::connector_enums::tests
#endif
