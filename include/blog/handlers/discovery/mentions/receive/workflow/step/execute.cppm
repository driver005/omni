export module blog_handlers:mentions_receive_step;
import :domain_all;
import :mentions_browse_step;
import std;
import core_workflow;
import core_serde;
import :outbox_shim;
import interfaces;

export namespace blog::handlers::mentions::receive::support {

using namespace blog::handlers::mentions::support;

/** @brief webmentions/routes.js:23 — bodyParser.urlencoded(limit:'64kb'). */
inline constexpr std::size_t kMaxBodyBytes = 64 * 1024;
/** @brief webmentionsBlock backoff budget (approximated as a fixed window). */
inline constexpr int kRateLimit = 10;
inline constexpr auto kRateWindow = std::chrono::seconds{60};

/** @brief Percent-decoding for application/x-www-form-urlencoded bodies. */
[[nodiscard]] inline std::string form_decode(std::string_view text)
{
    std::string out;
    out.reserve(text.size());
    for (std::size_t i = 0; i < text.size(); ++i) {
        if (text[i] == '+') { out += ' '; continue; }
        if (text[i] == '%' && i + 2 < text.size()) {
            const auto hex = [](char c) -> int {
                if (c >= '0' && c <= '9') return c - '0';
                if (c >= 'a' && c <= 'f') return c - 'a' + 10;
                if (c >= 'A' && c <= 'F') return c - 'A' + 10;
                return -1;
            };
            const int hi = hex(text[i + 1]);
            const int lo = hex(text[i + 2]);
            if (hi >= 0 && lo >= 0) {
                out += static_cast<char>((hi << 4) | lo);
                i += 2;
                continue;
            }
        }
        out += text[i];
    }
    return out;
}

/** @brief Ghost reads `{source, target}` from frame.data: urlencoded on the real
 *         webmentions router, JSON for the flat C++ wire client. */
[[nodiscard]] inline std::pair<std::string, std::string> parse_source_target(std::string_view body)
{
    std::string source;
    std::string target;
    if (body.find('=') != std::string_view::npos && body.find('{') == std::string_view::npos) {
        for (auto part : std::views::split(body, '&')) {
            const std::string text{std::ranges::begin(part), std::ranges::end(part)};
            const auto eq = text.find('=');
            if (eq == std::string::npos) continue;
            const auto key = form_decode(std::string_view{text}.substr(0, eq));
            const auto value = form_decode(std::string_view{text}.substr(eq + 1));
            if (key == "source") source = value;
            else if (key == "target") target = value;
        }
        return {source, target};
    }
    if (const auto parsed = core_serde::JsonCodec{}.decode<serde::Value>(std::string{body})) {
        source = row_field(core_serde::JsonCodec{}.encode(*parsed), "source");
        target = row_field(core_serde::JsonCodec{}.encode(*parsed), "target");
    }
    return {source, target};
}

/** @brief One observable processed-webmention side effect. */
struct ProcessedWebmention
{
    std::string type;
    std::string payload;
};

[[nodiscard]] inline std::vector<ProcessedWebmention>& processed_log()
{
    static std::vector<ProcessedWebmention> log;
    return log;
}

/** @brief The job adapter needs the in-flight context to persist its result. */
[[nodiscard]] inline blog::BlogContext*& job_context() noexcept
{
    static blog::BlogContext* ctx{nullptr};
    return ctx;
}

/**
 * @brief Outbox adapter standing in for Ghost's ProcessWebmentionJob worker.
 *        `job.dispatch` → webmention metadata fetch + mention persistence +
 *        `mention.added` domain event (MentionsAPI.processWebmention /
 *        BookshelfMentionRepository.save / DomainEvents.dispatch).
 */
class WebmentionJobAdapter final : public blog::handlers::outbox::Adapter
{
public:
    void dispatch(const blog::handlers::outbox::Event& event) override
    {
        processed_log().push_back({event.type, event.payload});
        if (event.type != "job.dispatch") return;

        auto* ctx = job_context();
        if (ctx == nullptr) return;
        const std::string source = row_field(event.payload, "source");
        const std::string target = row_field(event.payload, "target");

        // Outbound webmention metadata fetch (CONTRACTS.md §5 WebmentionAdapter).
        emit_event("webmention.fetch",
                   "{\"source\":\"" + source + "\",\"target\":\"" + target + "\"}", target);

        // Mention.create + repository.save; timestamp mirrors created_at.
        const std::string record =
            "{\"source\":\"" + source + "\",\"target\":\"" + target +
            "\",\"created_at\":\"" + std::to_string(std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch()).count()) +
            "\",\"verified\":false,\"deleted\":false,\"revalidation_failure_count\":0,\"payload\":null}";
        const std::string id = ctx->store().resources().create("mentions", record);
        emit_event("mention.added", "{\"id\":\"" + id + "\",\"source\":\"" + source +
                                    "\",\"target\":\"" + target + "\"}", id);
    }
};

