#ifndef DISJUNCTION_H_
#define DISJUNCTION_H_

#include <vector>
#include <set>
#include <string>

#include "concept.h"

using namespace std;

extern int ORDERING;

const int offset = (1<<10);
const int mask = offset-1;

class Disjunction {
  ConceptID* t;

  void allocate(int n);
  void from_set(const set<ConceptID>&);

public:
  static const Disjunction bottom;

  Disjunction(const Disjunction& rhs);
  Disjunction& operator=(const Disjunction& rhs);
  explicit Disjunction();
  explicit Disjunction(ConceptID id);
  explicit Disjunction(const set<ConceptID>& s);
  Disjunction(ConceptID c, ConceptID d);
  explicit Disjunction(const vector<const Concept*>& v, bool positive);
  ~Disjunction();
  bool operator==(const Disjunction& rhs) const;
  bool operator!=(const Disjunction& rhs) const;
  bool operator<(const Disjunction& rhs) const;
  bool subset(const Disjunction& rhs) const;

  ConceptID* begin() const;
  ConceptID* end() const;
  int size() const;
  ConceptID operator[](int i) const;

  Disjunction resolve(const Disjunction& a, ConceptID h) const;
  Disjunction resolve(const Disjunction& a1, ConceptID h1, const Disjunction &a2, ConceptID h2) const;

//  bool occurs(const unordered_multimap<ConceptID, Disjunction>& m) const;
  struct SizeLess {
      bool operator()(const Disjunction& a, const Disjunction& b) {
	  return a.size() < b.size();
      }
  };
};

inline ConceptID* Disjunction::begin() const {
	return t+1;
}

inline ConceptID* Disjunction::end() const {
	return t+1+size();
}

inline int Disjunction::size() const {
	return (t[0]&mask);
}
	
inline ConceptID Disjunction::operator[](int i) const {
  return t[i+1];
}

#endif /* DISJUNCTION_H_ */
