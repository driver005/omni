export module blog_handlers:snippets_create_dto;
import std;
export namespace blog::handlers::snippets::create::dto {
struct Response { int status{200}; std::string body{"{}"}; };
}
