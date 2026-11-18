export module blog_handlers:snippets_browse_dto;
import std;
export namespace blog::handlers::snippets::browse::dto {
struct Response { int status{200}; std::string body{"{}"}; };
}
