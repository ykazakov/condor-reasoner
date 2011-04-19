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

#ifndef FORMATTER_H_
#define FORMATTER_H_

#include <iostream>
#include <set>
#include <vector>

#include "header.h"
#include "concept.h"

using namespace std;

class Formatter {
  bool consistent;

  int n;
  unordered_map<ConceptID, int> order;
  vector<const AtomicConcept *> concepts;
  vector< vector<int> > super;;
  set<int> bot, top;

public:

  void init(const vector<const AtomicConcept*>& ord);
  void unsatisfiable(const Concept* x);
  void subsumption(const Concept* x, const AtomicConcept* y);
  void write(ostream &output = cout);
};

#endif /* FORMATTER_H_ */
