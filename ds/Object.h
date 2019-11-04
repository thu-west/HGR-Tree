#ifndef OBJECT_H
#define OBJECT_H

#include "../common/def.h"
#include <string>
#include <vector>

struct Point {
  PosVal x;
  PosVal y;
};

struct Rectangle {
  Point lb;   // left_bottom
  Point rt;   // right_top
  friend std::ostream& operator<<(std::ostream &os, const Rectangle & r);
};

// raw object
struct Object {
  // int obj_id;
  Rectangle rect;
  std::vector<std::string> words;
};

struct Query {
  int k;
  double alpha;
  double delta;
  double max_dist;
  Rectangle rect;
  std::vector<std::string> words;
};

#endif
