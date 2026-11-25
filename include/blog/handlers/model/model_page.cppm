export module blog_handlers:model_page;

import std;
import core_serde;
import core_database;
import :model_member;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::model {

class Page
{
public:
    Page() = default;

    void set_id(std::int64_t value)
    {
        m_id = std::move(value);
    }

    [[nodiscard]] const std::int64_t& get_id() const noexcept
    {
        return m_id;
    }

    void set_uuid(std::string value)
    {
        m_uuid = std::move(value);
    }

    [[nodiscard]] const std::string& get_uuid() const noexcept
    {
        return m_uuid;
    }

    void set_title(std::string value)
    {
        m_title = std::move(value);
    }

    [[nodiscard]] const std::string& get_title() const noexcept
    {
        return m_title;
    }

    void set_slug(std::string value)
    {
        m_slug = std::move(value);
    }

    [[nodiscard]] const std::string& get_slug() const noexcept
    {
        return m_slug;
    }

    void set_html(std::string value)
    {
        m_html = std::move(value);
    }

    [[nodiscard]] const std::string& get_html() const noexcept
    {
        return m_html;
    }

    void set_status(std::string value)
    {
        m_status = std::move(value);
    }

    [[nodiscard]] const std::string& get_status() const noexcept
    {
        return m_status;
    }

    [[nodiscard]] std::string wire_uuid() const
    {
        return m_uuid.empty() ? serde::uuid_for(m_id) : m_uuid;
    }

    [[nodiscard]] std::string wire_excerpt() const
    {
        return m_title;
    }

    [[nodiscard]] std::string wire_comment_id() const
    {
        return "comment-" + std::to_string(m_id);
    }

    [[nodiscard]] static std::string wire_lexical()
    {
        return "{}";
    }

    [[nodiscard]] std::string wire_lexical_value() const
    {
        return "{}";
    }

    [[nodiscard]] serde::RawJson wire_count() const
    {
        return {R"({"clicks":0,"positive_feedback":0,"negative_feedback":0})"};
    }

    [[nodiscard]] bool wire_featured() const
    {
        return false;
    }

    [[nodiscard]] serde::RawJson wire_timestamp() const
    {
        return {R"("2024-01-01T00:00:00.000Z")"};
    }

    [[nodiscard]] serde::RawJson wire_null() const
    {
        return {"null"};
    }

    [[nodiscard]] serde::RawJson wire_empty_array() const
    {
        return {"[]"};
    }

    void set_wire_noop(serde::RawJson) {}

    void set_wire_string_noop(std::string) {}

    std::int64_t m_id{};
    std::string m_uuid{};
    std::string m_title{};
    std::string m_slug{};
    std::string m_html{};
    std::string m_status{};
};


} // namespace blog::model

namespace serde {
template<>
struct Serializable<blog::model::Page>
{
    using M = blog::model::Page;

    static constexpr auto fields()
    {
        return std::tuple{
            FieldDesc<
                "id",
                &M::get_id,
                &M::set_id,
                FieldOptions::init().with_db(FieldOptionsDb::init().pk()),
                wire_id_normalizer{}>{},
            FieldDesc<"uuid", &M::wire_uuid, &M::set_uuid>{},
            FieldDesc<"title", &M::get_title, &M::set_title>{},
            FieldDesc<"slug", &M::get_slug, &M::set_slug>{},
            FieldDesc<"html", &M::get_html, &M::set_html>{},
            FieldDesc<"status", &M::get_status, &M::set_status>{},
            FieldDesc<"excerpt", &M::wire_excerpt, &M::set_wire_string_noop>{},
            FieldDesc<"canonical_url", &M::wire_null, &M::set_wire_noop>{},
            FieldDesc<"codeinjection_foot", &M::wire_null, &M::set_wire_noop>{},
            FieldDesc<"codeinjection_head", &M::wire_null, &M::set_wire_noop>{},
            FieldDesc<"comment_id", &M::wire_comment_id, &M::set_wire_string_noop>{},
            FieldDesc<"custom_excerpt", &M::wire_null, &M::set_wire_noop>{},
            FieldDesc<"custom_template", &M::wire_null, &M::set_wire_noop>{},
            FieldDesc<"feature_image", &M::wire_null, &M::set_wire_noop>{},
            FieldDesc<"feature_image_alt", &M::wire_null, &M::set_wire_noop>{},
            FieldDesc<"feature_image_caption", &M::wire_null, &M::set_wire_noop>{},
            FieldDesc<"featured", &M::wire_featured, &M::set_wire_noop>{},
            FieldDesc<"frontmatter", &M::wire_null, &M::set_wire_noop>{},
            FieldDesc<"lexical", &M::wire_lexical_value, &M::set_wire_string_noop>{},
            FieldDesc<"authors", &M::wire_empty_array, &M::set_wire_noop>{},
            FieldDesc<"count", &M::wire_count, &M::set_wire_noop>{},
            FieldDesc<"created_at", &M::wire_timestamp, &M::set_wire_noop>{},
            FieldDesc<"updated_at", &M::wire_timestamp, &M::set_wire_noop>{}
        };
    }

    static constexpr auto db_fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "id",
                &blog::model::Page::get_id,
                &blog::model::Page::set_id,
                serde::FieldOptions::init().with_db(serde::FieldOptionsDb::init().pk())>{},
            serde::FieldDesc<
                "uuid",
                &blog::model::Page::get_uuid,
                &blog::model::Page::set_uuid,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "title",
                &blog::model::Page::get_title,
                &blog::model::Page::set_title,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "slug",
                &blog::model::Page::get_slug,
                &blog::model::Page::set_slug,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "html",
                &blog::model::Page::get_html,
                &blog::model::Page::set_html,
                serde::FieldOptions{}>{},
            serde::FieldDesc<
                "status",
                &blog::model::Page::get_status,
                &blog::model::Page::set_status,
                serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "pages";
    }
};
} // namespace serde


#ifdef CONGELADO_TEST
namespace blog::model::tests {
using namespace boost::ut;

suite<"blog_model_page"> blog_model_page_suite = []
{
    "reflects Page fields"_test = []
    {
        static_assert(core::db::reflect::ReflectedEntity<Page>);
        expect(core::db::reflect::field_count_v<Page> == 6);
        expect(core::db::reflect::EntityMeta<Page>::table == "pages");
    };
};

} // namespace blog::model::tests
#endif
