export module blog_handlers:settings_settings_access_code_regenerate_post_step;
import :domain_all;
import std;
import core_workflow;
import core_serde;
import interfaces;
import :settings_browse_step;

export namespace blog::handlers::settings::settings_access_code_regenerate_post::workflow {
class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&; using Output=support::Result;
 static constexpr std::string_view name{"settings_settings_access_code_regenerate_post_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input&, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx;
  using namespace support;
  // Ghost settingsService.regeneratePrivateSiteAccessCode() writes key
  // `password` with a `word###` value under an internal context.
  const auto code=word_code();
  write_setting(ctx,"password",quote(code));
  emit_transaction({
   {.type="settings.access_code_regenerated", .payload=R"({"key":"password"})", .aggregate_id="settings"},
   {.type="settings.edited", .payload=R"({"key":"password","internal":true})", .aggregate_id="settings"},
   {.type="cache.invalidate", .payload=R"({"url":"/*"})", .aggregate_id="settings"}});

  auto result=support::browse(ctx,{});
  result["cache_invalidate"]="/*";
  return core::workflow::StepResponse<Output>{result};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
