export module core_events:stats_counter;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::events {

/** @brief Named counters and histograms for stats fan-out. */
class StatsCounter
{
public:
    void increment(std::string_view name, std::uint64_t by = 1)
    {
        m_counters[std::string{name}] += by;
    }

    void record(std::string_view name, double value)
    {
        m_histograms[std::string{name}].push_back(value);
    }

    [[nodiscard]] std::uint64_t value(std::string_view name) const
    {
        const auto it = m_counters.find(std::string{name});
        return it == m_counters.end() ? 0 : it->second;
    }

    [[nodiscard]] std::size_t samples(std::string_view name) const
    {
        const auto it = m_histograms.find(std::string{name});
        return it == m_histograms.end() ? 0 : it->second.size();
    }

    [[nodiscard]] std::size_t counter_count() const noexcept
    {
        return m_counters.size();
    }

    void clear() noexcept
    {
        m_counters.clear();
        m_histograms.clear();
    }

private:
    std::map<std::string, std::uint64_t> m_counters{};
    std::map<std::string, std::vector<double>> m_histograms{};
};

} // namespace core::events

#ifdef CONGELADO_TEST
namespace core::events::tests {
using namespace boost::ut;

suite<"core_events_stats_counter"> core_events_stats_counter_suite = []
{
    "counts and records samples"_test = []
    {
        StatsCounter stats;
        stats.increment("requests", 3);
        stats.record("latency", 1.5);
        expect(stats.value("requests") == 3);
        expect(stats.samples("latency") == 1);
        expect(stats.counter_count() == 1);
        stats.clear();
        expect(stats.value("requests") == 0);
    };
};

} // namespace core::events::tests
#endif
