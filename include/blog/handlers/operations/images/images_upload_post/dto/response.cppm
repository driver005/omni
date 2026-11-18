export module blog_operations:images_images_upload_post_dto;

import std;

export namespace blog::handlers::operations::images::images_upload_post::dto {

struct Response
{
    int status{200};
    std::string body{"{}"};
};

} // namespace
