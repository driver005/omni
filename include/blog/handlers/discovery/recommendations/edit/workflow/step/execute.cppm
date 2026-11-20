export module blog_handlers:recommendations_edit_step;
import :domain_all;
import :recommendations_browse_step;
import std;
import core_workflow;
import core_query;
import core_auth;
import core_serde;
import interfaces;

export namespace blog::handlers::recommendations::edit::workflow {
class ExecuteStep {
public:
  using Input = interfaces::io::IRequest&;
  using Output = std::map<std::string, std::string>;
  static constexpr std::string_view name{"recommendations_edit_execute"};
  [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in,
                                                              core::workflow::WorkflowContext&) const {
    using namespace blog::handlers::recommendations::support;
    auto& ctx = *s_ctx;

    const auto authorized = authorize(principal_for(in), "edit");
    if (!authorized.allowed)
      return core::workflow::StepResponse<Output>{{{"status", std::to_string(authorized.status)},
                                                   {"body", authorized.error_json}}};

    const std::string id = path_id(in.get_path());
    if (id.empty())
      return core::workflow::StepResponse<Output>{{{"status", "422"},
        {"body", validation_error("id is required", "Validation Error", "id")}}};

    const auto q = ghost_query(in.get_path(), kWriteOptions);
    if (!includes_ok(q))
      return core::workflow::StepResponse<Output>{{{"status", "422"},
        {"body", validation_error("Invalid include value", "Validation Error", "include")}}};

    const auto existing = find_by_id(ctx, id);
    if (!existing)
      return core::workflow::StepResponse<Output>{{{"status", "404"},
                                                   {"body", not_found_error(id)}}};

    const auto inner = unwrap_recommendation(in.body_as_string());
    if (!inner)
      return core::workflow::StepResponse<Output>{{{"status", "422"},
        {"body", validation_error("Invalid recommendation payload", "Validation Error",
                                  "recommendations")}}};

    const auto patch = decode(filtered_patch(*inner)).value_or(*inner);
    const auto merged = merge_input(as_object(&*existing), patch);
    ctx.store().resources().edit("recommendations", id, merged);

    const auto stored = ctx.store().resources().get("recommendations", id);
    const auto row = decode(stored);
    std::string data = "[]";
    if (row) {
        const auto serialized = serialize_rows(ctx, {*row}, include_counts(q));
        try {
            data = core::query::apply(serialized, q, 5);
        } catch (const std::exception&) {
            data = serialized;
        }
    }
    const auto body = local_mapping::wrap("data", data);

    std::string url;
    if (row) url = field_string(as_object(&*row), "url");
    // edit -> metadata refresh (event) + fire-and-forget webmention.
    emit_crud_side_effects("edited", id, url, true);
    return core::workflow::StepResponse<Output>{{{"status", "200"}, {"body", body}}};
  }
  void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
  static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
