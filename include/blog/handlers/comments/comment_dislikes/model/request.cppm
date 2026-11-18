export module blog_handlers:comments_comment_dislikes_model;
import std;
export namespace blog::handlers::comments::comment_dislikes::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
