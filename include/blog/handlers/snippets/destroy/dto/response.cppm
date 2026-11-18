export module blog_handlers:snippets_destroy_dto;
import std;
export namespace blog::handlers::snippets::destroy::dto {
struct Response { int status{200}; std::string body{"{}"}; };
}
