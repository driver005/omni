module;
export module app_dto:platform;

import core_serde;

import std;
import core_serde;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace app::dto::platform {

enum class Initiator
{
    Platform,
    Connected,
};

class PlatformCatalog : public serde::Serializer<PlatformCatalog>
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
} // namespace app::dto::platform

template<>
struct serde::Serializable<app::dto::platform::PlatformCatalog>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "name",
            &app::dto::platform::PlatformCatalog::get_name,
            &app::dto::platform::PlatformCatalog::set_name,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "platform_catalog";
    }
};

#ifdef CONGELADO_TEST
namespace app::dto::platform::tests {
using namespace boost::ut;
suite<"app_dto_platform"> app_dto_platform_suite = []
{
    "reflects converted types"_test = []
    {
        expect(
            std::tuple_size_v<
                decltype(serde::Serializable<app::dto::platform::PlatformCatalog>::fields())> > 0
        );
    };
};
} // namespace app::dto::platform::tests
#endif
