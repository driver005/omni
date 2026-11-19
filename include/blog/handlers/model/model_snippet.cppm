export module blog_handlers:model_snippet;

import std;
import core_serde;
import core_database;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::model {

/** @brief Ghost snippets entity — full field set from data/schema/schema.js. */
class Snippet
{
public:
    Snippet() = default;

    void set_id(std::int64_t value) { m_id = std::move(value); }
    [[nodiscard]] const std::int64_t& get_id() const noexcept { return m_id; }
    void set_name(std::string value) { m_name = std::move(value); }
    [[nodiscard]] const std::string& get_name() const noexcept { return m_name; }
    void set_mobiledoc(std::string value) { m_mobiledoc = std::move(value); }
    [[nodiscard]] const std::string& get_mobiledoc() const noexcept { return m_mobiledoc; }
    void set_lexical(std::string value) { m_lexical = std::move(value); }
    [[nodiscard]] const std::string& get_lexical() const noexcept { return m_lexical; }
    void set_created_at(std::chrono::system_clock::time_point value) { m_created_at = std::move(value); }
    [[nodiscard]] const std::chrono::system_clock::time_point& get_created_at() const noexcept { return m_created_at; }
    void set_updated_at(std::chrono::system_clock::time_point value) { m_updated_at = std::move(value); }
    [[nodiscard]] const std::chrono::system_clock::time_point& get_updated_at() const noexcept { return m_updated_at; }

    std::int64_t m_id{};
    std::string m_name{};
    std::string m_mobiledoc{};
    std::string m_lexical{};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
};




} // namespace blog::model

namespace serde {
template<>
struct Serializable<blog::model::Snippet>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"id", &blog::model::Snippet::get_id, &blog::model::Snippet::set_id, serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<"name", &blog::model::Snippet::get_name, &blog::model::Snippet::set_name, serde::FieldOptions{}>{},
            serde::FieldDesc<"mobiledoc", &blog::model::Snippet::get_mobiledoc, &blog::model::Snippet::set_mobiledoc, serde::FieldOptions{}>{},
            serde::FieldDesc<"lexical", &blog::model::Snippet::get_lexical, &blog::model::Snippet::set_lexical, serde::FieldOptions{}>{},
            serde::FieldDesc<"created_at", &blog::model::Snippet::get_created_at, &blog::model::Snippet::set_created_at, serde::FieldOptions{}>{},
            serde::FieldDesc<"updated_at", &blog::model::Snippet::get_updated_at, &blog::model::Snippet::set_updated_at, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "snippets"; }
};
} // namespace serde


#ifdef CONGELADO_TEST
namespace blog::model::tests {
using namespace boost::ut;
suite<"blog_model_snippet"> blog_model_snippet_suite = []
{
    "reflects full Ghost snippets schema"_test = []
    {
        static_assert(core::db::reflect::ReflectedEntity<Snippet>);
        expect(core::db::reflect::field_count_v<Snippet> == 6);
        expect(core::db::reflect::EntityMeta<Snippet>::table == "snippets");
    };
};
} // namespace blog::model::tests
#endif
