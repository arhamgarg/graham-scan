#include <cassert>

#include "rbt.hpp"

int main() {
  assert(cross({0, 0}, {1, 0}, {0, 1}) > 0);
  assert((Point{2, 3} == Point{2, 3}));
}
