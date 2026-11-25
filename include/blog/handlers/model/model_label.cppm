export module blog_handlers:model_label;

import std;
import core_serde;
import core_database;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::model {

/** @brief Ghost labels entity — full field set from data/schema/schema.js. */
class Label
{
public:
    Label() = default;

    void set_id(std::int64_t value)
    {
        m_id = std::move(value);
    }

    [[nodiscard]] const std::int64_t& get_id() const noexcept
    {
        return m_id;
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
    std::string m_name{};
    std::string m_slug{};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
};


} // namespace blog::model

namespace serde {
template<>
struct Serializable<blog::model::Label>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &blog::model::Label::get_id,
                &blog::model::Label::set_id,
                serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<
                "name",
                &blog::model::Label::get_name,
                &blog::model::Label::set_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "slug",
                &blog::model::Label::get_slug,
                &blog::model::Label::set_slug,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created_at",
                &blog::model::Label::get_created_at,
                &blog::model::Label::set_created_at,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "updated_at",
                &blog::model::Label::get_updated_at,
                &blog::model::Label::set_updated_at,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "labels";
    }
};
} // namespace serde


#ifdef CONGELADO_TEST
namespace blog::model::tests {
using namespace boost::ut;
suite<"blog_model_label"> blog_model_label_suite = []
{
    "reflects full Ghost labels schema"_test = []
    {
        static_assert(core::db::reflect::ReflectedEntity<Label>);
        expect(core::db::reflect::field_count_v<Label> == 5);
        expect(core::db::reflect::EntityMeta<Label>::table == "labels");
    };
};
} // namespace blog::model::tests
#endif
