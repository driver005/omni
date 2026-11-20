export module blog_handlers:exports_exports_download_get_step;
import :domain_all;
import std;
import core_workflow;
import core_serde;
import interfaces;

export namespace blog::handlers::exports::exports_download_get::workflow {

namespace download_detail {

using Object = serde::Value::Object;

[[nodiscard]] inline const serde::Value* value_for(const Object* object, std::string_view key)
{
    for (const auto& [name, value] : *object) if (name == key) return &value;
    return nullptr;
}

[[nodiscard]] inline std::string lower(std::string value)
{
    for (char& c : value) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return value;
}

[[nodiscard]] inline std::string trim(std::string_view value)
{
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string_view::npos) return {};
    const auto last = value.find_last_not_of(" \t\r\n");
    return std::string{value.substr(first, last - first + 1)};
}

[[nodiscard]] inline std::map<std::string, std::string> query(std::string_view path)
{
    std::map<std::string, std::string> out;
    const auto q = path.find('?');
    if (q == std::string_view::npos) return out;
    for (auto part : path.substr(q + 1) | std::views::split('&')) {
        const std::string kv{std::ranges::begin(part), std::ranges::end(part)};
        const auto eq = kv.find('=');
        if (eq != std::string::npos) out[kv.substr(0, eq)] = kv.substr(eq + 1);
    }
    return out;
}

[[nodiscard]] inline std::string slugify(std::string_view title)
{
    std::string out;
    bool gap = false;
    for (const char c : title) {
        if (std::isalnum(static_cast<unsigned char>(c))) {
            if (gap && !out.empty()) out += '-';
            out += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
            gap = false;
        } else {
            gap = true;
        }
    }
    return out;
}

/** @brief CSV-ish/JSON snapshot of one Ghost export component from the context store. */
[[nodiscard]] inline std::string snapshot(blog::BlogContext& ctx, std::string_view component)
{
    if (component == "members") return ctx.store().json_for("members", {});
    if (component == "content" || component == "analytics") return ctx.store().json_for("posts", {});
    return ctx.store().json_for("settings", {});
}

/** @brief Ghost file names for each component inside the archive. */
[[nodiscard]] inline std::vector<std::pair<std::string, std::string>> entries(
    blog::BlogContext& ctx, const std::vector<std::string>& selected)
{
    std::vector<std::pair<std::string, std::string>> out;
    for (const auto& component : selected) {
        if (component == "content") out.emplace_back("export.json", snapshot(ctx, component));
        else if (component == "members") out.emplace_back("members.csv", snapshot(ctx, component));
        else if (component == "analytics") out.emplace_back("post-analytics.csv", snapshot(ctx, component));
        else if (component == "themes") out.emplace_back("themes.zip", snapshot(ctx, component));
        else if (component == "routes") {
            out.emplace_back("routes.yaml", snapshot(ctx, component));
            out.emplace_back("redirects.yaml", snapshot(ctx, component));
        }
    }
    return out;
}

inline void put16(std::string& out, std::uint16_t value)
{
    out.push_back(static_cast<char>(value & 0xffu));
    out.push_back(static_cast<char>((value >> 8) & 0xffu));
}

inline void put32(std::string& out, std::uint32_t value)
{
    for (int i = 0; i < 4; ++i) out.push_back(static_cast<char>((value >> (8 * i)) & 0xffu));
}

[[nodiscard]] inline std::uint32_t crc32(std::string_view data)
{
    std::uint32_t crc = 0xffffffffu;
    for (const unsigned char byte : data) {
        crc ^= byte;
        for (int i = 0; i < 8; ++i)
            crc = (crc >> 1) ^ (0xedb88320u & (0u - (crc & 1u)));
    }
    return ~crc;
}

/** @brief Minimal stored (uncompressed) ZIP: local headers + central directory + EOCD. */
[[nodiscard]] inline std::string zip(const std::vector<std::pair<std::string, std::string>>& files)
{
    std::string local, central;
    std::uint32_t offset = 0;
    std::uint16_t count = 0;
    for (const auto& [name, data] : files) {
        const auto crc = crc32(data);
        const auto size = static_cast<std::uint32_t>(data.size());
        const auto name_len = static_cast<std::uint16_t>(name.size());

        put32(local, 0x04034b50u); // local file header
        put16(local, 20);          // version needed
        put16(local, 0);           // flags
        put16(local, 0);           // stored
        put16(local, 0);           // mod time
        put16(local, 0);           // mod date
        put32(local, crc);
        put32(local, size);
        put32(local, size);
        put16(local, name_len);
        put16(local, 0);           // extra length
        local += name;
        local += data;

        put32(central, 0x02014b50u); // central directory header
        put16(central, 20);          // version made by
        put16(central, 20);          // version needed
        put16(central, 0);           // flags
        put16(central, 0);           // stored
        put16(central, 0);           // mod time
        put16(central, 0);           // mod date
        put32(central, crc);
        put32(central, size);
        put32(central, size);
        put16(central, name_len);
        put16(central, 0);           // extra length
        put16(central, 0);           // comment length
        put16(central, 0);           // disk number start
        put16(central, 0);           // internal attributes
        put32(central, 0);           // external attributes
        put32(central, offset);      // local header offset
        central += name;

        offset += 30u + name_len + size;
        ++count;
    }

    std::string out = std::move(local);
    const auto central_offset = static_cast<std::uint32_t>(out.size());
    out += central;
    put32(out, 0x06054b50u); // end of central directory
    put16(out, 0);
    put16(out, 0);
    put16(out, count);
    put16(out, count);
    put32(out, static_cast<std::uint32_t>(central.size()));
    put32(out, central_offset);
    put16(out, 0);
    return out;
}

/** @brief `getExportFileName('export','zip')`: `<slug>.ghost.export.<date>.zip`. */
[[nodiscard]] inline std::string filename(blog::BlogContext& ctx)
{
    const auto today = std::chrono::year_month_day{
        std::chrono::floor<std::chrono::days>(std::chrono::system_clock::now())};
    const auto date = std::format("{:04d}-{:02d}-{:02d}",
        static_cast<int>(today.year()), static_cast<unsigned>(today.month()),
        static_cast<unsigned>(today.day()));

    std::string slug;
    const auto settings = ctx.store().json_for("settings", {});
    const auto parsed = core_serde::JsonCodec{}.decode<serde::Value>(settings);
    if (parsed && std::holds_alternative<serde::Value::Array>(parsed->get())) {
        for (const auto& record : std::get<serde::Value::Array>(parsed->get())) {
            if (!std::holds_alternative<Object>(record.get())) continue;
            const auto& object = std::get<Object>(record.get());
            const auto* key = value_for(&object, "key");
            if (key == nullptr || !std::holds_alternative<std::string>(key->get()) ||
                std::get<std::string>(key->get()) != "title") continue;
            const auto* value = value_for(&object, "value");
            if (value != nullptr && std::holds_alternative<std::string>(value->get()))
                slug = slugify(std::get<std::string>(value->get()));
            break;
        }
    }
    return (slug.empty() ? std::string{} : slug + ".") +
           "ghost.export." + date + ".zip";
}

} // namespace download_detail

