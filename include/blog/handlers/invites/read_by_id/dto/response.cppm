export module blog_handlers:invites_read_by_id_dto;
import std;
export namespace blog::handlers::invites::read_by_id::dto {
struct Response { int status{200}; std::string body{"{}"}; };
}
