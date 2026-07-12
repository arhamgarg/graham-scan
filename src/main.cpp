#include <cassert>
#include <cstring>

#include "rbt.hpp"

int main(int argc, char *argv[]) {
  assert(cross({0, 0}, {1, 0}, {0, 1}) > 0);
  assert((Point{2, 3} == Point{2, 3}));

  bool run_self_test = false;
  for (int i = 1; i < argc; ++i) {
    if (std::strcmp(argv[i], "--self-test") == 0) {
      run_self_test = true;
    }
  }

  if (run_self_test) {
    DynamicHull tree;
    assert(tree.insert({0, 0}));
    assert(tree.insert({2, 0}));
    assert(tree.insert({1, 1}));
    assert(!tree.insert({1, 1})); // duplicate
    assert(tree.size() == 3);
    assert(tree.valid());
    const auto points = tree.ordered_points();
    const std::vector<Point> expected{{0, 0}, {2, 0}, {1, 1}};
    assert(points == expected);
  }

  return 0;
}
