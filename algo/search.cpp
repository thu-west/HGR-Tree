#include "search.h"
#include <stack>
#include "../common/global.h"
#include "../utils/utils.h"
#include "../utils/munkres.h"

using namespace std;

MinHeap<Result> Searcher::search(Index &index, const Query &query) {
  init(query, index.rtree.m_root);
  search(index);
  return results;
}

void Searcher::verify(const int &object_id) {
  auto &obj = objects[object_id];
  double spatial_sim = cal_spatial_similarity(obj.rect, query.rect, query.max_dist);
  if (spatial_sim >= spatial_lower_bound) {
    double fuzzy_overlap = cal_overlap(query.words, obj.words);
    double text_sim = fuzzy_overlap / (obj.words.size() + n - fuzzy_overlap);
    double similarity = query.alpha * text_sim + (1 - query.alpha) * spatial_sim;
    if (similarity > tau) {
      results.pop();
      results.push(make_pair(similarity, object_id));
      tau = results.top().first;
      spatial_lower_bound = std::max(0.0, (tau - query.alpha) / (1 - query.alpha));
    }
  }
  visited[object_id] = true;
}

void Searcher::update(const int &object_id) {
  auto &obj = objects[object_id];
  double spatial_sim = cal_spatial_similarity(obj.rect, query.rect, query.max_dist);
  double fuzzy_overlap = cal_overlap(query.words, obj.words);
  double text_sim = fuzzy_overlap / (obj.words.size() + n - fuzzy_overlap);
  double similarity = query.alpha * text_sim + (1 - query.alpha) * spatial_sim;
  results.push(make_pair(similarity, object_id));
  visited[object_id] = true;
}

void Searcher::upper_verify(const int &object_id, double spatial_sim, double tau_t) {
  auto &obj = objects[object_id];
  double fuzzy_overlap = upper_cal_overlap(query.words, obj.words, tau_t);
  double text_sim = fuzzy_overlap / (obj.words.size() + n - fuzzy_overlap);
  double similarity = query.alpha * text_sim + (1 - query.alpha) * spatial_sim;
  if (similarity > tau) {
    results.pop();
    results.push(make_pair(similarity, object_id));
    tau = results.top().first;
    spatial_lower_bound = std::max(0.0, (tau - query.alpha) / (1 - query.alpha));
  }
  visited[object_id] = true;
}

void Searcher::init(const Query &query, Rtree::Node *root) {
  this->query = query;
  n = query.words.size();
  lambdas = vector<int>(n);
  sort(this->query.words.begin(),
       this->query.words.end(),
       [](string s1, string s2) { return counter[s1] < counter[s2]; });
  update_lambda();
  ability = vector<int>(n, 0);
  results = MinHeap<Result>();
  visited = vector<bool>(objects.size(), false);
  visited_node = vector<bool>(static_cast<unsigned long>(node_num + 1), false);
  for (auto &i : qgramers) {
    i.second.clear_cache();
  }
  init_queue(root);
  tau = results.top().first;
  spatial_lower_bound = std::max(0.0, (tau - query.alpha) / (1 - query.alpha));
}

void Searcher::init_queue(Rtree::Node *root) {
  int k = query.k;
  int cnt = 0;
  queue<Rtree::Node *> qu;
  qu.push(root);
  while (!qu.empty()) {
    auto node = qu.front();
    qu.pop();
    if (node->m_level == 0) {  // if is leaf
      while (node->ids.size() < k) {
        node = node->parent;
      }
      for (auto obj_id : node->ids) {
        update(obj_id);
        cnt++;
        visited[obj_id] = true;
        if (cnt == k) return;
      }
    } else {
      for (int i = 0; i < node->m_count; ++i) {
        auto *child = node->m_branch[i].m_child;
        if (overlap(child->rect, query.rect)) {
          qu.push(child);
        }
      }
    }
  }
}

double Searcher::cal_ned(const string &s1, const string &s2, int tau) {
  int ed = cal_ed(s1, s2, tau);
  if (ed > tau) {
    return 0.0;
  }
  return 1.0 - (double) ed / max(s1.length(), s2.length());
}

double Searcher::cal_ned(const string &s1, const string &s2, double delta) {
  auto tau = static_cast<int>((1.0 - delta) / delta * s1.length());
  int ed = cal_ed(s1, s2, tau);
  if (ed > tau) {
    return 0.0;
  }
  return 1.0 - (double) ed / max(s1.length(), s2.length());
}

