export module blog_handlers:session_bulk_destroy_dto;
import std;
export namespace blog::handlers::session::bulk_destroy::dto {
struct Response { int status{200}; std::string body{"{}"}; };
}
