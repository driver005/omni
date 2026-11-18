export module blog_handlers:comments_comment_replies_dto;
import std;
export namespace blog::handlers::comments::comment_replies::dto {
struct Response { int status{200}; std::string body{"{}"}; };
}
