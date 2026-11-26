export module app_store:configs_result;

import std;
import core_serde;

export namespace app::handlers::configs {

// One Result per handle_* method. Each mirrors the typed {key, value} wire
// shape. The Serializable field order is `key`, `value`, so
// serde::Ser::serialize emits the upstream key order.

class ConfigKeyCreateResult
{
public:
    void set_key(std::string value)
    {
        key = std::move(value);
    }

    [[nodiscard]] const std::string& get_key() const noexcept
    {
        return key;
    }

    void set_value(std::string value)
    {
        value = std::move(value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

private:
    std::string key{};
    std::string value{};
};

class ConfigKeyRetrieveResult
{
public:
    void set_key(std::string value)
    {
        key = std::move(value);
    }

    [[nodiscard]] const std::string& get_key() const noexcept
    {
        return key;
    }

    void set_value(std::string value)
    {
        value = std::move(value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

private:
    std::string key{};
    std::string value{};
};

class ConfigKeyUpdateResult
{
public:
    void set_key(std::string value)
    {
        key = std::move(value);
    }

    [[nodiscard]] const std::string& get_key() const noexcept
    {
        return key;
    }

    void set_value(std::string value)
    {
        value = std::move(value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

private:
    std::string key{};
    std::string value{};
};

class ConfigKeyDeleteResult
{
public:
    void set_key(std::string value)
    {
        key = std::move(value);
    }

    [[nodiscard]] const std::string& get_key() const noexcept
    {
        return key;
    }

    void set_value(std::string value)
    {
        value = std::move(value);
    }

    [[nodiscard]] const std::string& get_value() const noexcept
    {
        return value;
    }

private:
    std::string key{};
    std::string value{};
};

} // namespace app::handlers::configs

template<>
struct serde::Serializable<::app::handlers::configs::ConfigKeyCreateResult>
{
    static constexpr auto fields()
    {
        using T = ::app::handlers::configs::ConfigKeyCreateResult;
        return std::tuple{
            serde::FieldDesc<"key", &T::get_key, &T::set_key, serde::FieldOptions{}>{},
            serde::FieldDesc<"value", &T::get_value, &T::set_value, serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "config_key_create_result";
    }
};

template<>
struct serde::Serializable<::app::handlers::configs::ConfigKeyRetrieveResult>
{
    static constexpr auto fields()
    {
        using T = ::app::handlers::configs::ConfigKeyRetrieveResult;
        return std::tuple{
            serde::FieldDesc<"key", &T::get_key, &T::set_key, serde::FieldOptions{}>{},
            serde::FieldDesc<"value", &T::get_value, &T::set_value, serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "config_key_retrieve_result";
    }
};

template<>
struct serde::Serializable<::app::handlers::configs::ConfigKeyUpdateResult>
{
    static constexpr auto fields()
    {
        using T = ::app::handlers::configs::ConfigKeyUpdateResult;
        return std::tuple{
            serde::FieldDesc<"key", &T::get_key, &T::set_key, serde::FieldOptions{}>{},
            serde::FieldDesc<"value", &T::get_value, &T::set_value, serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "config_key_update_result";
    }
};

template<>
struct serde::Serializable<::app::handlers::configs::ConfigKeyDeleteResult>
{
    static constexpr auto fields()
    {
        using T = ::app::handlers::configs::ConfigKeyDeleteResult;
        return std::tuple{
            serde::FieldDesc<"key", &T::get_key, &T::set_key, serde::FieldOptions{}>{},
            serde::FieldDesc<"value", &T::get_value, &T::set_value, serde::FieldOptions{}>{}
        };
    }

    static constexpr std::string_view table_name()
    {
        return "config_key_delete_result";
    }
};
