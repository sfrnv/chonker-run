#include <cmath>
#include <exception>

#include "AABB.hpp"

namespace aabb {

AABB::AABB(int x1, int y1, int x2, int y2) : x1(x1), y1(y1), x2(x2), y2(y2){};

AABB AABB::unite(const AABB &aabb) {
  return AABB(std::min(x1, aabb.x1), std::min(y1, aabb.y1),
              std::max(x2, aabb.x2), std::max(y2, aabb.y2));
};

bool AABB::contains(const AABB &aabb) {
  return x1 <= aabb.x1 && y1 <= aabb.y1 && x2 >= aabb.x2 && y2 >= aabb.y2;
}

unsigned int AABB::area() { return (x2 - x1) * (y2 - y1); }

Node::Node()
    : id(entt::null), aabb(nullptr), fatten(0, 0, 0, 0), next(NULL_NODE),
      parent(NULL_NODE), left(NULL_NODE), right(NULL_NODE){};

bool Node::is_leaf() const { return left == NULL_NODE; }

bool Node::is_valid() {
  if (aabb == nullptr)
    return false;
  return fatten.contains(*aabb);
}

void Node::set_leaf(AABB *aabb) {}

void Node::set_branch(unsigned int left, unsigned int right) {}

void Node::update_AABB(float margin) {}

Tree::Tree(unsigned int margin, unsigned int init_cap)
    : root(NULL_NODE), margin(0.f), count(0), capacity(init_cap) {
  nodes.resize(capacity);
  for (auto i = 0; i < capacity - 1; ++i) {
    nodes[i].next = i + 1;
  }
  nodes[capacity - 1].next = NULL_NODE;
  empty_node = 0;
}

unsigned int Tree::alloc_node() {
  if (empty_node == NULL_NODE) {

    // There are no free nodes only when the tree is full
    assert(count == capacity);

    // Double pool
    capacity *= 2;
    nodes.resize(capacity);

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

unsigned int Tree::add(AABB *aabb) {
  auto node = alloc_node();
  nodes[node].set_leaf(aabb);
  nodes[node].update_AABB(margin);
  if (root == NULL_NODE) {
    root = node;
  } else {
    insert_node(node, root);
  }
  return node;
}

AABB *Tree::remove(unsigned int node) { return remove_node(node); }

void Tree::insert_node(unsigned int node, unsigned int target) {
  if (nodes[target].is_leaf()) {
    // Terget is leaf, simply split
    auto branch = alloc_node();
    nodes[branch].parent = nodes[target].parent;
    nodes[branch].set_branch(node, target);
    nodes[target].parent = branch;
  } else {
    // Target is branch
    auto left = nodes[target].left;
    auto right = nodes[target].right;
    auto area_diff0 = nodes[left].aabb->unite(*nodes[node].aabb).area() -
                      nodes[left].aabb->area();
    auto area_diff1 = nodes[right].aabb->unite(*nodes[node].aabb).area() -
                      nodes[right].aabb->area();

    // Insert to the child that gives less area increase
    if (area_diff0 < area_diff1) {
      insert_node(node, left);
    } else {
      insert_node(node, right);
    }

    // Propagates back up the recursion stack
    nodes[target].update_AABB(margin);
  }
}

AABB *Tree::remove_node(unsigned int node) {
  if (node != root) {
    pull_node(node);
    free_node(nodes[node].parent);
  }
  AABB *retval = nodes[node].aabb;

  free_node(node);

  return retval;
}

void Tree::pull_node(unsigned int node) {
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
}

void Tree::update() {
  if (root != NULL_NODE) {
    if (nodes[root].is_leaf()) {
      nodes[root].update_AABB(margin);
    } else {
      std::vector<unsigned int> invalid_nodes;
      invalid_nodes.reserve(64); // TODO: replace hardcoded constant
      find_invalid_nodes(root, invalid_nodes);
      for (auto node : invalid_nodes) {
        pull_node(node);
        nodes[node].update_AABB(margin);
        insert_node(node, root);
      }
    }
  }
}

void Tree::find_invalid_nodes(unsigned int node,
                              std::vector<unsigned int> &invalid_nodes) {
  if (nodes[node].is_leaf()) {
    if (!nodes[node].is_valid()) {
      invalid_nodes.push_back(node);
    }
  } else {
    find_invalid_nodes(nodes[node].left, invalid_nodes);
    find_invalid_nodes(nodes[node].right, invalid_nodes);
  }
}

} // namespace aabb