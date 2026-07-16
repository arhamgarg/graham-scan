#include "../include/rbt.hpp"

#include <functional>
#include <algorithm>

namespace {

bool compare_nodes(const rbt::Node *a, const rbt::Node *b) {
  const bool a_upper = a->dy > 0 || (a->dy == 0 && a->dx >= 0);
  const bool b_upper = b->dy > 0 || (b->dy == 0 && b->dx >= 0);

  if (a_upper != b_upper)
    return a_upper > b_upper;

  const __int128 cross_prod = static_cast<__int128>(a->dx) * b->dy -
                              static_cast<__int128>(a->dy) * b->dx;
  if (cross_prod != 0)
    return cross_prod > 0;

  return a->distance2 < b->distance2;
}

void rotate_left(rbt::Node *&root, rbt::Node *nil, rbt::Node *x) {
  rbt::Node *y = x->right;
  x->right = y->left;

  if (y->left != nil) {
    y->left->parent = x;
  }

  y->parent = x->parent;

  if (x->parent == nil) {
    root = y;
  } else if (x == x->parent->left) {
    x->parent->left = y;
  } else {
    x->parent->right = y;
  }

  y->left = x;
  x->parent = y;
}

void rotate_right(rbt::Node *&root, rbt::Node *nil, rbt::Node *x) {
  rbt::Node *y = x->left;
  x->left = y->right;

  if (y->right != nil) {
    y->right->parent = x;
  }

  y->parent = x->parent;

  if (x->parent == nil) {
    root = y;
  } else if (x == x->parent->right) {
    x->parent->right = y;
  } else {
    x->parent->left = y;
  }

  y->right = x;
  x->parent = y;
}

void fix_insert(rbt::Node *&root, rbt::Node *nil, rbt::Node *k) {
  while (k->parent->color == Color::RED) {
    if (k->parent == k->parent->parent->left) {
      rbt::Node *u = k->parent->parent->right;

      if (u->color == Color::RED) {
        k->parent->color = Color::BLACK;
        u->color = Color::BLACK;
        k->parent->parent->color = Color::RED;
        k = k->parent->parent;
      } else {
        if (k == k->parent->right) {
          k = k->parent;
          rotate_left(root, nil, k);
        }

        k->parent->color = Color::BLACK;
        k->parent->parent->color = Color::RED;
        rotate_right(root, nil, k->parent->parent);
      }
    } else {
      rbt::Node *u = k->parent->parent->left;

      if (u->color == Color::RED) {
        k->parent->color = Color::BLACK;
        u->color = Color::BLACK;
        k->parent->parent->color = Color::RED;
        k = k->parent->parent;
      } else {
        if (k == k->parent->left) {
          k = k->parent;
          rotate_right(root, nil, k);
        }

        k->parent->color = Color::BLACK;
        k->parent->parent->color = Color::RED;
        rotate_left(root, nil, k->parent->parent);
      }
    }
  }
  root->color = Color::BLACK;
}

bool validate_rbt(rbt::Node *node, rbt::Node *nil, int &black_height,
                  int current_black_count) {
  if (node == nil) {
    if (black_height == -1) {
      black_height = current_black_count;
    }
    return black_height == current_black_count;
  }

  if (node->color == Color::RED && node->parent->color == Color::RED) {
    return false;
  }

  int next_count = current_black_count;
  if (node->color == Color::BLACK) {
    ++next_count;
  }

  return validate_rbt(node->left, nil, black_height, next_count) &&
         validate_rbt(node->right, nil, black_height, next_count);
}

bool validate_sorted(rbt::Node *node, rbt::Node *nil, rbt::Node *&last) {
  if (node == nil)
    return true;

  if (!validate_sorted(node->left, nil, last))
    return false;

  if (last != nil && !compare_nodes(last, node)) {
    return false;
  }
  last = node;

  return validate_sorted(node->right, nil, last);
}

rbt::Node *find_minimum(rbt::Node *node, rbt::Node *nil) {
  while (node->left != nil) {
    node = node->left;
  }
  return node;
}

void transplant(rbt::Node *&root, rbt::Node *nil, rbt::Node *u, rbt::Node *v) {
  if (u->parent == nil) {
    root = v;
  } else if (u == u->parent->left) {
    u->parent->left = v;
  } else {
    u->parent->right = v;
  }
  v->parent = u->parent;
}

void fix_delete(rbt::Node *&root, rbt::Node *nil, rbt::Node *x) {
  while (x != root && x->color == Color::BLACK) {
    if (x == x->parent->left) {
      rbt::Node *s = x->parent->right;

      if (s->color == Color::RED) {
        s->color = Color::BLACK;
        x->parent->color = Color::RED;
        rotate_left(root, nil, x->parent);
        s = x->parent->right;
      }

      if (s->left->color == Color::BLACK && s->right->color == Color::BLACK) {
        s->color = Color::RED;
        x = x->parent;
      } else {
        if (s->right->color == Color::BLACK) {
          s->left->color = Color::BLACK;
          s->color = Color::RED;
          rotate_right(root, nil, s);
          s = x->parent->right;
        }

        s->color = x->parent->color;
        x->parent->color = Color::BLACK;
        s->right->color = Color::BLACK;
        rotate_left(root, nil, x->parent);
        x = root;
      }
    } else {
      rbt::Node *s = x->parent->left;

      if (s->color == Color::RED) {
        s->color = Color::BLACK;
        x->parent->color = Color::RED;
        rotate_right(root, nil, x->parent);
        s = x->parent->left;
      }

      if (s->right->color == Color::BLACK && s->left->color == Color::BLACK) {
        s->color = Color::RED;
        x = x->parent;
      } else {
        if (s->left->color == Color::BLACK) {
          s->right->color = Color::BLACK;
          s->color = Color::RED;
          rotate_left(root, nil, s);
          s = x->parent->left;
        }

        s->color = x->parent->color;
        x->parent->color = Color::BLACK;
        s->left->color = Color::BLACK;
        rotate_right(root, nil, x->parent);
        x = root;
      }
    }
  }
  x->color = Color::BLACK;
}

} // namespace

