export module blog_handlers:authors_browse_dto;
import std;
export namespace blog::handlers::authors::browse::dto {
struct Response { int status{200}; std::string body{"{}"}; };
}
