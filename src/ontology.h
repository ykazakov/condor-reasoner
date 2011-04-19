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

#ifndef ONTOLOGY_H_
#define ONTOLOGY_H_

#include <list>
#include <algorithm>
#include <set>
#include <map>

#include "header.h"
#include "tracker.h"
#include "role.h"
#include "concept.h"
#include "disjunction.h"

using namespace std;


class Ontology {

    class NegativeStructuralTransformation;
    class PositiveStructuralTransformation;

    NegativeStructuralTransformation* neg_str;
    PositiveStructuralTransformation* pos_str;

public:

  RoleHierarchy hierarchy;

  unordered_multimap<ConceptID, Disjunction> unary_axioms;
  unordered_multimap<ConceptID, pair<ConceptID, Disjunction> > binary_axioms;
  map<ConceptID, int> binary_count;

  set<const ExistentialConcept*> negative_existentials;
  set<const UniversalConcept*> positive_universals;
  multimap<pair<ConceptID, RoleID>, ConceptID> universal_axioms;  // could try hash_map instead
  set<RoleID> positive_roles;
  set<pair<RoleID, Disjunction> > role_range;
  set<RoleID> transitive_roles;

  void nullary(const Disjunction& a);
  void unary(ConceptID c, const Disjunction& a);
  void binary(ConceptID c1, ConceptID c2, const Disjunction& a);

  void subsumption(const Concept*, const Concept*);
  void disjoint(const Concept*, const Concept*);
  void transitive_role(const Role*);

  Ontology();
  ~Ontology();
    void normalize();
};

#endif /* ONTOLOGY_H_ */
