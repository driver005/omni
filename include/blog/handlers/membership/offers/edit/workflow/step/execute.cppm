export module blog_handlers:offers_edit_step;
import :domain_all;
import :offers_browse_step;
import std;
import core_workflow;
import core_serde;
import core_query;
import interfaces;

export namespace blog::handlers::offers::edit::workflow {
class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&; using Output=std::map<std::string,std::string>;
 static constexpr std::string_view name{"offers_edit_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx;

  // Ghost options:['id'] - the path id overrides any body id (offers.js:48,54-60).
  const std::string id=support::last_segment(in.get_path());
  if (id.empty())
   return core::workflow::StepResponse<Output>{{{"status","422"},
     {"body",blog::api::ApiError{}.validation("Validation error","id is required","id")}}};

  // getById(data.id) first: the controller throws NotFoundError when absent
  // (offers.js:62-66). The store's offers branch never returns null, so do the
  // existence check against the offer rows here.
  core::query::ParsedQuery by_id;
  by_id.filters.emplace_back("id", id);
  const auto existing=core::query::apply(support::clean(ctx.store().json_for("offers", {})), by_id, 15);
  const auto items=blog::api::parse_array(existing);
  if (items.empty())
   return core::workflow::StepResponse<Output>{{{"status","404"},
     {"body",blog::api::ApiError{}.json("Offer not found.","Not Found")}}};

  const auto inner=with_id(support::unwrap(in.body_as_string()), id);
  const auto result=ctx.store().edit("offers", id, inner);
  if (!result.ok())
   return core::workflow::StepResponse<Output>{{{"status",std::to_string(result.status())},
     {"body",result.body()}}};

  const auto offer=support::offer_dto(inner, id);
  // OfferCodeChangeEvent -> rewrite /<code> redirect (offers-module.js:25-34).
  const auto submitted=blog::api::JsonBody{}.parse(inner);
  const std::string code=submitted.contains("code")?submitted.at("code"):std::string{};
  const auto before=blog::api::JsonBody{}.parse(blog::api::record_to_json(items.front()));
  const std::string old_code=before.contains("code")?before.at("code"):std::string{};
  if (!code.empty() && code!=old_code) {
   support::emit("offer.code_changed",
    "{\"id\":"+support::quote(id)+",\"old_code\":"+support::quote(old_code)
     +",\"code\":"+support::quote(code)+"}", id);
  }

  // controller returns {data:[offer]}; default serializer -> {"offers":[DTO]}.
  return core::workflow::StepResponse<Output>{{{"status","200"},
    {"body",support::wrap(offer)}}};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
private:
 static std::string with_id(std::string offer, std::string_view id) {
  if (offer.empty()) offer="{}";
  const auto fields=blog::api::JsonBody{}.parse_raw(offer);
  std::string out="{";
  bool first=true;
  for (const auto& [key,value]:fields) {
   if (key=="id") continue;
   out += std::string{first?"":","}+blog::api::JsonBody{}.quote(key)+":"+value;
   first=false;
  }
  out += std::string{first?"":","}+"\"id\":"+blog::api::JsonBody{}.quote(id)+"}";
  return out;
 }
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