namespace rbt {

void DynamicHull::clear() {
  std::function<void(Node *)> delete_tree = [&](Node *node) {
    if (node == nil_)
      return;
    delete_tree(node->left);
    delete_tree(node->right);
    delete node;
  };
  delete_tree(root_);
  root_ = nullptr;
  size_ = 0;
}

void DynamicHull::rebuild(std::vector<Point> points) {
  clear();
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

DynamicHull::DynamicHull()
    : root_(nullptr), nil_(new Node({0, 0}, 0, 0)), pivot_({0, 0}),
      size_(0) {
  nil_->color = Color::BLACK;
  nil_->left = nil_->right = nil_->parent = nil_;
}

DynamicHull::~DynamicHull() {
  clear();
  delete nil_;
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

  Node *new_node = new Node(point, dx, dy);
  new_node->left = nil_;
  new_node->right = nil_;

  if (root_ == nullptr) {
    root_ = new_node;
    new_node->parent = nil_;
    new_node->color = Color::RED;
    size_ = 2;
    fix_insert(root_, nil_, new_node);
    return true;
  }

  Node *parent = nil_;
  Node *current = root_;

  while (current != nil_) {
    parent = current;

    if (new_node->point == current->point) {
      delete new_node;
      return false;
    }

    if (compare_nodes(new_node, current)) {
      current = current->left;
    } else {
      current = current->right;
    }
  }

  new_node->parent = parent;

  if (compare_nodes(new_node, parent)) {
    parent->left = new_node;
  } else {
    parent->right = new_node;
  }

  ++size_;
  fix_insert(root_, nil_, new_node);
  return true;
}

std::vector<Point> DynamicHull::ordered_points() const {
  std::vector<Point> result;

  if (size_ != 0) {
    result.push_back(pivot_);
  }

  std::function<void(Node *)> inorder = [&](Node *node) {
    if (node == nil_)
      return;
    inorder(node->left);
    result.push_back(node->point);
    inorder(node->right);
  };

  inorder(root_);
  return result;
}

bool DynamicHull::valid() const {
  if (size_ == 0)
    return root_ == nullptr;

  if (root_ == nil_)
    return size_ == 1;

  if (root_->color != Color::BLACK)
    return false;

  int black_height = -1;
  if (!validate_rbt(root_, const_cast<Node *>(nil_), black_height, 0))
    return false;

  Node *last = const_cast<Node *>(nil_);
  return validate_sorted(root_, const_cast<Node *>(nil_), last);
}

std::vector<Point> DynamicHull::hull(bool include_collinear) const {
  return scan_ordered_points(ordered_points(), include_collinear);
}

bool DynamicHull::erase(Point point) {
  if (point == pivot_) {
    if (size_ == 1) {
      size_ = 0;
      return true;
    } else if (root_ == nullptr) {
      return false;
    } else {
      auto points = ordered_points();
      points.erase(points.begin());
      rebuild(std::move(points));
      return true;
    }
  }

  const Node search(point, point.x - pivot_.x, point.y - pivot_.y);
  Node *current = root_;
  Node *to_delete = nullptr;
  while (current != nil_) {
    if (current->point == point) {
      to_delete = current;
      break;
    }

    if (compare_nodes(&search, current)) {
      current = current->left;
    } else {
      current = current->right;
    }
  }

  if (to_delete == nullptr) {
    return false;
  }

  Node *node_to_fix;
  Color removed_color;

  if (to_delete->left == nil_) {
    node_to_fix = to_delete->right;
    removed_color = to_delete->color;
    transplant(root_, nil_, to_delete, to_delete->right);
  } else if (to_delete->right == nil_) {
    node_to_fix = to_delete->left;
    removed_color = to_delete->color;
    transplant(root_, nil_, to_delete, to_delete->left);
  } else {
    Node *successor = find_minimum(to_delete->right, nil_);
    removed_color = successor->color;
    node_to_fix = successor->right;

    if (successor->parent == to_delete) {
      node_to_fix->parent = successor;
    } else {
      transplant(root_, nil_, successor, successor->right);
      successor->right = to_delete->right;
      successor->right->parent = successor;
    }

    transplant(root_, nil_, to_delete, successor);
    successor->left = to_delete->left;
    successor->left->parent = successor;
    successor->color = to_delete->color;
  }

  delete to_delete;
  --size_;

  if (removed_color == Color::BLACK) {
    fix_delete(root_, nil_, node_to_fix);
  }

  return true;
}

std::size_t DynamicHull::size() const { return size_; }

} // namespace rbt
