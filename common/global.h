#ifndef GLOBAL_H
#define GLOBAL_H

#include "../ds/Object.h"
#include "../ds/QGramer.h"

extern vector<Object> objects;
extern vector<Query> queries;
extern HashMap<int, QGramer> qgramers;
extern HashMap<std::string, int> counter;
extern int node_num;

#endif
