export module blog_handlers:session_session_verify_post_dto;
import std;
export namespace blog::handlers::session::session_verify_post::dto {
struct Response { int status{200}; std::string body{"{}"}; };
}
