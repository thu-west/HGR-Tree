#ifndef QGRAMER_H
#define QGRAMER_H

#include "Object.h"
#include "../common/def.h"

class QGramer {
 public:
  int q;
  HashMap<string, int> dict;  // <gram, id>
  HashMap<int, string> rdict;
  HashMap<int, int> counter; // <id, count>
  std::vector<PosGrams> cache;

 public:
  QGramer() = default;
  explicit QGramer(int _q) : q(_q) {}
  void clear_cache();
  // string to qgram
  StrGrams slice(const string& s);
  // string to int gram
  std::vector<PosGrams>& slice_query(const std::vector<string> &words);
  // string to positional gram
  std::vector<PosGrams> process(const std::vector<string> &words);
};

#endif
