#ifndef DISJUNCTION_H_
#define DISJUNCTION_H_

#include <vector>
#include <set>
#include <string>

#include "concept.h"

using namespace std;

extern int ORDERING;


class Disjunction {
    static const int size_mask;
    static const int has_annotated_mask;
    static const int reuse_offset;
    static const int reuse_mask;
  ConceptID* t;

  void allocate(int n);
  void from_set(const set<ConceptID, Concept::DecomposeLess>&);

public:
  static const Disjunction bottom;

  Disjunction(const Disjunction& rhs);
  Disjunction& operator=(const Disjunction& rhs);
  explicit Disjunction(bool bottom = true);
  explicit Disjunction(ConceptID id);
  explicit Disjunction(const set<ConceptID, Concept::DecomposeLess>& s);
  Disjunction(ConceptID c, ConceptID d);
  Disjunction(ConceptID c, Disjunction d);
  explicit Disjunction(const vector<const Concept*>& v, bool positive);
  ~Disjunction();
  bool operator==(const Disjunction& rhs) const;
  bool operator!=(const Disjunction& rhs) const;
  bool operator<(const Disjunction& rhs) const;
  bool subset(const Disjunction& rhs) const;

  ConceptID* begin() const;
  ConceptID* end() const;
  int size() const;
  bool has_annotated() const;
  ConceptID operator[](int i) const;
  ConceptID front() const;
  ConceptID back() const;

  Disjunction resolve(const Disjunction& a) const;
  Disjunction resolve(const Disjunction& a1, const Disjunction &a2) const;
  Disjunction annotate() const;

//  bool occurs(const unordered_multimap<ConceptID, Disjunction>& m) const;
  struct SizeLess {
      bool operator()(const Disjunction& a, const Disjunction& b) const {
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
	return (t[0]&size_mask);
}

/*
inline bool Disjunction::has_annotated() const {
    return (t[0]&has_annotated_mask) != 0;
}
*/

inline ConceptID Disjunction::operator[](int i) const {
  return t[i+1];
}

inline ConceptID Disjunction::front() const {
    return t[1];
}

inline ConceptID Disjunction::back() const {
    return t[size()];
}
#endif /* DISJUNCTION_H_ */
