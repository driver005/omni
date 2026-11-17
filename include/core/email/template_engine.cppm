export module core_email:template_engine;

import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

export namespace core::email {

/** @brief Minimal {{key}} template renderer for email bodies. */
class TemplateEngine
{
public:
    [[nodiscard]] std::string render(
        std::string_view source,
        const std::map<std::string, std::string>& values
    ) const
    {
        std::string out;
        out.reserve(source.size());
        std::size_t position = 0;
        while (position < source.size()) {
            const auto open = source.find("{{", position);
            if (open == std::string_view::npos) {
                out += source.substr(position);
                break;
            }
            out += source.substr(position, open - position);
            const auto close = source.find("}}", open + 2);
            if (close == std::string_view::npos) {
                out += source.substr(open);
                break;
            }
            const std::string key{std::string{source.substr(open + 2, close - open - 2)}};
            const auto it = values.find(key);
            out += it == values.end() ? "" : it->second;
            position = close + 2;
        }
        return out;
    }

private:
};

} // namespace core::email

#ifdef CONGELADO_TEST
namespace core::email::tests {
using namespace boost::ut;

suite<"core_email_template_engine"> core_email_template_engine_suite = []
{
    "substitutes placeholders"_test = []
    {
        const TemplateEngine engine;
        const std::map<std::string, std::string> values{{"name", "Ada"}, {"missing", "x"}};
        expect(engine.render("Hello {{name}}!", values) == "Hello Ada!");
        expect(engine.render("{{missing}}", values) == "x");
        expect(engine.render("no placeholders", {}) == "no placeholders");
    };
};

} // namespace core::email::tests
#endif
