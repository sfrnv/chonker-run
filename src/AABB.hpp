#ifndef AABB_H
#define AABB_H

#include <string>
#include <vector>

#include <entt/entt.hpp>

constexpr unsigned int NULL_NODE = 0xFFFFFFFF;

namespace aabb {

class AABB {
public:
  AABB(int x1, int y1, int x2, int y2);
  int x1, y1, x2, y2;
  AABB unite(const AABB &);
  bool contains(const AABB &);
  unsigned int area();

private:
};

struct Node {
  Node();

  entt::entity id;

  AABB *aabb;
  AABB fatten;

  unsigned int next;
  unsigned int parent;
  unsigned int left;
  unsigned int right;

  bool is_leaf() const;
  bool is_valid();
  void set_leaf(AABB *aabb);
  void set_branch(unsigned int left, unsigned int right);
  void update_AABB(float margin);
};

class Tree {
public:
  Tree(unsigned int margin, unsigned int capacity);
  ~Tree();

  unsigned int add(AABB *aabb);
  AABB *remove(unsigned int node);
  void update();
  // ColliderPairList &ComputePairs();
  // Collider *Pick(const Vec3 &point) const;
  // Query(const AABB &aabb, ColliderList &out) const;
  // RayCastResult RayCast(const Ray3 &ray) const;

private:
  // typedef std::vector<Node *> NodeList;
  // using NodeList = std::vector<Node>;

  unsigned int root;

  float margin;

  std::vector<Node> nodes;

  unsigned int count;
  unsigned int capacity;
  unsigned int empty_node;

  unsigned int alloc_node();
  void free_node(unsigned int node);

  void insert_node(unsigned int node, unsigned int parent);
  AABB *remove_node(unsigned int node);
  void pull_node(unsigned int node);
  void find_invalid_nodes(unsigned int node,
                          std::vector<unsigned int> &invalid_nodes);
};

} // namespace aabb

#endif // AABB_H