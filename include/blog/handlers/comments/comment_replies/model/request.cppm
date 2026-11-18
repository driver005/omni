export module blog_handlers:comments_comment_replies_model;
import std;
export namespace blog::handlers::comments::comment_replies::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
