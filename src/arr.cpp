#include "../include/arr.hpp"

#include <algorithm>

namespace {

bool compare_elements(const arr::Element &a, const arr::Element &b) {
  const bool a_upper = a.dy > 0 || (a.dy == 0 && a.dx >= 0);
  const bool b_upper = b.dy > 0 || (b.dy == 0 && b.dx >= 0);

  if (a_upper != b_upper)
    return a_upper > b_upper;

  const __int128 cross_prod = static_cast<__int128>(a.dx) * b.dy -
                              static_cast<__int128>(a.dy) * b.dx;
  if (cross_prod != 0)
    return cross_prod > 0;

  return a.distance2 < b.distance2;
}

} // namespace

namespace arr {

void DynamicHull::rebuild(std::vector<Point> points) {
  elements_.clear();
  size_ = 0;
  if (points.empty())
    return;

  const auto pivot =
      std::min_element(points.begin(), points.end(), [](Point a, Point b) {
        return std::tie(a.y, a.x) < std::tie(b.y, b.x);
      });
  std::iter_swap(points.begin(), pivot);
  for (const auto point : points) {
    insert(point);
  }
}

bool DynamicHull::insert(Point point) {
  if (size_ == 0) {
    pivot_ = point;
    size_ = 1;
    return true;
  }

  if (std::tie(point.y, point.x) < std::tie(pivot_.y, pivot_.x)) {
    std::vector<Point> all_points = ordered_points();
    all_points.push_back(point);
    rebuild(std::move(all_points));
    return true;
  }

  if (point == pivot_)
    return false;

  long long dx = point.x - pivot_.x;
  long long dy = point.y - pivot_.y;

  Element new_el(point, dx, dy);
  auto it = std::lower_bound(elements_.begin(), elements_.end(), new_el, compare_elements);

  if (it != elements_.end() && it->point == point) {
    return false;
  }

  elements_.insert(it, new_el);
  ++size_;
  return true;
}

std::vector<Point> DynamicHull::ordered_points() const {
  std::vector<Point> result;
  result.reserve(size_);

  if (size_ != 0) {
    result.push_back(pivot_);
  }

  for (const auto &el : elements_) {
    result.push_back(el.point);
  }

  return result;
}

bool DynamicHull::valid() const {
  if (size_ == 0)
    return elements_.empty();

  if (elements_.empty())
    return size_ == 1;

  if (elements_.size() + 1 != size_)
    return false;

  for (std::size_t i = 0; i < elements_.size(); ++i) {
    const auto &el = elements_[i];
    if (el.point == pivot_)
      return false;

    long long expected_dx = el.point.x - pivot_.x;
    long long expected_dy = el.point.y - pivot_.y;
    __int128 expected_dist2 = static_cast<__int128>(expected_dx) * expected_dx +
                              static_cast<__int128>(expected_dy) * expected_dy;

    if (el.dx != expected_dx || el.dy != expected_dy || el.distance2 != expected_dist2)
      return false;

    if (i > 0) {
      if (!compare_elements(elements_[i - 1], el))
        return false;
    }
  }

  return true;
}

std::vector<Point> DynamicHull::hull(bool include_collinear) const {
  return scan_ordered_points(ordered_points(), include_collinear);
}

bool DynamicHull::erase(Point point) {
  if (point == pivot_) {
    if (size_ == 1) {
      size_ = 0;
      return true;
    } else if (elements_.empty()) {
      return false;
    } else {
      auto points = ordered_points();
      points.erase(points.begin());
      rebuild(std::move(points));
      return true;
    }
  }

  long long dx = point.x - pivot_.x;
  long long dy = point.y - pivot_.y;

  Element target(point, dx, dy);
  auto it = std::lower_bound(elements_.begin(), elements_.end(), target, compare_elements);

  if (it != elements_.end() && it->point == point) {
    elements_.erase(it);
    --size_;
    return true;
  }

  return false;
}

std::size_t DynamicHull::size() const { return size_; }

} // namespace arr
