#ifndef FORMATTER_H_
#define FORMATTER_H_

#include <iostream>
#include <set>
#include <vector>

#include "header.h"
#include "concept.h"

using namespace std;

class Formatter {
  ostream &output;
  bool consistent;

  int n;
  unordered_map<ConceptID, int> order;
  vector<const AtomicConcept *> concepts;
  vector< vector<int> > super;;
  set<int> bot, top;

public:

  Formatter(ostream &out = cout);
  void init(const vector<const AtomicConcept*>& ord);
  void unsatisfiable(const Concept* x);
  void subsumption(const Concept* x, const AtomicConcept* y);
  void write();
};

#endif /* FORMATTER_H_ */