double Searcher::cal_overlap(const std::vector<string> &words1, const std::vector<string> &words2) {
  double similarity = 0;
  auto m = words1.size();
  auto n = words2.size();
  Matrix<double> origin(m, n);
  Matrix<double> matrix(m, n);
  for (size_t i = 0; i < m; ++i) {
    for (size_t j = 0; j < n; ++j) {
      double ned = cal_ned(words1[i], words2[j], lambdas[i]);
      origin(i, j) = ned >= query.delta ? ned : 0;
      matrix(i, j) = 2 - origin(i, j);
    }
  }
  candidates++;
  Munkres<double> km;
  km.solve(matrix);
  for (size_t i = 0; i < m; ++i) {
    for (size_t j = 0; j < n; ++j) {
      if (matrix(i, j) == 0) {
        similarity += origin(i, j);
      }
    }
  }
  return similarity;
}

double Searcher::upper_cal_overlap(const std::vector<string> &words1,
                                       const std::vector<string> &words2,
                                       double tau_t) {
  double similarity = 0;
  auto m = words1.size();
  auto n = words2.size();
  Matrix<double> origin(m, n);
  Matrix<double> matrix(m, n);
  vector<double> weights(m, 0.0);
  for (size_t i = 0; i < m; ++i) {
    for (size_t j = 0; j < n; ++j) {
      double ned = cal_ned(words1[i], words2[j], lambdas[i]);
      origin(i, j) = ned >= query.delta ? ned : 0;
      matrix(i, j) = 2 - origin(i, j);
      weights[i] = max(weights[i], origin(i, j));
    }
  }
  candidates++;
  double upper_bound = 0.0;
  for (auto &weight : weights) {
    upper_bound += weight;
  }
  if (upper_bound < tau_t * m) {
    return 0.0;
  }
  Munkres<double> km;
  km.solve(matrix);
  for (size_t i = 0; i < m; ++i) {
    for (size_t j = 0; j < n; ++j) {
      if (matrix(i, j) == 0) {
        similarity += origin(i, j);
      }
    }
  }
  return similarity;
}
void Searcher::update_lambda() {
  double c = query.delta;
  for (int i = 0; i < n; ++i) {
    auto &word = query.words[i];
    lambdas[i] = compute_lambda(c, static_cast<int>(word.length()));
  }
}
double Searcher::group_bound(int q, int lambda, int length, int word_id) {
  double fuzzy_overlap = 0;
  for (int i = 0; i < n; ++i) {
    double ned;
    if (i == word_id) {
      int l = std::max(length, (int) query.words[i].length());
      ned = 1.0 - (double) lambda / l;
    } else {
      ned = ned_map[q][(int) query.words[i].length()];
    }
    if (ned >= query.delta) {
      fuzzy_overlap += ned;
    }
  }
  return fuzzy_overlap;
}
double Searcher::estimate_ubt(Rtree::Node *node) {
  double fuzzy_overlap = 0;
  auto q = node->q;
  auto &v = qgramers[q].slice_query(query.words);
  auto &lists = node->inverted_lists;
  for (int word_id = 0; word_id < v.size(); ++word_id) {
    bool found = false;
    const auto &word = query.words[word_id];
    auto &grams = v[word_id];
    auto l = static_cast<int>(word.length());
    if (lists.find(l) != lists.end()) {
      auto &inv_list = lists[l].inv_list;
      if (inv_list.find(grams[0].id) != inv_list.end()) {
        if (inv_list[grams[0].id][0].pos == 1) {
          found = true;
        }
      }
    }
    if (!found) {
      double edit_sim = 1 - 1.0 / (l + 1);
      auto flag = edit_sim >= query.delta;
      fuzzy_overlap += flag ? edit_sim : 0;
    } else {
      fuzzy_overlap += 1;
    }
  }
  return fuzzy_overlap / n;
}

void Searcher::push_child(MaxHeap<NodeEntry> &qu, Rtree::Node *node) {
  for (int i = 0; i < node->m_count; ++i) {
    auto *child = node->m_branch[i].m_child;
    auto &rect = child->rect;
    auto ubs = estimate_ubs(query.rect, rect, query.max_dist);
    auto ubt = estimate_ubt(child);
    auto ub = cal_similarity(ubt, ubs, query.alpha);
    if (ub >= tau) {
      qu.push(make_tuple(child->q * 2 + ub, child->id, child));
    }
  }
}


