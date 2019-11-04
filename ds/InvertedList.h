#ifndef INVERTED_LIST_H
#define INVERTED_LIST_H

#include "../common/def.h"

// <object_id, pos>
struct Term {
  int obj_id;
  int word_id;
  int pos;
  Term(int _obj_id, int _word_id, int _pos) : obj_id(_obj_id), word_id(_word_id), pos(_pos) {}
  bool operator<(const Term &rhs) const {
    return pos < rhs.pos;
  }
};

class InvertedList {
 public:
  HashMap<int, std::vector<Term>> inv_list;

 public:
  InvertedList() = default;
  void add(int obj_id, const PosGram &gram);
};

#endif
