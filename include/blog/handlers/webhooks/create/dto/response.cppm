export module blog_handlers:webhooks_create_dto;
import std;
export namespace blog::handlers::webhooks::create::dto {
struct Response { int status{200}; std::string body{"{}"}; };
}
