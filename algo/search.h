#ifndef BASE_H
#define BASE_H

#include "../common/def.h"
#include "../ds/Index.h"
#include "../ds/QGramer.h"
#include <chrono>

typedef std::tuple<double, int, Rtree::Node *> NodeEntry;  // <upper bound, node_id, node>

class Searcher {
 public:
  std::vector<bool> visited;
  std::vector<bool> visited_node;
  Query query;
  double tau;
  double spatial_lower_bound;
  MinHeap<Result> results;
  std::vector<int> lambdas;
  std::vector<int> ability;
  unsigned long n;
  int candidates = 0;
  HashMap<int, HashMap<int, double>> ned_map = {
    {2, {
      {11, 0.785714286}, {10, 0.8}, {9, 0.777777778}, {8, 0.8}, {7, 0.777777778}, {6, 0.833333333},
      {5, 0.833333333}, {4, 0.8},
      {3, 0.75}, {2, 0.666667}, {1, 0.5}
    }},
    {3, {
      {11, 0.846153846}, {10, 0.833333}, {9, 0.818182}, {8, 0.875}, {7, 0.875}, {6, 0.857142857}, {5, 0.833333},
      {4, 0.8}, {3, 0.75}, {2, 0.666667}, {1, 0.5}
    }},
    {4, {
      {11, 0.846153846}, {10, 0.9}, {9, 0.9}, {8, 0.888888889}, {7, 0.875}, {6, 0.857142857}, {5, 0.833333},
      {4, 0.8},
      {3, 0.75}, {2, 0.666667}, {1, 0.5}
    }},
    {5, {
      {11, 0.916666667}, {10, 0.909090909}, {9, 0.9}, {8, 0.888889}, {7, 0.875}, {6, 0.857142857}, {5, 0.833333},
      {4, 0.8}, {3, 0.75},
      {2, 0.666667}, {1, 0.5}
    }},
  };

  MinHeap<Result> search(Index &index, const Query &query);
  void search(Index &index);
  void search(Rtree::Node *root);
  void init(const Query &query, Rtree::Node *root);
  void verify(const int &object_id);
  void update(const int &object_id);
  void upper_verify(const int &object_id, double spatial_sim, double tau_t);
  void init_queue(Rtree::Node *root);
  void update_lambda();
  double cal_ned(const string &s1, const string &s2, double delta);
  double cal_ned(const string &s1, const string &s2, int tau);
  double cal_overlap(const std::vector<string> &words1, const std::vector<string> &words2);
  double upper_cal_overlap(const std::vector<string> &words1, const std::vector<string> &words2, double tau_t);
  double group_bound(int q, int lambda, int length, int word_id);
  double estimate_ubt(Rtree::Node *node);
  void push_child(MaxHeap<NodeEntry> &qu, Rtree::Node *node);
};

#endif
