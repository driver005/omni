export module blog_handlers:pages_read_by_id_dto;
import std;
export namespace blog::handlers::pages::read_by_id::dto {
struct Response { int status{200}; std::string body{"{}"}; };
}
