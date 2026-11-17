module;
#include "entity_macros.h"

export module blog_model:comment;

import std;
import core_serde;
import core_database;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::model {

/** @brief Ghost comments entity — full field set from data/schema/schema.js. */
class Comment
{
public:
    Comment() = default;

    void set_id(std::int64_t value) { m_id = std::move(value); }
    [[nodiscard]] const std::int64_t& get_id() const noexcept { return m_id; }
    void set_post_id(std::string value) { m_post_id = std::move(value); }
    [[nodiscard]] const std::string& get_post_id() const noexcept { return m_post_id; }
    void set_member_id(std::string value) { m_member_id = std::move(value); }
    [[nodiscard]] const std::string& get_member_id() const noexcept { return m_member_id; }
    void set_parent_id(std::string value) { m_parent_id = std::move(value); }
    [[nodiscard]] const std::string& get_parent_id() const noexcept { return m_parent_id; }
    void set_in_reply_to_id(std::string value) { m_in_reply_to_id = std::move(value); }
    [[nodiscard]] const std::string& get_in_reply_to_id() const noexcept { return m_in_reply_to_id; }
    void set_status(std::string value) { m_status = std::move(value); }
    [[nodiscard]] const std::string& get_status() const noexcept { return m_status; }
    void set_html(std::string value) { m_html = std::move(value); }
    [[nodiscard]] const std::string& get_html() const noexcept { return m_html; }
    void set_edited_at(std::chrono::system_clock::time_point value) { m_edited_at = std::move(value); }
    [[nodiscard]] const std::chrono::system_clock::time_point& get_edited_at() const noexcept { return m_edited_at; }
    void set_pinned_at(std::chrono::system_clock::time_point value) { m_pinned_at = std::move(value); }
    [[nodiscard]] const std::chrono::system_clock::time_point& get_pinned_at() const noexcept { return m_pinned_at; }
    void set_created_at(std::chrono::system_clock::time_point value) { m_created_at = std::move(value); }
    [[nodiscard]] const std::chrono::system_clock::time_point& get_created_at() const noexcept { return m_created_at; }
    void set_updated_at(std::chrono::system_clock::time_point value) { m_updated_at = std::move(value); }
    [[nodiscard]] const std::chrono::system_clock::time_point& get_updated_at() const noexcept { return m_updated_at; }

    std::int64_t m_id{};
    std::string m_post_id{};
    std::string m_member_id{};
    std::string m_parent_id{};
    std::string m_in_reply_to_id{};
    std::string m_status{};
    std::string m_html{};
    std::chrono::system_clock::time_point m_edited_at{};
    std::chrono::system_clock::time_point m_pinned_at{};
    std::chrono::system_clock::time_point m_created_at{};
    std::chrono::system_clock::time_point m_updated_at{};
};




} // namespace blog::model

namespace serde {
template<>
struct Serializable<blog::model::Comment>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"id", &blog::model::Comment::get_id, &blog::model::Comment::set_id, serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<"post_id", &blog::model::Comment::get_post_id, &blog::model::Comment::set_post_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"member_id", &blog::model::Comment::get_member_id, &blog::model::Comment::set_member_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"parent_id", &blog::model::Comment::get_parent_id, &blog::model::Comment::set_parent_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"in_reply_to_id", &blog::model::Comment::get_in_reply_to_id, &blog::model::Comment::set_in_reply_to_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"status", &blog::model::Comment::get_status, &blog::model::Comment::set_status, serde::FieldOptions{}>{},
            serde::FieldDesc<"html", &blog::model::Comment::get_html, &blog::model::Comment::set_html, serde::FieldOptions{}>{},
            serde::FieldDesc<"edited_at", &blog::model::Comment::get_edited_at, &blog::model::Comment::set_edited_at, serde::FieldOptions{}>{},
            serde::FieldDesc<"pinned_at", &blog::model::Comment::get_pinned_at, &blog::model::Comment::set_pinned_at, serde::FieldOptions{}>{},
            serde::FieldDesc<"created_at", &blog::model::Comment::get_created_at, &blog::model::Comment::set_created_at, serde::FieldOptions{}>{},
            serde::FieldDesc<"updated_at", &blog::model::Comment::get_updated_at, &blog::model::Comment::set_updated_at, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "comments"; }
};
} // namespace serde


#ifdef CONGELADO_TEST
namespace blog::model::tests {
using namespace boost::ut;
suite<"blog_model_comment"> blog_model_comment_suite = []
{
    "reflects full Ghost comments schema"_test = []
    {
        static_assert(core::db::reflect::ReflectedEntity<Comment>);
        expect(core::db::reflect::field_count_v<Comment> == 11);
        expect(core::db::reflect::EntityMeta<Comment>::table == "comments");
    };
};
} // namespace blog::model::tests
#endif
