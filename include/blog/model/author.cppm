module;
#include "entity_macros.h"

export module blog_model:author;

import std;
import core_serde;
import core_database;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::model {

class Author
{
public:
    Author() = default;

    void set_id(std::int64_t value) { m_id = std::move(value); }
    [[nodiscard]] const std::int64_t& get_id() const noexcept { return m_id; }
    void set_name(std::string value) { m_name = std::move(value); }
    [[nodiscard]] const std::string& get_name() const noexcept { return m_name; }
    void set_slug(std::string value) { m_slug = std::move(value); }
    [[nodiscard]] const std::string& get_slug() const noexcept { return m_slug; }
    void set_email(std::string value) { m_email = std::move(value); }
    [[nodiscard]] const std::string& get_email() const noexcept { return m_email; }

    std::int64_t m_id{};
    std::string m_name{};
    std::string m_slug{};
    std::string m_email{};
};




} // namespace blog::model

namespace serde {
template<>
struct Serializable<blog::model::Author>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"id", &blog::model::Author::get_id, &blog::model::Author::set_id, serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<"name", &blog::model::Author::get_name, &blog::model::Author::set_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"slug", &blog::model::Author::get_slug, &blog::model::Author::set_slug, serde::FieldOptions{}>{},
            serde::FieldDesc<"email", &blog::model::Author::get_email, &blog::model::Author::set_email, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "authors"; }
};
} // namespace serde


#ifdef CONGELADO_TEST
namespace blog::model::tests {
using namespace boost::ut;

suite<"blog_model_author"> blog_model_author_suite = []
{
    "reflects Author fields"_test = []
    {
        static_assert(core::db::reflect::ReflectedEntity<Author>);
        expect(core::db::reflect::field_count_v<Author> == 4);
        expect(core::db::reflect::EntityMeta<Author>::table == "authors");
    };
};

} // namespace blog::model::tests
#endif
