export module core_adapters:outbox_hook;

import std;
import :json;

export namespace core::adapters {

/**
 * @brief One recorded outbound side effect.
 *
 * Wiring need: include/core/outbox (target //include/core/outbox:core_outbox, module
 * core_outbox) is not built yet. When it lands, record_outbound() should emit
 * core::outbox::Event{ .type = "adapter.outbound", .payload = call.to_json(),
 * .aggregate_id = call.operation }. Until then this file is the local recording hook
 * used by tests and by the main agent for verification.
 */
struct OutboundCall
{
    std::string adapter{};
    std::string operation{};
    std::string method{};
    std::string url{};
    std::string request_body{};
    std::string response_body{};
    std::string error{};
    int status{0};
    bool ok{false};

    [[nodiscard]] std::string to_json() const
    {
        return std::format(
            R"({{"adapter":{},"operation":{},"method":{},"url":{},"status":{},"ok":{},"error":{},"request_body":{},"response_body":{}}})",
            quote(adapter),
            quote(operation),
            quote(method),
            quote(url),
            status,
            ok ? "true" : "false",
            quote(error),
            quote(request_body),
            quote(response_body)
        );
    }
};

using OutboundHook = std::function<void(const OutboundCall&)>;

namespace detail {

    struct OutboxState
    {
        std::mutex mutex;
        OutboundHook hook;
        std::vector<OutboundCall> calls;
    };

    [[nodiscard]] inline OutboxState& outbox_state()
    {
        static OutboxState state;
        return state;
    }

} // namespace detail

/** @brief Installs the external recorder; an empty hook clears it. */
inline void set_outbound_hook(OutboundHook hook)
{
    auto& state = detail::outbox_state();
    std::lock_guard lock{state.mutex};
    state.hook = std::move(hook);
}

/** @brief Records a call to the external hook and to the in-process verification log. */
inline void record_outbound(const OutboundCall& call)
{
    auto& state = detail::outbox_state();
    OutboundHook hook_copy;
    {
        std::lock_guard lock{state.mutex};
        state.calls.push_back(call);
        hook_copy = state.hook;
    }
    if (hook_copy) {
        // Never let a host hook break an adapter call.
        try {
            hook_copy(call);
        } catch (...) { // NOLINT(bugprone-empty-catch) — best-effort side effect only
        }
    }
}

/** @brief Copies the in-process outbound log (safe while adapters are still running). */
[[nodiscard]] inline std::vector<OutboundCall> recorded_calls()
{
    auto& state = detail::outbox_state();
    std::lock_guard lock{state.mutex};
    return state.calls;
}

/** @brief Clears the in-process outbound log; leaves any external hook installed. */
inline void clear_recorded_calls()
{
    auto& state = detail::outbox_state();
    std::lock_guard lock{state.mutex};
    state.calls.clear();
}

} // namespace core::adapters
