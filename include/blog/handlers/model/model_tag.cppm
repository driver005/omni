export module blog_handlers:model_tag;

import std;
import core_serde;
import core_database;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::model {

class Tag
{
public:
    Tag() = default;

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

    void set_description(std::string value)
    {
        m_description = std::move(value);
    }

    [[nodiscard]] const std::string& get_description() const noexcept
    {
        return m_description;
    }

    std::int64_t m_id{};
    std::string m_name{};
    std::string m_slug{};
    std::string m_description{};
};


} // namespace blog::model

namespace serde {
template<>
struct Serializable<blog::model::Tag>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &blog::model::Tag::get_id,
                &blog::model::Tag::set_id,
                serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<
                "name",
                &blog::model::Tag::get_name,
                &blog::model::Tag::set_name,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "slug",
                &blog::model::Tag::get_slug,
                &blog::model::Tag::set_slug,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "description",
                &blog::model::Tag::get_description,
                &blog::model::Tag::set_description,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "tags";
    }
};
} // namespace serde


#ifdef CONGELADO_TEST
namespace blog::model::tests {
using namespace boost::ut;

suite<"blog_model_tag"> blog_model_tag_suite = []
{
    "reflects Tag fields"_test = []
    {
        static_assert(core::db::reflect::ReflectedEntity<Tag>);
        expect(core::db::reflect::field_count_v<Tag> == 4);
        expect(core::db::reflect::EntityMeta<Tag>::table == "tags");
    };
};

} // namespace blog::model::tests
#endif
