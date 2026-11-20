export module app_metrics:counters;

import std;

export namespace app::metrics {

/** @brief Monotonic in-memory counter for one named metric. */
class Counter
{
public:
    void increment(std::uint64_t by = 1) noexcept
    {
        m_value += by;
    }

    [[nodiscard]] std::uint64_t value() const noexcept
    {
        return m_value;
    }

private:
    std::uint64_t m_value{0};
};

/** @brief Owns named counters and observed flow names. Instance methods only. */
class MetricsRegistry
{
public:
    [[nodiscard]] Counter& counter(std::string_view name)
    {
        return m_counters[std::string{name}];
    }

    void record_flow(std::string_view flow)
    {
        m_flows.emplace_back(flow);
    }

    [[nodiscard]] std::size_t flow_count() const noexcept
    {
        return m_flows.size();
    }

private:
    std::map<std::string, Counter> m_counters{};
    std::vector<std::string> m_flows{};
};

} // namespace app::metrics
