module;
export module app_context:serde;

import std;
import interfaces;
import core_serde;

export namespace app::context {

/** @brief Built-in JSON serde format used by every app handler. */
class JsonSerdeFormat final : public interfaces::ISerdeFormat
{
public:
    [[nodiscard]] std::string_view content_type() const noexcept override
    {
        return "application/json";
    }

    [[nodiscard]] std::string_view format_name() const noexcept override
    {
        return "json";
    }

    [[nodiscard]] std::expected<std::string, std::string>
    encode(const serde::Value& value) const override
    {
        return core_serde::JsonCodec{}.encode(value);
    }

    [[nodiscard]] std::expected<serde::Value, std::string>
    decode(std::string_view data) const override
    {
        const auto value = core_serde::JsonCodec{}.decode<serde::Value>(data);
        if (!value) {
            return std::unexpected{std::string{"invalid JSON payload"}};
        }
        return *value;
    }
};

/** @brief Registers the JSON format exactly once before any Ser call. */
class SerdeSetup
{
public:
    void install() const
    {
        static serde::SerdeFormatRegistry registry;
        static const bool installed = [this] {
            registry.add_format(std::make_shared<JsonSerdeFormat>());
            serde::SerdeFormatRegistry::set_active(&registry);
            return true;
        }();
        (void)installed;
    }
};

} // namespace app::context
