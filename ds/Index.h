#ifndef INDEX_H
#define INDEX_H
#include <chrono>

#include "../common/def.h"
#include "InvertedList.h"
#include "Object.h"
#include "RTree.h"
#include "QGramer.h"

using Rtree = RTree<int, double, 2, double, MAX_CHILD>;

class Index {
 public:
  // build index from file
  Rtree rtree;
  Index();
  double size();
  std::chrono::duration<double, std::milli> total_time;
  std::chrono::duration<double, std::milli> slice_time;

 private:
  // rtree
  void build_spatial_index();
  // qgram
  void build_textual_index(Rtree::Node *node);
  void attach_info(Rtree::Node *root);
  int size(Rtree::Node *node);
};

#endif
