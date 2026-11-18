export module blog_handlers:settings_bulk_edit_model;
import std;
export namespace blog::handlers::settings::bulk_edit::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
