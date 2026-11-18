export module blog_handlers:settings_bulk_edit_dto;
import std;
export namespace blog::handlers::settings::bulk_edit::dto {
struct Response { int status{200}; std::string body{"{}"}; };
}
