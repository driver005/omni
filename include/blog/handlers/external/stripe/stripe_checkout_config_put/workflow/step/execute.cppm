export module blog_handlers:stripe_stripe_checkout_config_put_step;
import :domain_all;
import std;
import core_workflow;
import core_serde;
import interfaces;
import :stripe;
import :stripe_stripe_checkout_config_get_step;

export namespace blog::handlers::stripe::stripe_checkout_config_put::workflow {

class ExecuteStep {
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"stripe_stripe_checkout_config_put_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output> invoke(
        const Input& in, core::workflow::WorkflowContext&) const
    {
        using namespace blog::handlers::stripe::support;
        auto& ctx = *s_ctx;
        auto& store = ctx.store();

        // Ghost: permissions:{docName:'products',method:'edit'} on config.edit.
        const auto authorized = authorize(in, "edit");
        if (!authorized.allowed) {
            return core::workflow::StepResponse<Output>{{{"status", std::to_string(authorized.status)},
                                                          {"body", authorized.error_json}}};
        }
        const auto principal = principal_for(in);

        const auto doc = core_serde::JsonCodec{}.decode<serde::Value>(in.body_as_string());
        const auto* root = doc ? std::get_if<serde::Value::Object>(&doc->get()) : nullptr;
        const serde::Value* cfg = nullptr;
        if (root != nullptr) {
            for (const auto& [key, value] : *root) {
                if (key == "checkout_config") { cfg = &value; break; }
            }
        }
        const auto* array = cfg ? std::get_if<serde::Value::Array>(&cfg->get()) : nullptr;
        if (array == nullptr || array->empty()) {
            return validation("checkout_config is required.", "checkout_config");
        }
        const auto* object = std::get_if<serde::Value::Object>(&array->at(0).get());
        if (object == nullptr) {
            return validation("checkout_config must be an object.", "checkout_config");
        }

        // strictObject({design, shipping?, phone?}) + refuseCollectionWhileOff.
        std::string encoded_design;
        bool has_design = false;
        std::string bad_key;
        for (const auto& [key, value] : *object) {
            if (key == "design") {
                has_design = true;
                if (!valid_design_input(value, encoded_design)) {
                    return validation("Invalid checkout design.", "design");
                }
            } else if (key == "shipping") {
                bad_key = "Shipping address collection is not available yet.";
            } else if (key == "phone") {
                bad_key = "Phone number collection is not available yet.";
            } else if (bad_key.empty()) {
                bad_key = "Unrecognized key in checkout_config: " + key;
            }
        }
        if (!has_design) return validation("design is required.", "design");
        if (!bad_key.empty()) return validation(bad_key, "checkout_config");

        const auto before = store.json_for("stripe_checkout_config", {});
        std::string existing_id;
        std::string created_at;
        for (const auto& row : blog::api::parse_array(before)) {
            if (unquote(field_text(row, "slug")) != "default") continue;
            existing_id = unquote(field_text(row, "id"));
            created_at = unquote(field_text(row, "created_at"));
            break;
        }
        const auto stamp = now_iso();
        if (created_at.empty()) created_at = stamp;
        const auto id = existing_id.empty()
            ? std::format("{:024x}", static_cast<unsigned long long>(
                  std::chrono::duration_cast<std::chrono::seconds>(
                      std::chrono::system_clock::now().time_since_epoch()).count()))
            : existing_id;

        const auto row_json = std::format(
            R"({{"id":{},"slug":"default","design":{},"created_at":{},"updated_at":{}}})",
            json_quote(id), encoded_design, json_quote(created_at), json_quote(stamp));

        // ensureTransaction: stage the row write and the action record together.
        auto& outbox = blog::handlers::outbox::global_outbox();
        outbox.begin_transaction();
        if (!store.edit("stripe_checkout_config", id, row_json).ok()) {
            [[maybe_unused]] const auto created = store.create("stripe_checkout_config", row_json);
        }

        // Ghost recordAction: staff user actor, structured context, no-op when
        // there is no actor. OUTBOX keeps the action observable for verification.
        if (!principal.id.empty()) {
            const auto action = std::format(
                R"({{"event":"edited","resource_type":"stripe_checkout_config","resource_id":{},"actor_type":"user","actor_id":{},"context":{{"primary_name":"Stripe Checkout"}},"created_at":{}}})",
                json_quote(id), json_quote(principal.id), json_quote(stamp));
            [[maybe_unused]] const auto recorded = store.create("actions", action);
            domain_event("stripe_checkout.config.edited", action, id);
        }
        outbox.commit();

        const auto after = store.json_for("stripe_checkout_config", {});
        const bool changed = before != after;
        return core::workflow::StepResponse<Output>{{{"status", "200"},
                                                      {"changed", changed ? "1" : "0"},
                                                      {"body", config_response(ctx)}}};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;
    static blog::api::StripeHandler* s_stripe;

private:
    [[nodiscard]] static core::workflow::StepResponse<Output> validation(
        std::string_view message, std::string_view property)
    {
        return core::workflow::StepResponse<Output>{{{"status", "422"},
            {"body", blog::api::ApiError{}.validation(message, "ValidationError", property)}}};
    }
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};
blog::api::StripeHandler* ExecuteStep::s_stripe{nullptr};

} // namespace blog::handlers::stripe::stripe_checkout_config_put::workflow
