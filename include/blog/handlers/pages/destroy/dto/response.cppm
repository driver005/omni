export module blog_handlers:pages_destroy_dto;
import std;
export namespace blog::handlers::pages::destroy::dto {
struct Response { int status{200}; std::string body{"{}"}; };
}