class ExecuteStep {
public:
 using Input=interfaces::io::IRequest&; using Output=std::map<std::string,std::string>;
 static constexpr std::string_view name{"exports_exports_download_get_execute"};
 [[nodiscard]] core::workflow::StepResponse<Output> invoke(const Input& in, core::workflow::WorkflowContext&) const {
  auto& ctx=*s_ctx;
  using namespace download_detail;

  // Ghost `download.options=['components']`; absent -> all, explicitly empty ->
  // ValidationError('No export components selected'), unknown -> 422 AllowedValues.
  static constexpr std::array<std::string_view,5> sync_components{
   "members","analytics","content","themes","routes"};
  const auto p=query(in.get_path());
  std::vector<std::string> selected;
  if (const auto it=p.find("components"); it!=p.end()) {
   const auto raw=lower(trim(it->second));
   if (raw.empty()) return invalid("No export components selected");
   for (auto piece:raw|std::views::split(',')) {
    const std::string component{std::ranges::begin(piece),std::ranges::end(piece)};
    if (std::ranges::find(sync_components,std::string_view{component})==sync_components.end())
     return invalid("Validation (AllowedValues) failed for components");
    selected.push_back(component);
   }
  } else {
   for (auto component:sync_components) selected.emplace_back(component);
  }

  const auto archive=zip(entries(ctx,selected));
  return core::workflow::StepResponse<Output>{{{"status","200"},
   {"body",archive},{"filename",filename(ctx)}}};
 }
 void compensate(const core::workflow::WorkflowValue&, core::workflow::WorkflowContext&) const {}
 static blog::BlogContext* s_ctx;
private:
 static core::workflow::StepResponse<Output> invalid(std::string_view message) {
  return core::workflow::StepResponse<Output>{{{"status","422"},
   {"body",blog::api::ApiError{}.validation(message, "", "", "ValidationError")}}};
 }
};
blog::BlogContext* ExecuteStep::s_ctx{nullptr};
}
