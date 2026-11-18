export module blog_handlers:users_read_by_email_dto;
import std;
export namespace blog::handlers::users::read_by_email::dto {
struct Response { int status{200}; std::string body{"{}"}; };
}
