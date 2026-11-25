export module blog_handlers:model_recommendation;

import std;
import core_serde;
import core_database;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::model {

/** @brief Ghost recommendations entity — full field set from data/schema/schema.js. */
class Recommendation
{
public:
    Recommendation() = default;

    void set_id(std::int64_t value)
    {
        m_id = std::move(value);
    }

    [[nodiscard]] const std::int64_t& get_id() const noexcept
    {
        return m_id;
    }

    void set_url(std::string value)
    {
        m_url = std::move(value);
    }

    [[nodiscard]] const std::string& get_url() const noexcept
    {
        return m_url;
    }

    void set_title(std::string value)
    {
        m_title = std::move(value);
    }

    [[nodiscard]] const std::string& get_title() const noexcept
    {
        return m_title;
    }

    void set_excerpt(std::string value)
    {
        m_excerpt = std::move(value);
    }

    [[nodiscard]] const std::string& get_excerpt() const noexcept
    {
        return m_excerpt;
    }

    void set_featured_image(std::string value)
    {
        m_featured_image = std::move(value);
    }

    [[nodiscard]] const std::string& get_featured_image() const noexcept
    {
        return m_featured_image;
    }

    void set_favicon(std::string value)
    {
        m_favicon = std::move(value);
    }

    [[nodiscard]] const std::string& get_favicon() const noexcept
    {
        return m_favicon;
    }

    void set_description(std::string value)
    {
        m_description = std::move(value);
    }

    [[nodiscard]] const std::string& get_description() const noexcept
    {
        return m_description;
    }

    void set_one_click_subscribe(bool value)
    {
        m_one_click_subscribe = std::move(value);
    }

    [[nodiscard]] const bool& get_one_click_subscribe() const noexcept
    {
        return m_one_click_subscribe;
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
    std::string m_url{};
    std::string m_title{};
    std::string m_excerpt{};
    std::string m_featured_image{};
    std::string m_favicon{};
    std::string m_description{};
    bool m_one_click_subscribe{};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
};


} // namespace blog::model

namespace serde {
template<>
struct Serializable<blog::model::Recommendation>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &blog::model::Recommendation::get_id,
                &blog::model::Recommendation::set_id,
                serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<
                "url",
                &blog::model::Recommendation::get_url,
                &blog::model::Recommendation::set_url,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "title",
                &blog::model::Recommendation::get_title,
                &blog::model::Recommendation::set_title,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "excerpt",
                &blog::model::Recommendation::get_excerpt,
                &blog::model::Recommendation::set_excerpt,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "featured_image",
                &blog::model::Recommendation::get_featured_image,
                &blog::model::Recommendation::set_featured_image,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "favicon",
                &blog::model::Recommendation::get_favicon,
                &blog::model::Recommendation::set_favicon,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "description",
                &blog::model::Recommendation::get_description,
                &blog::model::Recommendation::set_description,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "one_click_subscribe",
                &blog::model::Recommendation::get_one_click_subscribe,
                &blog::model::Recommendation::set_one_click_subscribe,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "created_at",
                &blog::model::Recommendation::get_created_at,
                &blog::model::Recommendation::set_created_at,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "updated_at",
                &blog::model::Recommendation::get_updated_at,
                &blog::model::Recommendation::set_updated_at,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "recommendations";
    }
};
} // namespace serde


#ifdef CONGELADO_TEST
namespace blog::model::tests {
using namespace boost::ut;
suite<"blog_model_recommendation"> blog_model_recommendation_suite = []
{
    "reflects full Ghost recommendations schema"_test = []
    {
        static_assert(core::db::reflect::ReflectedEntity<Recommendation>);
        expect(core::db::reflect::field_count_v<Recommendation> == 10);
        expect(core::db::reflect::EntityMeta<Recommendation>::table == "recommendations");
    };
};
} // namespace blog::model::tests
#endif
