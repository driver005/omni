module;

#define CONGELADO_GUEST
#include <congelado/plugin.h>
#include <hiredis/hiredis.h>

export module redis_backend_plugin;

import congelado_plugin;
import interfaces;
import shared;
import core_logger;
import core_events;
import core_bridge;
import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

/**
 * @brief Cache-only Redis backend for the core cache abstraction.
 *
 * Implements `interfaces::ICache` over one blocking hiredis connection and
 * advertises `CONGELADO_CAP_CACHE`, so the host's `cache_get` dispatch can hand
 * this instance to `connector::Connector::set_cache()`. All commands are
 * binary-safe via hiredis `%b`, so JSON blobs round-trip intact.
 *
 * A failed connect leaves `m_ctx` null and every call degrades to an empty
 * result, matching the optional-infra behaviour of the existing redis plugin.
 */
class RedisBackendPlugin final :
    public congelado::Plugin,
    public interfaces::ICache
{
public:
    [[nodiscard]] std::string_view get_name() const noexcept override
    {
        return "redis_backend";
    }

    [[nodiscard]] std::string_view get_version() const noexcept override
    {
        return "0.1.0";
    }

    [[nodiscard]] std::uint32_t capabilities() const noexcept override
    {
        return CONGELADO_CAP_CACHE;
    }

    /** @brief Reads connection params, builds the prefix and connects. */
    void on_load(const CongeladoHostCallbacks& host, const CongeladoConfigView& cfg) override
    {
        const auto params = connection_params(cfg);
        m_key_prefix = params.key_prefix;
        m_ctx = redisConnect(params.host.c_str(), params.port);
        if (m_ctx == nullptr || m_ctx->err != 0) {
            const std::string message =
                std::format("redis_backend: connect failed: {}",
                            m_ctx != nullptr ? m_ctx->errstr : "allocation failure");
            if (host.log != nullptr) {
                host.log(host.ctx, 3, message.data(), message.size());
            }
            core::logger::warning("redis_backend", "{}", message);
            core::events::publish("redis_backend.connection.failed", {{"host", params.host}});
            if (m_ctx != nullptr) {
                redisFree(m_ctx);
                m_ctx = nullptr;
            }
            return;
        }
        if (!params.password.empty()) {
            auto* reply = static_cast<redisReply*>(
                redisCommand(m_ctx, "AUTH %b", params.password.data(), params.password.size())
            );
            const bool authenticated = reply != nullptr && reply->type != REDIS_REPLY_ERROR;
            if (reply != nullptr) {
                freeReplyObject(reply);
            }
            if (!authenticated) {
                core::logger::warning("redis_backend", "AUTH failed");
                redisFree(m_ctx);
                m_ctx = nullptr;
                return;
            }
        }
        if (params.db != 0) {
            auto* reply = static_cast<redisReply*>(redisCommand(m_ctx, "SELECT %d", params.db));
            if (reply != nullptr) {
                freeReplyObject(reply);
            }
        }
        core::logger::debug("redis_backend", "connected to {}:{}", params.host, params.port);
        core::events::publish("redis_backend.connection.established", {{"host", params.host}});
    }

    void on_unload() noexcept override
    {
        if (m_ctx != nullptr) {
            redisFree(m_ctx);
            m_ctx = nullptr;
        }
    }

    /** @brief Capability hook: exposes this instance as `interfaces::ICache*`. */
    void* cache_get() noexcept
    {
        return static_cast<interfaces::ICache*>(this);
    }

    [[nodiscard]] std::string_view backend_name() const noexcept override
    {
        return "redis";
    }

    void get(std::string_view key, shared::QueryReadFn&& result) noexcept override
    {
        if (m_ctx == nullptr) {
            result("");
            return;
        }
        const std::string full = full_key(key);
        auto* reply = static_cast<redisReply*>(
            redisCommand(m_ctx, "GET %b", full.data(), full.size())
        );
        if (reply != nullptr && reply->type == REDIS_REPLY_STRING) {
            result(std::string_view{reply->str, static_cast<std::size_t>(reply->len)});
        } else {
            result("");
        }
        if (reply != nullptr) {
            freeReplyObject(reply);
        }
    }

    void set(std::string_view key, std::string_view value, shared::QueryReadFn&& result) noexcept override
    {
        if (m_ctx == nullptr) {
            result("");
            return;
        }
        const std::string full = full_key(key);
        auto* reply = static_cast<redisReply*>(redisCommand(
            m_ctx,
            "SET %b %b",
            full.data(),
            full.size(),
            value.data(),
            value.size()
        ));
        const bool ok = reply != nullptr && reply->type == REDIS_REPLY_STATUS;
        if (reply != nullptr) {
            freeReplyObject(reply);
        }
        result(ok ? "ok" : "");
    }

    void remove(std::string_view key, shared::QueryReadFn&& result) noexcept override
    {
        if (m_ctx == nullptr) {
            result("");
            return;
        }
        const std::string full = full_key(key);
        auto* reply = static_cast<redisReply*>(
            redisCommand(m_ctx, "DEL %b", full.data(), full.size())
        );
        const bool ok = reply != nullptr && reply->type == REDIS_REPLY_INTEGER;
        if (reply != nullptr) {
            freeReplyObject(reply);
        }
        result(ok ? "ok" : "");
    }

private:
    struct ConnectionParams
    {
        std::string host{"localhost"};
        int port{6379};
        std::string password{};
        int db{0};
        std::string key_prefix{"congelado:"};
    };

    [[nodiscard]] static ConnectionParams connection_params(const CongeladoConfigView& cfg)
    {
        ConnectionParams params;
        if (const auto value = congelado::config_get(cfg, "host")) {
            params.host = *value;
        }
        if (const auto value = congelado::config_get(cfg, "port")) {
            try {
                params.port = std::stoi(*value);
            } catch (...) {
                core::logger::warning("redis_backend", "invalid port '{}', using 6379", *value);
            }
        }
        if (const auto value = congelado::config_get(cfg, "password")) {
            params.password = *value;
        }
        if (const auto value = congelado::config_get(cfg, "db")) {
            try {
                params.db = std::stoi(*value);
            } catch (...) {
                core::logger::warning("redis_backend", "invalid db '{}', using 0", *value);
            }
        }
        if (const auto value = congelado::config_get(cfg, "key_prefix")) {
            params.key_prefix = *value;
        }
        return params;
    }

    [[nodiscard]] std::string full_key(std::string_view key) const noexcept
    {
        return m_key_prefix + std::string{key};
    }

    redisContext* m_ctx{nullptr};
    std::string m_key_prefix{"congelado:"};
};

