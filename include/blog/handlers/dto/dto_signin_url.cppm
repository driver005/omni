module;
export module blog_handlers:dto_signin_url;

import std;
import core_serde;


export namespace blog::dto {

/** @brief Ghost API DTO: SigninUrl. */
class SigninUrl
{
public:
    void set_member_id(std::string value) { member_id = std::move(value); }
    [[nodiscard]] const std::string& get_member_id() const noexcept { return member_id; }
    void set_url(std::string value) { url = std::move(value); }
    [[nodiscard]] const std::string& get_url() const noexcept { return url; }

    std::string member_id{};
    std::string url{};
};



} // namespace blog::dto

template<>
struct serde::Serializable<blog::dto::SigninUrl>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"member_id", &blog::dto::SigninUrl::get_member_id, &blog::dto::SigninUrl::set_member_id, serde::FieldOptions{}>{},
            serde::FieldDesc<"url", &blog::dto::SigninUrl::get_url, &blog::dto::SigninUrl::set_url, serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "signin_url";
    }
};

