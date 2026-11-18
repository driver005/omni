export module blog_handlers:comments_create_model;
import std;
export namespace blog::handlers::comments::create::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
