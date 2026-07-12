#pragma once

#include <cstddef>
#include <vector>

struct Point {
  long long x, y;
  bool operator==(const Point &other) const {
    return x == other.x && y == other.y;
  }
};

int cross(Point a, Point b, Point c);

class DynamicHull {
public:
  DynamicHull();
  ~DynamicHull();
  DynamicHull(const DynamicHull &) = delete;
  DynamicHull &operator=(const DynamicHull &) = delete;

  bool insert(Point point);
  bool erase(Point point);
  std::vector<Point> ordered_points() const;
  std::vector<Point> hull(bool include_collinear = false) const;
  bool valid() const;
  std::size_t size() const;

private:
  struct Node;
  Node *root_;
  Node *nil_;
  Point pivot_;
  bool has_pivot_;
  std::size_t size_;
};
