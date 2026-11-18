export module blog_handlers:feedback_read_by_id_dto;
import std;
export namespace blog::handlers::feedback::read_by_id::dto {
struct Response { int status{200}; std::string body{"{}"}; };
}
