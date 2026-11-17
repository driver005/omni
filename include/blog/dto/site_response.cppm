module;
#include <rfl/Generic.hpp>
export module blog_dto:site_response;

import std;
import core_serde;

export namespace blog::dto {

class SiteResponse
{
public:
    void set_title(std::string value) { title = std::move(value); }
    [[nodiscard]] const std::string& get_title() const noexcept { return title; }
    void set_description(std::string value) { description = std::move(value); }
    [[nodiscard]] const std::string& get_description() const noexcept { return description; }
    void set_accent_color(std::string value) { accent_color = std::move(value); }
    [[nodiscard]] const std::string& get_accent_color() const noexcept { return accent_color; }
    void set_locale(std::string value) { locale = std::move(value); }
    [[nodiscard]] const std::string& get_locale() const noexcept { return locale; }
    void set_timezone(std::string value) { timezone = std::move(value); }
    [[nodiscard]] const std::string& get_timezone() const noexcept { return timezone; }
    void set_url(std::string value) { url = std::move(value); }
    [[nodiscard]] const std::string& get_url() const noexcept { return url; }

    std::string title{"Ghost"};
    std::string description{"Thoughts, stories and ideas"};
    std::string accent_color{"#FF1A75"};
    std::string locale{"en"};
    std::string timezone{"Etc/UTC"};
    std::string url{"http://127.0.0.1:2369/"};
};

} // namespace blog::dto

template<>
struct serde::Serializable<blog::dto::SiteResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"title", &blog::dto::SiteResponse::get_title, &blog::dto::SiteResponse::set_title, serde::FieldOptions{}>{},
            serde::FieldDesc<"description", &blog::dto::SiteResponse::get_description, &blog::dto::SiteResponse::set_description, serde::FieldOptions{}>{},
            serde::FieldDesc<"accent_color", &blog::dto::SiteResponse::get_accent_color, &blog::dto::SiteResponse::set_accent_color, serde::FieldOptions{}>{},
            serde::FieldDesc<"locale", &blog::dto::SiteResponse::get_locale, &blog::dto::SiteResponse::set_locale, serde::FieldOptions{}>{},
            serde::FieldDesc<"timezone", &blog::dto::SiteResponse::get_timezone, &blog::dto::SiteResponse::set_timezone, serde::FieldOptions{}>{},
            serde::FieldDesc<"url", &blog::dto::SiteResponse::get_url, &blog::dto::SiteResponse::set_url, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "site_response"; }
};
