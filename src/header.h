#ifndef HEADER_H_
#define HEADER_H_

#include <tr1/unordered_map>
#include <tr1/unordered_set>

using std::tr1::unordered_set;
using std::tr1::unordered_map;
using std::tr1::unordered_multimap;

#define EQUAL_RANGE(i, m, v) pair<__typeof((m).begin()), __typeof((m).begin())> pp##i = (m).equal_range(v); for(__typeof((m).begin()) i = pp##i.first; i != pp##i.second; i++)
#define EQRANGE(i, m, v) for (__typeof((m).begin()) i = (m).find(v); i != (m).end() && i->first == v; i++)
#define FOREACH(i, c) for(__typeof((c).begin()) i = (c).begin(); i != (c).end(); i++)


#endif /*  HEADER_H_ */
