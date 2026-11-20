export module blog_handlers:members_members_stripe_connect_get_step;
import :domain_all;
import std;
import core_workflow;
import interfaces;
import :members_operations;

// Strict-parity GET /members/stripe_connect (auth). Ghost options mode in
// {live,test} default live (422 Validation (AllowedValues) on invalid);
// checkCanConnect() -> 400 when production site is not https; state =
// base64(JSON.stringify({mode,randomState:randomBytes(16).toString('hex')}))
// written to req.session; 302 + Location to the Stripe OAuth URL.
export namespace blog::handlers::members::members_stripe_connect_get::workflow {
namespace support=blog::api::members::members_support;
class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&;
 using Output=std::map<std::string,std::string>;
 static constexpr std::string_view name{"members_members_stripe_connect_get_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx;
  std::string mode="live";
  const auto p=support::params(in.get_path());
  if (const auto it=p.find("mode"); it!=p.end()) mode=it->second;
  if (mode!="live" && mode!="test")
   return core::workflow::StepResponse<Output>{{{"status","422"},
     {"body",support::invalid("Validation (AllowedValues) failed for mode", mode, "mode")}}};

  // checkCanConnect(): production + non-https site URL -> 400 BadRequestError.
  if (production_without_https(ctx))
   return core::workflow::StepResponse<Output>{{{"status","400"},
     {"body",support::bad_request("Cannot connect to stripe unless site is using https://")}}};

  const auto nonce=support::random_hex(16);
  const auto state=support::base64(
      std::format(R"({{"mode":"{}","randomState":"{}"}})", mode, nonce));
  // Persist the OAuth state as context-local state (session stand-in) and an
  // observable outbox event so the callback-side comparison has a source.
  ctx.store().resources().create("stripe_connect_states",
      std::format(R"({{"state":"{}","mode":"{}"}})", state, mode));
  support::emit_event("StripeConnectStateCreated",
      std::format(R"({{"state":"{}","mode":"{}"}})", state, mode), state);

  const auto client_id=mode=="live"
      ?"ca_8LBuZWhYshxF0A55KgCXu8PRTquCKC5x"
      :"ca_8LBum4Ctv3mmJ1oD0ZRmxjdAhNrrBUy3";
  const auto url=std::format(
      "https://connect.stripe.com/oauth/authorize?response_type=code&scope=read_write"
      "&client_id={}&redirect_uri=https://stripe.ghost.org&state={}", client_id, state);
  return core::workflow::StepResponse<Output>{{{"status","302"},
      {"location",url},{"body",""}}};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
private:
 static bool production_without_https(blog::BlogContext& ctx) {
  const auto settings=support::decode(ctx.store().json_for("settings", {}));
  if (!settings) return false;
  const auto* root=std::get_if<support::Object>(&settings->get());
  if (root==nullptr) return false;
  const auto env=support::scalar(root, "env");
  if (env!="production") return false;
  const auto* site=support::value_for(root, "site");
  const auto* site_object=site==nullptr?nullptr:std::get_if<support::Object>(&site->get());
  const auto url=site_object==nullptr?support::scalar(root, "url")
                                     :support::scalar(site_object, "url");
  return !url.starts_with("https");
 }
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
