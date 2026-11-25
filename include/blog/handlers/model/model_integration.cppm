export module blog_handlers:model_integration;

import std;
import core_serde;
import core_database;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::model {

/** @brief Ghost integrations entity — full field set from data/schema/schema.js. */
class Integration
{
public:
    Integration() = default;

    void set_id(std::int64_t value)
    {
        m_id = std::move(value);
    }

    [[nodiscard]] const std::int64_t& get_id() const noexcept
    {
        return m_id;
    }

    void set_type(std::string value)
    {
        m_type = std::move(value);
    }

    [[nodiscard]] const std::string& get_type() const noexcept
    {
        return m_type;
    }

    void set_name(std::string value)
    {
        m_name = std::move(value);
    }

    [[nodiscard]] const std::string& get_name() const noexcept
    {
        return m_name;
    }

    void set_slug(std::string value)
    {
        m_slug = std::move(value);
    }

    [[nodiscard]] const std::string& get_slug() const noexcept
    {
        return m_slug;
    }

    void set_icon_image(std::string value)
    {
        m_icon_image = std::move(value);
    }

    [[nodiscard]] const std::string& get_icon_image() const noexcept
    {
        return m_icon_image;
    }

    void set_description(std::string value)
    {
        m_description = std::move(value);
    }

    [[nodiscard]] const std::string& get_description() const noexcept
    {
        return m_description;
    }

    void set_created_at(std::chrono::system_clock::time_point value)
    {
        m_created_at = std::move(value);
    }

    [[nodiscard]] const std::chrono::system_clock::time_point& get_created_at() const noexcept
    {
        return m_created_at;
    }

    void set_updated_at(std::chrono::system_clock::time_point value)
    {
        m_updated_at = std::move(value);
    }

    [[nodiscard]] const std::chrono::system_clock::time_point& get_updated_at() const noexcept
    {
        return m_updated_at;
    }

    std::int64_t m_id{};
    std::string m_type{};
    std::string m_name{};
    std::string m_slug{};
    std::string m_icon_image{};
    std::string m_description{};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
};


} // namespace blog::model

namespace serde {
template<>
struct Serializable<blog::model::Integration>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &blog::model::Integration::get_id,
                &blog::model::Integration::set_id,
                serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<
                "type",
                &blog::model::Integration::get_type,
                &blog::model::Integration::set_type,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "name",
                &blog::model::Integration::get_name,
                &blog::model::Integration::set_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "slug",
                &blog::model::Integration::get_slug,
                &blog::model::Integration::set_slug,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "icon_image",
                &blog::model::Integration::get_icon_image,
                &blog::model::Integration::set_icon_image,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "description",
                &blog::model::Integration::get_description,
                &blog::model::Integration::set_description,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created_at",
                &blog::model::Integration::get_created_at,
                &blog::model::Integration::set_created_at,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "updated_at",
                &blog::model::Integration::get_updated_at,
                &blog::model::Integration::set_updated_at,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "integrations";
    }
};
} // namespace serde


#ifdef CONGELADO_TEST
namespace blog::model::tests {
using namespace boost::ut;
suite<"blog_model_integration"> blog_model_integration_suite = []
{
    "reflects full Ghost integrations schema"_test = []
    {
        static_assert(core::db::reflect::ReflectedEntity<Integration>);
        expect(core::db::reflect::field_count_v<Integration> == 8);
        expect(core::db::reflect::EntityMeta<Integration>::table == "integrations");
    };
};
} // namespace blog::model::tests
#endif
