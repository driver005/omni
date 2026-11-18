export module blog_handlers:files_files_upload_post_model;
import std;
export namespace blog::handlers::files::files_upload_post::model {
struct Request { std::string path{}; std::string body{}; std::string id{}; };
}
