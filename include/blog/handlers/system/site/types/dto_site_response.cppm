module;
#include <rfl/Generic.hpp>
export module blog_handlers:site_dto_site_response;

import std;
import core_serde;

export namespace blog::dto::site {

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
    void set_site_uuid(std::string value) { site_uuid = std::move(value); }
    [[nodiscard]] const std::string& get_site_uuid() const noexcept { return site_uuid; }
    void set_version(std::string value) { version = std::move(value); }
    [[nodiscard]] const std::string& get_version() const noexcept { return version; }
    void set_cover_image(std::string value) { cover_image = std::move(value); }
    [[nodiscard]] const std::string& get_cover_image() const noexcept { return cover_image; }
    void set_allow_external_signup(bool value) { allow_external_signup = std::move(value); }
    [[nodiscard]] const bool& get_allow_external_signup() const noexcept { return allow_external_signup; }
    void set_icon(rfl::Generic value) { icon = std::move(value); }
    [[nodiscard]] const rfl::Generic& get_icon() const noexcept { return icon; }
    void set_logo(rfl::Generic value) { logo = std::move(value); }
    [[nodiscard]] const rfl::Generic& get_logo() const noexcept { return logo; }
    void set_sentry_dsn(std::string value) { sentry_dsn = std::move(value); }
    [[nodiscard]] const std::string& get_sentry_dsn() const noexcept { return sentry_dsn; }
    void set_sentry_env(std::string value) { sentry_env = std::move(value); }
    [[nodiscard]] const std::string& get_sentry_env() const noexcept { return sentry_env; }

    std::string title{"Ghost"};
    std::string description{"Thoughts, stories and ideas"};
    std::string accent_color{"#FF1A75"};
    std::string locale{"en"};
    std::string timezone{"Etc/UTC"};
    std::string url{"http://127.0.0.1:2369/"};
    std::string site_uuid{"d5432f87-1d1a-4d2f-9f95-5c1a2f0a9e13"};
    std::string version{"5.0.0"};
    std::string cover_image{"https://static.ghost.org/v5.0.0/images/publication-cover.jpg"};
    bool allow_external_signup{true};
    rfl::Generic icon{};
    rfl::Generic logo{};
    std::string sentry_dsn{};
    std::string sentry_env{};
};

} // namespace blog::dto::site

template<>
struct serde::Serializable<blog::dto::site::SiteResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"title", &blog::dto::site::SiteResponse::get_title, &blog::dto::site::SiteResponse::set_title, serde::FieldOptions{}>{},
            serde::FieldDesc<"description", &blog::dto::site::SiteResponse::get_description, &blog::dto::site::SiteResponse::set_description, serde::FieldOptions{}>{},
            serde::FieldDesc<"accent_color", &blog::dto::site::SiteResponse::get_accent_color, &blog::dto::site::SiteResponse::set_accent_color, serde::FieldOptions{}>{},
            serde::FieldDesc<"locale", &blog::dto::site::SiteResponse::get_locale, &blog::dto::site::SiteResponse::set_locale, serde::FieldOptions{}>{},
            serde::FieldDesc<"timezone", &blog::dto::site::SiteResponse::get_timezone, &blog::dto::site::SiteResponse::set_timezone, serde::FieldOptions{}>{},
            serde::FieldDesc<"url", &blog::dto::site::SiteResponse::get_url, &blog::dto::site::SiteResponse::set_url, serde::FieldOptions{}>{},
            serde::FieldDesc<"site_uuid", &blog::dto::site::SiteResponse::get_site_uuid, &blog::dto::site::SiteResponse::set_site_uuid, serde::FieldOptions{}>{},
            serde::FieldDesc<"version", &blog::dto::site::SiteResponse::get_version, &blog::dto::site::SiteResponse::set_version, serde::FieldOptions{}>{},
            serde::FieldDesc<"cover_image", &blog::dto::site::SiteResponse::get_cover_image, &blog::dto::site::SiteResponse::set_cover_image, serde::FieldOptions{}>{},
            serde::FieldDesc<"allow_external_signup", &blog::dto::site::SiteResponse::get_allow_external_signup, &blog::dto::site::SiteResponse::set_allow_external_signup, serde::FieldOptions{}>{},
            serde::FieldDesc<"icon", &blog::dto::site::SiteResponse::get_icon, &blog::dto::site::SiteResponse::set_icon, serde::FieldOptions{}>{},
            serde::FieldDesc<"logo", &blog::dto::site::SiteResponse::get_logo, &blog::dto::site::SiteResponse::set_logo, serde::FieldOptions{}>{},
            serde::FieldDesc<"sentry_dsn", &blog::dto::site::SiteResponse::get_sentry_dsn, &blog::dto::site::SiteResponse::set_sentry_dsn, serde::FieldOptions{}>{},
            serde::FieldDesc<"sentry_env", &blog::dto::site::SiteResponse::get_sentry_env, &blog::dto::site::SiteResponse::set_sentry_env, serde::FieldOptions{}>{}
        };
    }
    static constexpr std::string_view table_name() { return "site_response"; }
};
