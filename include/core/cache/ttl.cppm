export module core_cache:ttl;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::cache {

/** @brief Time-to-live value attached to a write; never-expiring by default. */
class Ttl
{
public:
    using Clock = std::chrono::system_clock;
    using Duration = Clock::duration;

    [[nodiscard]] Ttl never() noexcept
    {
        return Ttl{Duration::zero(), true};
    }

    [[nodiscard]] Ttl seconds(std::int64_t count) noexcept
    {
        return Ttl{Duration{std::chrono::seconds{count}}, false};
    }

    [[nodiscard]] Ttl minutes(std::int64_t count) noexcept
    {
        return Ttl{Duration{std::chrono::minutes{count}}, false};
    }

    [[nodiscard]] Ttl hours(std::int64_t count) noexcept
    {
        return Ttl{Duration{std::chrono::hours{count}}, false};
    }

    [[nodiscard]] bool is_never() const noexcept
    {
        return m_never;
    }

    [[nodiscard]] Duration duration() const noexcept
    {
        return m_duration;
    }

    [[nodiscard]] Clock::time_point deadline_from(Clock::time_point now) const noexcept
    {
        return m_never ? Clock::time_point::max() : now + m_duration;
    }

public:
    Ttl() = default;

private:

    constexpr Ttl(Duration duration, bool never) noexcept :
        m_duration{duration},
        m_never{never}
    {
    }

    Duration m_duration{Duration::zero()};
    bool m_never{true};
};

} // namespace core::cache

#ifdef CONGELADO_TEST
namespace core::cache::tests {
using namespace boost::ut;

suite<"core_cache_ttl"> core_cache_ttl_suite = []
{
    "never has no deadline"_test = []
    {
        Ttl ttl = Ttl{}.never();
        expect(ttl.is_never());
        expect(ttl.deadline_from(Ttl::Clock::now()) == Ttl::Clock::time_point::max());
    };

    "finite ttls convert units"_test = []
    {
        expect(Ttl{}.seconds(30).duration() == std::chrono::seconds{30});
        expect(Ttl{}.minutes(2).duration() == std::chrono::minutes{2});
        expect(Ttl{}.hours(1).duration() == std::chrono::hours{1});
        expect(!Ttl{}.seconds(1).is_never());
    };
};

} // namespace core::cache::tests
#endif
