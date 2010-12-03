#include <iostream>
#include <vector>
#include <set>
#include <algorithm>
#include <cstring>

#include "header.h"
#include "disjunction.h"

const Disjunction Disjunction::bottom = Disjunction();

void Disjunction::allocate(int n) {
	t = new ConceptID[n+1];
	t[0] = n;
}

inline void Disjunction::from_set(const set<ConceptID>& s) {
	allocate(s.size());
	ConceptID* i = t+s.size();
	FOREACH(x, s) {
		*i = *x;
		i--;
	}
}

Disjunction& Disjunction::operator=(const Disjunction& rhs) {
    if ((t[0] & ~mask) == 0)
	delete[] t;
    else
	t[0] -= offset;
    t = rhs.t;
    t[0] += offset;
    return *this;
}

Disjunction::Disjunction(const Disjunction& rhs) {
    t = rhs.t;
    t[0] += offset;
}

Disjunction::~Disjunction() {
    if ((t[0] & ~mask) == 0)
	delete[] t;
    else
	t[0] -= offset;
}

Disjunction::Disjunction() {
	allocate(0);
}


Disjunction::Disjunction(ConceptID id) {
	allocate(1);
	t[1] = id;
}

Disjunction::Disjunction(const set<ConceptID>& s) {
    from_set(s);
}

Disjunction::Disjunction(ConceptID c, ConceptID d) {
    set<ConceptID> s;
    s.insert(c);
    s.insert(d);
    from_set(s);
}

Disjunction::Disjunction(const vector<const Concept*>& v, bool positive) {
    set<ConceptID> s;
    FOREACH(x, v)
	s.insert(positive ? (*x)->positive() : (*x)->negative());
    from_set(s);
}


bool Disjunction::operator==(const Disjunction& rhs) const {
	if (size() != rhs.size())
		return false;
	for (int i = 1; i <= size(); i++) 
		if (t[i] != rhs.t[i])
			return false;
	return true;
}

bool Disjunction::operator!=(const Disjunction& rhs) const {
  return !(*this == rhs);
}

bool Disjunction::operator<(const Disjunction& rhs) const {
	if (size() == rhs.size()) {
		int i = 1;
		while (i <= size() && t[i] == rhs.t[i]) 
			i++;
		if (i <= size())
			return t[i] < rhs.t[i];
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
		} while (t[i] != rhs.t[j++]);
	return true;
}

Disjunction Disjunction::resolve(const Disjunction& a, ConceptID h) const {
  if (a.size() == 1)
      return *this;

  set<ConceptID> s;
  ConceptID* i;
  for (i = a.begin(); *i != h; i++)
      s.insert((ORDERING == 2) ? Concept::annotate(*i) : *i);

  for (i++; i != a.end(); i++)
      if (s.find(Concept::annotate(*i)) == s.end())
          s.insert(*i);

  for (i = begin(); i != end(); i++)
      if (s.find(Concept::annotate(*i)) == s.end())
	  s.insert(*i);

  return Disjunction(s);
}

Disjunction Disjunction::resolve(const Disjunction& a1, ConceptID h1, const Disjunction& a2, ConceptID h2) const {
    if (a1.size() == 1 && a2.size() == 1)
	return *this;

  set<ConceptID> s;
  ConceptID *i, *j;

  for (i = a1.begin(); *i != h1; i++)
      s.insert((ORDERING == 2) ? Concept::annotate(*i) : *i);
  for (j = a2.begin(); *j != h2; j++)
      s.insert((ORDERING == 2) ? Concept::annotate(*j) : *j);

  for (i++; i != a1.end(); i++)
      if (s.find(Concept::annotate(*i)) == s.end())
          s.insert(*i);
  for (j++; j != a2.end(); j++)
      if (s.find(Concept::annotate(*j)) == s.end())
          s.insert(*j);

  for (i = begin(); i != end(); i++)
      if (s.find(Concept::annotate(*i)) == s.end())
	  s.insert(*i);

  return Disjunction(s);
}

/*
bool Disjunction::occurs(const unordered_multimap<ConceptID, Disjunction>& m) const {
  for (int i = 1; i <= size(); i++) 
   EQRANGE(j, m, t[i])
      if (j->second.subset(*this))
		return true;
  return false;
}
*/
