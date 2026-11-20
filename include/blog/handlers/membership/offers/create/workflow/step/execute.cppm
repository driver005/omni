export module blog_handlers:offers_create_step;
import :domain_all;
import :offers_browse_step;
import std;
import core_workflow;
import core_serde;
import interfaces;

export namespace blog::handlers::offers::create::workflow {
class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&; using Output=std::map<std::string,std::string>;
 static constexpr std::string_view name{"offers_create_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx;

  // Ghost creates from frame.data.offers[0] (offers.js:80).
  const auto inner=support::unwrap(in.body_as_string());
  const auto result=ctx.store().create("offers", inner);
  if (!result.ok())
   return core::workflow::StepResponse<Output>{{{"status",std::to_string(result.status())},
     {"body",result.body()}}};

  const std::string id=support::extract_id(result.body());
  // controller returns {data:[offer]}; default serializer -> {"offers":[DTO]}.
  const auto offer=support::offer_dto(inner, id);
  // OfferCreatedEvent -> /<code> -> /#/portal/offers/<id> redirect (offers-module.js:36-42).
  const auto fields=blog::api::JsonBody{}.parse(inner);
  const std::string code=fields.contains("code")?fields.at("code"):std::string{};
  support::emit("offer.created",
   "{\"id\":"+support::quote(id)+",\"code\":"+support::quote(code)+"}", id);

  // offers.js declares no statusCode -> HTTP 200; method add -> Location header.
  return core::workflow::StepResponse<Output>{{{"status","200"},
    {"body",support::wrap(offer)},
    {"location",support::location_for(in.get_path(), id)}}};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
