#pragma once

#include <cstdint>
#include <utility>
#include <vector>

struct Point {
    int64_t x;
    int64_t y;
};

bool operator==(const Point& lhs, const Point& rhs);
bool operator!=(const Point& lhs, const Point& rhs);

class DynamicHull {
public:
    DynamicHull();
    ~DynamicHull();

    void insert(const Point& point);
    void remove(const Point& point);
    std::vector<Point> hull() const;
    std::vector<Point> points() const;
    std::size_t size() const;
    bool empty() const;
    void clear();

private:
    struct Node {
        explicit Node(Point value)
            : point(std::move(value)), left(nullptr), right(nullptr), height(1) {}

        Point point;
        Node* left;
        Node* right;
        int height;
    };

    Node* root_;
    Point pivot_;
    std::vector<Point> points_;

    static int height(Node* node);
    static int balanceFactor(Node* node);
    static __int128 cross(const Point& a, const Point& b, const Point& c);
    static __int128 squaredDistance(const Point& a, const Point& b);

    Node* rotateRight(Node* node);
    Node* rotateLeft(Node* node);
    Node* insertNode(Node* node, const Point& point);
    Node* removeNode(Node* node, const Point& point, bool& removed);
    Node* findMin(Node* node) const;
    void updateHeight(Node* node);
    bool lessByPolar(const Point& lhs, const Point& rhs) const;
    void rebuildFrom(const std::vector<Point>& points);
    std::vector<Point> inorder(Node* node) const;
    void destroy(Node* node);
};
