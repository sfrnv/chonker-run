#ifndef AABB_H
#define AABB_H

#include <string>
#include <vector>

constexpr unsigned int NULL_NODE = 0xFFFFFFFF;

namespace aabb {

class AABB {
public:
  AABB(int x1, int y1, int x2, int y2);
  int x1, y1, x2, y2;
  AABB unite(const AABB &) const;
  bool contains(const AABB &) const;
  bool overlaps(const AABB &) const;
  unsigned int area();

private:
};

struct Node {
  Node();

  AABB aabb;
  AABB fatten;

  unsigned int next;
  unsigned int parent;
  unsigned int left;
  unsigned int right;

  bool is_leaf() const;
  bool is_valid();
  void set_leaf(const AABB &aabb);
  void set_branch(unsigned int left, unsigned int right);
  void update_AABB(float margin);
};

class Tree {
public:
  Tree(float margin, unsigned int capacity);
  ~Tree(){};

  unsigned int add(const AABB &aabb);
  void remove(unsigned int node);
  void update();
  std::vector<unsigned int> query(unsigned int node) const;
  // ColliderPairList &ComputePairs();
  // Collider *Pick(const Vec3 &point) const;
  // Query(const AABB &aabb, ColliderList &out) const;
  // RayCastResult RayCast(const Ray3 &ray) const;

  const Node &operator[](const unsigned int i) const { return nodes[i]; }
  Node &operator[](const unsigned int i) { return nodes[i]; }

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

  void insert_node(unsigned int node, unsigned int &target);
  void remove_node(unsigned int node);
  void pull_node(unsigned int node);
  void update_node(unsigned int node, float margin);
  void check_nodes(unsigned int node, std::vector<unsigned int> &invalid_nodes);
};

} // namespace aabb

#endif // AABB_H