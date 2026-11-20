export module core_outbox;

import std;

/**
 * @file outbox.cppm
 * @brief Transactional outbox for events / jobs / side effects (CONTRACTS.md §4).
 *
 * Design notes
 * ------------
 * - "Context-local" means all state (the pending queue AND the transaction stack)
 *   lives on the `Outbox` instance itself. There is no external DB and no shared
 *   global outbox state for transactions: two `Outbox` objects are two independent
 *   contexts and never observe each other's emits or transactions.
 * - `emit()` inside a transaction is staged, not queued. It becomes visible to
 *   `drain()`/`pending()` only on `commit()`; `rollback()` discards it. This is the
 *   observable transaction contract.
 * - Nested transactions are supported: an inner `commit()` merges its staged events
 *   into the parent transaction's buffer (or into the pending queue when no parent
 *   exists); an inner `rollback()` discards only its own buffer.
 * - `drain()` is the flush point: it hands every committed event to each registered
 *   `Adapter::dispatch()` and, where practical, reuses `core_events` by publishing
 *   the drained event onto the ambient event bus when one is configured.
 */

export namespace core::outbox {

/** One record in the outbox: an event type, an opaque JSON-ish payload and the id of the
 * aggregate that produced it. Mirrors the Ghost "events / jobs / side effects" tuple. */
struct Event
{
    std::string type;
    std::string payload;
    std::string aggregate_id;
};

/** A sink for drained outbox events (e.g. a real external adapter — see `core_adapters`). */
struct Adapter
{
    virtual ~Adapter() = default;
    virtual void dispatch(const Event&) = 0;
};

namespace detail {

/** @brief Optional host event sink, wired by the host instead of importing core_events here. */
using EventSink = void (*)(const Event&);
[[nodiscard]] inline EventSink& event_sink() noexcept
{
    static EventSink sink{nullptr};
    return sink;
}

/** Process-wide adapter registry. Function-local static avoids static-initialisation order
 * issues; the registry is intentionally global because `register_adapter()` is a free
 * function with no `Outbox` context argument. */
[[nodiscard]] inline std::vector<std::shared_ptr<Adapter>>& adapters()
{
    static std::vector<std::shared_ptr<Adapter>> registry;
    return registry;
}

} // namespace detail

/** Context-local transactional outbox. */
class Outbox
{
public:
    /** Adds an event. Inside a transaction it is staged; otherwise it is queued now. */
    void emit(Event event)
    {
        if (m_transactions.empty()) {
            m_pending.push_back(std::move(event));
        } else {
            m_transactions.back().push_back(std::move(event));
        }
    }

    /** Removes and returns every committed event in emit order, then clears the queue.
     * Staged (uncommitted) events are never returned. Each drained event is dispatched to
     * every registered adapter and, when an event bus is active, published on it. */
    [[nodiscard]] std::vector<Event> drain()
    {
        std::vector<Event> drained;
        drained.swap(m_pending);

        for (const Event& event: drained) {
            for (const auto& adapter: detail::adapters()) {
                if (adapter != nullptr) {
                    adapter->dispatch(event);
                }
            }
            if (detail::event_sink() != nullptr) {
                detail::event_sink()(event);
            }
        }
        return drained;
    }

    /** Opens a (possibly nested) transaction. Emits after this are staged. */
    void begin_transaction()
    {
        m_transactions.emplace_back();
    }

    /** Commits the innermost transaction: its staged events merge into the parent
     * transaction (if any) or into the pending queue, preserving emit order. No-op if
     * no transaction is open. */
    void commit()
    {
        if (m_transactions.empty()) {
            return;
        }
        auto staged = std::move(m_transactions.back());
        m_transactions.pop_back();
        auto& target = m_transactions.empty() ? m_pending : m_transactions.back();
        target.insert(target.end(),
            std::make_move_iterator(staged.begin()),
            std::make_move_iterator(staged.end()));
    }

    /** Rolls back the innermost transaction, discarding its staged events. No-op if no
     * transaction is open. */
    void rollback() noexcept
    {
        if (!m_transactions.empty()) {
            m_transactions.pop_back();
        }
    }

    /** Number of committed events currently waiting to be drained. Staged events are
     * deliberately excluded — transactions are observable through this value and `drain()`. */
    [[nodiscard]] std::size_t pending() const noexcept
    {
        return m_pending.size();
    }

private:
    std::vector<Event> m_pending;
    std::vector<std::vector<Event>> m_transactions; // stack; back() is innermost
};

/** Registers a global adapter that receives every event passed to `Outbox::drain()`.
 * Null adapters are ignored. */
inline void register_adapter(std::shared_ptr<Adapter> adapter)
{
    if (adapter != nullptr) {
        detail::adapters().push_back(std::move(adapter));
    }
}

/** @brief Host wiring: install a sink that forwards outbox events to the host bus. */
inline void set_event_sink(detail::EventSink sink) noexcept
{
    detail::event_sink() = sink;
}

/** @brief Process-wide outbox instance for adapter hooks and host integration. */
[[nodiscard]] inline Outbox& global_outbox() noexcept
{
    static Outbox instance;
    return instance;
}

} // namespace core::outbox
