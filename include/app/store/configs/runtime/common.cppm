export module app_store:configs_runtime_common;

import std;
import core_behavior;
import core_error;
import app_context;
import app_metrics;

export namespace app::handlers::configs {

/**
 * @brief Shared support for the configs sub-runtimes.
 *
 * Carries the AppContext reference, the local `#[instrument(flow = ...)]`
 * substitute, and the two upstream error values. HE_01 / HE_02 live in
 * kind_name and the human-readable text in message, so the shared AppError
 * serializer preserves both on the wire.
 */
class ConfigsRuntimeSupport
{
public:
    explicit ConfigsRuntimeSupport(::app::context::AppContext& context) noexcept :
        m_context{context}
    {
    }

    /** DuplicateConfig -> BadRequest HE_01 (api_error_response.rs:444-446). */
    [[nodiscard]] core::error::AppError duplicate_config_error() const
    {
        auto error = core::error::AppErrorBuilder{}(
            core::error::AppErrorKind::INVALID_REQUEST,
            "The config with the specified key already exists in our records"
        );
        error.set_kind_name("HE_01");
        return error;
    }

    /** ConfigNotFound -> NotFound HE_02 (api_error_response.rs:456-458). */
    [[nodiscard]] core::error::AppError config_not_found_error() const
    {
        auto error = core::error::AppErrorBuilder{}(
            core::error::AppErrorKind::NOT_FOUND,
            "Config key does not exist in our records."
        );
        error.set_kind_name("HE_02");
        return error;
    }

    /** Local substitute for the upstream `#[instrument(flow = ?Flow::...)]` tracing span. */
    void record_flow(std::string_view flow) const
    {
        m_context.metrics().record_flow(flow);
        m_context.metrics().counter(flow).increment();
    }

    /**
     * @brief web::Path<String> key: /configs/:key (index 1) or
     * /v2/configs/:key (index 2 when a leading "v2" segment is present).
     */
    [[nodiscard]] static std::string config_key(const core::behavior::CallContext& ctx)
    {
        const bool is_v2 =
            !ctx.path_segments.empty() && ctx.path_segments.front() == "v2";
        const std::size_t index{is_v2 ? 2U : 1U};
        return ctx.path_segments.size() > index ? ctx.path_segments[index] : std::string{};
    }

private:
    ::app::context::AppContext& m_context;
};

} // namespace app::handlers::configs
