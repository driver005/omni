export module core_cron:cron_field;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::cron {

/** @brief One parsed cron field: the explicit set of values it matches. */
class CronField
{
public:
    using Bounds = std::pair<int, int>;

    [[nodiscard]] static std::optional<CronField> parse(std::string_view field, int min, int max)
    {
        CronField result;
        result.m_min = min;
        result.m_max = max;
        std::size_t start = 0;
        while (start <= field.size()) {
            const auto comma = field.find(',', start);
            const auto token = comma == std::string_view::npos
                                   ? field.substr(start)
                                   : field.substr(start, comma - start);
            if (!result.parse_token(token)) {
                return std::nullopt;
            }
            if (comma == std::string_view::npos) {
                break;
            }
            start = comma + 1;
        }
        if (result.m_values.empty()) {
            return std::nullopt;
        }
        std::ranges::sort(result.m_values);
        const auto unique_range = std::ranges::unique(result.m_values);
        result.m_values.erase(unique_range.begin(), unique_range.end());
        return result;
    }

    [[nodiscard]] bool matches(int value) const noexcept
    {
        return std::ranges::find(m_values, value) != m_values.end();
    }

    [[nodiscard]] const std::vector<int>& values() const noexcept
    {
        return m_values;
    }

private:
    [[nodiscard]] bool parse_token(std::string_view token)
    {
        if (token.empty()) {
            return false;
        }
        if (token == "*") {
            for (int value = m_min; value <= m_max; ++value) {
                m_values.push_back(value);
            }
            return true;
        }
        if (token.starts_with("*/")) {
            const auto step = parse_number(token.substr(2));
            if (!step || *step <= 0) {
                return false;
            }
            for (int value = m_min; value <= m_max; value += *step) {
                m_values.push_back(value);
            }
            return true;
        }
        if (const auto dash = token.find('-'); dash != std::string_view::npos) {
            const auto from = parse_number(token.substr(0, dash));
            const auto to = parse_number(token.substr(dash + 1));
            if (!from || !to || *from > *to || *from < m_min || *to > m_max) {
                return false;
            }
            for (int value = *from; value <= *to; ++value) {
                m_values.push_back(value);
            }
            return true;
        }
        const auto value = parse_number(token);
        if (!value || *value < m_min || *value > m_max) {
            return false;
        }
        m_values.push_back(*value);
        return true;
    }

    [[nodiscard]] static std::optional<int> parse_number(std::string_view text)
    {
        if (text.empty()) {
            return std::nullopt;
        }
        int result = 0;
        for (const char c: text) {
            if (c < '0' || c > '9') {
                return std::nullopt;
            }
            result = result * 10 + (c - '0');
        }
        return result;
    }

    int m_min{0};
    int m_max{0};
    std::vector<int> m_values{};
};

} // namespace core::cron

#ifdef CONGELADO_TEST
namespace core::cron::tests {
using namespace boost::ut;

suite<"core_cron_field"> core_cron_field_suite = []
{
    "parses stars, steps, ranges and lists"_test = []
    {
        const auto star = CronField::parse("*", 0, 59);
        expect(star.has_value());
        expect(star->values().size() == 60);
        expect(star->matches(37));

        const auto step = CronField::parse("*/15", 0, 59);
        expect(step.has_value());
        expect(step->matches(0));
        expect(step->matches(45));
        expect(!step->matches(7));

        const auto list = CronField::parse("1,3-5", 0, 59);
        expect(list.has_value());
        expect(list->matches(4));
        expect(!list->matches(2));
        expect(!CronField::parse("99", 0, 59).has_value());
    };
};

} // namespace core::cron::tests
#endif
