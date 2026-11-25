module;
export module app_dto:process_tracker;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::dto::process_tracker {

class ProcessTrackerCatalog
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

} // namespace app::dto::process_tracker

template<>
struct serde::Serializable<app::dto::process_tracker::ProcessTrackerCatalog>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "name",
            &app::dto::process_tracker::ProcessTrackerCatalog::get_name,
            &app::dto::process_tracker::ProcessTrackerCatalog::set_name,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "process_tracker_catalog";
    }
};

#ifdef CONGELADO_TEST
namespace app::dto::process_tracker::tests {
using namespace boost::ut;
suite<"app_dto_process_tracker"> app_dto_process_tracker_suite = []
{
    "reflects converted types"_test = []
    {
        expect(
            std::tuple_size_v<
                decltype(serde::Serializable<
                         app::dto::process_tracker::ProcessTrackerCatalog>::fields())> > 0
        );
    };
};
} // namespace app::dto::process_tracker::tests
#endif
