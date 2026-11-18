export module blog_handlers:recommendations_read_by_id_dto;
import std;
export namespace blog::handlers::recommendations::read_by_id::dto {
struct Response { int status{200}; std::string body{"{}"}; };
}