inline void ensure_adapter()
{
    static const bool registered = [] {
        blog::handlers::outbox::register_adapter(std::make_shared<WebmentionJobAdapter>());
        return true;
    }();
    (void)registered;
}

/** @brief Fixed-window limiter keyed by (context, client IP). */
[[nodiscard]] inline bool rate_limited(blog::BlogContext* ctx, std::string_view ip)
{
    struct Window { std::chrono::steady_clock::time_point start; int count{0}; };
    static std::mutex mutex;
    static std::map<std::pair<blog::BlogContext*, std::string>, Window> windows;
    const auto now = std::chrono::steady_clock::now();
    std::lock_guard lock{mutex};
    auto& window = windows[{ctx, std::string{ip}}];
    if (now - window.start >= kRateWindow) window = {now, 0};
    if (++window.count > kRateLimit) return true;
    return false;
}

} // namespace blog::handlers::mentions::receive::support

export namespace blog::handlers::mentions::receive::workflow {

class ExecuteStep {
public:
  using Input = interfaces::io::IRequest&;
  using Output = std::map<std::string, std::string>;
  static constexpr std::string_view name{"mentions_receive_execute"};
  [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
    using namespace blog::handlers::mentions::receive::support;
    auto& ctx = *s_ctx;

    const std::string body = in.body_as_string().empty() ? std::string{"{}"}
                                                         : std::string{in.body_as_string()};
    if (body.size() > kMaxBodyBytes) {
      return core::workflow::StepResponse<Output>{{{"status", "413"},
        {"body", blog::api::ApiError{}.validation(
            "Request Entity Too Large", "Payload Too Large", "body", "PayloadTooLargeError")}}};
    }

    // permissions:false — the only gate is webmentionsBlock (429 on excess).
    const auto ip = in.find_header("x-forwarded-for");
    if (rate_limited(&ctx, ip.empty() ? "unknown" : ip)) {
      return core::workflow::StepResponse<Output>{{{"status", "429"},
        {"body", blog::api::ApiError{}.validation(
            "Too many mention attempts", "Too many mention attempts", "", "TooManyRequestsError")}}};
    }

    const auto [source, target] = parse_source_target(body);

    // MentionController.receive queues ProcessWebmentionJob and returns null.
    const std::string job = "{\"type\":\"process-webmention\",\"source\":\"" + source +
                            "\",\"target\":\"" + target + "\",\"replayAfterRestart\":true}";
    emit_event("job.dispatch", job, target);

    // Run the queued job through the registered outbox adapter. Its own emits
    // (webmention.fetch / mention.added) stay pending for verification.
    ensure_adapter();
    job_context() = &ctx;
    auto& outbox = blog::handlers::outbox::global_outbox();
    [[maybe_unused]] const auto drained = outbox.drain();
    job_context() = nullptr;

    // statusCode 202 + response.format 'plain' + `return null` -> empty body.
    return core::workflow::StepResponse<Output>{{{"status", "202"}, {"body", ""}}};
  }
  void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
  static blog::BlogContext* s_ctx;
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
