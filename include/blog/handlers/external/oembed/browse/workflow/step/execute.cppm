export module blog_handlers:oembed_browse_step;
import :domain_all;
import :oembed_dto_more_resources;
import :outbox_shim;
import std;
import core_workflow;
import interfaces;
import blog_handlers_middleware;

export namespace blog::handlers::oembed::browse::workflow {

/**
 * Ghost `oembed.read` has no DB model call: it calls
 * `oembed.fetchOembedDataFromUrl(url, type)` (externalRequest HTTP + imageStore.saveRaw)
 * and serializes the resulting object. The allowed module set has no HTTP client, so the
 * external fetch and the thumbnail store are represented by the registered shared outbox
 * (`blog_handlers:outbox_shim`, CONTRACTS.md §4) plus a locally registered adapter. This is
 * the observable stand-in for the provider fetch chain — no `find_by_resource("oembed", …)`
 * call remains (that central store has no `oembed` branch and never returned a row).
 */
namespace support {

    inline constexpr std::string_view kFetchEvent{"oembed.fetch"};
    inline constexpr std::string_view kThumbnailEvent{"oembed.thumbnail.saveRaw"};

    struct FetchResult
    {
        bool attempted{false};
        bool found{false};
        std::string error{}; // Ghost ValidationError message (422) when set
        std::string body{};  // Ghost-shaped response object / error envelope
    };

    [[nodiscard]] inline FetchResult& last_fetch() noexcept
    {
        static FetchResult result;
        return result;
    }

    [[nodiscard]] inline std::string json_quote(std::string_view raw)
    {
        std::string out{"\""};
        for (const char c: raw) {
            switch (c) {
                case '"':
                    out += "\\\"";
                    break;
                case '\\':
                    out += "\\\\";
                    break;
                case '\n':
                    out += "\\n";
                    break;
                case '\r':
                    out += "\\r";
                    break;
                case '\t':
                    out += "\\t";
                    break;
                default:
                    out.push_back(c);
            }
        }
        out.push_back('"');
        return out;
    }

    /** @brief Minimal JSON string-field reader for the outbox event payloads. */
    [[nodiscard]] inline std::string json_field(std::string_view json, std::string_view key)
    {
        const std::string needle = std::format("\"{}\"", key);
        const auto at = json.find(needle);
        if (at == std::string_view::npos) {
            return {};
        }
        auto rest = json.substr(at + needle.size());
        const auto colon = rest.find(':');
        if (colon == std::string_view::npos) {
            return {};
        }
        rest = rest.substr(colon + 1);
        while (!rest.empty() && std::isspace(static_cast<unsigned char>(rest.front()))) {
            rest.remove_prefix(1);
        }
        if (rest.empty() || rest.front() != '"') {
            return {};
        }
        rest.remove_prefix(1);
        std::string out;
        for (std::size_t i = 0; i < rest.size(); ++i) {
            if (rest[i] == '\\' && i + 1 < rest.size()) {
                ++i;
                switch (rest[i]) {
                    case 'n':
                        out.push_back('\n');
                        break;
                    case 'r':
                        out.push_back('\r');
                        break;
                    case 't':
                        out.push_back('\t');
                        break;
                    default:
                        out.push_back(rest[i]);
                }
            } else if (rest[i] == '"') {
                break;
            } else {
                out.push_back(rest[i]);
            }
        }
        return out;
    }

    [[nodiscard]] inline std::map<std::string, std::string> query_params(std::string_view path)
    {
        std::map<std::string, std::string> out;
        const auto q = path.find('?');
        if (q == std::string_view::npos) {
            return out;
        }
        for (auto part: std::views::split(path.substr(q + 1), '&')) {
            const std::string pair{std::ranges::begin(part), std::ranges::end(part)};
            if (const auto eq = pair.find('='); eq != std::string::npos) {
                out[pair.substr(0, eq)] = pair.substr(eq + 1);
            }
        }
        return out;
    }

    [[nodiscard]] inline std::string host_of(std::string_view url)
    {
        const auto scheme = url.find("://");
        if (scheme == std::string_view::npos) {
            return {};
        }
        auto rest = url.substr(scheme + 3);
        const auto end = rest.find_first_of("/?#");
        rest = rest.substr(0, end);
        const auto at = rest.find('@');
        if (at != std::string_view::npos) {
            rest = rest.substr(at + 1);
        }
        const auto colon = rest.rfind(':');
        if (colon != std::string_view::npos) {
            rest = rest.substr(0, colon);
        }
        return std::string{rest};
    }

