export module blog_handlers:themes_themes_upload_post_step;
import :domain_all;
import :outbox_shim;
import std;
import core_workflow;
import core_serde;
import interfaces;

export namespace blog::handlers::themes::themes_upload_post::workflow {

namespace upload_detail {

    [[nodiscard]] inline std::string trim_copy(std::string_view v)
    {
        while (!v.empty() && std::isspace(static_cast<unsigned char>(v.front()))) {
            v.remove_prefix(1);
        }
        while (!v.empty() && std::isspace(static_cast<unsigned char>(v.back()))) {
            v.remove_suffix(1);
        }
        return std::string{v};
    }

    [[nodiscard]] inline std::string lower(std::string_view s)
    {
        std::string out{s};
        for (char& c: out) {
            c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        }
        return out;
    }

    [[nodiscard]] inline std::string unquote(std::string_view raw)
    {
        const auto t = trim_copy(raw);
        if (t.size() >= 2 && t.front() == '"') {
            if (auto s = core_serde::JsonCodec{}.decode<std::string>(std::string{t})) {
                return *s;
            }
        }
        return std::string{t};
    }

    [[nodiscard]] inline std::string query_value(std::string_view path, std::string_view key)
    {
        const auto q = path.find('?');
        if (q == std::string_view::npos) {
            return {};
        }
        for (auto part: std::views::split(path.substr(q + 1), '&')) {
            std::string_view kv{std::ranges::begin(part), std::ranges::end(part)};
            const auto eq = kv.find('=');
            if (eq != std::string_view::npos && kv.substr(0, eq) == key) {
                return std::string{kv.substr(eq + 1)};
            }
        }
        return {};
    }

    struct Part
    {
        std::string filename;
        std::string payload;
    };

    /** @brief Stand-in for `apiMw.upload.themeZip('file')`: extract the multipart
     * `name="file"` part. Non-multipart clients fall back to the whole body so the
     * step stays binary-safe and validation-driven. */
    [[nodiscard]] inline Part extract_part(std::string_view body, std::string_view content_type)
    {
        Part out;
        const auto bpos = content_type.find("boundary=");
        if (bpos == std::string_view::npos) {
            out.payload = std::string{body};
            return out;
        }

        std::string boundary{trim_copy(content_type.substr(bpos + 9))};
        if (const auto semi = boundary.find(';'); semi != std::string::npos) {
            boundary.resize(semi);
        }
        if (boundary.size() >= 2 && boundary.front() == '"' && boundary.back() == '"') {
            boundary = boundary.substr(1, boundary.size() - 2);
        }
        if (boundary.empty()) {
            out.payload = std::string{body};
            return out;
        }

        const std::string marker = "--" + boundary;
        std::size_t pos = 0;
        while ((pos = body.find(marker, pos)) != std::string_view::npos) {
            pos += marker.size();
            if (body.substr(pos, 2) == "--") {
                break;
            }
            if (body.substr(pos, 2) == "\r\n") {
                pos += 2;
            } else if (body.substr(pos, 1) == "\n") {
                pos += 1;
            } else {
                continue;
            }

            std::size_t head_end = body.find("\r\n\r\n", pos), sep = 4;
            if (head_end == std::string_view::npos) {
                head_end = body.find("\n\n", pos);
                sep = 2;
            }
            if (head_end == std::string_view::npos) {
                break;
            }
            const auto headers = body.substr(pos, head_end - pos);
            const auto payload_start = head_end + sep;
            const auto next = body.find(marker, payload_start);
            auto payload = body.substr(
                payload_start,
                (next == std::string_view::npos ? body.size() : next) - payload_start
            );
            while (!payload.empty() && (payload.back() == '\r' || payload.back() == '\n')) {
                payload.remove_suffix(1);
            }

            if (headers.find("name=\"file\"") != std::string_view::npos) {
                const auto fpos = headers.find("filename=\"");
                if (fpos != std::string_view::npos) {
                    const auto end = headers.find('"', fpos + 10);
                    if (end != std::string_view::npos) {
                        out.filename = std::string{headers.substr(fpos + 10, end - (fpos + 10))};
                    }
                }
                out.payload = std::string{payload};
                return out;
            }
            pos = payload_start;
        }
        out.payload = std::string{body};
        return out;
    }

