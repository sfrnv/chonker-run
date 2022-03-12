#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <iostream>

namespace geom {

template <typename T> class Vector {
public:
  T x;
  T y;
  Vector() {}
  Vector(T x, T y) {
    this->x = x;
    this->y = y;
  }
  Vector(T xy) {
    this->x = xy;
    this->y = xy;
  }

  Vector &operator+=(const Vector &v) {
    x += v.x;
    y += v.y;
    return (*this);
  }

  Vector &operator-=(const Vector &v) {
    x -= v.x;
    y -= v.y;
    return (*this);
  }

  Vector &operator*=(T t) {
    x *= t;
    y *= t;
    return (*this);
  }

  Vector &operator/=(T t) {
    T f = T{1} / t;
    x *= f;
    y *= f;
    return (*this);
  }

  Vector &operator&=(const Vector &v) {
    x *= v.x;
    y *= v.y;
    return (*this);
  }

  Vector operator-(void) const { return (Vector(-x, -y)); }

  Vector operator+(const Vector &v) const { return (Vector(x + v.x, y + v.y)); }

  Vector operator-(const Vector &v) const { return (Vector(x - v.x, y - v.y)); }

  Vector operator*(T t) const { return (Vector(x * t, y * t)); }

  Vector operator/(T t) const {
    T f = T{1} / t;
    return (Vector(x * f, y * f));
  }

  T operator*(const Vector &v) const { return (x * v.x + y * v.y); }

  Vector operator&(const Vector &v) const { return (Vector(x * v.x, y * v.y)); }

  bool operator==(const Vector &v) const { return ((x == v.x) && (y == v.y)); }

  bool operator!=(const Vector &v) const { return ((x != v.x) || (y != v.y)); }

  bool operator<=(const Vector &v) const { return ((x <= v.x) && (y <= v.y)); }

  bool operator<(const Vector &v) const { return ((x < v.x) && (y < v.y)); }

  bool operator>=(const Vector &v) const { return ((x >= v.x) && (y >= v.y)); }

  bool operator>(const Vector &v) const { return ((x > v.x) && (y > v.y)); }

  Vector &normalize(void) { return (*this /= sqrtf(x * x + y * y)); }

  Vector &rotate(T angle);

  friend std::ostream &operator<<(std::ostream &os, const Vector &v) {
    os << '[' << v.x << ':' << v.y << ']';
    return os;
  };
};

template <typename T> class Point : public Vector<T> {
public:
  Point() {}

  Point(T r, T s) : Vector<T>(r, s) {}

  Point &operator=(const Vector<T> &v) {
    this->x = v.x;
    this->y = v.y;
    return (*this);
  }

  Point &operator*=(T t) {
    this->x *= t;
    this->y *= t;
    return (*this);
  }

  Point &operator/=(T t) {
    T f = T{1} / t;
    this->x *= f;
    this->y *= f;
    return (*this);
  }

  Point operator-(void) const { return (Point(-this->x, -this->y)); }

  Point operator+(const Vector<T> &v) const {
    return (Point(this->x + v.x, this->y + v.y));
  }

  Point operator-(const Vector<T> &v) const {
    return (Point(this->x - v.x, this->y - v.y));
  }

  Vector<T> operator-(const Point &p) const {
    return (Vector<T>(this->x - p.x, this->y - p.y));
  }

  Point operator*(T t) const { return (Point(this->x * t, this->y * t)); }

  Point operator/(T t) const {
    T f = T{1} / t;
    return (Point(this->x * f, this->y * f));
  }
};

template <typename T> Point<T> min(const Point<T> &p1, const Point<T> &p2) {
  return (Point<T>(p1.x < p2.x ? p1.x : p2.x, p1.y < p2.y ? p1.y : p2.y));
}

template <typename T> Point<T> max(const Point<T> &p1, const Point<T> &p2) {
  return (Point<T>(p1.x > p2.x ? p1.x : p2.x, p1.y > p2.y ? p1.y : p2.y));
}

} // namespace geom

#endif // GEOMETRY_H