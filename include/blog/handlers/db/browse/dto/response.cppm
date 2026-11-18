export module blog_handlers:db_browse_dto;
import std;
export namespace blog::handlers::db::browse::dto {
struct Response { int status{200}; std::string body{"{}"}; };
}
