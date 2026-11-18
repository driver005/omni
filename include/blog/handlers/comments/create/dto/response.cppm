export module blog_handlers:comments_create_dto;
import std;
export namespace blog::handlers::comments::create::dto {
struct Response { int status{200}; std::string body{"{}"}; };
}
