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

#ifndef PARSER_H_
#define PARSER_H_

#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <set>
#include <vector>

#include "role.h"
#include "concept.h"
#include "factory.h"
#include "ontology.h"

using namespace std;

extern Ontology ontology;
extern Factory factory;

class Parser {
  set<string> unsupported_axiom, unsupported_constructor;

  int brackets(const string &s);
  vector<string> split(const string &s);
  const Role *read_role(const string &s);
  const Concept *read_concept(const string &s);
  void concept_subsumption(const Concept *c, const Concept *d);
  void disjoint_classes(const Concept *c, const Concept *d);
  void role_inclusion(const Role *r, const Role *s);

public:
  void read(istream &input = cin); // call only once!
};

#endif /* PARSER_H_ */
