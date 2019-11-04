#ifndef TYPEDEF_H
#define TYPEDEF_H

#include <array>
#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <queue>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#define EXAMPLE

using std::ifstream;
using std::pair;
using std::string;
using std::unordered_set;
using std::vector;

const int MAX_CHILD = 256;

typedef std::vector<string> StrGrams;  // gram

struct PosGram {
  int id;
  int word_id;
  int pos;
  PosGram(int gid, int wid, int pos) : id(gid), word_id(wid), pos(pos) {}
  bool operator<(const PosGram &rhs) const { return pos < rhs.pos; }
};

typedef std::vector<int> IntGrams;
typedef std::vector<PosGram> PosGrams;

typedef pair<double, int> Result;  // <similarity, obj_id>

typedef double PosVal;

template <typename K, typename H = std::hash<K>>
using HashSet = std::unordered_set<K, H>;

template <typename K, typename V, typename H = std::hash<K>>
using HashMap = std::unordered_map<K, V, H>;

template <typename K, typename V>
using Map = std::map<K, V>;

template <typename K>
using MaxHeap = std::priority_queue<K>;

template <typename K>
using MinHeap = std::priority_queue<K, std::vector<K>, std::greater<K>>;

template <typename K>
using Heap = MinHeap<K>;

#endif
