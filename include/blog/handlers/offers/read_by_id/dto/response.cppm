export module blog_handlers:offers_read_by_id_dto;
import std;
export namespace blog::handlers::offers::read_by_id::dto {
struct Response { int status{200}; std::string body{"{}"}; };
}
