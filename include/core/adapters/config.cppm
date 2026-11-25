export module core_adapters:config;

import std;

export namespace core::adapters {

/** @brief Outbound credentials/endpoints for one provider. Empty fields mean "unconfigured". */
struct Config
{
    std::string base_url{};
    std::string api_key{};
    std::string workspace_id{};
    std::string from_address{};
};

namespace detail {

    [[nodiscard]] inline std::string upper(std::string_view text)
    {
        std::string result;
        result.reserve(text.size());
        for (const char character: text) {
            result.push_back(
                static_cast<char>(std::toupper(static_cast<unsigned char>(character)))
            );
        }
        return result;
    }

    /// @brief First non-empty env var from the candidate list, or an empty string.
    [[nodiscard]] inline std::string env_first(std::initializer_list<std::string_view> names)
    {
        for (const auto& name: names) {
            // std::getenv is env access, not locale/concurrency-sensitive in this boot-time path.
            const char* value =
                std::getenv(std::string{name}.c_str()); // NOLINT(concurrency-mt-unsafe)
            if (value != nullptr && *value != '\0') {
                return std::string{value};
            }
        }
        return {};
    }

} // namespace detail

/**
 * @brief Reads one provider's config from the environment.
 *
 * Reads, in order of preference (first non-empty wins):
 *   base_url      : <PREFIX>_BASE_URL, <PREFIX>_URL, <PREFIX>_ENDPOINT
 *   api_key       : <PREFIX>_API_KEY, <PREFIX>_TOKEN, <PREFIX>_ADMIN_TOKEN
 *   workspace_id  : <PREFIX>_WORKSPACE_ID, <PREFIX>_SITE_UUID
 *   from_address  : <PREFIX>_FROM_ADDRESS, <PREFIX>_FROM
 *
 * Canonical vars from the contract are therefore covered:
 *   STRIPE_BASE_URL, TINYBIRD_BASE_URL, SMTP_URL, WEBMENTION_ENDPOINT
 * plus the matching *_API_KEY / *_WORKSPACE_ID / *_FROM_ADDRESS keys.
 * No live credentials are required — point the *_URL vars at a local mock server.
 */
[[nodiscard]] inline Config from_env(std::string_view prefix)
{
    const auto key = detail::upper(prefix);
    const auto base = std::format("{}_", key);
    return Config{
        .base_url = detail::env_first({
            std::format("{}BASE_URL", base),
            std::format("{}URL", base),
            std::format("{}ENDPOINT", base),
        }),
        .api_key = detail::env_first({
            std::format("{}API_KEY", base),
            std::format("{}TOKEN", base),
            std::format("{}ADMIN_TOKEN", base),
        }),
        .workspace_id = detail::env_first({
            std::format("{}WORKSPACE_ID", base),
            std::format("{}SITE_UUID", base),
        }),
        .from_address = detail::env_first({
            std::format("{}FROM_ADDRESS", base),
            std::format("{}FROM", base),
        }),
    };
}

/// @brief Named helpers — thin wrappers over from_env(), no extra env semantics.
[[nodiscard]] inline Config stripe_from_env()
{
    return from_env("STRIPE");
}

[[nodiscard]] inline Config tinybird_from_env()
{
    return from_env("TINYBIRD");
}

[[nodiscard]] inline Config smtp_from_env()
{
    return from_env("SMTP");
}

[[nodiscard]] inline Config webmention_from_env()
{
    return from_env("WEBMENTION");
}

} // namespace core::adapters
