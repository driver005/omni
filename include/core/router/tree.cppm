export module core_router:tree;

import std;
import interfaces;
import :utils;
import :consts;
import :handler;
import :middleware;
import :node;

export namespace core::router {

/// @brief One registration to insert into the routing tree. Handlers/middlewares are
/// borrowed from the caller's route array and only read during build().
struct RouteInput
{
    std::string_view segment{};
    std::size_t parent_router{0};
    std::size_t router_number{0};
    const Handler<8>* handlers{nullptr};
    const Middleware<192>* middlewares{nullptr};
};

/// @brief One entry of the flattened, contiguous child-table representation consumed
/// by RouteHandler. `children_offset` is relative to this entry's own flat index.
struct FlatRoute
{
    EdgeKind kind{EdgeKind::PATH};
    std::string segment{};
    Handler<8> handlers{};
    Middleware<192> middlewares{};
    std::size_t children_offset{0};
    std::size_t children_length{0};
};

/**
 * @brief Canonical, hierarchical view of every registered route.
 *
 * Duplicate registrations of the same (parent, segment) are merged into one
 * RouteTreeNode with the union of their method handlers. flatten() then emits the
 * final flat vector in breadth-first order, where each node's children occupy a
 * contiguous slice directly after it — so offsets and child counts are exact by
 * construction instead of being inferred from registration rows.
 */
class RouteTree
{
public:
    void build(std::span<const RouteInput> routes)
    {
        m_root = std::make_unique<RouteTreeNode>();
        m_index = 0;
        m_merged = 0;

        // Router number 0 doubles as "root" for routes hanging straight off the base.
        m_by_number.clear();
        m_by_number[0] = m_root.get();

        static const Handler<8> empty_handlers{};
        static const Middleware<192> empty_middlewares{};

        for (const auto& input : routes) {
            RouteTreeNode* parent = m_root.get();
            if (input.parent_router != 0) {
                auto found = m_by_number.find(input.parent_router);
                if (found != m_by_number.end()) {
                    parent = found->second;
                }
                // An unknown parent is not fatal: keep the route reachable at the root
                // rather than silently dropping it from the table.
            }

            auto [node, created] = parent->child(input.segment);
            if (created) ++m_index;
            else ++m_merged;
            node.merge(
                input.handlers ? *input.handlers : empty_handlers,
                input.middlewares ? *input.middlewares : empty_middlewares
            );

            if (input.router_number != 0) {
                // A merged node can be referenced by several router numbers; map them all
                // to the same canonical node so every child lands in one place.
                m_by_number[input.router_number] = &node;
            }
        }
    }

    /**
     * @brief Produces the final flat route vector in breadth-first order.
     *
     * Children are assigned consecutive indices right after their parent, so a node at
     * index i with C children at indices [first, first+C) gets children_offset =
     * first - i and children_length = C. This is the single source of truth for the
     * offsets the matcher relies on.
     */
    std::vector<FlatRoute> flatten() const
    {
        std::vector<FlatRoute> out;
        if (!m_root) return out;

        std::vector<RouteTreeNode*> order;
        std::deque<RouteTreeNode*> pending;
        for (const auto& [key, child]: m_root->children()) pending.push_back(child.get());
        while (!pending.empty()) {
            auto* node = pending.front();
            pending.pop_front();
            order.push_back(node);
            for (const auto& [key, child]: node->children()) pending.push_back(child.get());
        }

        std::unordered_map<const RouteTreeNode*, std::size_t> indices;
        indices.reserve(order.size());
        // Flat index 0 is reserved for the root node; BFS nodes start at 1.
        for (std::size_t i = 0; i < order.size(); ++i) indices[order[i]] = i + 1;

        out.reserve(order.size() + 1);
        FlatRoute root_entry;
        root_entry.kind = EdgeKind::PATH;
        root_entry.children_offset = 1;
        root_entry.children_length = m_root->children().size();
        out.push_back(std::move(root_entry));

        for (std::size_t i = 0; i < order.size(); ++i) {
            const RouteTreeNode& node = *order[i];
            const std::size_t flat_index = i + 1;
            FlatRoute entry;
            entry.kind = node.kind();
            entry.segment = node.segment();
            entry.handlers = node.handlers();
            entry.middlewares = node.middlewares();
            const auto& children = node.children();
            entry.children_length = children.size();
            if (!children.empty()) {
                // BFS guarantees the children are the next `children_length` entries.
                entry.children_offset = indices.at(children.begin()->second.get()) - flat_index;
            }
            out.push_back(std::move(entry));
        }
        return out;
    }

    [[nodiscard]] std::size_t node_count() const noexcept { return m_index; }
    [[nodiscard]] std::size_t merged_count() const noexcept { return m_merged; }

private:
    std::unique_ptr<RouteTreeNode> m_root{};
    std::unordered_map<std::size_t, RouteTreeNode*> m_by_number{};
    std::size_t m_index{0};
    std::size_t m_merged{0};
};

} // namespace core::router
