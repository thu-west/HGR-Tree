#include <chrono>
#include <cstdio>
#include "algo/search.h"
#include "common/global.h"
#include "utils/utils.h"

using namespace std;

void print_result(const Result &result) {
  cout << "<" << result.second;
  cout << ",";
  cout << result.first << ",[";
  print_words(objects[result.second].words);
  cout << "]>  ";
}

void output(MinHeap<Result> &results) {
  int i = 1;
  while (!results.empty()) {
    auto &result = results.top();
    cout << i++ << ". ";
    print_result(result);
    results.pop();
    cout << endl;
  }
  cout << endl;
}

void run(Index &index) {
  Searcher searcher;
  for (auto &query : queries) {
    auto result = searcher.search(index, query);
    output(result);
  }
}

int main(int argc, char *argv[]) {
  string temp;
  string data_set(argv[1]);
  string query_set(argv[2]);
  int k = stoi(argv[3]);
  double max_dist = stod(argv[4]);
  double alpha = stod(argv[5]);
  double delta = stod(argv[6]);
  load_objects(data_set);
  load_queries(query_set, k, alpha, delta, max_dist);
  Index index;
  run(index);
  return 0;
}
