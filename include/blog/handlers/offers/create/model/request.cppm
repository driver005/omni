export module blog_handlers:offers_create_model;
import std;
export namespace blog::handlers::offers::create::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