void Searcher::search(Index &index) { search(index.rtree.m_root); }

void Searcher::search(Rtree::Node *root) {
  MaxHeap<NodeEntry> qu;
  push_child(qu, root);
  while (!qu.empty()) {
    auto node_count = static_cast<int>(qu.size());
    int q = (std::get<2>(qu.top()))->q;
    for (int i = 0; i < n; ++i) {
      auto &word = query.words[i];
      ability[i] = max(0, (static_cast<int>(word.length()) - q) / q);
    }
    auto &v = qgramers[q].slice_query(query.words);
    while (node_count > 0) {
      auto *node = std::get<2>(qu.top());
      auto ub = std::get<0>(qu.top()) - 2 * q;
      if (ub < tau) {
        while (node_count > 0) {
          qu.pop();
          --node_count;
        }
        break;
      }
      qu.pop();
      --node_count;
      auto ubs = estimate_ubs(query.rect, node->rect, query.max_dist);
      auto tau_t = (tau - (1 - query.alpha) * ubs) / query.alpha;
      if (tau_t <= 0) {
        if (node->m_level == 0) {
          for (auto obj_id : node->ids) {
            if (!visited[obj_id]) {
              verify(obj_id);
            }
          }
        } else {
          push_child(qu, node);
        }
        continue;
      } else if (tau_t > 1) {
        continue;
      }
      auto distinct_num = static_cast<int>(ceil(n * tau_t));
      visited_node[node->id] = true;
      auto &lists = node->inverted_lists;
      bool is_first = !visited_node[node->parent->id];
      for (int word_id = 0; word_id < v.size(); ++word_id) {
        if (word_id + 1 > n - (distinct_num - 1)) {
          break;
        }
        const auto &word = query.words[word_id];
        auto &grams = v[word_id];
        auto l = static_cast<int>(word.length());
        int &lambda_j = lambdas[word_id];
        // length filter
        for (int t = max(node->l_min, l - lambda_j);
             t <= min(node->l_max, l + lambda_j); ++t) {
          if (lists.find(t) == lists.end()) {
            continue;
          }
          auto &inv_list = lists[t].inv_list;
          // enumerate lambda_j (incremental)
          int lambda = is_first ? 0 : max(0, (min(l, t) - q - 1)) / (q + 1) + 1;
          for (; lambda <= lambda_j && lambda <= ability[word_id]; ++lambda) {
            if (!is_first) {
              double fuzzy_overlap = group_bound(q, lambda, t, word_id);
              if (fuzzy_overlap < tau_t * n) {
                break;
              }
            }
            if (t - l > lambda || t - l < -lambda) {
              continue;
            }
            for (const auto &gram : grams) {
              // prefix filter of query token
              if (gram.pos < q * (lambda - 1) + 1) {
                continue;
              }
              if (gram.pos > q * lambda + 1) {
                break;
              }
              if (inv_list.find(gram.id) == inv_list.end()) {
                continue;
              }
              auto &items = inv_list[gram.id];
              for (auto &item : items) {
                // prefix filter on object token (incremental)
                if (item.pos < q * (lambda - 1) + 1) {
                  continue;
                }
                if (item.pos > q * lambda + 1) {
                  break;
                }
                auto obj_id = item.obj_id;
                if (visited[obj_id]) {
                  continue;
                }
                auto &obj = objects[obj_id];
                double spatial_sim = cal_spatial_similarity(obj.rect, query.rect, query.max_dist);
                if (spatial_sim >= spatial_lower_bound) {
                  double tau_t = std::max(0.0, (tau - (1 - query.alpha) * spatial_sim) / query.alpha);
                  auto &word2 = obj.words[item.word_id];
                  double delta = std::max(tau_t, query.delta);
                  if (cal_ned(word, word2, delta) < delta) {
                    continue;
                  }
                  upper_verify(obj_id, spatial_sim, tau_t);
                } else {
                  visited[obj_id] = true;
                }
                if (tau == 1.0) {
                  return;
                }
              }
            }
          }
        }
      }
      if (node->m_level == 0) {
        continue;
      }
      push_child(qu, node);
    }
  }
}