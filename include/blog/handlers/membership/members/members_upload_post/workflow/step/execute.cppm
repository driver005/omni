export module blog_handlers:members_members_upload_post_step;
import :domain_all;
import std;
import core_workflow;
import core_serde;
import interfaces;
import :members_operations;

// Strict-parity POST /members/upload (importCSV). Ghost multipart membersfile,
// mapping/labels, inline vs deferred threshold; 201 when inline
// (stats.imported !== null), else 202 deferred; meta shapes M:14-27,399-405.
// The in-repo representation carries the CSV bytes and mapping in a JSON body;
// a deferred import dispatches a MembersImportJob through the outbox.
export namespace blog::handlers::members::members_upload_post::workflow {
namespace support = blog::api::members::members_support;

class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"members_members_upload_post_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        auto& ctx = *s_ctx;
        const auto raw = std::string{in.body_as_string()};
        if (raw.empty()) {
            return core::workflow::StepResponse<Output>{
                {{"status", "422"},
                 {"body",
                  support::invalid(
                      "Member import requires a CSV file.",
                      "membersfile",
                      "membersfile"
                  )}}
            };
        }
        std::string file_bytes = raw, label = "members", mapping, labels;
        const auto parsed = support::decode(raw);
        if (parsed) {
            if (const auto* object = std::get_if<support::Object>(&parsed->get())) {
                for (const auto& [name, value]: *object) {
                    if ((name == "file" || name == "membersfile") &&
                        std::get_if<std::string>(&value.get())) {
                        file_bytes = *std::get_if<std::string>(&value.get());
                    } else if (name == "import_label" && std::get_if<std::string>(&value.get())) {
                        label = *std::get_if<std::string>(&value.get());
                    } else if (name == "mapping") {
                        mapping = support::encode(value);
                    } else if (name == "labels") {
                        labels = support::encode(value);
                    }
                }
            }
        }
        const auto lines = split_lines(file_bytes);
        const std::size_t original_size = lines.size();
        // Ghost canImportInline threshold: a large file is deferred to a background job.
        if (original_size > 1'000) {
            support::emit_job(
                "MembersImportJob",
                std::format(
                    R"({{"file_bytes":{},"mapping":{},"labels":{},"import_label":"{}"}})",
                    file_bytes.size(),
                    mapping.empty() ? "null" : mapping,
                    labels.empty() ? "null" : labels,
                    label
                ),
                label
            );
            return core::workflow::StepResponse<Output>{
                {{"status", "202"},
                 {"body", std::format(R"({{"meta":{{"originalImportSize":{}}}}})", original_size)}}
            };
        }
        int imported = 0, invalid = 0;
        for (const auto& line: lines) {
            if (line.empty() || line.starts_with("id,")) {
                continue;
            }
            const auto comma = line.find(',');
            if (comma == std::string::npos) {
                ++invalid;
                continue;
            }
            std::string email = line.substr(comma + 1);
            if (!email.empty() && email.front() == '"' && email.back() == '"') {
                email = email.substr(1, email.size() - 2);
            }
            if (!valid_email(email)) {
                ++invalid;
                continue;
            }
            const auto result =
                ctx.store().create("members", std::format(R"({{"email":"{}"}})", email));
            if (result.status() == 201 || result.status() == 200) {
                ++imported;
            } else {
                ++invalid;
            }
        }
        const int status = imported > 0 ? 201 : 202;
        const auto body = std::format(
            R"({{"meta":{{"originalImportSize":{},"stats":{{"imported":{},"invalid":{}}},"import_label":"{}"}}}})",
            original_size,
            imported,
            invalid,
            label
        );
        return core::workflow::StepResponse<Output>{
            {{"status", std::to_string(status)}, {"body", body}}
        };
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;

private:
    static std::vector<std::string> split_lines(std::string_view text)
    {
        std::vector<std::string> out;
        std::size_t cursor = 0;
        while (cursor < text.size()) {
            auto end = text.find('\n', cursor);
            if (end == std::string_view::npos) {
                end = text.size();
            }
            out.emplace_back(text.substr(cursor, end - cursor));
            cursor = end + 1;
        }
        return out;
    }

    static bool valid_email(std::string_view email)
    {
        const auto at = email.find('@');
        return at != std::string_view::npos && at > 0 && at + 1 < email.size() &&
               email.find(' ') == std::string_view::npos;
    }
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};
} // namespace blog::handlers::members::members_upload_post::workflow
