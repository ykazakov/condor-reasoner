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
  istream &input;
  set<string> unsupported_axiom, unsupported_constructor;

  int brackets(const string &s);
  vector<string> split(const string &s);
  const Role *read_role(const string &s);
  const Concept *read_concept(const string &s);
  void concept_subsumption(const Concept *c, const Concept *d);
  void disjoint_classes(const Concept *c, const Concept *d);
  void role_inclusion(const Role *r, const Role *s);

public:

  Parser(istream &input = cin);
  void read(); // call only once!
};

#endif /* PARSER_H_ */
