#ifndef FACTORY_H_
#define FACTORY_H_

#include <set>
#include <vector>
#include <string>
#include <algorithm>

#include "tracker.h"
#include "role.h"
#include "concept.h"

using namespace std;

class Factory {
  hash_tracker<string, const AtomicRole> role_tracker;
  tracker<const Concept*, const NegationConcept> negation_tracker;
  tracker<vector<const Concept*>, const ConjunctionConcept> conjunction_tracker;
  tracker<vector<const Concept*>, const DisjunctionConcept> disjunction_tracker;
  tracker<pair<const Role*, const Concept*>, const ExistentialConcept> existential_tracker;
  tracker<pair<const Role*, const Concept*>, const UniversalConcept> universal_tracker;
  tracker<pair<int, string>, const DummyConcept> dummy_tracker;
  const TopConcept* top_tracker;
  const BottomConcept* bottom_tracker;

  unordered_map<RoleID, const Role*> role_register;
  unordered_map<ConceptID, const Concept*> concept_register;

  public:
  hash_tracker<string, const AtomicConcept> atomic_tracker;

  Factory();
  ~Factory();

  const Role* role(RoleID);
  const Concept* concept(ConceptID);

  const Role* role(const string& name);
  const AtomicConcept* atomic(const string& name);
  const TopConcept* top();
  const BottomConcept* bottom();
  const NegationConcept* negation(const Concept* c);
  const ConjunctionConcept* conjunction(vector<const Concept*>& v);
  const Concept* improper_conjunction(vector<const Concept*>& v);
  const DisjunctionConcept* disjunction(vector<const Concept*>& v);
  const Concept* improper_disjunction(vector<const Concept*>& v);
  const ExistentialConcept* existential(const Role* r, const Concept* c);
  const UniversalConcept* universal(const Role* r, const Concept* c);
  const DummyConcept* dummy(int id);
  const DummyConcept* dummy(string name);

  vector<const AtomicConcept*> all_atomic_ordered();
};

#endif /* FACTORY_H_ */
