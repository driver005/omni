export module blog_handlers:schedules_put_detail_model;
import std;
export namespace blog::handlers::schedules::put_detail::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
