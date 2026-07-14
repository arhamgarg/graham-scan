#include "../include/avl.hpp"

#include <algorithm>

namespace {
struct PivotLess {
    bool operator()(const Point& lhs, const Point& rhs) const {
        return std::tie(lhs.y, lhs.x) < std::tie(rhs.y, rhs.x);
    }
};
}  // namespace

bool operator==(const Point& lhs, const Point& rhs) {
    return lhs.x == rhs.x && lhs.y == rhs.y;
}

bool operator!=(const Point& lhs, const Point& rhs) {
    return !(lhs == rhs);
}

DynamicHull::DynamicHull() : root_(nullptr), pivot_{0, 0} {}

DynamicHull::~DynamicHull() {
    destroy(root_);
}

void DynamicHull::clear() {
    destroy(root_);
    root_ = nullptr;
    points_.clear();
    pivot_ = {0, 0};
}

std::vector<Point> DynamicHull::points() const {
    return points_;
}

std::size_t DynamicHull::size() const {
    return points_.size();
}

bool DynamicHull::empty() const {
    return points_.empty();
}

int DynamicHull::height(Node* node) {
    return node ? node->height : 0;
}

int DynamicHull::balanceFactor(Node* node) {
    return node ? height(node->left) - height(node->right) : 0;
}

__int128 DynamicHull::cross(const Point& a, const Point& b, const Point& c) {
    return (__int128)(b.x - a.x) * (c.y - a.y) - (__int128)(b.y - a.y) * (c.x - a.x);
}

__int128 DynamicHull::squaredDistance(const Point& a, const Point& b) {
    const auto dx = a.x - b.x;
    const auto dy = a.y - b.y;
    return (__int128)dx * dx + (__int128)dy * dy;
}

void DynamicHull::updateHeight(Node* node) {
    if (!node) {
        return;
    }
    node->height = 1 + std::max(height(node->left), height(node->right));
}

DynamicHull::Node* DynamicHull::rotateRight(Node* node) {
    Node* left = node->left;
    Node* leftRight = left->right;

    left->right = node;
    node->left = leftRight;

    updateHeight(node);
    updateHeight(left);
    return left;
}

DynamicHull::Node* DynamicHull::rotateLeft(Node* node) {
    Node* right = node->right;
    Node* rightLeft = right->left;

    right->left = node;
    node->right = rightLeft;

    updateHeight(node);
    updateHeight(right);
    return right;
}

bool DynamicHull::lessByPolar(const Point& lhs, const Point& rhs) const {
    if (lhs == rhs) {
        return false;
    }

    const Point leftRelative{lhs.x - pivot_.x, lhs.y - pivot_.y};
    const Point rightRelative{rhs.x - pivot_.x, rhs.y - pivot_.y};

    const bool leftUpper = leftRelative.y > 0 || (leftRelative.y == 0 && leftRelative.x > 0);
    const bool rightUpper = rightRelative.y > 0 || (rightRelative.y == 0 && rightRelative.x > 0);

    if (leftUpper != rightUpper) {
        return leftUpper;
    }

    const auto orientation = cross({pivot_.x, pivot_.y}, lhs, rhs);
    if (orientation != 0) {
        return orientation > 0;
    }

    return squaredDistance(pivot_, lhs) < squaredDistance(pivot_, rhs);
}

DynamicHull::Node* DynamicHull::insertNode(Node* node, const Point& point) {
    if (!node) {
        return new Node(point);
    }

    if (lessByPolar(point, node->point)) {
        node->left = insertNode(node->left, point);
    } else if (lessByPolar(node->point, point)) {
        node->right = insertNode(node->right, point);
    } else {
        return node;
    }

    updateHeight(node);
    const int balance = balanceFactor(node);

    if (balance > 1 && lessByPolar(point, node->left->point)) {
        return rotateRight(node);
    }
    if (balance < -1 && lessByPolar(node->right->point, point)) {
        return rotateLeft(node);
    }
    if (balance > 1) {
        node->left = rotateLeft(node->left);
        return rotateRight(node);
    }
    if (balance < -1) {
        node->right = rotateRight(node->right);
        return rotateLeft(node);
    }

    return node;
}

