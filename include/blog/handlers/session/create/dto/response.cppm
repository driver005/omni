export module blog_handlers:session_create_dto;
import std;
export namespace blog::handlers::session::create::dto {
struct Response { int status{200}; std::string body{"{}"}; };
}
