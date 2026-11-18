export module blog_handlers:notifications_destroy_dto;
import std;
export namespace blog::handlers::notifications::destroy::dto {
struct Response { int status{200}; std::string body{"{}"}; };
}