DynamicHull::Node* DynamicHull::removeNode(Node* node, const Point& point, bool& removed) {
    if (!node) {
        return nullptr;
    }

    if (lessByPolar(point, node->point)) {
        node->left = removeNode(node->left, point, removed);
    } else if (lessByPolar(node->point, point)) {
        node->right = removeNode(node->right, point, removed);
    } else {
        removed = true;
        if (!node->left || !node->right) {
            Node* replacement = node->left ? node->left : node->right;
            delete node;
            return replacement;
        }

        Node* successor = findMin(node->right);
        node->point = successor->point;
        node->right = removeNode(node->right, successor->point, removed);
    }

    if (!removed) {
        return node;
    }

    updateHeight(node);
    const int balance = balanceFactor(node);

    if (balance > 1) {
        if (balanceFactor(node->left) >= 0) {
            return rotateRight(node);
        }
        node->left = rotateLeft(node->left);
        return rotateRight(node);
    }
    if (balance < -1) {
        if (balanceFactor(node->right) <= 0) {
            return rotateLeft(node);
        }
        node->right = rotateRight(node->right);
        return rotateLeft(node);
    }

    return node;
}

DynamicHull::Node* DynamicHull::findMin(Node* node) const {
    while (node && node->left) {
        node = node->left;
    }
    return node;
}

void DynamicHull::rebuildFrom(const std::vector<Point>& points) {
    destroy(root_);
    root_ = nullptr;

    if (points.empty()) {
        pivot_ = {0, 0};
        return;
    }

    pivot_ = *std::min_element(points.begin(), points.end(), PivotLess{});
    for (const Point& point : points) {
        if (point == pivot_) {
            continue;
        }
        root_ = insertNode(root_, point);
    }
}

std::vector<Point> DynamicHull::inorder(Node* node) const {
    std::vector<Point> values;
    if (!node) {
        return values;
    }

    auto left = inorder(node->left);
    values.insert(values.end(), left.begin(), left.end());
    values.push_back(node->point);
    auto right = inorder(node->right);
    values.insert(values.end(), right.begin(), right.end());
    return values;
}

void DynamicHull::destroy(Node* node) {
    if (!node) {
        return;
    }
    destroy(node->left);
    destroy(node->right);
    delete node;
}

void DynamicHull::insert(const Point& point) {
    if (points_.empty()) {
        points_.push_back(point);
        pivot_ = point;
        root_ = nullptr;
        return;
    }

    if (point == pivot_) {
        return;
    }

    if (std::find(points_.begin(), points_.end(), point) != points_.end()) {
        return;
    }

    points_.push_back(point);
    root_ = insertNode(root_, point);
}

void DynamicHull::remove(const Point& point) {
    if (points_.empty()) {
        return;
    }

    if (point == pivot_) {
        auto it = std::find(points_.begin(), points_.end(), point);
        if (it == points_.end()) {
            return;
        }

        points_.erase(it);
        if (points_.empty()) {
            clear();
            return;
        }

        rebuildFrom(points_);
        return;
    }

    auto it = std::find(points_.begin(), points_.end(), point);
    if (it == points_.end()) {
        return;
    }

    points_.erase(it);
    bool removed = false;
    root_ = removeNode(root_, point, removed);
}

std::vector<Point> DynamicHull::hull() const {
    std::vector<Point> ordered;
    ordered.push_back(pivot_);

    const auto values = inorder(root_);
    ordered.insert(ordered.end(), values.begin(), values.end());

    if (ordered.size() < 3) {
        return ordered;
    }

    std::vector<Point> stack;
    stack.push_back(ordered.front());
    for (std::size_t i = 1; i < ordered.size(); ++i) {
        const Point& candidate = ordered[i];
        while (stack.size() >= 2 && cross(stack[stack.size() - 2], stack.back(), candidate) <= 0) {
            stack.pop_back();
        }
        stack.push_back(candidate);
    }
    return stack;
}
