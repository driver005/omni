export module blog_handlers:db_create_dto;
import std;
export namespace blog::handlers::db::create::dto {
struct Response { int status{200}; std::string body{"{}"}; };
}