    [[nodiscard]] inline blog::api::RecordDto theme_record(std::string_view payload)
    {
        blog::api::RecordDto rec;
        if (auto arr = blog::api::parse_array(payload); !arr.empty()) {
            rec = arr.front();
        } else {
            rec = blog::api::parse_object(payload);
        }
        return rec;
    }

} // namespace upload_detail

/** @brief Ghost `upload` (`themes.js:104-135`): body-parser `themeZip('file')`,
 * lowercased `originalname`, `copy_settings_from`, `setFromZip`, `theme.uploaded`
 * and (on override) `site.changed` through `core_outbox`. */
class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"themes_themes_upload_post_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        auto& ctx = *s_ctx;
        const auto body = in.body_as_string();
        const auto content_type =
            std::string{in.find_header(interfaces::io::types::Token::CONTENT_TYPE)};
        const auto part = upload_detail::extract_part(body, content_type);
        const auto path = in.get_path();

        auto rec = upload_detail::theme_record(part.payload);
        std::string name =
            part.filename.empty() ? std::string{} : upload_detail::lower(part.filename);
        if (const auto it = rec.find("name"); it != rec.end()) {
            const auto parsed = upload_detail::unquote(it->second);
            if (name.empty()) {
                name = parsed;
            }
            if (!parsed.empty()) {
                rec["name"] = core_serde::JsonCodec{}.encode(parsed);
            }
        }
        if (name.empty()) {
            return core::workflow::StepResponse<Output>{Output{{"status", "400"}}};
        }
        rec["name"] = core_serde::JsonCodec{}.encode(name);

        auto copy = upload_detail::query_value(path, "copy_settings_from");
        if (copy.empty()) {
            if (const auto it = rec.find("copy_settings_from"); it != rec.end()) {
                copy = upload_detail::unquote(it->second);
            }
        }
        if (!copy.empty()) {
            rec["copy_settings_from"] = core_serde::JsonCodec{}.encode(copy);
        }

        auto& rs = ctx.store().resources();
        std::string override_id;
        for (const auto& existing: blog::api::parse_array(rs.list("themes"))) {
            const auto it = existing.find("name");
            if (it == existing.end()) {
                continue;
            }
            if (upload_detail::unquote(it->second) != name) {
                continue;
            }
            const auto idit = existing.find("id");
            if (idit != existing.end()) {
                override_id = upload_detail::unquote(idit->second);
            }
            break;
        }
        const bool overridden = !override_id.empty();
        const auto theme_json = blog::api::record_to_json(rec);

        auto& outbox = blog::handlers::outbox::global_outbox();
        outbox.begin_transaction();
        if (overridden) {
            rs.edit("themes", override_id, theme_json);
        } else {
            rs.create("themes", theme_json);
        }
        // Ghost `events.emit('theme.uploaded', {name: theme.name})` (`themes.js:132`).
        outbox.emit(
            blog::handlers::outbox::Event{
                .type = "theme.uploaded",
                .payload = "{\"name\":" + core_serde::JsonCodec{}.encode(name) + "}",
                .aggregate_id = name
            }
        );
        if (overridden) {
            outbox.emit(
                blog::handlers::outbox::Event{
                    .type = "site.changed",
                    .payload = "{}",
                    .aggregate_id = "site"
                }
            );
        }
        outbox.commit();

        return core::workflow::StepResponse<Output>{Output{
            {"status", "200"},
            {"body", "{\"themes\":[" + theme_json + "]}"},
            {"override", overridden ? "1" : "0"}
        }};
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};

} // namespace blog::handlers::themes::themes_upload_post::workflow
