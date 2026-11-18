export module blog_handlers:custom_theme_settings_bulk_edit_model;
import std;
export namespace blog::handlers::custom_theme_settings::bulk_edit::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
