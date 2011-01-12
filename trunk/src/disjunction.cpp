#include <iostream>
#include <vector>
#include <set>
#include <algorithm>
#include <cstring>

#include "header.h"
#include "disjunction.h"

const Disjunction Disjunction::bottom = Disjunction();

const int Disjunction::has_annotated_mask = 1<<10;
const int Disjunction::size_mask = Disjunction::has_annotated_mask-1;;
const int Disjunction::reuse_offset = 1<<11;
const int Disjunction::reuse_mask = ~(Disjunction::reuse_offset-1);

//extern string write_disjunction(const Disjunction&);

void Disjunction::allocate(int n) {
	t = new ConceptID[n+1];
	t[0] = n;
}

inline void Disjunction::from_set(const set<ConceptID, Concept::DecomposeLess>& s) {
	allocate(s.size());
	ConceptID* i = t;
	FOREACH(x, s) {
	    i++;
	    *i = *x;
	    /*
	    if (Concept::is_annotated(*x))
		t[0] |= has_annotated_mask;
		*/
	}
}

Disjunction& Disjunction::operator=(const Disjunction& rhs) {
    if ((t[0] & reuse_mask) == 0)
	delete[] t;
    else
	t[0] -= reuse_offset;
    t = rhs.t;
    t[0] += reuse_offset;
    return *this;
}

Disjunction::Disjunction(const Disjunction& rhs) {
    t = rhs.t;
    t[0] += reuse_offset;
}

Disjunction::~Disjunction() {
    if ((t[0] & reuse_mask) == 0)
	delete[] t;
    else
	t[0] -= reuse_offset;
}

Disjunction::Disjunction(bool bottom) {
    if (bottom)
	allocate(0);
}


Disjunction::Disjunction(ConceptID id) {
	allocate(1);
	t[1] = id;
	/*
	if (Concept::is_annotated(id))
	    t[0] |= has_annotated_mask;
	    */
}

Disjunction::Disjunction(ConceptID c, ConceptID d) {
    set<ConceptID, Concept::DecomposeLess> s;
    s.insert(c);
    s.insert(d);
    from_set(s);
}

Disjunction::Disjunction(ConceptID c, Disjunction d) {
    set<ConceptID, Concept::DecomposeLess> s;
    FOREACH(i, d)
	s.insert(*i);
    s.insert(c);
    from_set(s);
}

Disjunction::Disjunction(const set<ConceptID, Concept::DecomposeLess>& s) {
    from_set(s);
}

Disjunction::Disjunction(const vector<const Concept*>& v, bool decompose) {
    set<ConceptID, Concept::DecomposeLess> s;
    FOREACH(x, v)
	s.insert(decompose ? Concept::concept_decompose(*x) : (*x)->ID());
    from_set(s);
}

bool Disjunction::operator==(const Disjunction& rhs) const {
	if (size() != rhs.size())
		return false;
	for (int i = 1; i <= size(); i++) 
		if (Concept::clear_decompose(t[i]) != Concept::clear_decompose(rhs.t[i]))
			return false;
	return true;
}

bool Disjunction::operator!=(const Disjunction& rhs) const {
  return !(*this == rhs);
}

bool Disjunction::operator<(const Disjunction& rhs) const {
	if (size() == rhs.size()) {
		int i = 1;
		while (i <= size() && Concept::clear_decompose(t[i]) == Concept::clear_decompose(rhs.t[i])) 
			i++;
		if (i <= size())
			return Concept::clear_decompose(t[i]) < Concept::clear_decompose(rhs.t[i]);
		else
			return false;
	}
	else
		return size() < rhs.size();
}

bool Disjunction::subset(const Disjunction& rhs) const {
	int j = 1;
	for (int i = 1; i <= size(); i++) 
		do {
			if (j > rhs.size()) //|| t[i] > rhs.t[j])
				return false;
		} while (Concept::clear_decompose(t[i]) != Concept::clear_decompose(rhs.t[j++]));
	return true;
}

const int INF = 1<<30;
Disjunction Disjunction::resolve(const Disjunction& a) const {

  if (a.size() == 1)
      return *this;
  Disjunction d(false);
  d.allocate(size()+a.size()-1);

  ConceptID *i = t+1;
  ConceptID *j = a.t+2;
  ConceptID *k = d.t+1;

  while (i != end() || j != a.end()) {
      ConceptID ci = (i == end()) ? INF : Concept::clear_decompose(*i);
      ConceptID cj = (j == a.end()) ? INF : Concept::clear_decompose(*j);
      if (ci <= cj) {
	  *k = *i;
	  k++;
	  i++;
	  if (ci == cj)
	      j++;
      }
      else {
	  *k = *j;
	  k++;
	  j++;
      }
  }

    d.t[0] = k-d.t-1;
    /*
      if (has_annotated() || a.has_annotated())
	  d.t[0] |= has_annotated_mask;
	  */
      return d;
}

Disjunction Disjunction::resolve(const Disjunction& a1, const Disjunction& a2) const {
    if (a1.size() == 1 && a2.size() == 1)
	return *this;

    Disjunction d(false);
    d.allocate(size()+a1.size()+a2.size()-2);

    ConceptID *i = t+1;
    ConceptID *j1 = a1.t+2;
    ConceptID *j2 = a2.t+2;
    ConceptID *k = d.t+1;

    while (i != end() || j1 != a1.end() || j2 != a2.end()) {
	ConceptID ci = (i == end()) ? INF : Concept::clear_decompose(*i); 
	ConceptID cj1 = (j1 == a1.end()) ? INF : Concept::clear_decompose(*j1); 
	ConceptID cj2 = (j2 == a2.end()) ? INF : Concept::clear_decompose(*j2); 

	if (ci <= cj1 && ci <= cj2) {
	    *k = *i;
	    k++;
	    i++;
	    if (ci == cj1)
		j1++;
	    if (ci == cj2)
		j2++;
	}
	else if (cj1 <= cj2) {
	    *k = *j1;
	    k++;
	    j1++;
	    if (cj1 == cj2)
		j2++;
	}
	else {
	    *k = *j2;
	    k++;
	    j2++;
	}
    }

    d.t[0] = k-d.t-1;
    /*
    if (has_annotated() || a1.has_annotated() || a2.has_annotated())
	    d.t[0] |= has_annotated_mask;
	    */
    return d;
}

Disjunction Disjunction::annotate() const {
    if (size() == 0 || Concept::is_annotated(front())) 
	cerr << "Internal error: annotate on empty" << endl;

    Disjunction d(Concept::annotate(Concept::normalize(front())));
    return d.resolve(*this);
}