    /** @brief Ghost `new URL(url)` guard — only absolute http(s) URLs are providers. */
    [[nodiscard]] inline bool absolute_http_url(std::string_view url)
    {
        const auto scheme = url.find("://");
        if (scheme == std::string_view::npos) {
            return false;
        }
        const auto prefix = url.substr(0, scheme);
        return (prefix == "http" || prefix == "https") && !host_of(url).empty();
    }

    struct Provider
    {
        std::string type{"rich"};
        std::string provider_name{};
        std::string provider_url{};
        std::string author_name{};
        std::string author_url{};
    };

    /** @brief Stand-in for Ghost's custom-provider registry / extractus allowlist. */
    [[nodiscard]] inline Provider provider_for(std::string_view host, std::string_view url)
    {
        Provider p;
        p.provider_name = std::string{host};
        p.provider_url = std::format("https://{}", host);
        p.author_url = std::string{url};
        if (host.contains("youtube") || host.contains("youtu.be")) {
            p.type = "video";
            p.provider_name = "YouTube";
            p.provider_url = "https://www.youtube.com/";
            p.author_name = "YouTube";
        } else if (host.contains("twitter") || host.contains("x.com")) {
            p.provider_name = "Twitter";
            p.provider_url = "https://twitter.com/";
            p.author_name = "Twitter";
        } else if (host.contains("opensea")) {
            p.provider_name = "OpenSea";
            p.provider_url = "https://opensea.io/";
            p.author_name = "OpenSea";
        }
        return p;
    }

    [[nodiscard]] inline std::string embed_html(std::string_view url)
    {
        return std::format(
            R"(<iframe src="{}" width="480" height="270" allowfullscreen></iframe>)",
            url
        );
    }

    /** @brief Provider thumbnail URL stand-in (YouTube hqdefault, else origin favicon). */
    [[nodiscard]] inline std::string provider_thumbnail(std::string_view host, std::string_view url)
    {
        if (host.contains("youtube") || host.contains("youtu.be")) {
            std::string id;
            if (const auto v = url.find("v="); v != std::string_view::npos) {
                id = std::string{url.substr(v + 2)};
                if (const auto amp = id.find('&'); amp != std::string::npos) {
                    id.resize(amp);
                }
            } else if (const auto slash = url.rfind('/'); slash != std::string_view::npos) {
                id = std::string{url.substr(slash + 1)};
            }
            if (!id.empty()) {
                return std::format("https://i.ytimg.com/vi/{}/hqdefault.jpg", id);
            }
        }
        return std::format("https://{}/favicon.ico", host);
    }

    /** @brief `imageStore.saveRaw` + `url.forImage` stand-in: content/images/<kind>/<id>.<ext>. */
    [[nodiscard]] inline std::string
    saved_image_path(std::string_view original, std::string_view kind = "thumbnail")
    {
        const auto hash = std::hash<std::string>{}(std::string{original});
        const std::string_view ext = kind == "icon" ? ".ico" : ".jpg";
        return std::format("content/images/{}/{:016x}{}", kind, hash, ext);
    }

    /**
     * @brief Outbox adapter standing in for the external provider fetch and the thumbnail
     *        image-store write. Emits `oembed.thumbnail.saveRaw` (left pending for verification)
     *        exactly as Ghost's `storeThumbnail` writes through `imageStore.saveRaw`.
     */
    class OembedFetchAdapter final : public blog::handlers::outbox::Adapter
    {
    public:
        void dispatch(const blog::handlers::outbox::Event& event) override
        {
            if (event.type == kFetchEvent) {
                fetch(event.payload);
            } else if (event.type == kThumbnailEvent) {
                // No byte storage is available; the rewritten path was computed in fetch().
            }
        }

