export module blog_handlers:custom_theme_settings_browse_model;
import std;
export namespace blog::handlers::custom_theme_settings::browse::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
