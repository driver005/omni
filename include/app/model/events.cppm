module;
export module app_model:events;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::model::events {

class EventsCatalog
{
public:
    void set_name(std::string value) { name = std::move(value); }
    [[nodiscard]] const std::string& get_name() const noexcept { return name; }
    std::string name{};
};

} // namespace app::model::events

template<>
struct serde::Serializable<app::model::events::EventsCatalog>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"name", &app::model::events::EventsCatalog::get_name, &app::model::events::EventsCatalog::set_name, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "events_catalog"; }
};

#ifdef CONGELADO_TEST
namespace app::model::events::tests {
using namespace boost::ut;
suite<"app_model_events"> app_model_events_suite = [] {
    "reflects converted types"_test = [] {
        expect(std::tuple_size_v<decltype(serde::Serializable<app::model::events::EventsCatalog>::fields())> > 0);
    };
};
}
#endif
