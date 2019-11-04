#include "QGramer.h"

using namespace std;

void QGramer::clear_cache() {
  cache.clear();
}

vector<PosGrams> QGramer::process(const std::vector<string> &words) {
  vector<PosGrams> v;
  for (int j = 0; j < words.size(); ++j) {
    IntGrams int_grams;
    PosGrams pos_grams;
    StrGrams str_grams = slice(words[j]);
    for (auto &str_gram : str_grams) {
      if (dict.find(str_gram) == dict.end()) {
        dict[str_gram] = static_cast<int>(dict.size());
        rdict[dict[str_gram]] = str_gram;
      }
      int_grams.emplace_back(dict[str_gram]);
    }
    // sort to get prefix
    sort(int_grams.begin(), int_grams.end());
    for (int i = 0; i < int_grams.size(); ++i) {
      auto &gram = int_grams[i];
      pos_grams.emplace_back(gram, j, i + 1);
    }
    v.push_back(pos_grams);
  }
  return v;
}

vector<PosGrams> &QGramer::slice_query(const std::vector<string> &words) {
  if (cache.empty()) {
    cache = process(words);
  }
  return cache;
}

StrGrams QGramer::slice(const string &s) {
  auto l = static_cast<int>(s.length());
  StrGrams grams;
  if (l < q) {
    grams.emplace_back(s);
  } else {
    for (int i = 0; i < l - q + 1; ++i) {
      grams.emplace_back(s.substr(i, q));
    }
  }
  return grams;
}

