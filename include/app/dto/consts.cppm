module;
export module app_dto:consts;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::dto::consts {

class ConstsCatalog
{
public:
    void set_name(std::string value) { name = std::move(value); }
    [[nodiscard]] const std::string& get_name() const noexcept { return name; }
    std::string name{};
};

}

template<>
struct serde::Serializable<app::dto::consts::ConstsCatalog>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"name", &app::dto::consts::ConstsCatalog::get_name, &app::dto::consts::ConstsCatalog::set_name, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "consts_catalog"; }
};

#ifdef CONGELADO_TEST
namespace app::dto::consts::tests {
using namespace boost::ut;
suite<"app_dto_consts"> app_dto_consts_suite = [] {
    "reflects converted types"_test = [] {
        expect(std::tuple_size_v<decltype(serde::Serializable<app::dto::consts::ConstsCatalog>::fields())> > 0);
    };
};
}
#endif
