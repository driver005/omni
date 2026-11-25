module;
export module blog_handlers:members_dto_signin_urls_response;

import std;
import core_serde;
import :members_dto_signin_url;

export namespace blog::dto::members {

/** @brief Ghost API DTO: SigninUrlsResponse. */
class SigninUrlsResponse
{
public:
    void set_signin_urls(std::vector<SigninUrl> value)
    {
        signin_urls = std::move(value);
    }

    [[nodiscard]] const std::vector<SigninUrl>& get_signin_urls() const noexcept
    {
        return signin_urls;
    }

    std::vector<SigninUrl> signin_urls{};
};


} // namespace blog::dto::members

template<>
struct serde::Serializable<blog::dto::members::SigninUrlsResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{serde::FieldDesc<
            "member_signin_urls",
            &blog::dto::members::SigninUrlsResponse::get_signin_urls,
            &blog::dto::members::SigninUrlsResponse::set_signin_urls,
            serde::FieldOptions{}>{}};
    }

    static constexpr std::string_view table_name()
    {
        return "signin_urls_response";
    }
};
