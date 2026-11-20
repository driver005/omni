export module blog_handlers:recommendations_recommendations_check_post_step;
import :domain_all;
import :recommendations_browse_step;
import std;
import core_workflow;
import core_query;
import core_auth;
import core_serde;
import interfaces;

export namespace blog::handlers::recommendations::recommendations_check_post::workflow {
class ExecuteStep {
public:
  using Input = interfaces::io::IRequest&;
  using Output = std::map<std::string, std::string>;
  static constexpr std::string_view name{"recommendations_recommendations_check_post_execute"};
  [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in,
                                                              core::workflow::WorkflowContext&) const {
    using namespace blog::handlers::recommendations::support;
    auto& ctx = *s_ctx;

    // Ghost uses permissions:{method:'add'} for check (everyone who can add can check).
    const auto authorized = authorize(principal_for(in), "add");
    if (!authorized.allowed)
      return core::workflow::StepResponse<Output>{{{"status", std::to_string(authorized.status)},
                                                   {"body", authorized.error_json}}};

    const auto q = ghost_query(in.get_path(), kWriteOptions);
    if (!includes_ok(q))
      return core::workflow::StepResponse<Output>{{{"status", "422"},
        {"body", validation_error("Invalid include value", "Validation Error", "include")}}};

    const auto inner = unwrap_recommendation(in.body_as_string());
    if (!inner)
      return core::workflow::StepResponse<Output>{{{"status", "422"},
        {"body", validation_error("Invalid recommendation payload", "Validation Error",
                                  "recommendations")}}};
    const auto url = field_string(as_object(&*inner), "url");
    if (url.empty())
      return core::workflow::StepResponse<Output>{{{"status", "422"},
        {"body", validation_error("url is required", "Validation Error", "url")}}};

    // check is a preview/duplicate-check: it refreshes an EXISTING row or returns
    // a transient object. It never inserts a new recommendation.
    const auto existing = find_by_url(ctx, url);
    if (existing) {
        const auto id = row_id(*existing);
        const auto merged = merge_input(as_object(&*existing), *inner);
        ctx.store().resources().edit("recommendations", id, merged);
        const auto stored = ctx.store().resources().get("recommendations", id);
        const auto row = decode(stored);
        const auto body = row ? local_mapping::wrap("data",
            serialize_rows(ctx, {*row}, include_counts(q)))
                              : local_mapping::wrap("data", "[]");
        emit_event("recommendation.metadata_refreshed", id,
                   "{\"url\":" + core_serde::JsonCodec{}.encode(url) + "}");
        return core::workflow::StepResponse<Output>{{{"status", "200"}, {"body", body}}};
    }

    // No metadata-fetch primitive: emit the null-field Ghost serializer shape.
    return core::workflow::StepResponse<Output>{{{"status", "200"},
        {"body", local_mapping::wrap("data", "[" + preview_row(url) + "]")}}};
  }
  void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
  static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
