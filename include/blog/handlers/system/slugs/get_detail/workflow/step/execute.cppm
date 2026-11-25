export module blog_handlers:slugs_get_detail_step;
import :domain_all;
import std;
import core_workflow;
import core_serde;
import interfaces;

export namespace blog::handlers::slugs::get_detail::workflow {

namespace local_mapping {
    [[nodiscard]] inline std::string
    wrap(std::string_view resource, std::string_view rows, std::string_view meta = {})
    {
        return "{\"" + std::string{resource} + "\":" + std::string{rows} +
               (meta.empty() ? std::string{} : ",\"meta\":" + std::string{meta}) + "}";
    }

    [[nodiscard]] inline std::string clean(std::string_view json, std::string_view)
    {
        return std::string{json};
    }

    [[nodiscard]] inline std::string pagination(
        std::size_t total,
        std::size_t limit,
        std::size_t page,
        std::size_t pages,
        std::string_view prev,
        std::string_view next
    )
    {
        return std::format(
            R"("total":{},"limit":{},"page":{},"pages":{},"prev":{},"next":{})",
            total,
            limit,
            page,
            pages,
            prev.empty() ? std::string{"null"} : std::string{prev},
            next.empty() ? std::string{"null"} : std::string{next}
        );
    }
} // namespace local_mapping

/**
 * @brief Ghost `slugs.generate` query (slugs.js:41-53) plus the shared slug
 * engine (models/base/plugins/generate-slug.js:21-122).
 *
 * Strict-parity implementation (group 58). The observable contract:
 * - options ['include','type','id'], data ['name']; `type` required and in
 *   {post,tag,user}, `name` required, `id` optional -> 422 via ApiError;
 * - base name is the model table name without trailing 's' (post/tag/user);
 * - `security.string.safe` sanitization (lower, transliterate common Latin-1,
 *   non-alnum -> single '-', trim), 185-char cap, tag `#` -> `hash-`, protected
 *   slug suffix, empty -> baseName (generate-slug.js:75-114);
 * - uniqueness via a `status:'all'` read of the mapped table with `modelId`
 *   self-exclusion and the `base-2`, `base-3`, ... suffix loop
 *   (generate-slug.js:29-73);
 * - success envelope `{slugs:[{slug}]}` via local_mapping::wrap
 *   (output/slugs.js:4-10), no pagination meta.
 *
 * No real Knex `findOne` exists in C++; the context-local store is the
 * accepted stand-in (recorded residual). Non-Latin unidecode transliteration
 * has no C++ primitive and is recorded as a new-partition need.
 */
class ExecuteStep
{
public:
    using Input = interfaces::io::IRequest&;
    using Output = std::map<std::string, std::string>;
    static constexpr std::string_view name{"slugs_generate_execute"};

    [[nodiscard]] core::workflow::StepResponse<Output>
    invoke(const Input& in, core::workflow::WorkflowContext&) const
    {
        // Ghost merges URL params into options/data (frame.ts:73-114). Split once.
        const auto seg = split_path(in.get_path());
        const std::string type = seg.size() > 1 ? seg[1] : std::string{};
        const std::string name = seg.size() > 2 ? seg[2] : std::string{};
        const std::string model_id = seg.size() > 3 ? seg[3] : std::string{};

        if (!valid_type(type)) {
            return invalid("Invalid value.", "type");
        }
        if (name.empty()) {
            return invalid("Invalid value.", "name");
        }

        const std::string_view resource = type == "post"  ? "posts"
                                          : type == "tag" ? "tags"
                                                          : "users";
        const std::string_view base_name = type; // table name minus trailing 's'
        const auto rows = blog::api::parse_array((*s_ctx).store().json_for(resource, {}));

        std::string base = security_safe(name);
        if (base.size() > kSlugLimit) {
            base.resize(kSlugLimit); // generate-slug.js:80-85
        }
        if (type == "tag" && !name.empty() && name.front() == '#') {
            base = "hash-" + base; // generate-slug.js:99-105
        }
        if (is_protected(base, type)) {
            base += "-" + std::string{base_name}; // generate-slug.js:107-109
        }
        if (base.empty()) {
            base = std::string{base_name}; // generate-slug.js:111-114
        }

        std::string slug = base;
        for (int n = 2; collision(rows, slug, model_id); ++n) {
            slug = base + "-" + std::to_string(n);
        }

        if (slug.empty()) {
            return core::workflow::StepResponse<Output>{
                {{"status", "500"},
                 {"body",
                  blog::api::ApiError{}
                      .validation("Could not generate slug.", "", "", "InternalServerError")}}
            };
        }

        const std::string row = "{\"slug\":" + core_serde::JsonCodec{}.encode(slug) + "}";
        return core::workflow::StepResponse<Output>{
            {{"status", "200"}, {"body", local_mapping::wrap("slugs", row)}}
        };
    }

