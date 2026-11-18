export module blog_handlers:users_destroy_dto;
import std;
export namespace blog::handlers::users::destroy::dto {
struct Response { int status{200}; std::string body{"{}"}; };
}
