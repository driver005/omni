export module blog_handlers:emails_batches_get_dto;
import std;
export namespace blog::handlers::emails::batches_get::dto {
struct Response { int status{200}; std::string body{"{}"}; };
}
