// Deterministic OpenAPI spec dump. Installs the Hyperswitch route set into a router
// context so ApiRoute/OpenAPI metadata is populated, then serializes the resulting document
// without needing a running server or a wire-format plugin.
#include <nlohmann/json.hpp>

#include <cstdio>

import std;

import app_handlers;
import core_router;
import core_serde;
import openapi_generator_plugin;

namespace {

// The C++ OpenAPI model emits empty strings and empty containers for unset optional fields.
// Strict generators (progenitor/openapiv3) reject those, so drop them and give every operation
// a default 200 response.
class SanitizeSpec
{
public:
    void operator()(nlohmann::json& node) const
    {
        if (node.is_array()) {
            for (auto& element: node) {
                (*this)(element);
            }
            return;
        }
        if (!node.is_object()) {
            return;
        }
        for (auto entry = node.begin(); entry != node.end();) {
            const auto& key = entry.key();
            auto& value = entry.value();
            if (key == "responses" && value.is_object() && value.empty()) {
                value = nlohmann::json{{"200", {{"description", "OK"}}}};
                ++entry;
                continue;
            }
            if (is_empty(value)) {
                entry = node.erase(entry);
                continue;
            }
            (*this)(value);
            ++entry;
        }
    }

private:
    [[nodiscard]] static bool is_empty(const nlohmann::json& value)
    {
        if (value.is_string()) {
            return value.get_ref<const std::string&>().empty();
        }
        return (value.is_array() || value.is_object()) && value.empty();
    }
};

} // namespace

int main(int argc, char** argv)
{
    const std::filesystem::path output_path =
        argc > 1 ? std::filesystem::path{argv[1]} : std::filesystem::path{"openapi.json"};

    app::api::AppHandlers handlers;
    core::router::RouterContext<> context;
    app::api::AppRoutes{}.install(context, handlers);

    const auto generator = utils::openapi::Generator{}
                               .title("Congelado API")
                               .version("1.0.0")
                               .serve_path("openapi");
    auto spec = nlohmann::json::parse(serde::Ser::encode_json(generator.generate()));
    SanitizeSpec{}(spec);

    const auto json = spec.dump();

    std::ofstream out{output_path};
    if (!out) {
        std::println(stderr, "openapi_dump: failed to open {}", output_path.string());
        return 1;
    }
    out << json;
    if (!out) {
        std::println(stderr, "openapi_dump: failed to write {}", output_path.string());
        return 1;
    }
    std::println("openapi_dump: wrote {} bytes to {}", json.size(), output_path.string());
    return 0;
}
