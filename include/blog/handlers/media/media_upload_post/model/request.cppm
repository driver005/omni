export module blog_handlers:media_media_upload_post_model;
import std;
export namespace blog::handlers::media::media_upload_post::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
