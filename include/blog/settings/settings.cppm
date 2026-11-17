export module blog_settings:settings;

import std;
import core_settings;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace blog::settings {

/** @brief Ghost settings service — typed keys over the core settings facade. */
class SettingsService
{
public:
    SettingsService() :
        m_settings{core::settings::SettingsBuilder{}.build()}
    {
        register_defaults();
    }

    [[nodiscard]] bool set(std::string_view key, std::string value)
    {
        return m_settings.set(key, core::settings::SettingValue{std::move(value)});
    }

    [[nodiscard]] std::string get(std::string_view key, std::string fallback = {}) const
    {
        const auto value = m_settings.get_or_default(key);
        if (const auto* text = std::get_if<std::string>(&value)) {
            return *text;
        }
        return fallback;
    }

    [[nodiscard]] bool has(std::string_view key) const noexcept
    {
        return m_settings.contains(key);
    }

private:
    void register_defaults()
    {
        m_settings.register_key(core::settings::SettingKey{
            "title", core::settings::SettingValue{std::string{"Ghost"}}, "nonempty"
        });
        m_settings.register_key(core::settings::SettingKey{
            "description", core::settings::SettingValue{std::string{}}, ""
        });
        // Real Ghost default-settings.json scalar keys (first 40).
        m_settings.register_key(core::settings::SettingKey{"last_mentions_report_email_timestamp", core::settings::SettingValue{0.0}, "positive"});
        m_settings.register_key(core::settings::SettingKey{"db_hash", core::settings::SettingValue{std::string{}}, "nonempty"});
        m_settings.register_key(core::settings::SettingKey{"next_update_check", core::settings::SettingValue{0.0}, "positive"});
        m_settings.register_key(core::settings::SettingKey{"admin_session_secret", core::settings::SettingValue{std::string{}}, "nonempty"});
        m_settings.register_key(core::settings::SettingKey{"theme_session_secret", core::settings::SettingValue{std::string{}}, "nonempty"});
        m_settings.register_key(core::settings::SettingKey{"ghost_public_key", core::settings::SettingValue{std::string{}}, "nonempty"});
        m_settings.register_key(core::settings::SettingKey{"ghost_private_key", core::settings::SettingValue{std::string{}}, "nonempty"});
        m_settings.register_key(core::settings::SettingKey{"ghost_next_private_key", core::settings::SettingValue{std::string{}}, "nonempty"});
        m_settings.register_key(core::settings::SettingKey{"ghost_previous_public_key", core::settings::SettingValue{std::string{}}, "nonempty"});
        m_settings.register_key(core::settings::SettingKey{"members_public_key", core::settings::SettingValue{std::string{}}, "nonempty"});
        m_settings.register_key(core::settings::SettingKey{"members_private_key", core::settings::SettingValue{std::string{}}, "nonempty"});
        m_settings.register_key(core::settings::SettingKey{"members_next_private_key", core::settings::SettingValue{std::string{}}, "nonempty"});
        m_settings.register_key(core::settings::SettingKey{"members_previous_public_key", core::settings::SettingValue{std::string{}}, "nonempty"});
        m_settings.register_key(core::settings::SettingKey{"members_email_auth_secret", core::settings::SettingValue{std::string{}}, "nonempty"});
        m_settings.register_key(core::settings::SettingKey{"members_stripe_webhook_id", core::settings::SettingValue{std::string{}}, "nonempty"});
        m_settings.register_key(core::settings::SettingKey{"members_stripe_webhook_secret", core::settings::SettingValue{std::string{}}, "nonempty"});
        m_settings.register_key(core::settings::SettingKey{"members_otc_secret", core::settings::SettingValue{std::string{}}, "nonempty"});
        m_settings.register_key(core::settings::SettingKey{"machine_payments_secret", core::settings::SettingValue{std::string{}}, "nonempty"});
        m_settings.register_key(core::settings::SettingKey{"machine_payments_deposit_address", core::settings::SettingValue{std::string{}}, "nonempty"});
        m_settings.register_key(core::settings::SettingKey{"site_uuid", core::settings::SettingValue{std::string{}}, "nonempty"});
        m_settings.register_key(core::settings::SettingKey{"title", core::settings::SettingValue{std::string{}}, "nonempty"});
        m_settings.register_key(core::settings::SettingKey{"description", core::settings::SettingValue{std::string{}}, "nonempty"});
        m_settings.register_key(core::settings::SettingKey{"heading_font", core::settings::SettingValue{std::string{}}, "nonempty"});
        m_settings.register_key(core::settings::SettingKey{"body_font", core::settings::SettingValue{std::string{}}, "nonempty"});
        m_settings.register_key(core::settings::SettingKey{"logo", core::settings::SettingValue{std::string{}}, "nonempty"});
        m_settings.register_key(core::settings::SettingKey{"cover_image", core::settings::SettingValue{std::string{}}, "nonempty"});
        m_settings.register_key(core::settings::SettingKey{"icon", core::settings::SettingValue{std::string{}}, "nonempty"});
        m_settings.register_key(core::settings::SettingKey{"accent_color", core::settings::SettingValue{std::string{}}, "nonempty"});
        m_settings.register_key(core::settings::SettingKey{"locale", core::settings::SettingValue{std::string{}}, "nonempty"});
        m_settings.register_key(core::settings::SettingKey{"timezone", core::settings::SettingValue{std::string{}}, "nonempty"});
        m_settings.register_key(core::settings::SettingKey{"codeinjection_head", core::settings::SettingValue{std::string{}}, "nonempty"});
        m_settings.register_key(core::settings::SettingKey{"codeinjection_foot", core::settings::SettingValue{std::string{}}, "nonempty"});
        m_settings.register_key(core::settings::SettingKey{"facebook", core::settings::SettingValue{std::string{}}, "nonempty"});
        m_settings.register_key(core::settings::SettingKey{"twitter", core::settings::SettingValue{std::string{}}, "nonempty"});
        m_settings.register_key(core::settings::SettingKey{"threads", core::settings::SettingValue{std::string{}}, "nonempty"});
        m_settings.register_key(core::settings::SettingKey{"bluesky", core::settings::SettingValue{std::string{}}, "nonempty"});
        m_settings.register_key(core::settings::SettingKey{"mastodon", core::settings::SettingValue{std::string{}}, "nonempty"});
        m_settings.register_key(core::settings::SettingKey{"tiktok", core::settings::SettingValue{std::string{}}, "nonempty"});
        m_settings.register_key(core::settings::SettingKey{"youtube", core::settings::SettingValue{std::string{}}, "nonempty"});
        m_settings.register_key(core::settings::SettingKey{"instagram", core::settings::SettingValue{std::string{}}, "nonempty"});
    }

    core::settings::Settings m_settings{};
};

} // namespace blog::settings

#ifdef CONGELADO_TEST
namespace blog::settings::tests {
using namespace boost::ut;
suite<"blog_settings_settings"> blog_settings_settings_suite = []
{
    "reads and writes settings"_test = []
    {
        SettingsService service;
        expect(service.get("title") == "Ghost");
        expect(service.set("title", "My Blog"));
        expect(service.get("title") == "My Blog");
        expect(service.has("title"));
        expect(service.has("db_hash"));
    };
};
} // namespace blog::settings::tests
#endif
