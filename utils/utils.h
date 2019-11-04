#ifndef UTILS_H
#define UTILS_H

#include "../common/def.h"
#include "../ds/Object.h"
#include "../ds/InvertedList.h"

// load raw data from file
void load_objects(const string &filename);
void load_queries(const string &filename, int k, double alpha, double delta, double max_dist);

bool overlap(const Rectangle &r1, const Rectangle &r2);
bool contain(const Rectangle &r1, const Rectangle &r2);
int slow_cal_ed(const std::string &s1, const std::string &s2);
int cal_ed(const string &s1, const string &s2, int tau);
double cal_spatial_similarity(const Rectangle &r1, const Rectangle &r2, double max_dist);
double estimate_ubs(const Rectangle &r1, const Rectangle &r2, double max_dist);
void print_words(const std::vector<string> &words);
int compute_lambda(double c, int l);
double cal_similarity(double tsim, double ssim, double alpha);

#endif
