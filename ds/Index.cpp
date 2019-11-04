#include "Index.h"
#include "../common/global.h"

using namespace std;

HashMap<int, QGramer> qgramers;
int node_num = 0;

void init_qgramers(int height) {
  for (int i = 2; i < height + 2; ++i) {
    qgramers[i] = QGramer(i);
  }
}

void attach_node_id(Rtree::Node *root) {
  queue<Rtree::Node *> qu;
  qu.push(root);
  while (!qu.empty()) {
    auto *node = qu.front();
    qu.pop();
    node->id = ++node_num;
    if (node->m_level > 0) {
      for (int i = 0; i < node->m_count; ++i) {
        auto *child = node->m_branch[i].m_child;
        qu.push(child);
      }
    }
  }
}

Index::Index() {
  slice_time = std::chrono::steady_clock::duration::zero();
  auto t0 = std::chrono::high_resolution_clock::now();
  build_spatial_index();
  attach_node_id(rtree.m_root);
  init_qgramers(rtree.m_root->m_level + 1);
  build_textual_index(rtree.m_root);
  auto t1 = std::chrono::high_resolution_clock::now();
  total_time = t1 - t0;
}

void Index::build_spatial_index() {
  node_num = -1;
  for (int i = 0; i < objects.size(); ++i) {
    auto &obj = objects[i];
    PosVal lb[2] = {obj.rect.lb.x, obj.rect.lb.y};
    PosVal rt[2] = {obj.rect.rt.x, obj.rect.rt.y};
    rtree.Insert(lb, rt, i);
  }
  auto &rect = rtree.m_root->rect;
  rect.lb.x = -180;
  rect.lb.y = -180;
  rect.rt.x = 180;
  rect.rt.y = 180;
  rtree.m_root->parent = nullptr;
  attach_info(rtree.m_root);
}

void Index::attach_info(Rtree::Node *node) {
  int q = node->m_level + 2;
  node->q = q;
  if (node->m_level > 0) {
    for (int i = 0; i < node->m_count; ++i) {
      auto *child = node->m_branch[i].m_child;
      auto &rect = node->m_branch[i].m_rect;
      child->parent = node;
      child->rect.lb.x = rect.m_min[0];
      child->rect.lb.y = rect.m_min[1];
      child->rect.rt.x = rect.m_max[0];
      child->rect.rt.y = rect.m_max[1];
      attach_info(child);
    }
  } else {
    for (int i = 0; i < node->m_count; ++i) {
      int obj_id = node->m_branch[i].m_data;
      node->ids.push_back(obj_id);
    }
    auto *p = node->parent;
    while (!p->is_root()) {
      for (auto &obj_id : node->ids) {
        p->ids.push_back(obj_id);
      }
      p = p->parent;
    }
  }
}

void Index::build_textual_index(Rtree::Node *node) {
  int q = node->q;
  auto &lists = node->inverted_lists;
  for (auto &obj_id: node->ids) {
    auto &obj = objects[obj_id];
    auto t0 = std::chrono::high_resolution_clock::now();
    auto v = qgramers[q].process(obj.words);
    auto t1 = std::chrono::high_resolution_clock::now();
    slice_time += t1 - t0;
    for (int i = 0; i < v.size(); ++i) {
      auto l = static_cast<int>(obj.words[i].length());
      auto &grams = v[i];
      for (auto &gram : grams) {
        node->l_min = min(node->l_min, l);
        node->l_max = max(node->l_max, l);
        lists[l].add(obj_id, gram);
      }
    }
  }
  for (auto &list : lists) {
    for (auto &terms : list.second.inv_list) {
      sort(terms.second.begin(), terms.second.end());
    }
  }
  if (node->m_level > 0) {
    for (int i = 0; i < node->m_count; ++i) {
      auto *child = node->m_branch[i].m_child;
      build_textual_index(child);
    }
  }
}

double Index::size() {
  return (double) size(rtree.m_root) / (1024 * 1024);
}

int Index::size(Rtree::Node *node) {
  int res = 0;
  for (auto &len : node->inverted_lists) {
    for (auto &gram : len.second.inv_list) {
      res += gram.second.size() * 2 * 4;
    }
  }
  if (node->m_level > 0) {
    for (int i = 0; i < node->m_count; ++i) {
      auto *child = node->m_branch[i].m_child;
      res += size(child);
    }
  }
  return res;
}
