export module blog_handlers:labels_read_by_id_dto;
import std;
export namespace blog::handlers::labels::read_by_id::dto {
struct Response { int status{200}; std::string body{"{}"}; };
}
