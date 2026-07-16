#pragma once

#include <cstddef>
#include <vector>

#include "geometry.hpp"

enum class Color { RED, BLACK };

namespace rbt {

struct Node {
  Point point;
  long long dx, dy;
  __int128 distance2;
  Color color;
  Node *left;
  Node *right;
  Node *parent;

  Node(Point p, long long dx_val, long long dy_val)
      : point(p), dx(dx_val), dy(dy_val),
        distance2(static_cast<__int128>(dx_val) * dx_val +
                  static_cast<__int128>(dy_val) * dy_val),
        color(Color::RED), left(nullptr), right(nullptr), parent(nullptr) {}
};

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
  Node *root_;
  Node *nil_;
  Point pivot_;
  std::size_t size_;

  void clear();
  void rebuild(std::vector<Point> points);
};

} // namespace rbt
