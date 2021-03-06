#ifndef AABB_H
#define AABB_H

#include <entt/entt.hpp>

#include <string>
#include <vector>

#include "Geometry.hpp"

constexpr unsigned int NULL_NODE = 0xFFFFFFFF;

namespace aabb {

class AABB {
public:
  AABB(float x, float y, float width, float height);
  AABB(geom::Point<float> pos, geom::Vector<float> dim);
  AABB(geom::Point<float> pos, float width, float height);
  geom::Point<float> pos;
  geom::Vector<float> dim;
  geom::Point<float> center() const;
  AABB unite(const AABB &) const;
  AABB overlap(const AABB &) const;
  bool contains(const AABB &) const;
  bool overlaps(const AABB &) const;
  unsigned int area();

private:
};

struct Node {
  Node();

  entt::entity id;

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

// TODO: add iteration support
class Tree {
public:
  Tree(float margin, unsigned int capacity);
  ~Tree(){};

  unsigned int add(entt::entity id, const AABB &aabb);
  void remove(unsigned int node);
  void update();
  void print();
  inline unsigned int size() { return count; };
  std::vector<entt::entity> query(unsigned int node) const;
  std::vector<std::pair<unsigned int, unsigned int>> overlaps() const;
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
  void get_overlaps(unsigned int n0, unsigned int n1,
                    std::vector<std::pair<unsigned int, unsigned int>> &result,
                    std::vector<bool> &checked) const;
  void get_overlaps(unsigned int node,
                    std::vector<std::pair<unsigned int, unsigned int>> &result,
                    std::vector<bool> &checked) const;
};

} // namespace aabb

#endif // AABB_H