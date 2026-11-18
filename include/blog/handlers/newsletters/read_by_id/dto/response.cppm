export module blog_handlers:newsletters_read_by_id_dto;
import std;
export namespace blog::handlers::newsletters::read_by_id::dto {
struct Response { int status{200}; std::string body{"{}"}; };
}