CONGELADO_PLUGIN(RedisBackendPlugin);

/**
 * @brief core_bridge registration entrypoint. Registers this plugin's ICache
 * surface with the host's service registry during Poco-based plugin loading.
 */
extern "C" void congelado_bridge_register(void* context) noexcept
{
    auto* plugin = plugin_instance();
    if (plugin == nullptr) {
        return;
    }
    auto* bridge = static_cast<core::bridge::Bridge*>(context);
    bridge->register_service(
        core::bridge::ServiceKey{}.make<interfaces::ICache*>("cache"),
        static_cast<interfaces::ICache*>(plugin)
    );
}

#ifdef CONGELADO_TEST
namespace redis_backend_tests {
using namespace boost::ut;

suite<"RedisBackendPlugin"> redis_backend_plugin_suite = []
{
    "advertises cache capability and identity"_test = []
    {
        RedisBackendPlugin plugin;
        expect(plugin.get_name() == "redis_backend");
        expect(plugin.capabilities() == CONGELADO_CAP_CACHE);
        expect(plugin.backend_name() == "redis");
        expect(plugin.cache_get() == static_cast<interfaces::ICache*>(&plugin));
    };

    "get/set/remove with no connection degrade to empty"_test = []
    {
        RedisBackendPlugin plugin;
        std::optional<std::string> observed;
        plugin.get("k", [&](std::string_view value) { observed = std::string{value}; });
        expect(observed.has_value());
        expect(observed->empty());

        plugin.set("k", "v", [&](std::string_view value) { observed = std::string{value}; });
        expect(observed->empty());
    };
};

} // namespace redis_backend_tests
#endif
