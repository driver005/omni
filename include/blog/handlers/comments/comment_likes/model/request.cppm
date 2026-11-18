export module blog_handlers:comments_comment_likes_model;
import std;
export namespace blog::handlers::comments::comment_likes::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
