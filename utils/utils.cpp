#include "utils.h"
#include <array>
#include <sstream>
#include <vector>
#include "../common/def.h"
#include "../ds/Object.h"
#include "munkres.h"

using namespace std;

std::vector<Object> objects;
std::vector<Query> queries;
HashMap<std::string, int> counter;

template<typename Out>
void split(const std::string &s, char delim, Out result) {
  std::stringstream ss(s);
  std::string item;
  while (std::getline(ss, item, delim)) {
    *(result++) = item;
  }
}

inline bool contain(const std::string &str, const std::string &end) {
  return str.find(end) != std::string::npos;
}

std::vector<std::string> split(const std::string &s, char delim) {
  std::vector<std::string> elems;
  split(s, delim, std::back_inserter(elems));
  return elems;
}

void load_queries(const string &filename, int k, double alpha, double delta,
                  double max_dist) {
  ifstream in(filename);
  PosVal x, y;
//  PosVal x0, y0, x1, y1, x2, y2;
  string s;
  char delim;
  if (contain(filename, "CAN14m")) {
    delim = '?';
  } else {
    delim = ',';
  }
  while (!in.eof()) {
#ifdef EXAMPLE
    in >> y >> x >> s;
    Rectangle rect = {{x, y}, {x, y}};
#else
    in >> y >> x >> s;
    Rectangle rect = {{x, y}, {x, y}};
#endif
    auto words = split(s, delim);
    Query query = {k, alpha, delta, max_dist, rect, words};
    queries.push_back(query);
  }
  in.close();
}

void load_objects(const string &filename) {
  ifstream in(filename);
  PosVal x, y;
  string s;
  char delim;
  if (contain(filename, "CAN14m")) {
    delim = '?';
  } else {
    delim = ',';
  }
  while (!in.eof()) {
    in >> y >> x >> s;
    auto words = split(s, delim);
    for (const auto &word : words) {
      counter[word] += 1;
    }
    // for (const auto &word : words) {
    // length_count += word.size();
    // }
    // token_count += words.size();
    Rectangle rect = {{x, y}, {x, y}};
    objects.push_back({rect, words});
  }
  // cout << "avg tokens: " << (double)token_count / objects.size() << endl;
  // cout << "avg length: " << (double)length_count / token_count << endl;
  in.close();
}

int slow_cal_ed(const std::string &s1, const std::string &s2) {
  int m = s1.length();
  int n = s2.length();
  int dp[m + 1][n + 1];
  for (int i = 0; i <= m; i++) {
    for (int j = 0; j <= n; j++) {
      if (i == 0)
        dp[i][j] = j;  // Min. operations = j
      else if (j == 0)
        dp[i][j] = i;  // Min. operations = i
      else if (s1[i - 1] == s2[j - 1])
        dp[i][j] = dp[i - 1][j - 1];
      else
        dp[i][j] = 1 + min({dp[i][j - 1],        // Insert
                            dp[i - 1][j],        // Remove
                            dp[i - 1][j - 1]});  // Replace
    }
  }
  return dp[m][n];
}

int cal_ed(const std::string &s1, const std::string &s2, int THRESHOLD) {
  //  cout << s1 << " " << s2 << " " << THRESHOLD << endl;
  int xlen = s1.length();
  int ylen = s2.length();
  if (xlen > ylen + THRESHOLD || ylen > xlen + THRESHOLD) {
    return THRESHOLD + 1;
  }
  if (xlen == 0) {
    return ylen;
  }

  int lam[xlen + 1][2 * THRESHOLD + 1];
  for (int k = 0; k <= THRESHOLD; k++) {
    lam[0][THRESHOLD + k] = k;
  }

  int right = (THRESHOLD + (ylen - xlen)) / 2;
  int left = (THRESHOLD - (ylen - xlen)) / 2;
  for (int i = 1; i <= xlen; i++) {
    bool valid = false;
    if (i <= left) {
      lam[i][THRESHOLD - i] = i;
      valid = true;
    }
    for (int j = (i - left >= 1 ? i - left : 1);
         j <= (i + right <= ylen ? i + right : ylen); j++) {
      if (s1[i - 1] == s2[j - 1])
        lam[i][j - i + THRESHOLD] = lam[i - 1][j - i + THRESHOLD];
      else
        lam[i][j - i + THRESHOLD] =
            min({lam[i - 1][j - i + THRESHOLD],
                 j - 1 >= i - left ? lam[i][j - i + THRESHOLD - 1] : THRESHOLD,
                 j + 1 <= i + right ? lam[i - 1][j - i + THRESHOLD + 1]
                                    : THRESHOLD}) +
                1;
      if (abs(xlen - ylen - i + j) + lam[i][j - i + THRESHOLD] <= THRESHOLD)
        valid = true;
    }
    if (!valid) {
      return THRESHOLD + 1;
    }
  }
  return lam[xlen][ylen - xlen + THRESHOLD];
}

double cal_spatial_similarity(const Rectangle &r1, const Rectangle &r2, double max_dist) {
  auto x1 = r1.lb.x, y1 = r1.lb.y, x2 = r2.lb.x, y2 = r2.lb.y;
  double distance = sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
  if (distance > max_dist) {
    return 0;
  }
  return 1 - distance / max_dist;
}

bool contain(const Rectangle &r1, const Rectangle &r2) {
  double x11 = r1.lb.x;
  double y11 = r1.lb.y;
  double x12 = r1.rt.x;
  double y12 = r1.rt.y;
  //
  double x21 = r2.lb.x;
  double y21 = r2.lb.y;
  double x22 = r2.rt.x;
  double y22 = r2.rt.y;
  //
  bool b1 = x21 <= x11 && x12 <= x22;
  bool b2 = y21 <= y11 && y12 <= y22;
  return b1 && b2;
}

bool overlap(const Rectangle &r1, const Rectangle &r2) {
  double x11 = r1.lb.x;
  double y11 = r1.lb.y;
  double x12 = r1.rt.x;
  double y12 = r1.rt.y;
  //
  double x21 = r2.lb.x;
  double y21 = r2.lb.y;
  double x22 = r2.rt.x;
  double y22 = r2.rt.y;
  //
  bool b1 = x21 <= x12 && x11 <= x22;
  bool b2 = y21 <= y12 && y11 <= y22;
  return b1 && b2;
}

void print_words(const std::vector<string> &words) {
  bool first = true;
  for (const string &word : words) {
    if (!first) cout << ",";
    cout << word;
    first = false;
  }
}

int compute_lambda(double c, int l) {
  int res = c == 0 ? 1 : static_cast<int>((1.0 - c) / c * l);
  return res;
}

double estimate_ubs(const Rectangle &q, const Rectangle &r, double max_dist) {
  auto dx = std::max(r.lb.x - q.lb.x, std::max(0.0, q.lb.x - r.rt.x));
  auto dy = std::max(r.lb.y - q.lb.y, std::max(0.0, q.lb.y - r.rt.y));
  auto distance = sqrt(dx * dx + dy * dy);
  if (distance > max_dist) {
    return 0;
  }
  return 1 - distance / max_dist;
}

std::ostream &operator<<(std::ostream &os, const Rectangle &r) {
  os << "(" << r.lb.x << "," << r.lb.y << ") (" << r.rt.x << "," << r.rt.y
     << ")";
  return os;
}
double cal_similarity(double tsim, double ssim, double alpha) {
  return alpha * tsim + (1 - alpha) * ssim;
}
