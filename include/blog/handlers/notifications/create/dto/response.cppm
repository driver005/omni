export module blog_handlers:notifications_create_dto;
import std;
export namespace blog::handlers::notifications::create::dto {
struct Response { int status{200}; std::string body{"{}"}; };
}
