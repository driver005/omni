export module blog_handlers:comments_comment_likes_dto;
import std;
export namespace blog::handlers::comments::comment_likes::dto {
struct Response { int status{200}; std::string body{"{}"}; };
}
