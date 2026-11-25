export module blog_handlers:members_members_stats_count_get_step;
import :domain_all;
import std;
import core_workflow;
import interfaces;
import :members_operations;

// Strict-parity GET /members/stats/count (memberStats). Ghost
// events.getStatuses() -> _MemberStatusEvent.findAll({aggregateStatusCounts})
// cumulative per-day deltas (ER:1246-1277), total from last cumulative row
// (M:433-434). The `events` resource is the C++ stand-in for the event table;
// when no status events are seeded the live member snapshot is the cumulative
// baseline. Output {resource,total,data:[{date,paid,free,comped,gift}]}.
export namespace blog::handlers::members::members_stats_count_get::workflow {
namespace support = blog::api::members::members_support;

class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::string;
    static constexpr std::string_view name{"members_members_stats_count_get_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        auto& ctx = *s_ctx;
        std::map<std::string, std::array<int, 4>> daily; // date -> paid,free,comped,gift
        for (const auto& event: support::rows_of(ctx.store().json_for("events", {}))) {
            const auto* object = std::get_if<support::Object>(&event.get());
            if (object == nullptr) {
                continue;
            }
            const auto type = support::scalar(object, "type");
            if (type != "MemberStatusEvent" && type != "member_status") {
                continue;
            }
            const auto date = support::scalar(object, "date");
            if (date.empty()) {
                continue;
            }
            auto& bucket = daily[date];
            bucket[0] += to_int(support::scalar(object, "paid"));
            bucket[1] += to_int(support::scalar(object, "free"));
            bucket[2] += to_int(support::scalar(object, "comped"));
            bucket[3] += to_int(support::scalar(object, "gift"));
        }
        if (daily.empty()) {
            int paid = 0, free = 0, comped = 0, gift = 0;
            for (const auto& row: support::rows_of(ctx.store().json_for("members", {}))) {
                const auto* object = std::get_if<support::Object>(&row.get());
                if (object == nullptr) {
                    continue;
                }
                const auto status = support::scalar(object, "status");
                if (status == "paid") {
                    ++paid;
                } else if (status == "comped") {
                    ++comped;
                } else if (status == "gifted" || status == "gift") {
                    ++gift;
                } else {
                    ++free;
                }
            }
            daily[support::today()] = {paid, free, comped, gift};
        }
        std::string data;
        int paid = 0, free = 0, comped = 0, gift = 0;
        for (const auto& [date, bucket]: daily) {
            paid += bucket[0];
            free += bucket[1];
            comped += bucket[2];
            gift += bucket[3];
            if (!data.empty()) {
                data += ',';
            }
            data += std::format(
                R"({{"date":"{}","paid":{},"free":{},"comped":{},"gift":{}}})",
                date,
                paid,
                free,
                comped,
                gift
            );
        }
        const int total = paid + free + comped + gift;
        return core::workflow::StepResponse<Output>{
            std::format(R"({{"resource":"members","total":{},"data":[{}]}})", total, data)
        };
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;

private:
    static int to_int(std::string_view text)
    {
        if (text.empty()) {
            return 0;
        }
        try {
            return std::stoi(std::string{text});
        } catch (...) {
            return 0;
        }
    }
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};
} // namespace blog::handlers::members::members_stats_count_get::workflow
