export module core_cron:expression;

import std;
import :field;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::cron {

/** @brief A five-field cron expression with next-fire computation. */
class CronExpression
{
public:
    using TimePoint = std::chrono::system_clock::time_point;

    [[nodiscard]] std::optional<CronExpression> parse(std::string_view expression)
    {
        std::vector<std::string_view> fields;
        std::size_t start = 0;
        while (start < expression.size()) {
            while (start < expression.size() && expression[start] == ' ') {
                ++start;
            }
            if (start >= expression.size()) {
                break;
            }
            auto end = expression.find(' ', start);
            if (end == std::string_view::npos) {
                end = expression.size();
            }
            fields.push_back(expression.substr(start, end - start));
            start = end;
        }
        if (fields.size() != 5) {
            return std::nullopt;
        }

        CronExpression result;
        result.m_minutes = CronField{}.parse(fields[0], 0, 59);
        result.m_hours = CronField{}.parse(fields[1], 0, 23);
        result.m_days = CronField{}.parse(fields[2], 1, 31);
        result.m_months = CronField{}.parse(fields[3], 1, 12);
        result.m_weekdays = CronField{}.parse(fields[4], 0, 6);
        if (!result.m_minutes || !result.m_hours || !result.m_days || !result.m_months ||
            !result.m_weekdays) {
            return std::nullopt;
        }
        return result;
    }

    [[nodiscard]] bool matches(TimePoint candidate) const noexcept
    {
        using namespace std::chrono;
        if (!m_minutes || !m_hours || !m_days || !m_months || !m_weekdays) {
            return false;
        }
        const auto day_point = floor<days>(candidate);
        const year_month_day ymd{day_point};
        const auto time = hh_mm_ss{candidate - day_point};
        const weekday weekday_value{day_point};

        return m_minutes->matches(static_cast<int>(time.minutes().count())) &&
               m_hours->matches(static_cast<int>(time.hours().count())) &&
               m_days->matches(static_cast<int>(static_cast<unsigned>(ymd.day()))) &&
               m_months->matches(static_cast<int>(static_cast<unsigned>(ymd.month()))) &&
               m_weekdays->matches(static_cast<int>(weekday_value.c_encoding()));
    }

    /**
     * @brief Next matching minute strictly after `base`, scanning at most four
     * years forward (covers a Feb-29-only expression).
     */
    [[nodiscard]] std::optional<TimePoint> next_after(TimePoint base) const noexcept
    {
        using namespace std::chrono;
        auto candidate = floor<minutes>(base) + minutes{1};
        const auto deadline = candidate + minutes{4 * 366 * 24 * 60};
        while (candidate < deadline) {
            if (matches(candidate)) {
                return candidate;
            }
            candidate += minutes{1};
        }
        return std::nullopt;
    }

    /** @brief Computes up to `count` successive fire times after `base`. */
    [[nodiscard]] std::vector<TimePoint> next_runs(TimePoint base, std::size_t count) const
    {
        std::vector<TimePoint> result;
        auto cursor = base;
        while (result.size() < count) {
            const auto next = next_after(cursor);
            if (!next) {
                break;
            }
            result.push_back(*next);
            cursor = *next;
        }
        return result;
    }

private:
    std::optional<CronField> m_minutes{};
    std::optional<CronField> m_hours{};
    std::optional<CronField> m_days{};
    std::optional<CronField> m_months{};
    std::optional<CronField> m_weekdays{};
};

} // namespace core::cron

#ifdef CONGELADO_TEST
namespace core::cron::tests {
using namespace boost::ut;

suite<"core_cron_expression"> core_cron_expression_suite = []
{
    "validates and rejects expressions"_test = []
    {
        expect(CronExpression{}.parse("*/5 * * * *").has_value());
        expect(CronExpression{}.parse("0 9 * * 1-5").has_value());
        expect(!CronExpression{}.parse("* * * *").has_value());
        expect(!CronExpression{}.parse("nope * * * *").has_value());
    };

    "computes next fire times"_test = []
    {
        const auto expression = CronExpression{}.parse("0 0 * * *");
        expect(expression.has_value());
        const auto base =
            std::chrono::sys_days{std::chrono::year{2'026} / std::chrono::January / 1} +
            std::chrono::hours{12};
        const auto next = expression->next_after(base);
        expect(next.has_value());
        const std::chrono::year_month_day ymd{std::chrono::floor<std::chrono::days>(*next)};
        expect(static_cast<unsigned>(ymd.day()) == 2);
        expect(expression->next_runs(base, 3).size() == 3);
    };
};

} // namespace core::cron::tests
#endif
