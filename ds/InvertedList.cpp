#include "InvertedList.h"

void InvertedList::add(int obj_id, const PosGram &gram) {
  inv_list[gram.id].emplace_back(obj_id, gram.word_id, gram.pos);
}

