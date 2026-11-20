export module blog_handlers:settings_settings_stripe_connect_delete_step;
import :domain_all;
import std;
import core_workflow;
import core_query;
import core_serde;
import interfaces;
import :settings_browse_step;

export namespace blog::handlers::settings::settings_stripe_connect_delete::workflow {
class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&; using Output=support::Result;
 static constexpr std::string_view name{"settings_settings_stripe_connect_delete_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input&, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx;
  using namespace support;

  // Ghost memberBREADService.browse({limit:0,filter:'status:paid'}) ->
  // meta.pagination.total !== 0. core_query realizes the NQL filter + limit.
  constexpr std::string_view kAllowed[]{"filter","limit"};
  const auto query=core::query::parse("?filter=status:paid&limit=all", kAllowed);
  const auto paid=core::query::apply(ctx.store().json_for("members",{}), query, 0);
  if (!blog::api::parse_array(paid).empty())
   return core::workflow::StepResponse<Output>{fail(400, bad_request("Cannot disconnect Stripe whilst you have active subscriptions."))};

  // Ghost stripeService.disconnect() -> outbox event (real adapter residual).
  emit_transaction({
   {.type="stripe.disconnected", .payload="{}", .aggregate_id="settings"},
   {.type="cache.invalidate", .payload=R"({"url":"/*"})", .aggregate_id="settings"}});

  // Ghost models.Settings.edit([7 keys -> null], frame.options).
  for (std::string_view key : {"stripe_connect_publishable_key", "stripe_connect_secret_key",
           "stripe_connect_livemode", "stripe_connect_display_name", "stripe_connect_account_id",
           "members_stripe_webhook_id", "members_stripe_webhook_secret"})
   write_setting(ctx,key,"null");

  // Ghost statusCode:204, no body, cacheInvalidate:false.
  return core::workflow::StepResponse<Output>{Result{{"status","204"},{"body",""}}};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
