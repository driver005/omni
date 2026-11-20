export module blog_handlers:settings_settings_verifications_put_step;
import :domain_all;
import std;
import core_workflow;
import core_serde;
import interfaces;
import :settings_browse_step;

export namespace blog::handlers::settings::settings_verifications_put::workflow {
class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&; using Output=support::Result;
 static constexpr std::string_view name{"settings_settings_verifications_put_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx;
  using namespace support;
  const auto doc=decode(in.body_as_string());
  const auto* root=as_object(doc?&*doc:nullptr);
  const auto token=string_field(root,"token");
  if (token.empty())
   return core::workflow::StepResponse<Output>{fail(400, validation_error("A token is required","token"))};

  // Ghost magicLinkService.getDataFromToken -> {key,value}. The one-shot token
  // record is the context-local stand-in for the signed single-use token.
  auto& rs=ctx.store().resources();
  std::string key,value,record_id;
  for (const auto& rec : blog::api::parse_array(rs.list("settings_verifications"))) {
   if (record_text(&rec,"token")!=token) continue;
   key=record_text(&rec,"key");
   const auto vit=rec.find("value");
   value=vit==rec.end()?std::string{"null"}:vit->second;
   const auto id=rec.find("id");
   if (id!=rec.end()) record_id=core_serde::JsonCodec{}.decode<std::string>(id->second).value_or(id->second);
   break;
  }
  if (key.empty())
   return core::workflow::StepResponse<Output>{fail(400, bad_request("Invalid verification token"))};

  // Ghost EMAIL_KEYS guard (['members_support_address']).
  if (key!="members_support_address")
   return core::workflow::StepResponse<Output>{fail(400, ghost_error("Incorrect usage of verification endpoint","IncorrectUsageError"))};

  if (!record_id.empty()) rs.destroy("settings_verifications", record_id);
  write_setting(ctx,key,value);
  emit_transaction({
   {.type="email.verified", .payload=R"({"key":)"+quote(key)+"}", .aggregate_id="settings"},
   {.type="settings.edited", .payload=R"({"key":)"+quote(key)+R"(,"value":)"+value+"}", .aggregate_id="settings"},
   {.type="cache.invalidate", .payload=R"({"url":"/*"})", .aggregate_id="settings"}});

  // Ghost: full browse because calculated settings might change.
  auto result=support::browse(ctx,{});
  result["cache_invalidate"]="/*";
  return core::workflow::StepResponse<Output>{result};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
