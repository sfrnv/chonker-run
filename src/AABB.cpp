#include <cmath>
#include <exception>
#include <iostream>

#include "AABB.hpp"

namespace aabb {

AABB::AABB(int x1, int y1, int x2, int y2) : x1(x1), y1(y1), x2(x2), y2(y2){};

AABB AABB::unite(const AABB &aabb) const {
  return AABB(std::min(x1, aabb.x1), std::min(y1, aabb.y1),
              std::max(x2, aabb.x2), std::max(y2, aabb.y2));
};

bool AABB::contains(const AABB &aabb) const {
  return x1 <= aabb.x1 && y1 <= aabb.y1 && x2 >= aabb.x2 && y2 >= aabb.y2;
}

bool AABB::overlaps(const AABB &aabb) const {
  return !(x1 >= aabb.x2 || y1 >= aabb.y2 || x2 <= aabb.x1 || y2 <= aabb.y1);
}

unsigned int AABB::area() { return (x2 - x1) * (y2 - y1); }

Node::Node()
    : aabb(0, 0, 0, 0), fatten(0, 0, 0, 0), next(NULL_NODE), parent(NULL_NODE),
      left(NULL_NODE), right(NULL_NODE){};

bool Node::is_leaf() const { return left == NULL_NODE; }

bool Node::is_valid() { return fatten.contains(aabb); }

Tree::Tree(float margin, unsigned int init_cap)
    : root(NULL_NODE), margin(margin), count(0), capacity(init_cap) {
  nodes.resize(capacity);
  for (auto i = 0; i < capacity - 1; ++i) {
    nodes[i].next = i + 1;
  }
  nodes[capacity - 1].next = NULL_NODE;
  empty_node = 0;
}

unsigned int Tree::add(const AABB &aabb) {
  auto node = alloc_node();
  nodes[node].aabb = aabb;
  update_node(node, margin);
  if (root == NULL_NODE) {
    root = node;
  } else {
    insert_node(node, root);
  }
  return node;
}

void Tree::remove(unsigned int node) { remove_node(node); }

void Tree::update() {
  if (root != NULL_NODE) {
    if (nodes[root].is_leaf()) {
      update_node(root, margin);
    } else {
      std::vector<unsigned int> invalid_nodes;
      invalid_nodes.reserve(64); // TODO: replace hardcoded constant
      check_nodes(root, invalid_nodes);
      for (auto &node : invalid_nodes) {
        pull_node(node);
        update_node(node, margin);
        insert_node(node, root);
      }
      for (auto &node : invalid_nodes) {
        std::vector<unsigned int> overlaps = query(node);
        if (!overlaps.empty()) {
          std::cout << "Node " << node << " overlaps with the following nodes:";
          for (auto &o : overlaps) {
            std::cout << " " << o << ",";
          }
          std::cout << std::endl;
        }
      }
    }
  }
}

std::vector<unsigned int> Tree::query(unsigned int node) const {
  std::vector<unsigned int> stack;
  stack.reserve(256);
  stack.push_back(root);

  std::vector<unsigned int> result;

  while (stack.size()) {
    auto current = stack.back();
    stack.pop_back();

    if (current == NULL_NODE)
      continue;

    // Test for overlap between the AABBs.
    if (nodes[current].aabb.overlaps(nodes[node].aabb)) {
      // Check that we're at a leaf node.
      if (nodes[current].is_leaf()) {
        // Can't interact with itself.
        if (current != node) {
          result.push_back(current);
        }
      } else {
        stack.push_back(nodes[current].right);
        stack.push_back(nodes[current].left);
      }
    }
  }
  return result;
}

// Use to print current tree (from left to right)
void Tree::print() {
  std::vector<unsigned int> stack;
  stack.reserve(256);
  stack.push_back(root);

  while (stack.size()) {
    auto current = stack.back();
    stack.pop_back();

    if (current == NULL_NODE)
      continue;

    std::cout << "node[" << current << "] ";
    if (nodes[current].is_leaf()) {
      std::cout << "leaf\n";
      continue;
    }
    std::cout << "branch\n";

    stack.push_back(nodes[current].right);
    stack.push_back(nodes[current].left);
  }
}

// Unfinished
std::vector<std::pair<unsigned int, unsigned int>> Tree::overlaps() const {
  if (root == NULL_NODE || nodes[root].is_leaf())
    return {};

  // std::vector<unsigned int> stack;
  // stack.reserve(256);
  // stack.push_back(nodes[root].left);
  // stack.push_back(nodes[root].right);

  std::vector<std::pair<unsigned int, unsigned int>> result;
  std::vector<bool> checked(count, false);

  get_overlaps(nodes[root].left, nodes[root].right, result, checked);
  // while (stack.size() > 1) {
  //   unsigned int n0 = stack.back();
  //   unsigned int n1 = stack.back();
  //   stack.pop_back();
  //   stack.pop_back();

  //   // Move on only if nodes have overlaps
  //   if (nodes[n0].aabb.overlaps(nodes[n1].aabb)) {
  //     if (nodes[n0].is_leaf()) {
  //       if (nodes[n1].is_leaf()) { // both nodes are leaves
  //         result.push_back({n0, n1});
  //       } else { // n0 is leaf, n1 is branch
  //         stack.push_back(n0);
  //         stack.push_back(nodes[n1].right);
  //         stack.push_back(n0);
  //         stack.push_back(nodes[n1].left);
  //       }
  //     } else if (nodes[n1].is_leaf()) { // n0 is branch, n1 is leaf
  //       stack.push_back(n1);
  //       stack.push_back(nodes[n0].right);
  //       stack.push_back(n1);
  //       stack.push_back(nodes[n0].left);
  //     } else { // both nodes are branches
  //       stack.push_back(n1);
  //       stack.push_back(nodes[n0].right);
  //       stack.push_back(n1);
  //       stack.push_back(nodes[n0].left);
  //       stack.push_back(n0);
  //       stack.push_back(nodes[n1].right);
  //       stack.push_back(n0);
  //       stack.push_back(nodes[n1].left);
  //     }
  //   }
  // }

  return result;
}

//Unfinished
void Tree::get_overlaps(
    unsigned int n0, unsigned int n1,
    std::vector<std::pair<unsigned int, unsigned int>> &result,
    std::vector<bool> &checked) const {
  // Move on only if nodes have overlaps
  // std::cout << "Tree::get_overlaps(" << std::to_string(n0) << ", "
  //           << std::to_string(n1) << ")\n";
  // std::cout << "[" << std::to_string(nodes[n0].aabb.x1) << ":"
  //           << std::to_string(nodes[n0].aabb.y1) << "] - "
  //           << "[" << std::to_string(nodes[n0].aabb.x2) << ":"
  //           << std::to_string(nodes[n0].aabb.y2) << "] vs "
  //           << "[" << std::to_string(nodes[n1].aabb.x1) << ":"
  //           << std::to_string(nodes[n1].aabb.y1) << "] - "
  //           << "[" << std::to_string(nodes[n1].aabb.x2) << ":"
  //           << std::to_string(nodes[n1].aabb.y2) << "]\n";
  if (nodes[n0].is_leaf()) {
    if (nodes[n1].is_leaf()) { // both nodes are leaves
      if (nodes[n0].aabb.overlaps(nodes[n1].aabb)) {
        result.push_back({n0, n1});
      }
    } else { // n0 is leaf, n1 is branch
      get_overlaps(n1, result, checked);
      get_overlaps(n0, nodes[n1].left, result, checked);
      get_overlaps(n0, nodes[n1].right, result, checked);
    }
  } else if (nodes[n1].is_leaf()) { // n0 is branch, n1 is leaf
    get_overlaps(n0, result, checked);
    get_overlaps(n1, nodes[n0].left, result, checked);
    get_overlaps(n1, nodes[n0].right, result, checked);
  } else { // both nodes are branches
    get_overlaps(n0, result, checked);
    get_overlaps(n1, result, checked);
    get_overlaps(n0, nodes[n1].left, result, checked);
    get_overlaps(n0, nodes[n1].right, result, checked);
    get_overlaps(n1, nodes[n0].left, result, checked);
    get_overlaps(n1, nodes[n0].right, result, checked);
  }
}

// Unfinished
void Tree::get_overlaps(
    unsigned int node,
    std::vector<std::pair<unsigned int, unsigned int>> &result,
    std::vector<bool> &checked) const {
  if (!checked[node]) {
    get_overlaps(nodes[node].left, nodes[node].right, result, checked);
    checked[node] = true;
  }
}

unsigned int Tree::alloc_node() {
  if (empty_node == NULL_NODE) {

    // There are no free nodes only when the tree is full
    assert(count == capacity);

    // Double pool
    nodes.resize(capacity *= 2);

    // Continue linked list
    for (auto i = count; i < capacity; ++i) {
      nodes[i].next = i + 1;
    }
    nodes[capacity - 1].next = NULL_NODE;

    empty_node = count;
  }

  // Get new node from pool
  auto node = empty_node;
  empty_node = nodes[node].next;
  nodes[node].parent = NULL_NODE;
  nodes[node].left = NULL_NODE;
  nodes[node].right = NULL_NODE;
  ++count;

  return node;
}

void Tree::free_node(unsigned int node) {
  assert(node < capacity);
  assert(count > 0);

  nodes[node].next = empty_node;
  empty_node = node;
  --count;
}

void Tree::insert_node(unsigned int node, unsigned int &target) {
  if (nodes[target].is_leaf()) {
    // Terget is leaf, simply split
    auto branch = alloc_node();
    // Make it branch
    if (auto grandparent = nodes[target].parent; grandparent != NULL_NODE) {
      nodes[branch].parent = grandparent;
      (nodes[grandparent].left == target ? nodes[grandparent].left
                                         : nodes[grandparent].right) = branch;
    }
    nodes[branch].left = node;
    nodes[branch].right = target;
    nodes[node].parent = branch;
    nodes[target].parent = branch;
    target = branch; // in case of root node - change it
  } else {
    // Target is branch
    auto left = nodes[target].left;
    auto right = nodes[target].right;
    auto area_diff0 = nodes[left].aabb.unite(nodes[node].aabb).area() -
                      nodes[left].aabb.area();
    auto area_diff1 = nodes[right].aabb.unite(nodes[node].aabb).area() -
                      nodes[right].aabb.area();

    // Insert to the child that gives less area increase
    if (area_diff0 < area_diff1) {
      insert_node(node, left);
    } else {
      insert_node(node, right);
    }
  }
  // Propagates back up the recursion stack
  update_node(target, margin);
}

void Tree::remove_node(unsigned int node) {
  if (node != root) {
    pull_node(node);
    free_node(nodes[node].parent);
  }
  free_node(node);
}

void Tree::pull_node(unsigned int node) {
  assert(nodes[node].is_leaf());
  if (node == root) {
    root = NULL_NODE;
  } else {
    auto parent = nodes[node].parent;
    auto sibling =
        nodes[parent].left == node ? nodes[parent].right : nodes[parent].left;
    auto grandparent = nodes[parent].parent;
    if (grandparent != NULL_NODE) { // if there's a grandparent
      nodes[sibling].parent = grandparent;
      (nodes[grandparent].left == parent ? nodes[grandparent].left
                                         : nodes[grandparent].right) = sibling;
    } else { // no grandparent
      root = sibling;
      nodes[sibling].parent = NULL_NODE;
    }
    free_node(parent);
  }
}

void Tree::update_node(unsigned int node, float margin) {
  if (nodes[node].is_leaf()) {
    nodes[node].fatten.x1 = nodes[node].aabb.x1 - margin;
    nodes[node].fatten.y1 = nodes[node].aabb.y1 - margin;
    nodes[node].fatten.x2 = nodes[node].aabb.x2 + margin;
    nodes[node].fatten.y2 = nodes[node].aabb.y2 + margin;
  } else {
    auto left = nodes[node].left;
    auto right = nodes[node].right;
    nodes[node].aabb = nodes[left].aabb.unite(nodes[right].aabb);
    nodes[node].fatten = nodes[left].fatten.unite(nodes[right].fatten);
  }
}

void Tree::check_nodes(unsigned int node,
                       std::vector<unsigned int> &invalid_nodes) {
  if (nodes[node].is_leaf()) {
    if (!nodes[node].is_valid()) {
      invalid_nodes.push_back(node);
    }
  } else {
    check_nodes(nodes[node].left, invalid_nodes);
    check_nodes(nodes[node].right, invalid_nodes);
  }
}

} // namespace aabb