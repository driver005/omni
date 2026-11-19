module;
#include <rfl.hpp>
export module blog_handlers:dto_config_response;

import std;
import core_serde;
import :dto_config_auth;

export namespace blog::dto {

/** @brief Ghost API DTO: ConfigResponse. */
class ConfigResponse
{
public:
    void set_auth(ConfigAuth value) { auth = std::move(value); }
    [[nodiscard]] const ConfigAuth& get_auth() const noexcept { return auth; }
    void set_clientExtensions(rfl::Generic value) { clientExtensions = std::move(value); }
    [[nodiscard]] const rfl::Generic& get_clientExtensions() const noexcept { return clientExtensions; }
    void set_database(std::string value) { database = std::move(value); }
    [[nodiscard]] const std::string& get_database() const noexcept { return database; }
    void set_emailAnalytics(bool value) { emailAnalytics = std::move(value); }
    [[nodiscard]] const bool& get_emailAnalytics() const noexcept { return emailAnalytics; }
    void set_enableDeveloperExperiments(bool value) { enableDeveloperExperiments = std::move(value); }
    [[nodiscard]] const bool& get_enableDeveloperExperiments() const noexcept { return enableDeveloperExperiments; }
    void set_environment(std::string value) { environment = std::move(value); }
    [[nodiscard]] const std::string& get_environment() const noexcept { return environment; }
    void set_mail(std::string value) { mail = std::move(value); }
    [[nodiscard]] const std::string& get_mail() const noexcept { return mail; }
    void set_mailgunIsConfigured(bool value) { mailgunIsConfigured = std::move(value); }
    [[nodiscard]] const bool& get_mailgunIsConfigured() const noexcept { return mailgunIsConfigured; }
    void set_stripeDirect(bool value) { stripeDirect = std::move(value); }
    [[nodiscard]] const bool& get_stripeDirect() const noexcept { return stripeDirect; }
    void set_useGravatar(bool value) { useGravatar = std::move(value); }
    [[nodiscard]] const bool& get_useGravatar() const noexcept { return useGravatar; }
    void set_labs(rfl::Generic value) { labs = std::move(value); }
    [[nodiscard]] const rfl::Generic& get_labs() const noexcept { return labs; }
    void set_version(std::string value) { version = std::move(value); }
    [[nodiscard]] const std::string& get_version() const noexcept { return version; }
    void set_exploreTestimonialsUrl(std::string value) { exploreTestimonialsUrl = std::move(value); }
    [[nodiscard]] const std::string& get_exploreTestimonialsUrl() const noexcept { return exploreTestimonialsUrl; }
    void set_klipy(rfl::Generic value) { klipy = std::move(value); }
    [[nodiscard]] const rfl::Generic& get_klipy() const noexcept { return klipy; }
    void set_security(rfl::Generic value) { security = std::move(value); }
    [[nodiscard]] const rfl::Generic& get_security() const noexcept { return security; }
    void set_signupForm(rfl::Generic value) { signupForm = std::move(value); }
    [[nodiscard]] const rfl::Generic& get_signupForm() const noexcept { return signupForm; }

    ConfigAuth auth{};
    rfl::Generic clientExtensions{rfl::Object<rfl::Generic>{}};
    std::string database{"mysql8"};
    bool emailAnalytics{true};
    bool enableDeveloperExperiments{false};
    std::string environment{"testing-mysql"};
    std::string mail{};
    bool mailgunIsConfigured{false};
    bool stripeDirect{false};
    bool useGravatar{false};
    rfl::Generic labs{rfl::Object<rfl::Generic>{}};
    std::string version{"5.0.0"};
    std::string exploreTestimonialsUrl{"https://testing.com/that/this/is/set/correctly"};
    rfl::Generic klipy{rfl::Object<rfl::Generic>{}};
    rfl::Generic security{rfl::Object<rfl::Generic>{}};
    rfl::Generic signupForm{rfl::Object<rfl::Generic>{}};
};



class ConfigEnvelope
{
public:
    void set_config(ConfigResponse value) { config = std::move(value); }
    [[nodiscard]] const ConfigResponse& get_config() const noexcept { return config; }

    ConfigResponse config{};
};

} // namespace blog::dto

template<>
struct serde::Serializable<blog::dto::ConfigResponse>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"auth", &blog::dto::ConfigResponse::get_auth, &blog::dto::ConfigResponse::set_auth, serde::FieldOptions{}>{},
            serde::FieldDesc<"clientExtensions", &blog::dto::ConfigResponse::get_clientExtensions, &blog::dto::ConfigResponse::set_clientExtensions, serde::FieldOptions{}>{},
            serde::FieldDesc<"database", &blog::dto::ConfigResponse::get_database, &blog::dto::ConfigResponse::set_database, serde::FieldOptions{}>{},
            serde::FieldDesc<"emailAnalytics", &blog::dto::ConfigResponse::get_emailAnalytics, &blog::dto::ConfigResponse::set_emailAnalytics, serde::FieldOptions{}>{},
            serde::FieldDesc<"enableDeveloperExperiments", &blog::dto::ConfigResponse::get_enableDeveloperExperiments, &blog::dto::ConfigResponse::set_enableDeveloperExperiments, serde::FieldOptions{}>{},
            serde::FieldDesc<"environment", &blog::dto::ConfigResponse::get_environment, &blog::dto::ConfigResponse::set_environment, serde::FieldOptions{}>{},
            serde::FieldDesc<"mail", &blog::dto::ConfigResponse::get_mail, &blog::dto::ConfigResponse::set_mail, serde::FieldOptions{}>{},
            serde::FieldDesc<"mailgunIsConfigured", &blog::dto::ConfigResponse::get_mailgunIsConfigured, &blog::dto::ConfigResponse::set_mailgunIsConfigured, serde::FieldOptions{}>{},
            serde::FieldDesc<"stripeDirect", &blog::dto::ConfigResponse::get_stripeDirect, &blog::dto::ConfigResponse::set_stripeDirect, serde::FieldOptions{}>{},
            serde::FieldDesc<"useGravatar", &blog::dto::ConfigResponse::get_useGravatar, &blog::dto::ConfigResponse::set_useGravatar, serde::FieldOptions{}>{},
            serde::FieldDesc<"labs", &blog::dto::ConfigResponse::get_labs, &blog::dto::ConfigResponse::set_labs, serde::FieldOptions{}>{},
            serde::FieldDesc<"version", &blog::dto::ConfigResponse::get_version, &blog::dto::ConfigResponse::set_version, serde::FieldOptions{}>{},
            serde::FieldDesc<"exploreTestimonialsUrl", &blog::dto::ConfigResponse::get_exploreTestimonialsUrl, &blog::dto::ConfigResponse::set_exploreTestimonialsUrl, serde::FieldOptions{}>{},
            serde::FieldDesc<"klipy", &blog::dto::ConfigResponse::get_klipy, &blog::dto::ConfigResponse::set_klipy, serde::FieldOptions{}>{},
            serde::FieldDesc<"security", &blog::dto::ConfigResponse::get_security, &blog::dto::ConfigResponse::set_security, serde::FieldOptions{}>{},
            serde::FieldDesc<"signupForm", &blog::dto::ConfigResponse::get_signupForm, &blog::dto::ConfigResponse::set_signupForm, serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "config_response";
    }
};

template<>
struct serde::Serializable<blog::dto::ConfigEnvelope>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<"config", &blog::dto::ConfigEnvelope::get_config, &blog::dto::ConfigEnvelope::set_config, serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "config_envelope";
    }
};
