export module blog_handlers:newsletters_newsletters_verifications_put_step;
import :domain_all;
import std;
import core_workflow;
import core_serde;
import interfaces;
import :outbox_shim;
import :newsletters_browse_step;

export namespace blog::handlers::newsletters::newsletters_verifications_put::workflow {
class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"newsletters_newsletters_verifications_put_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        using namespace support;
        auto& ctx = *s_ctx;
        const std::string raw = std::string{in.body_as_string()};
        const auto parsed = raw.empty() ? std::nullopt : decode(raw);

        // Ghost controller declares data:['token']; accept the legacy root token too.
        std::string token;
        if (parsed) {
            const auto* root = std::get_if<Object>(&parsed->get());
            if (root != nullptr) {
                const auto* data = value_for(root, "data");
                if (data != nullptr) {
                    if (const auto* d = std::get_if<Object>(&data->get())) {
                        token = field(d, "token");
                    }
                }
                if (token.empty()) {
                    token = field(root, "token");
                }
            }
        }
        if (token.empty()) {
            return core::workflow::StepResponse<Output>{
                {{"status", "400"},
                 {"body", bad_request("This verification link is not valid for a newsletter.")}}
            };
        }

        // service.verifyPropertyUpdate -> magicLinkService.getDataFromToken(token).
        // The signed single-use token codec is a recorded new-partition need; the
        // fixture-backed token resource carries the same {id, property, value} data.
        const auto payload = consume_verification_token(ctx, token);
        if (!payload) {
            return core::workflow::StepResponse<Output>{
                {{"status", "400"},
                 {"body", bad_request("This verification link is not valid for a newsletter.")}}
            };
        }
        const std::string property = field(&*payload, "property");
        const std::string id = field(&*payload, "id");
        if (id.empty() || !std::ranges::any_of(
                              kVerifiable,
                              [&](auto v)
                              {
                                  return property == v;
                              }
                          )) {
            return core::workflow::StepResponse<Output>{
                {{"status", "400"},
                 {"body", bad_request("This verification link is not valid for a newsletter.")}}
            };
        }

        // model.edit(attrs,{id}); a missing row is a 400 in Ghost's error mapping.
        const std::string value = field(&*payload, "value");
        Object edit;
        edit[property] = serde::Value{value};
        auto& outbox = blog::handlers::outbox::global_outbox();
        outbox.begin_transaction();
        const auto result = ctx.store().edit("newsletters", id, encode(serde::Value{edit}));
        outbox.emit(
            blog::handlers::outbox::Event{
                "email.sender_verified",
                "{\"property\":\"" + escape(property) + "\",\"value\":\"" + escape(value) + "\"}",
                id
            }
        );
        outbox.commit();
        verification_side_effect::ensure();
        [[maybe_unused]] const auto drained = outbox.drain();
        if (result.status() == 404) {
            return core::workflow::StepResponse<Output>{
                {{"status", "400"},
                 {"body", bad_request("This verification link is not valid for a newsletter.")}}
            };
        }

        std::string model;
        const auto reloaded = ctx.store().find_by_resource("newsletters", id);
        if (reloaded) {
            model = *reloaded;
        } else {
            model = first_row(result.body()).empty()
                        ? std::string{"{}"}
                        : encode(serde::Value{first_row(result.body())});
        }
        const std::string meta = "{\"email_verified\":\"" + escape(property) + "\"}";
        return core::workflow::StepResponse<Output>{
            {{"status", "200"}, {"body", wrap("newsletters", model, meta)}}
        };
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};
} // namespace blog::handlers::newsletters::newsletters_verifications_put::workflow
