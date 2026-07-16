#pragma once

#include <vector>

struct Point {
  long long x, y;
  bool operator==(const Point &other) const {
    return x == other.x && y == other.y;
  }
};

inline int cross(Point a, Point b, Point c) {
  const __int128 value =
      (static_cast<__int128>(b.x) - a.x) * (static_cast<__int128>(c.y) - a.y) -
      (static_cast<__int128>(b.y) - a.y) * (static_cast<__int128>(c.x) - a.x);
  return (value > 0) - (value < 0);
}

inline std::vector<Point> scan_ordered_points(const std::vector<Point> &points,
                                              bool include_collinear) {
  std::vector<Point> hull;
  for (const Point point : points) {
    while (hull.size() > 1 &&
           (include_collinear
                ? cross(hull[hull.size() - 2], hull.back(), point) < 0
                : cross(hull[hull.size() - 2], hull.back(), point) <= 0))
      hull.pop_back();
    hull.push_back(point);
  }
  return hull;
}
