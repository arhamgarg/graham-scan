#include "rbt.hpp"

#include <cassert>
#include <cstdlib>
#include <functional>
#include <vector>

int cross(Point a, Point b, Point c) {
  const __int128 value =
      (static_cast<__int128>(b.x) - a.x) * (static_cast<__int128>(c.y) - a.y) -
      (static_cast<__int128>(b.y) - a.y) * (static_cast<__int128>(c.x) - a.x);
  return (value > 0) - (value < 0);
}

namespace {

// Compare two nodes for ordering. Returns true if a < b in Graham-scan order.
bool compare_nodes(const Node *a, const Node *b) {
  // Upper-half plane test
  const bool a_upper = a->dy > 0 || (a->dy == 0 && a->dx >= 0);
  const bool b_upper = b->dy > 0 || (b->dy == 0 && b->dx >= 0);

  if (a_upper != b_upper)
    return a_upper > b_upper; // upper half comes first

  // Cross product for angular ordering
  const __int128 cross_prod = static_cast<__int128>(a->dx) * b->dy -
                              static_cast<__int128>(a->dy) * b->dx;
  if (cross_prod != 0)
    return cross_prod > 0;

  // Distance squared (closer first)
  return a->distance2 < b->distance2;
}

// Check if two nodes are equal (duplicates)
bool nodes_equal(const Node *a, const Node *b) { return a->point == b->point; }

void rotate_left(Node *&root, Node *nil, Node *x) {
  Node *y = x->right;
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

void rotate_right(Node *&root, Node *nil, Node *x) {
  Node *y = x->left;
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

void fix_insert(Node *&root, Node *nil, Node *k) {
  while (k->parent->color == Color::RED) {
    if (k->parent == k->parent->parent->left) {
      Node *u = k->parent->parent->right;

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
      Node *u = k->parent->parent->left;

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

bool validate_rbt(Node *node, Node *nil, int &black_height,
                  int current_black_count) {
  if (node == nil) {
    if (black_height == -1) {
      black_height = current_black_count;
    }
    return black_height == current_black_count;
  }

  // Red node cannot have red parent
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

bool validate_sorted(Node *node, Node *nil, Node *&last) {
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

} // namespace

DynamicHull::DynamicHull()
    : root_(nullptr), nil_(new Node({0, 0}, 0, 0)), pivot_({0, 0}),
      has_pivot_(false), size_(0) {
  nil_->color = Color::BLACK;
  nil_->left = nil_->right = nil_->parent = nil_;
}

DynamicHull::~DynamicHull() {
  std::function<void(Node *)> delete_tree = [&](Node *node) {
    if (node == nil_)
      return;
    delete_tree(node->left);
    delete_tree(node->right);
    delete node;
  };
  delete_tree(root_);
  delete nil_;
}

bool DynamicHull::insert(Point point) {
  if (!has_pivot_) {
    pivot_ = point;
    has_pivot_ = true;
    size_ = 1;
    return true;
  }

  // Check for duplicate
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

    if (nodes_equal(new_node, current)) {
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

  if (has_pivot_) {
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
  if (!has_pivot_)
    return root_ == nullptr && size_ == 0;

  if (root_ == nil_)
    return size_ == 1;

  // Root must be black
  if (root_->color != Color::BLACK)
    return false;

  // Check red-black properties
  int black_height = -1;
  if (!validate_rbt(root_, const_cast<Node *>(nil_), black_height, 0))
    return false;

  // Check sorted order
  Node *last = const_cast<Node *>(nil_);
  return validate_sorted(root_, const_cast<Node *>(nil_), last);
}

std::vector<Point> DynamicHull::hull(bool include_collinear) const {
  (void)include_collinear; // unused in Task 2
  return {};
}

bool DynamicHull::erase(Point point) {
  (void)point; // unused in Task 2
  return false;
}

std::size_t DynamicHull::size() const { return size_; }