    private:
        void fetch(std::string_view payload)
        {
            auto& result = last_fetch();
            result.attempted = true;
            const std::string url = json_field(payload, "url");
            const std::string type = json_field(payload, "type");

            // Ghost #fetchOembedDataFromUrl -> unknownProvider(url) -> 422.
            if (!absolute_http_url(url)) {
                result.error = "No provider found for supplied URL.";
                result.body = blog::api::ApiError{}.validation(result.error, url, "");
                return;
            }

            const std::string host = host_of(url);
            const Provider provider = provider_for(host, url);
            const std::string original_thumb = provider_thumbnail(host, url);
            emit_thumbnail(original_thumb);

            // Ghost: explicit `bookmark` / `mention` requests return the bookmark shape.
            if (type == "bookmark" || type == "mention") {
                const std::string title = std::format("Bookmark: {}", url);
                const std::string metadata = std::format(
                    R"("url":{},"title":{},"description":null,"author":null,"publisher":{},"thumbnail":{},"icon":{})",
                    json_quote(url),
                    json_quote(title),
                    json_quote(provider.provider_name),
                    json_quote(saved_image_path(original_thumb)),
                    json_quote(saved_image_path(host, "icon"))
                );
                std::string body = std::format(
                    R"({{"version":"1.0","type":"bookmark","url":{},"metadata":{{{}}}}})",
                    json_quote(url),
                    metadata
                );
                if (type == "mention") {
                    body.pop_back();
                    body += std::format(
                        R"(,"body":{},"contentType":"text/html"}})",
                        json_quote(
                            std::format("<html><head><title>{}</title></head></html>", title)
                        )
                    );
                }
                result.found = true;
                result.body = std::move(body);
                return;
            }

            // Raw oEmbed object: Ghost knownFields (oembed-service.js:769-784) plus the
            // thumbnail fields added by fetchOembedDataFromUrl (:838-850).
            blog::dto::oembed::OembedResponse dto;
            dto.set_type(provider.type);
            dto.set_version("1.0");
            dto.set_html(embed_html(url));
            dto.set_url(url);
            dto.set_title(std::format("{} embed", provider.provider_name));
            dto.set_width(480);
            dto.set_height(270);
            dto.set_author_name(provider.author_name);
            dto.set_author_url(provider.author_url);
            dto.set_provider_name(provider.provider_name);
            dto.set_provider_url(provider.provider_url);
            dto.set_thumbnail_width(480);
            dto.set_thumbnail_height(360);
            dto.set_thumbnail_url_original(original_thumb);
            dto.set_thumbnail_url(saved_image_path(original_thumb));
            result.found = true;
            result.body = blog::handlers::middleware::encode(dto);
        }

        void emit_thumbnail(std::string_view original)
        {
            blog::handlers::outbox::global_outbox().emit(
                blog::handlers::outbox::Event{
                    std::string{kThumbnailEvent},
                    std::format(R"({{"url":{}}})", json_quote(original)),
                    std::string{original}
                }
            );
        }
    };

    inline void ensure_adapter()
    {
        static const bool registered = []
        {
            blog::handlers::outbox::register_adapter(std::make_shared<OembedFetchAdapter>());
            return true;
        }();
        (void)registered;
    }

    /** @brief Emit `oembed.fetch`, drain the outbox, and return the adapter's result. */
    [[nodiscard]] inline const FetchResult& run_fetch(std::string_view url, std::string_view type)
    {
        ensure_adapter();
        last_fetch() = FetchResult{};
        auto& outbox = blog::handlers::outbox::global_outbox();
        outbox.emit(
            blog::handlers::outbox::Event{
                std::string{kFetchEvent},
                std::format(R"({{"url":{},"type":{}}})", json_quote(url), json_quote(type)),
                std::string{url}
            }
        );
        [[maybe_unused]] const auto drained = outbox.drain();
        return last_fetch();
    }

} // namespace support

class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"oembed_browse_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        // Ghost controller: data:['url','type'] read from the query string.
        const auto params = support::query_params(in.get_path());
        const std::string url = params.count("url") ? params.at("url") : std::string{};
        const std::string type = params.count("type") ? params.at("type") : std::string{};

        // Ghost query() -> fetchOembedDataFromUrl(url, type). Provider failures map to
        // ValidationError (422); the outbox adapter produces the stand-in response.
        const auto& result = support::run_fetch(url, type);
        if (!result.error.empty()) {
            return core::workflow::StepResponse<Output>{{{"status", "422"}, {"body", result.body}}};
        }
        return core::workflow::StepResponse<Output>{{{"status", "200"}, {"body", result.body}}};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};

} // namespace blog::handlers::oembed::browse::workflow
