export module blog_handlers:integrations_integrations_id_api_key_keyid_refresh_post_step;
import :domain_all;
import :integrations_dto_more_resources;
import :outbox_shim;
import std;
import core_workflow;
import interfaces;

export namespace blog::handlers::integrations::integrations_id_api_key_keyid_refresh_post::
    workflow {

namespace support = blog::handlers::integrations::support;

namespace refresh_support {

    inline blog::BlogContext* s_ctx{nullptr};
    inline bool s_failed{false};
    inline std::vector<std::pair<std::string, std::string>> s_log;

    /**
     * @brief Outbox adapter standing in for ApiKeyModel.refreshSecret (the write
     *        side effect of integrations.service.edit's keyid branch). The handler
     *        queues `api_key.secret_refreshed`; this adapter rotates the stored
     *        secret and records the `integration.edited` domain event.
     */
    class ApiKeyRefreshAdapter final : public blog::handlers::outbox::Adapter
    {
    public:
        void dispatch(const blog::handlers::outbox::Event& event) override
        {
            s_log.emplace_back(event.type, event.payload);
            if (event.type != "api_key.secret_refreshed") {
                return;
            }
            if (s_ctx == nullptr) {
                s_failed = true;
                return;
            }
            const auto fields = support::object(event.payload);
            const auto key_it = fields.find("keyid");
            const auto id_it = fields.find("integration_id");
            if (key_it == fields.end() || id_it == fields.end()) {
                s_failed = true;
                return;
            }
            const std::string keyid = support::unquote(key_it->second);
            const std::string integration_id = support::unquote(id_it->second);
            const std::string rotated = support::rotate_api_key(s_ctx->store(), keyid);
            if (rotated.empty()) {
                s_failed = true;
                return;
            }
            blog::handlers::outbox::global_outbox().emit(
                blog::handlers::outbox::Event{
                    .type = "integration.edited",
                    .payload = std::format(
                        R"({{"id":{},"keyid":{}}})",
                        support::quote(integration_id),
                        support::quote(keyid)
                    ),
                    .aggregate_id = integration_id
                }
            );
        }
    };

    inline void ensure_adapter()
    {
        static const bool registered = []
        {
            blog::handlers::outbox::register_adapter(std::make_shared<ApiKeyRefreshAdapter>());
            return true;
        }();
        (void)registered;
    }

} // namespace refresh_support

class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{
        "integrations_integrations_id_api_key_keyid_refresh_post_execute"
    };

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        auto& ctx = *s_ctx;
        const std::string_view path{in.get_path()};

        const auto options = support::query_options(path, {"id", "keyid", "include"});
        const std::string include =
            options.contains("include") ? options.at("include") : std::string{};
        if (!support::valid_include(include)) {
            return core::workflow::StepResponse<Output>{
                {{"status", "422"},
                 {"body",
                  blog::api::ApiError{}
                      .validation("Validation error", "Invalid include value", "include")}}
            };
        }

        const auto [id, keyid] = support::path_ids(path);
        std::string record = ctx.store().resources().get("integrations", id);
        if (id.empty() || record.empty()) {
            return core::workflow::StepResponse<Output>{
                {{"status", "404"}, {"body", support::not_found_integration()}}
            };
        }
        if (keyid.empty() || support::find_api_key(ctx.store(), keyid).empty()) {
            return core::workflow::StepResponse<Output>{
                {{"status", "404"}, {"body", support::not_found_api_key()}}
            };
        }

        // Route the rotation through the shared outbox; the adapter performs the
        // store write so the side effect is observable through drain()/pending().
        refresh_support::ensure_adapter();
        refresh_support::s_ctx = &ctx;
        refresh_support::s_failed = false;
        auto& outbox = blog::handlers::outbox::global_outbox();
        outbox.emit(
            blog::handlers::outbox::Event{
                .type = "api_key.secret_refreshed",
                .payload = std::format(
                    R"({{"keyid":{},"integration_id":{}}})",
                    support::quote(keyid),
                    support::quote(id)
                ),
                .aggregate_id = id
            }
        );
        [[maybe_unused]] const auto drained = outbox.drain();
        refresh_support::s_ctx = nullptr;

        if (refresh_support::s_failed) {
            return core::workflow::StepResponse<Output>{
                {{"status", "500"}, {"body", support::internal_error()}}
            };
        }

        // withRelated:['api_keys','webhooks'] (integrations-service.js:31-36).
        record = ctx.store().resources().get("integrations", id);
        record = support::embed_relations(ctx.store(), record, include, true);
        record = support::prefix_admin_secrets(record);
        return core::workflow::StepResponse<Output>{
            {{"status", "200"}, {"body", support::wrap("integrations", record)}}
        };
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};
} // namespace blog::handlers::integrations::integrations_id_api_key_keyid_refresh_post::workflow
