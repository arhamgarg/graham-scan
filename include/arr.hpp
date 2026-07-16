#pragma once

#include <cstddef>
#include <vector>

#ifndef POINT_DEFINED
#define POINT_DEFINED
struct Point {
  long long x, y;
  bool operator==(const Point &other) const {
    return x == other.x && y == other.y;
  }
};
#endif

int cross(Point a, Point b, Point c);

namespace arr {

struct Element {
  Point point;
  long long dx, dy;
  __int128 distance2;

  Element(Point p, long long dx_val, long long dy_val)
      : point(p), dx(dx_val), dy(dy_val),
        distance2(static_cast<__int128>(dx_val) * dx_val +
                  static_cast<__int128>(dy_val) * dy_val) {}
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
  std::vector<Element> elements_;
  Point pivot_;
  bool has_pivot_;
  std::size_t size_;

  void clear();
  void rebuild(std::vector<Point> points);
};

} // namespace arr
