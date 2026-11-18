export module blog_handlers:members_bulk_destroy_dto;
import std;
export namespace blog::handlers::members::bulk_destroy::dto {
struct Response { int status{200}; std::string body{"{}"}; };
}
