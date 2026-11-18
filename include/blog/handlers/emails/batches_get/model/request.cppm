export module blog_handlers:emails_batches_get_model;
import std;
export namespace blog::handlers::emails::batches_get::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
