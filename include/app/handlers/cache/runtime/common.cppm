export module app_handlers_cache:runtime_common;

import std;
import app_context;
import core_cache;

export namespace app::handlers::cache {

/**
 * @brief Process-wide, in-process substitute for Redis pub/sub — the exact
 * infrastructure upstream uses to broadcast cache invalidations
 * (storage_impl/src/redis/pub_sub.rs:9-29,32-89). `publish()` fans the message
 * out to every subscriber of the channel and returns the number of subscribers
 * that received it — precisely the `usize` Redis `PUBLISH` returns and the
 * `result` summed by `redact_from_redis_and_publish`
 * (storage_impl/src/redis/cache.rs:988-1012).
 *
 * Upstream installs one process-wide subscriber whose `on_message` invalidates
 * the shared `Caches` for every tenant (app.rs:566; pub_sub.rs:34-69,92-133).
 * Here each AppContext registers one subscription, so a publish invalidates
 * every registered instance's cache — a real, in-process cross-instance fan-out
 * rather than a single local `cache().erase()`.
 *
 * The instance is intentionally never destroyed (leaky singleton): it is a
 * process-wide registry whose subscriptions live for the whole process, so it
 * must not be torn down by static destruction before a late publish.
 */
class InProcessPubSubBroker
{
public:
    using Handler = std::function<void(std::string_view)>;

    // Preserved singleton accessor: the leaky allocation is required so the
    // process-wide registry outlives static destruction and a late publish.
    [[nodiscard]] static InProcessPubSubBroker& instance()
    {
        static InProcessPubSubBroker* broker = new InProcessPubSubBroker{};
        return *broker;
    }

    /** @brief Mirrors `subscriber.subscribe(channel)` (pub_sub.rs:34-69). */
    [[nodiscard]] std::size_t subscribe(std::string channel, Handler handler)
    {
        std::lock_guard lock{m_mutex};
        const std::size_t id = m_next_id++;
        m_channels[std::move(channel)].emplace_back(id, std::move(handler));
        return id;
    }

    void unsubscribe(std::size_t id)
    {
        std::lock_guard lock{m_mutex};
        for (auto& [channel, handlers]: m_channels) {
            (void)channel;
            std::erase_if(
                handlers,
                [id](const auto& entry)
                {
                    return entry.first == id;
                }
            );
        }
    }

    /**
     * @brief Mirrors `publisher.publish(channel, message)` (pub_sub.rs:72-89):
     * delivers the payload to all subscribers and returns the receiver count.
     */
    [[nodiscard]] std::size_t publish(std::string_view channel, std::string_view message) const
    {
        std::vector<Handler> targets;
        {
            std::lock_guard lock{m_mutex};
            const auto it = m_channels.find(std::string{channel});
            if (it == m_channels.end()) {
                return 0;
            }
            targets.reserve(it->second.size());
            for (const auto& [id, handler]: it->second) {
                (void)id;
                targets.push_back(handler);
            }
        }
        for (const auto& handler: targets) {
            handler(message);
        }
        return targets.size();
    }

private:
    mutable std::mutex m_mutex{};
    std::unordered_map<std::string, std::vector<std::pair<std::size_t, Handler>>> m_channels{};
    std::size_t m_next_id{0};
};

/**
 * @brief Wire codec for the CacheRedact pub/sub payload.
 *
 * Stateless, so a copy can be captured into the long-lived subscription
 * callback without capturing the facade's `this` pointer.
 */
class CacheKeyCodec
{
public:
    /** @brief DEFAULT_IMC_INVALIDATION_CHANNEL (storage_impl/redis/cache.rs:23,190-193). */
    static constexpr std::string_view kInvalidationChannel{"hyperswitch_invalidate"};

    [[nodiscard]] std::string json_escape(std::string_view value) const
    {
        std::string out{};
        out.reserve(value.size());
        for (const char ch: value) {
            switch (ch) {
                case '\\':
                    out += "\\\\";
                    break;
                case '"':
                    out += "\\\"";
                    break;
                case '\n':
                    out += "\\n";
                    break;
                case '\r':
                    out += "\\r";
                    break;
                case '\t':
                    out += "\\t";
                    break;
                default:
                    out.push_back(ch);
                    break;
            }
        }
        return out;
    }

    /**
     * @brief serde wire of `CacheRedact { tenant, kind: CacheKind::All(key) }`
     * (storage_impl/src/redis/cache.rs:372-392,413-421): the externally-tagged
     * enum kind serializes as {"All":"<key>"}. "public" is DEFAULT_TENANT
     * (common_utils/src/consts.rs:103), the local tenant-prefix substitute.
     */
    [[nodiscard]] std::string encode_cache_redact(std::string_view key) const
    {
        return std::format(
            "{{\"tenant\":\"public\",\"kind\":{{\"All\":\"{}\"}}}}",
            json_escape(key)
        );
    }

    /** @brief Inverse of encode_cache_redact, used by the on_message subscriber. */
    [[nodiscard]] std::string key_from_cache_redact(std::string_view message) const
    {
        constexpr std::string_view marker{R"("All":")"};
        const auto pos = message.find(marker);
        if (pos == std::string_view::npos) {
            return {};
        }
        const std::size_t begin = pos + marker.size();
        std::size_t end = begin;
        while (end < message.size()) {
            if (message[end] == '\\' && end + 1 < message.size()) {
                end += 2; // skip the whole escape sequence, incl. an escaped quote
                continue;
            }
            if (message[end] == '"') {
                break;
            }
            ++end;
        }
        std::string out{};
        out.reserve(end - begin);
        for (std::size_t i = begin; i < end; ++i) {
            if (message[i] == '\\' && i + 1 < end) {
                switch (message[i + 1]) {
                    case '"':
                        out.push_back('"');
                        break;
                    case '\\':
                        out.push_back('\\');
                        break;
                    case 'n':
                        out.push_back('\n');
                        break;
                    case 'r':
                        out.push_back('\r');
                        break;
                    case 't':
                        out.push_back('\t');
                        break;
                    default:
                        out.push_back(message[i + 1]);
                        break;
                }
                ++i;
            } else {
                out.push_back(message[i]);
            }
        }
        return out;
    }
};

/** @brief Shared business helpers for the cache sub-runtime. */
class CacheRuntimeSupport
{
public:
    explicit CacheRuntimeSupport(app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    /**
     * @brief Port of storage_impl::redis::cache::redact_from_redis_and_publish
     * (core/cache.rs:13; storage_impl/src/redis/cache.rs:967-1012):
     *   1. DEL the unprefixed key from the cache (miss is logged, not an error).
     *   2. PUBLISH CacheRedact { tenant, kind: All(key) } to the invalidation
     *      channel; every subscriber runs on_message and invalidates its caches.
     *   3. Return the published-channel count (sum of PUBLISH replies).
     */
    [[nodiscard]] std::size_t redact_from_redis_and_publish(std::string_view key) const
    {
        m_context.cache().erase(key);
        return InProcessPubSubBroker::instance().publish(
            std::string{CacheKeyCodec::kInvalidationChannel},
            m_codec.encode_cache_redact(key)
        );
    }

private:
    app::context::AppContext& m_context;
    CacheKeyCodec m_codec{};
};

} // namespace app::handlers::cache
