export module blog_handlers:webhooks_destroy_dto;
import std;
export namespace blog::handlers::webhooks::destroy::dto {
struct Response { int status{200}; std::string body{"{}"}; };
}