    void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}

    static blog::BlogContext* s_ctx;

private:
    static constexpr std::size_t kSlugLimit = 185;

    [[nodiscard]] static bool valid_type(std::string_view t) noexcept
    {
        return t == "post" || t == "tag" || t == "user";
    }

    [[nodiscard]] static core::workflow::StepResponse<Output>
    invalid(std::string_view message, std::string_view property)
    {
        return core::workflow::StepResponse<Output>{
            {{"status", "422"}, {"body", blog::api::ApiError{}.validation(message, "", property)}}
        };
    }

    [[nodiscard]] static std::vector<std::string> split_path(std::string_view p)
    {
        if (const auto q = p.find('?'); q != std::string_view::npos) {
            p = p.substr(0, q);
        }
        std::vector<std::string> out;
        for (std::size_t b = 0; b <= p.size();) {
            const std::size_t e = p.find('/', b);
            const std::size_t stop = (e == std::string_view::npos) ? p.size() : e;
            if (stop > b) {
                out.emplace_back(p.substr(b, stop - b));
            }
            if (e == std::string_view::npos) {
                break;
            }
            b = e + 1;
        }
        return out;
    }

    /** @brief Minimum UTF-8 decode: returns the code point and its byte length. */
    [[nodiscard]] static std::pair<unsigned int, std::size_t>
    code_point(std::string_view s, std::size_t i) noexcept
    {
        const auto b0 = static_cast<unsigned char>(s[i]);
        if (b0 < 0x80) {
            return {b0, 1};
        }
        if ((b0 & 0xE0) == 0xC0 && i + 1 < s.size()) {
            return {((b0 & 0x1Fu) << 6) | (static_cast<unsigned char>(s[i + 1]) & 0x3Fu), 2};
        }
        if ((b0 & 0xF0) == 0xE0 && i + 2 < s.size()) {
            return {
                ((b0 & 0x0Fu) << 12) | ((static_cast<unsigned char>(s[i + 1]) & 0x3Fu) << 6) |
                    (static_cast<unsigned char>(s[i + 2]) & 0x3Fu),
                3
            };
        }
        if ((b0 & 0xF8) == 0xF0 && i + 3 < s.size()) {
            return {
                ((b0 & 0x07u) << 18) | ((static_cast<unsigned char>(s[i + 1]) & 0x3Fu) << 12) |
                    ((static_cast<unsigned char>(s[i + 2]) & 0x3Fu) << 6) |
                    (static_cast<unsigned char>(s[i + 3]) & 0x3Fu),
                4
            };
        }
        return {0xFF'FDu, 1};
    }

    /** @brief Latin-1 / Latin Extended-A transliteration for common code points. */
    [[nodiscard]] static std::string_view transliterate(unsigned int cp) noexcept
    {
        switch (cp) {
            case 0x00'C0:
            case 0x00'C1:
            case 0x00'C2:
            case 0x00'C3:
            case 0x00'C4:
            case 0x00'C5:
            case 0x00'E0:
            case 0x00'E1:
            case 0x00'E2:
            case 0x00'E3:
            case 0x00'E4:
            case 0x00'E5:
            case 0x01'00:
            case 0x01'01:
            case 0x01'02:
            case 0x01'03:
            case 0x01'04:
            case 0x01'05:
                return "a";
            case 0x00'C7:
            case 0x00'E7:
            case 0x01'06:
            case 0x01'07:
            case 0x01'08:
            case 0x01'09:
            case 0x01'0A:
            case 0x01'0B:
            case 0x01'0C:
            case 0x01'0D:
                return "c";
            case 0x00'C8:
            case 0x00'C9:
            case 0x00'CA:
            case 0x00'CB:
            case 0x00'E8:
            case 0x00'E9:
            case 0x00'EA:
            case 0x00'EB:
            case 0x01'14:
            case 0x01'15:
            case 0x01'16:
            case 0x01'17:
            case 0x01'18:
            case 0x01'19:
                return "e";
            case 0x00'CC:
            case 0x00'CD:
            case 0x00'CE:
            case 0x00'CF:
            case 0x00'EC:
            case 0x00'ED:
            case 0x00'EE:
            case 0x00'EF:
            case 0x01'28:
            case 0x01'29:
            case 0x01'2A:
            case 0x01'2B:
            case 0x01'30:
            case 0x01'31:
                return "i";
            case 0x00'D1:
            case 0x00'F1:
            case 0x01'43:
            case 0x01'44:
            case 0x01'45:
            case 0x01'46:
                return "n";
            case 0x00'D2:
            case 0x00'D3:
            case 0x00'D4:
            case 0x00'D5:
            case 0x00'D6:
            case 0x00'D8:
            case 0x00'F2:
            case 0x00'F3:
            case 0x00'F4:
            case 0x00'F5:
            case 0x00'F6:
            case 0x00'F8:
            case 0x01'4C:
            case 0x01'4D:
            case 0x01'4E:
            case 0x01'4F:
            case 0x01'50:
            case 0x01'51:
                return "o";
            case 0x00'D9:
            case 0x00'DA:
            case 0x00'DB:
            case 0x00'DC:
            case 0x00'F9:
            case 0x00'FA:
            case 0x00'FB:
            case 0x00'FC:
            case 0x01'68:
            case 0x01'69:
            case 0x01'6A:
            case 0x01'6B:
            case 0x01'6C:
            case 0x01'6D:
                return "u";
            case 0x00'DD:
            case 0x00'FD:
            case 0x00'FF:
            case 0x01'78:
                return "y";
            case 0x00'DF:
                return "ss";
            case 0x00'C6:
            case 0x00'E6:
                return "ae";
            case 0x01'52:
            case 0x01'53:
                return "oe";
            default:
                return {};
        }
    }

    /** @brief Stand-in for @tryghost/security `string.safe` (generate-slug.js:75). */
    [[nodiscard]] static std::string security_safe(std::string_view text)
    {
        std::string out;
        bool gap = false;
        for (std::size_t i = 0; i < text.size();) {
            const auto [cp, len] = code_point(text, i);
            i += len;
            const std::string_view mapped = transliterate(cp);
            if (!mapped.empty()) {
                for (const char c: mapped) {
                    if (gap && !out.empty()) {
                        out.push_back('-');
                    }
                    out.push_back(c);
                    gap = false;
                }
                continue;
            }
            if (cp < 0x80) {
                const unsigned char c = static_cast<unsigned char>(cp);
                const bool alnum =
                    (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9');
                if (alnum) {
                    if (gap && !out.empty()) {
                        out.push_back('-');
                    }
                    out.push_back(static_cast<char>(c >= 'A' && c <= 'Z' ? c + 32 : c));
                    gap = false;
                } else {
                    gap = true;
                }
            } else {
                // Unmapped non-Latin code points: no unidecode primitive in this folder,
                // treat as a separator (recorded new-partition need).
                gap = true;
            }
        }
        return out;
    }

    /** @brief urlUtils.getProtectedSlugs() (overrides.json:18) + Tag.protectedSlugs. */
    [[nodiscard]] static bool is_protected(std::string_view slug, std::string_view type) noexcept
    {
        return slug == "ghost" || slug == "rss" || slug == "amp" ||
               (type == "tag" && slug == "new");
    }

    [[nodiscard]] static bool collision(
        const std::vector<blog::api::RecordDto>& rows,
        std::string_view slug,
        std::string_view model_id
    )
    {
        const std::string encoded_slug = core_serde::JsonCodec{}.encode(slug);
        const std::string encoded_id =
            model_id.empty() ? std::string{} : core_serde::JsonCodec{}.encode(model_id);
        for (const auto& row: rows) {
            const auto s = row.find("slug");
            if (s == row.end() || s->second != encoded_slug) {
                continue;
            }
            if (!encoded_id.empty()) {
                const auto id = row.find("id");
                if (id != row.end() && id->second == encoded_id) {
                    continue; // found.id === modelId
                }
            }
            return true;
        }
        return false;
    }
};

blog::BlogContext* ExecuteStep::s_ctx{nullptr};
} // namespace blog::handlers::slugs::get_detail::workflow
