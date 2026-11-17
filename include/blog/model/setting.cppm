module;
#include "entity_macros.h"

export module blog_model:setting;

import std;
import core_serde;
import core_database;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::model {

class Setting
{
public:
    Setting() = default;

    void set_id(std::int64_t value) { m_id = std::move(value); }
    [[nodiscard]] const std::int64_t& get_id() const noexcept { return m_id; }
    void set_key(std::string value) { m_key = std::move(value); }
    [[nodiscard]] const std::string& get_key() const noexcept { return m_key; }
    void set_value(std::string value) { m_value = std::move(value); }
    [[nodiscard]] const std::string& get_value() const noexcept { return m_value; }

    std::int64_t m_id{};
    std::string m_key{};
    std::string m_value{};
};




} // namespace blog::model

namespace serde {
template<>
struct Serializable<blog::model::Setting>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"id", &blog::model::Setting::get_id, &blog::model::Setting::set_id, serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<"key", &blog::model::Setting::get_key, &blog::model::Setting::set_key, serde::FieldOptions{}>{},
            serde::FieldDesc<"value", &blog::model::Setting::get_value, &blog::model::Setting::set_value, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "settings"; }
};
} // namespace serde


#ifdef CONGELADO_TEST
namespace blog::model::tests {
using namespace boost::ut;

suite<"blog_model_setting"> blog_model_setting_suite = []
{
    "reflects Setting fields"_test = []
    {
        static_assert(core::db::reflect::ReflectedEntity<Setting>);
        expect(core::db::reflect::field_count_v<Setting> == 3);
        expect(core::db::reflect::EntityMeta<Setting>::table == "settings");
    };
};

} // namespace blog::model::tests
#endif
