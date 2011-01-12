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
