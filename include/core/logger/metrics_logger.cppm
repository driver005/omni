export module core_logger:metrics_logger;

import std;
import :facade;

export namespace core::logger {

/** @brief Sink shape required by MetricsLogger — named counters plus flow recording. */
template<class M>
concept MetricSink = requires(M& m, std::string_view name) {
    { m.counter(name).increment() };
    { m.record_flow(name) };
};

/**
 * @brief Logging facade that mirrors every emitted event into a metrics sink.
 *
 * Each info/warning/error call writes through the named core_logger facade and records the
 * same event as a counter increment and a flow. counter()/record_flow() are forwarded
 * unchanged so callers can keep using the sink directly.
 */
template<MetricSink Sink>
class MetricsLogger
{
public:
    explicit MetricsLogger(Sink& sink) noexcept :
        m_sink{sink}
    {
    }

    [[nodiscard]] decltype(auto) counter(std::string_view name)
    {
        return m_sink.counter(name);
    }

    void record_flow(std::string_view name)
    {
        m_sink.record_flow(name);
    }

    template<typename... Args>
        requires(std::formattable<Args, char> && ...)
    void info(std::string_view event, std::format_string<Args...> fmt, Args&&... args)
    {
        core::logger::info(event, fmt, std::forward<Args>(args)...);
        counter(event).increment();
        record_flow(event);
    }

    template<typename... Args>
        requires(std::formattable<Args, char> && ...)
    void warning(std::string_view event, std::format_string<Args...> fmt, Args&&... args)
    {
        core::logger::warning(event, fmt, std::forward<Args>(args)...);
        counter(event).increment();
        record_flow(event);
    }

    template<typename... Args>
        requires(std::formattable<Args, char> && ...)
    void error(std::string_view event, std::format_string<Args...> fmt, Args&&... args)
    {
        core::logger::error(event, fmt, std::forward<Args>(args)...);
        counter(event).increment();
        record_flow(event);
    }

private:
    Sink& m_sink;
};

} // namespace core::logger
