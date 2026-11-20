export module blog_handlers:email_previews_get_detail_step;
import :domain_all;
import :email_previews_dto_subresources;
import std;
import core_workflow;
import core_query;
import core_serde;
import interfaces;

export namespace blog::handlers::email_previews::get_detail::workflow {

namespace local_mapping {
[[nodiscard]] inline std::string wrap(std::string_view resource, std::string_view rows,
                                      std::string_view meta = {})
{
    return "{\"" + std::string{resource} + "\":" + std::string{rows} +
           (meta.empty() ? std::string{} : ",\"meta\":" + std::string{meta}) + "}";
}
[[nodiscard]] inline std::string clean(std::string_view json, std::string_view) { return std::string{json}; }
[[nodiscard]] inline std::string pagination(std::size_t total, std::size_t limit, std::size_t page,
                                            std::size_t pages, std::string_view prev,
                                            std::string_view next)
{
    return std::format(R"("total":{},"limit":{},"page":{},"pages":{},"prev":{},"next":{})",
                       total, limit, page, pages,
                       prev.empty() ? std::string{"null"} : std::string{prev},
                       next.empty() ? std::string{"null"} : std::string{next});
}
} // namespace local_mapping
class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&;
 using PreviewInput=blog::dto::email_previews::preview_support::Input;
 using Output=std::map<std::string,std::string>;
 static constexpr std::string_view name{"email_previews_get_detail_execute"};

 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx;
  using namespace blog::dto::email_previews::preview_support;

  const std::string path{in.get_path()};
  const std::string id=last_segment(path);
  if (id.empty()) {
   return fail("422", blog::api::ApiError{}.validation("id is required", "", "id"));
  }

  // Ghost `options[fields,member_status,member_tier,memberSegment,newsletter]`
  // plus EmailController._getFrameData service validation.
  PreviewInput options;
  parse_options(path, options);
  if (const auto error=validate(options, path)) {
   return fail("422", blog::api::ApiError{}.validation(error->message, "", error->property));
  }
  fold_legacy_segment(options);

  // Post.findOne({...options, status:'all'}, {withRelated:[...]}) -> NotFoundError.
  const auto post=select_post(ctx.store().json_for("posts", {{"filter[id]", id}}), id);
  if (!post) {
   return fail("404", blog::api::ApiError{}.json("Post not found.", ""));
  }

  // Newsletter.findOne({slug},{require:true}) or post relation / default newsletter.
  std::map<std::string,std::string> newsletter_params;
  if (options.newsletter) {
   newsletter_params["filter[slug]"]=*options.newsletter;
  }
  const auto newsletters=ctx.store().json_for("newsletters", newsletter_params);
  const auto newsletter=select_newsletter(newsletters, options.newsletter.value_or(""));
  if (options.newsletter && !newsletter) {
   return fail("404", blog::api::ApiError{}.json("Newsletter not found.", ""));
  }

  // EmailService.previewEmail -> {subject,html,plaintext}, projected by `fields`.
  const auto preview=project(render(&*post, newsletter ? &*newsletter : nullptr), options.fields);
  return core::workflow::StepResponse<Output>{{{"status","200"},
   {"body", local_mapping::wrap("email_previews", preview)}}};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
private:
 static core::workflow::StepResponse<Output> fail(std::string_view status, std::string body) {
  return core::workflow::StepResponse<Output>{{{"status",std::string{status}},{"body",std::move(body)}}};
 }

 static std::string last_segment(std::string_view path) {
  const auto bare=path.substr(0, path.find('?'));
  auto tail=bare;
  while (tail.size()>1 && tail.back()=='/') tail.remove_suffix(1);
  const auto slash=tail.find_last_of('/');
  return std::string{slash==std::string_view::npos?tail:tail.substr(slash+1)};
 }
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
