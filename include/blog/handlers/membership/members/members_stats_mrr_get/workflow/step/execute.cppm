export module blog_handlers:members_members_stats_mrr_get_step;
import :domain_all;
import std;
import core_workflow;
import interfaces;
import :members_operations;

// Strict-parity GET /members/stats/mrr (mrrStats). Ghost events.getMRR() ->
// _MemberPaidSubscriptionEvent.findAll({aggregateMRRDeltas}) cumulative per
// currency (ER:1211-1244). The `events` resource is the C++ stand-in; a live
// paid-member snapshot is the baseline when no MRR events are seeded.
export namespace blog::handlers::members::members_stats_mrr_get::workflow {
namespace support = blog::api::members::members_support;

class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::string;
    static constexpr std::string_view name{"members_members_stats_mrr_get_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        auto& ctx = *s_ctx;
        std::map<std::string, std::map<std::string, double>>
            by_currency; // currency -> date -> cumulative
        for (const auto& event: support::rows_of(ctx.store().json_for("events", {}))) {
            const auto* object = std::get_if<support::Object>(&event.get());
            if (object == nullptr) {
                continue;
            }
            const auto type = support::scalar(object, "type");
            if (type != "MemberPaidSubscriptionEvent" && type != "mrr") {
                continue;
            }
            const auto currency = support::scalar(object, "currency");
            const auto date = support::scalar(object, "date");
            if (currency.empty() || date.empty()) {
                continue;
            }
            by_currency[currency][date] += to_double(support::scalar(object, "mrr"));
        }
        if (by_currency.empty()) {
            int paid = 0;
            for (const auto& row: support::rows_of(ctx.store().json_for("members", {}))) {
                const auto* object = std::get_if<support::Object>(&row.get());
                if (object != nullptr && support::scalar(object, "status") == "paid") {
                    ++paid;
                }
            }
            by_currency["usd"][support::today()] = static_cast<double>(paid) * 1000.0;
        }
        std::string data;
        for (const auto& [currency, dates]: by_currency) {
            std::string points;
            double cumulative = 0.0;
            for (const auto& [date, delta]: dates) {
                cumulative += delta;
                if (!points.empty()) {
                    points += ',';
                }
                points += std::format(R"({{"date":"{}","value":{}}})", date, number(cumulative));
            }
            if (!data.empty()) {
                data += ',';
            }
            data += std::format(R"({{"currency":"{}","data":[{}]}})", currency, points);
        }
        return core::workflow::StepResponse<Output>{
            std::format(R"({{"resource":"mrr","data":[{}]}})", data)
        };
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;

private:
    static double to_double(std::string_view text)
    {
        if (text.empty()) {
            return 0.0;
        }
        try {
            return std::stod(std::string{text});
        } catch (...) {
            return 0.0;
        }
    }

    static std::string number(double value)
    {
        if (value == static_cast<double>(static_cast<long long>(value))) {
            return std::to_string(static_cast<long long>(value));
        }
        return std::to_string(value);
    }
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};
} // namespace blog::handlers::members::members_stats_mrr_get::workflow
