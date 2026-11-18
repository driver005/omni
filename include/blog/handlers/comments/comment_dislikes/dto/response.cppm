export module blog_handlers:comments_comment_dislikes_dto;
import std;
export namespace blog::handlers::comments::comment_dislikes::dto {
struct Response { int status{200}; std::string body{"{}"}; };
}
