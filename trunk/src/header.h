/* 
 * Copyright (c) 2010 Frantisek Simancik
 * <frantisek.simancik@comlab.ox.ac.uk>, Yevgeny Kazakov
 * <yevgeny.kazakov@comlab.ox.ac.uk> and University of Oxford
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HEADER_H_
#define HEADER_H_

#include <tr1/unordered_map>
#include <tr1/unordered_set>

using std::tr1::unordered_set;
using std::tr1::unordered_map;
using std::tr1::unordered_multimap;

#define EQUAL_RANGE(i, m, v) pair<__typeof((m).begin()), __typeof((m).begin())> pp##i = (m).equal_range(v); for(__typeof((m).begin()) i = pp##i.first; i != pp##i.second; i++)
#define EQRANGE(i, m, v) for (__typeof((m).begin()) i = (m).find(v); i != (m).end() && i->first == (v); i++)
#define FOREACH(i, c) for(__typeof((c).begin()) i = (c).begin(); i != (c).end(); i++)


#endif /*  HEADER_H_ */
