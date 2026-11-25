export module blog_handlers:members_members_id_commenting_disable_post_step;
import :domain_all;
import std;
import core_workflow;
import interfaces;
import :members_operations;

// Strict-parity POST /members/:id/commenting/disable. Ghost: option id required,
// data reason/expires_at/hide_comments; ISO-8601 expires_at validation;
// saveCommenting('commenting_disabled') audit action; optional hide-comment bulk
// status; X-Cache-Invalidate: /members/; output {members:[member]}.
export namespace blog::handlers::members::members_id_commenting_disable_post::workflow {
namespace support = blog::api::members::members_support;

class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"members_members_id_commenting_disable_post_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        auto& ctx = *s_ctx;
        const std::string id = support::segment_before(in.get_path(), "/commenting");
        if (id.empty() || !ctx.store().find_by_resource("members", id)) {
            return core::workflow::StepResponse<Output>{
                {{"status", "404"}, {"body", support::not_found("Member not found.")}}
            };
        }
        const auto body = std::string{in.body_as_string()};
        if (!has_field(body, "reason")) {
            return core::workflow::StepResponse<Output>{
                {{"status", "422"},
                 {"body",
                  support::invalid(
                      "A reason is required when disabling commenting.",
                      "",
                      "reason"
                  )}}
            };
        }
        if (const auto until = field(body, "expires_at"); !until.empty() && !iso_date(until)) {
            return core::workflow::StepResponse<Output>{
                {{"status", "422"},
                 {"body",
                  support::invalid(
                      "expires_at must be a valid ISO 8601 date string",
                      until,
                      "expires_at"
                  )}}
            };
        }
        const auto r = s_ops->commenting_set(id, body, true);
        // Ghost audit action + optional hide-comments side effect.
        support::emit_event(
            "commenting_disabled",
            std::format(
                R"({{"id":"{}","reason":{},"expires_at":{}}})",
                id,
                json_value(body, "reason"),
                json_value(body, "expires_at")
            ),
            id
        );
        if (body.contains("\"hide_comments\":true")) {
            support::emit_job(
                "comments.bulk_update_status",
                std::format(
                    R"({{"filter":"member_id:'{}'+status:published","status":"hidden"}})",
                    id
                ),
                id
            );
        }
        const auto member = support::member_json(ctx, id);
        const auto out =
            member.empty()
                ? r.second
                : support::wrap("members", "[" + s_ops->apply_commenting(id, member) + "]");
        return core::workflow::StepResponse<Output>{
            {{"status", std::to_string(r.first)}, {"body", out}}
        };
    }

    static blog::BlogContext* s_ctx;
    static blog::api::members::MembersOperations* s_ops;

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

private:
    static bool has_field(std::string_view body, std::string_view key)
    {
        return body.find('"' + std::string{key} + '"') != std::string_view::npos;
    }

    static std::string field(std::string_view body, std::string_view key)
    {
        const auto at = body.find('"' + std::string{key} + '"');
        if (at == std::string_view::npos) {
            return {};
        }
        const auto first = body.find('"', at + key.size() + 2);
        if (first == std::string_view::npos) {
            return {};
        }
        const auto last = body.find('"', first + 1);
        return last == std::string_view::npos
                   ? std::string{}
                   : std::string{body.substr(first + 1, last - first - 1)};
    }

    static std::string json_value(std::string_view body, std::string_view key)
    {
        const auto value = field(body, key);
        return value.empty() ? "null" : "\"" + value + "\"";
    }

    static bool iso_date(std::string_view text)
    {
        if (text.size() < 10) {
            return false;
        }
        for (std::size_t i = 0; i < 10; ++i) {
            if (i == 4 || i == 7) {
                if (text[i] != '-') {
                    return false;
                }
            } else if (text[i] < '0' || text[i] > '9') {
                return false;
            }
        }
        return true;
    }
};

blog::api::members::MembersOperations* ExecuteStep::s_ops{nullptr};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
} // namespace blog::handlers::members::members_id_commenting_disable_post::workflow
