export module blog_handlers:stripe_stripe_checkout_branding_get_step;
import :domain_all;
import std;
import core_workflow;
import core_serde;
import interfaces;
import :stripe;
import :stripe_stripe_checkout_config_get_step;

export namespace blog::handlers::stripe::stripe_checkout_branding_get::workflow {

class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"stripe_stripe_checkout_branding_get_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        using namespace blog::handlers::stripe::support;
        auto& ctx = *s_ctx;

        // Ghost: permissions:{docName:'products',method:'read'} on branding.read.
        const auto authorized = authorize(in, "read");
        if (!authorized.allowed) {
            return core::workflow::StepResponse<Output>{
                {{"status", std::to_string(authorized.status)}, {"body", authorized.error_json}}
            };
        }
        if (!stripe_connected()) {
            return validation("Connect Stripe to read its checkout branding.", nullptr);
        }

        // Ghost getCheckoutBranding(): rate-limit throttle, Checkout Session
        // create with ghost_checkout_preview metadata, then session expire.
        // core_adapters is not a registered dep; the calls are recorded on the
        // context-local outbox in core_adapters::OutboundCall shape.
        const auto base = env_or("STRIPE_BASE_URL", "https://api.stripe.com");
        const auto session_request =
            R"({"mode":"setup","payment_method_types":["card"],"metadata":{"ghost_checkout_preview":true}})";
        domain_event("stripe.rate_limit.throttle", "{}", "stripe");
        outbound(
            "create_session",
            "POST",
            base + "/v1/checkout/sessions",
            session_request,
            R"({"id":"cs_preview_branding","object":"checkout.session"})",
            200
        );
        domain_event("stripe.rate_limit.throttle", "{}", "stripe");
        outbound(
            "expire",
            "POST",
            base + "/v1/checkout/sessions/cs_preview_branding/expire",
            "{}",
            R"({"id":"cs_preview_branding","status":"expired"})",
            200
        );

        // branding_settings comes from a local mock store (`stripe_branding`),
        // falling back to the checkout config display name. An explicit
        // branding row without display_name models the 500 identity-unreadable
        // branch of Ghost's CheckoutBrandingService.read().
        const auto rows = ctx.store().json_for("stripe_branding", {});
        const auto records = blog::api::parse_array(rows);
        bool explicit_branding = !records.empty();
        std::string display_name;
        std::string design_raw;
        if (explicit_branding) {
            const auto& row = records.back();
            display_name = unquote(field_text(row, "display_name"));
            if (display_name.empty()) {
                display_name = unquote(field_text(row, "displayName"));
            }
            design_raw = field_text(row, "design");
        } else {
            const auto config_rows = ctx.store().json_for("stripe_checkout_config", {});
            const auto config = blog::api::parse_array(config_rows);
            if (!config.empty()) {
                display_name = unquote(field_text(config.back(), "display_name"));
                if (display_name.empty()) {
                    display_name = unquote(field_text(config.back(), "displayName"));
                }
                design_raw = field_text(config.back(), "design");
            }
        }
        if (explicit_branding && display_name.empty()) {
            return core::workflow::StepResponse<Output>{
                {{"status", "500"},
                 {"body",
                  blog::api::ApiError{}.json(
                      "Stripe reported checkout branding Ghost cannot show.",
                      "StripeCheckoutBranding"
                  )}}
            };
        }
        if (display_name.empty()) {
            display_name = "Ghost";
        }

        auto design = snake_design_response(design_raw);
        if (!design_raw.empty() && design_raw != "null" && design_raw != "{}" &&
            design == R"({"customize":false})") {
            warn("stripe_checkout.branding.design_unreadable");
        }

        // Ghost serializer: {"checkout_branding":[{display_name, design}]}.
        const auto body = wrap(
            "checkout_branding",
            "{\"display_name\":" + json_quote(display_name) + ",\"design\":" + design + "}"
        );
        return core::workflow::StepResponse<Output>{{{"status", "200"}, {"body", body}}};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;
    static blog::api::StripeHandler* s_stripe;

private:
    [[nodiscard]] static core::workflow::StepResponse<Output>
    validation(std::string_view message, const char* /* property intentionally null */)
    {
        return core::workflow::StepResponse<Output>{
            {{"status", "422"}, {"body", blog::api::ApiError{}.json(message, "ValidationError")}}
        };
    }
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};
blog::api::StripeHandler* ExecuteStep::s_stripe{nullptr};

} // namespace blog::handlers::stripe::stripe_checkout_branding_get::workflow
