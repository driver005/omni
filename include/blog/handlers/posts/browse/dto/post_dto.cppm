module;
#include <rfl.hpp>
export module blog_handlers_posts_browse:dto_post;

import std;
import core_serde;

export namespace blog::handlers::posts::browse::dto {

/** @brief Ghost-shaped post DTO — full field set from the snapshot schema. */
struct PostDto
{
    std::string id{"1"};
    std::string uuid{"post-1"};
    std::string title{};
    std::string slug{};
    std::string mobiledoc{};
    std::string lexical{};
    std::string html{};
    std::string comment_id{};
    std::string excerpt{};
    std::string custom_excerpt{};
    rfl::Generic canonical_url{};
    rfl::Generic codeinjection_head{};
    rfl::Generic codeinjection_foot{};
    rfl::Generic feature_image{};
    rfl::Generic feature_image_alt{};
    rfl::Generic feature_image_caption{};
    bool featured{false};
    rfl::Generic custom_template{};
    rfl::Generic frontmatter{};
    rfl::Generic meta_title{};
    rfl::Generic meta_description{};
    rfl::Generic email{};
    bool email_only{false};
    std::string email_segment{"all"};
    rfl::Generic email_subject{};
    std::string created_at{"2024-01-01T00:00:00.000Z"};
    std::string updated_at{"2024-01-01T00:00:00.000Z"};
    std::string status{"published"};
    std::vector<rfl::Generic> authors{};
    rfl::Generic count{rfl::Object<rfl::Generic>{}};
    std::string type{"post"};
    std::string locale{"en"};
    std::string visibility{"public"};

#define POST_DTO_ACCESSORS(Field)                                                   \
    void set_##Field(std::string value)                                              \
    {                                                                               \
        Field = std::move(value);                                                    \
    }                                                                               \
    [[nodiscard]] const std::string& get_##Field() const noexcept                    \
    {                                                                               \
        return Field;                                                                \
    }
    POST_DTO_ACCESSORS(id)
    POST_DTO_ACCESSORS(uuid)
    POST_DTO_ACCESSORS(title)
    POST_DTO_ACCESSORS(slug)
    POST_DTO_ACCESSORS(mobiledoc)
    POST_DTO_ACCESSORS(lexical)
    POST_DTO_ACCESSORS(html)
    POST_DTO_ACCESSORS(comment_id)
    POST_DTO_ACCESSORS(excerpt)
    POST_DTO_ACCESSORS(custom_excerpt)
    POST_DTO_ACCESSORS(email_segment)
    POST_DTO_ACCESSORS(created_at)
    POST_DTO_ACCESSORS(updated_at)
    POST_DTO_ACCESSORS(status)
    POST_DTO_ACCESSORS(type)
    POST_DTO_ACCESSORS(locale)
    POST_DTO_ACCESSORS(visibility)
#undef POST_DTO_ACCESSORS

#define POST_DTO_GENERIC_ACCESSORS(Field)                                            \
    void set_##Field(rfl::Generic value)                                             \
    {                                                                               \
        Field = std::move(value);                                                    \
    }                                                                               \
    [[nodiscard]] const rfl::Generic& get_##Field() const noexcept                   \
    {                                                                               \
        return Field;                                                                \
    }
    POST_DTO_GENERIC_ACCESSORS(meta_title)
    POST_DTO_GENERIC_ACCESSORS(meta_description)
    POST_DTO_GENERIC_ACCESSORS(canonical_url)
    POST_DTO_GENERIC_ACCESSORS(codeinjection_head)
    POST_DTO_GENERIC_ACCESSORS(codeinjection_foot)
    POST_DTO_GENERIC_ACCESSORS(feature_image)
    POST_DTO_GENERIC_ACCESSORS(feature_image_alt)
    POST_DTO_GENERIC_ACCESSORS(feature_image_caption)
    POST_DTO_GENERIC_ACCESSORS(custom_template)
    POST_DTO_GENERIC_ACCESSORS(frontmatter)
    POST_DTO_GENERIC_ACCESSORS(email)
    POST_DTO_GENERIC_ACCESSORS(email_subject)
    POST_DTO_GENERIC_ACCESSORS(count)
#undef POST_DTO_GENERIC_ACCESSORS

    void set_featured(bool value) { featured = value; }
    [[nodiscard]] const bool& get_featured() const noexcept { return featured; }
    void set_email_only(bool value) { email_only = value; }
    [[nodiscard]] const bool& get_email_only() const noexcept { return email_only; }
    void set_authors(std::vector<rfl::Generic> value) { authors = std::move(value); }
    [[nodiscard]] const std::vector<rfl::Generic>& get_authors() const noexcept { return authors; }
};

} // namespace blog::handlers::posts::browse::dto

#define POST_DTO_FIELD(Name, Member)                                                \
    serde::FieldDesc<Name, &blog::handlers::posts::browse::dto::PostDto::get_##Member, \
                     &blog::handlers::posts::browse::dto::PostDto::set_##Member,      \
                     serde::FieldOptions{}>{}

