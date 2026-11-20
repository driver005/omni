export module blog_handlers:offers_read_by_id_step;
import :domain_all;
import :offers_browse_step;
import std;
import core_workflow;
import core_query;
import interfaces;

export namespace blog::handlers::offers::read_by_id::workflow {
class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&; using Output=std::map<std::string,std::string>;
 static constexpr std::string_view name{"offers_read_by_id_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx;

  // Ghost data:['id'] from the :id path param (offers.js:31,34).
  const std::string id=support::last_segment(in.get_path());
  if (id.empty())
   return core::workflow::StepResponse<Output>{{{"status","404"},
     {"body",blog::api::ApiError{}.json("Offer not found.","Not Found")}}};

  // getById + mapToOffer: look the id up over the offer rows; the store's
  // fixture branch is id-agnostic, so filter here instead of inside the store.
  core::query::ParsedQuery by_id;
  by_id.filters.emplace_back("id", id);
  const auto rows=core::query::apply(support::clean(ctx.store().json_for("offers", {})), by_id, 15);
  const auto items=blog::api::parse_array(rows);
  if (items.empty())
   return core::workflow::StepResponse<Output>{{{"status","404"},
     {"body",blog::api::ApiError{}.json("Offer not found.","Not Found")}}};

  // getOffer returns one offer; default serializer -> {"offers":[OfferDTO]}.
  return core::workflow::StepResponse<Output>{{{"status","200"},
    {"body",support::wrap(support::offer_dto(blog::api::record_to_json(items.front()), {}))}}};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
