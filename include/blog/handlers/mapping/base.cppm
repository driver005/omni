// Module interface for the Ghost output-mapping layer.
//
// Public API (re-exported from the :clean and :wrap partitions):
//
//   export namespace blog::mapping {
//     [[nodiscard]] std::string clean(std::string_view json_object,
//                                     std::string_view entity);
//     [[nodiscard]] std::string wrap(std::string_view resource,
//                                    std::string_view rows_json,
//                                    std::string_view meta_json = {});
//     [[nodiscard]] std::string pagination(std::size_t total,
//                                          std::size_t limit,
//                                          std::size_t page,
//                                          std::size_t pages,
//                                          std::string_view prev,
//                                          std::string_view next);
//   }
//
// Wire into //include/blog/handlers:blog_handlers by adding these three
// partitions (in this order) to its `partitions` list and adding
// `export import :mapping;` to include/blog/handlers/base.cppm.
export module blog_handlers:mapping;

export import :clean;
export import :wrap;
