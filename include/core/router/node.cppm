export module core_router:node;

import std;
import interfaces;
import :utils;
import :consts;
import :handler;
import :middleware;

export namespace core::router {

/**
 * @brief One logical node in the routing tree.
 *
 * Unlike the old flat-table build, children are keyed by (edge kind, literal), so
 * registering the same scope twice ("v2", "payments", ...) merges into a single node
 * instead of producing duplicate branches that the matcher has to guess between.
 * Method handlers are merged into the same node too, so a path split across several
 * Route{"x"}.get(...) / .post(...) registrations ends up with one dispatch node.
 */
class RouteTreeNode
{
public:
    using Key = std::pair<EdgeKind, std::string>;

    RouteTreeNode() = default;

    explicit RouteTreeNode(EdgeKind kind, std::string segment) :
        m_kind{kind},
        m_segment{std::move(segment)}
    {
    }

    /// @brief Classify a raw path segment into an edge kind and the stored literal.
    static std::pair<EdgeKind, std::string> classify(std::string_view segment)
    {
        if (segment.empty()) {
            return {EdgeKind::PATH, std::string{}};
        }
        if (segment == "*") {
            return {EdgeKind::WILD, std::string{}};
        }
        if (segment.front() == ':') {
            return {EdgeKind::PARAM, std::string{}};
        }
        return {EdgeKind::PATH, std::string{segment}};
    }

    /**
     * @brief Gets the child for `segment`, creating it if it doesn't exist yet.
     * @return reference to the canonical child node, and true when a brand-new node
     * had to be created (false when an existing duplicate was reused).
     */
    std::pair<RouteTreeNode&, bool> child(std::string_view segment)
    {
        auto [kind, literal] = classify(segment);
        Key key{kind, literal};
        auto it = m_children.find(key);
        if (it != m_children.end()) {
            return {*it->second, false};
        }
        auto node = std::make_unique<RouteTreeNode>(kind, std::move(literal));
        auto* raw = node.get();
        m_children.emplace(std::move(key), std::move(node));
        return {*raw, true};
    }

    /// @brief Merges another registration of the same path into this node.
    void merge(const Handler<8>& handlers, const Middleware<192>& middlewares)
    {
        for (std::size_t method = 0; method < 8; ++method) {
            const auto fn = handlers.find(static_cast<interfaces::io::types::Method>(method));
            if (fn && !m_handlers.find(static_cast<interfaces::io::types::Method>(method))) {
                m_handlers.add_handler(static_cast<interfaces::io::types::Method>(method), fn);
            }
        }
        for (std::uint8_t i = 0; i < middlewares.get_size(); ++i) {
            m_middlewares.add_middleware(middlewares.get_middlewares()[i]);
        }
    }

    [[nodiscard]] EdgeKind kind() const noexcept
    {
        return m_kind;
    }

    [[nodiscard]] const std::string& segment() const noexcept
    {
        return m_segment;
    }

    [[nodiscard]] const Handler<8>& handlers() const noexcept
    {
        return m_handlers;
    }

    [[nodiscard]] const Middleware<192>& middlewares() const noexcept
    {
        return m_middlewares;
    }

    [[nodiscard]] const std::map<Key, std::unique_ptr<RouteTreeNode>>& children() const noexcept
    {
        return m_children;
    }

private:
    EdgeKind m_kind{EdgeKind::PATH};
    std::string m_segment{};
    Handler<8> m_handlers{};
    Middleware<192> m_middlewares{};
    std::map<Key, std::unique_ptr<RouteTreeNode>> m_children{};
};

} // namespace core::router
