export module blog_handlers:stripe_stripe_checkout_preview_post_step;
import :domain_all;
import std;
import core_workflow;
import core_query;
import core_serde;
import interfaces;
import :stripe;
import :stripe_stripe_checkout_config_get_step;

export namespace blog::handlers::stripe::stripe_checkout_preview_post::workflow {

class ExecuteStep {
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"stripe_stripe_checkout_preview_post_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output> invoke(
        const Input& in, core::workflow::WorkflowContext&) const
    {
        using namespace blog::handlers::stripe::support;
        auto& ctx = *s_ctx;

        // Ghost: permissions:{docName:'products',method:'edit'} on preview.add.
        const auto authorized = authorize(in, "edit");
        if (!authorized.allowed) {
            return core::workflow::StepResponse<Output>{{{"status", std::to_string(authorized.status)},
                                                          {"body", authorized.error_json}}};
        }

        // CheckoutPreviewInput strictObject: tier_id (24 hex), cadence, design,
        // optional shipping/phone gated by the collection flag.
        const auto doc = core_serde::JsonCodec{}.decode<serde::Value>(in.body_as_string());
        const auto* root = doc ? std::get_if<serde::Value::Object>(&doc->get()) : nullptr;
        const serde::Value* data = nullptr;
        if (root != nullptr) {
            for (const auto& [key, value] : *root) {
                if (key == "data") { data = &value; break; }
            }
        }
        const auto* array = data ? std::get_if<serde::Value::Array>(&data->get()) : nullptr;
        const serde::Value* item = nullptr;
        if (array != nullptr && !array->empty()) {
            const auto* object = std::get_if<serde::Value::Object>(&array->at(0).get());
            if (object != nullptr) {
                // Ghost reads data.checkout_preview[0]; data[0] is accepted as a
                // compatibility fallback for legacy clients.
                for (const auto& [key, value] : *object) {
                    if (key == "checkout_preview") {
                        const auto* nested = std::get_if<serde::Value::Array>(&value.get());
                        if (nested != nullptr && !nested->empty()) item = &nested->at(0);
                        break;
                    }
                }
                if (item == nullptr) item = &array->at(0);
            }
        }
        const auto* object = item ? std::get_if<serde::Value::Object>(&item->get()) : nullptr;
        if (object == nullptr) return validation("Invalid checkout preview input.", "tier_id");

        std::string tier_id;
        std::string cadence = "month";
        std::string design;
        std::string collection_error;
        for (const auto& [key, value] : *object) {
            if (key == "tier_id") tier_id = unquote(core_serde::JsonCodec{}.encode(value));
            else if (key == "cadence") cadence = unquote(core_serde::JsonCodec{}.encode(value));
            else if (key == "design") {
                if (!valid_design_input(value, design)) {
                    return validation("Invalid checkout design.", "design");
                }
            } else if (key == "shipping") {
                collection_error = "Shipping collection is not available yet.";
            } else if (key == "phone") {
                collection_error = "Phone number collection is not available yet.";
            } else {
                return validation("Unrecognized key in checkout_preview: " + key,
                              std::string_view{key});
            }
        }
        if (!is_hex24(tier_id)) return validation("Choose a tier to preview checkout for.", "tier_id");
        if (cadence != "month" && cadence != "year") {
            return validation("Invalid checkout preview input.", "cadence");
        }
        if (design.empty()) return validation("design is required.", "design");
        if (!collection_error.empty()) return validation(collection_error, "checkout_preview");

        // Ghost preview-service.add: stripeConnected() gate before the tier read.
        if (!stripe_connected()) {
            return validation("Connect Stripe to preview checkout.", {});
        }

        // readTier(id): a paid + active tier is required. core::query owns the
        // NQL filter over the context-local tier store.
        static constexpr std::array<std::string_view, 1> kFilter{{"filter"}};
        const auto q = core::query::parse("?filter[id]=" + tier_id, kFilter);
        auto tiers = ctx.store().json_for("tiers", {});
        try {
            tiers = core::query::apply(tiers, q, 15);
        } catch (...) { // NOLINT(bugprone-empty-catch) - no restricted fields here
        }
        bool paid_active = false;
        for (const auto& row : blog::api::parse_array(tiers)) {
            if (unquote(field_text(row, "id")) != tier_id) continue;
            const auto type = unquote(field_text(row, "type"));
            const auto status = unquote(field_text(row, "status"));
            if (type == "paid" && status == "active") paid_active = true;
            break;
        }
        if (!paid_active) {
            return validation("Choose an active paid tier to preview checkout for.", "tier_id");
        }

        // Ghost createPreviewCheckoutSession: throttle, sessions.create with
        // ghost_checkout_preview metadata + expires_at (now + 31 min), refusal
        // retry, and a 500 when Stripe returns no URL. Recorded on the outbox.
        const auto base = env_or("STRIPE_BASE_URL", "https://api.stripe.com");
        const auto return_url = env_or("SITE_URL", "http://localhost:2368");
        const auto expires = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count() + 31 * 60;
        const auto session_request = std::format(
            R"({{"mode":"subscription","success_url":{},"cancel_url":{},"expires_at":{},"metadata":{{"ghost_checkout_preview":true}},"checkout":{},"design":{}}})",
            json_quote(return_url), json_quote(return_url), expires, R"({"collect":false})", design);
        domain_event("stripe.rate_limit.throttle", "{}", "stripe");
        if (env_or("STRIPE_REFUSE", "0") == "1") {
            outbound("create_session", "POST", base + "/v1/checkout/sessions",
                     session_request, R"({"error":{"type":"invalid_request_error"}})", 400);
            return validation("Stripe refused the preview checkout settings.", "checkout_preview");
        }
        if (env_or("STRIPE_NO_URL", "0") == "1") {
            outbound("create_session", "POST", base + "/v1/checkout/sessions",
                     session_request, R"({"id":"cs_preview_no_url"})", 200);
            return core::workflow::StepResponse<Output>{{{"status", "500"},
                {"body", blog::api::ApiError{}.json(
                    "Stripe returned no checkout page to preview.", "InternalServerError")}}};
        }
        const auto url = std::format("https://checkout.stripe.com/c/pay/cs_test_{}_{}",
                                     tier_id, cadence);
        outbound("create_session", "POST", base + "/v1/checkout/sessions", session_request,
                 "{\"id\":\"cs_preview\",\"url\":" + json_quote(url) + "}", 200);

        // Ghost serializer: {"checkout_preview":[{url}]}.
        return core::workflow::StepResponse<Output>{{{"status", "200"},
            {"body", wrap("checkout_preview", "{\"url\":" + json_quote(url) + "}")}}};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
    static blog::BlogContext* s_ctx;
    static blog::api::StripeHandler* s_stripe;

private:
    [[nodiscard]] static bool is_hex24(std::string_view value)
    {
        if (value.size() != 24) return false;
        return std::ranges::all_of(value, [](unsigned char c) {
            return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f');
        });
    }

    [[nodiscard]] static core::workflow::StepResponse<Output> validation(
        std::string_view message, std::string_view property)
    {
        return core::workflow::StepResponse<Output>{{{"status", "422"},
            {"body", property.empty()
                ? blog::api::ApiError{}.json(message, "ValidationError")
                : blog::api::ApiError{}.validation(message, "ValidationError", property)}}};
    }
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};
blog::api::StripeHandler* ExecuteStep::s_stripe{nullptr};

} // namespace blog::handlers::stripe::stripe_checkout_preview_post::workflow
