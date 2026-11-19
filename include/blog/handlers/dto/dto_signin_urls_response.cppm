module;
export module blog_handlers:dto_signin_urls_response;

import std;
import core_serde;
import :dto_signin_url;

export namespace blog::dto {

/** @brief Ghost API DTO: SigninUrlsResponse. */
class SigninUrlsResponse
{
public:
    void set_signin_urls(std::vector<SigninUrl> value) { signin_urls = std::move(value); }
    [[nodiscard]] const std::vector<SigninUrl>& get_signin_urls() const noexcept { return signin_urls; }

    std::vector<SigninUrl> signin_urls{};
};



} // namespace blog::dto

template<>
struct serde::Serializable<blog::dto::SigninUrlsResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"signin_urls", &blog::dto::SigninUrlsResponse::get_signin_urls, &blog::dto::SigninUrlsResponse::set_signin_urls, serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "signin_urls_response";
    }
};

