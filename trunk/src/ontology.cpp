#include <iostream>

#include "ontology.h"
#include "factory.h"

extern Factory factory;

void Ontology::nullary(const Disjunction& a) {
    unary(factory.top()->positive(), a);
}

void Ontology::unary(ConceptID c, const Disjunction& a) {
  unary_axioms.insert(make_pair(c, a));
}

void Ontology::binary(ConceptID c1, ConceptID c2, const Disjunction& a) {
  binary_axioms.insert(make_pair(c1, make_pair(c2, a)));
  binary_axioms.insert(make_pair(c2, make_pair(c1, a)));
}

class Ontology::NegativeStructuralTransformation : public ConceptVisitor {
  Ontology* ontology;
  unordered_set<ConceptID> seen;

  virtual void atomic(const AtomicConcept *c);
  virtual void top(const TopConcept *c);
  virtual void bottom(const BottomConcept *c);
  virtual void negation(const NegationConcept *c);
  virtual void conjunction(const ConjunctionConcept *c);
  virtual void disjunction(const DisjunctionConcept *c);
  virtual void existential(const ExistentialConcept *c);
  virtual void universal(const UniversalConcept *c);


  public:
  PositiveStructuralTransformation* positive;

  NegativeStructuralTransformation(Ontology *ontology) : ontology(ontology) {}
  virtual ~NegativeStructuralTransformation() {}
  bool not_seen(const Concept *c);
};

class Ontology::PositiveStructuralTransformation : public ConceptVisitor {
  Ontology* ontology;
  unordered_set<ConceptID> seen; 

  virtual void atomic(const AtomicConcept *c);
  virtual void top(const TopConcept *c);
  virtual void bottom(const BottomConcept *c);
  virtual void negation(const NegationConcept *c);
  virtual void conjunction(const ConjunctionConcept *c);
  virtual void disjunction(const DisjunctionConcept *c);
  virtual void existential(const ExistentialConcept *c);
  virtual void universal(const UniversalConcept *c);


  public:
  NegativeStructuralTransformation* negative;

  PositiveStructuralTransformation(Ontology *ontology) : ontology(ontology) {}
  virtual ~PositiveStructuralTransformation() {}
  bool not_seen(const Concept *c);
};

bool Ontology::NegativeStructuralTransformation::not_seen(const Concept* c) {
    if (seen.find(c->negative()) == seen.end()) {
	seen.insert(c->negative());
	return true;
    }
    return false;
}
bool Ontology::PositiveStructuralTransformation::not_seen(const Concept* c) {
    if (seen.find(c->positive()) == seen.end()) {
	seen.insert(c->positive());
	return true;
    }
    return false;
}

void Ontology::NegativeStructuralTransformation::atomic(const AtomicConcept *c) {}
void Ontology::PositiveStructuralTransformation::atomic(const AtomicConcept *c) {}

void Ontology::NegativeStructuralTransformation::top(const TopConcept *c) {}
void Ontology::PositiveStructuralTransformation::top(const TopConcept *c) {}

void Ontology::NegativeStructuralTransformation::bottom(const BottomConcept *c) {}
void Ontology::PositiveStructuralTransformation::bottom(const BottomConcept *c) { 
    if (not_seen(c))
	ontology->unary(c->positive(), Disjunction()); 
}

void Ontology::NegativeStructuralTransformation::negation(const NegationConcept *c) {
    if (not_seen(c)) {
	ontology->nullary(Disjunction(c->positive(), c->concept()->negative()));
	c->concept()->accept(*positive);
    }
}
void Ontology::PositiveStructuralTransformation::negation(const NegationConcept *c) { 
    if (not_seen(c)) {
	ontology->binary(c->positive(), c->concept()->negative(), Disjunction());
	c->concept()->accept(*negative);
    }
}

void Ontology::NegativeStructuralTransformation::conjunction(const ConjunctionConcept *c) {
    if (not_seen(c)) {
	vector<const Concept*> u, q(c->elements());
	set<ConceptID> s;
        for (int i = 0; i < q.size(); i++) {
	    if (q[i]->type() == 'C') {
		FOREACH(x, ((const ConjunctionConcept*) q[i])->elements())
		    q.push_back(*x);
	    } else if (q[i]->type() == 'N') {
		const NegationConcept* x = (const NegationConcept*) q[i];
		s.insert(x->concept()->positive());
		x->concept()->accept(*positive);
	    }
	    else if (q[i]->type() == 'U') {
		const UniversalConcept* y = (const UniversalConcept*) q[i];
		const ExistentialConcept* x = factory.existential(y->role(), factory.negation(y->concept()));
		s.insert(x->positive());
		x->accept(*positive);
	    }
	    else {
		u.push_back(q[i]);
		q[i]->accept(*this);
	    }
	}
	s.insert(c->negative());

	if (u.empty())
	    ontology->nullary(Disjunction(s));
	else if (u.size() == 1)
	    ontology->unary(u[0]->negative(), Disjunction(s));
	else {
	    const Concept *d = u[0];
	    vector<const Concept*> t;
	    t.push_back(d);
	    for (int i = 1; i < u.size()-1; i++) {
		t.push_back(u[i]);
		const Concept *conj = factory.conjunction(t);
		ontology->binary(d->negative(), u[i]->negative(), Disjunction(conj->negative()));
		d = conj;
	    }
	    ontology->binary(d->negative(), u.back()->negative(), Disjunction(s));
	}
    }
}
void Ontology::PositiveStructuralTransformation::conjunction(const ConjunctionConcept *c) {
    if (not_seen(c)) 
	for (vector<const Concept*>::const_iterator i = c->elements().begin(); i != c->elements().end(); i++) {
	    ontology->unary(c->positive(), Disjunction((*i)->positive()));
	    (*i)->accept(*this);
	}
}

void Ontology::NegativeStructuralTransformation::disjunction(const DisjunctionConcept* c) {
    if (not_seen(c))
	for (vector<const Concept*>::const_iterator i = c->elements().begin(); i != c->elements().end(); i++) {
	    ontology->unary((*i)->negative(), Disjunction(c->negative()));
	    (*i)->accept(*this);
	}
}
void Ontology::PositiveStructuralTransformation::disjunction(const DisjunctionConcept* c) {
    if (not_seen(c)) {
	ontology->unary(c->positive(), Disjunction(c->elements(), true));
	for (vector<const Concept*>::const_iterator i = c->elements().begin(); i != c->elements().end(); i++) 
	    (*i)->accept(*this);
    }
}

void Ontology::NegativeStructuralTransformation::existential(const ExistentialConcept *c) {
    if (not_seen(c)) {
	ontology->universal_axioms.insert(make_pair(make_pair(c->concept()->negative(), c->role()->ID()), c->negative()));
	c->concept()->accept(*this);
    }
}
void Ontology::PositiveStructuralTransformation::existential(const ExistentialConcept *c) {
    if (not_seen(c)) {
	ontology->positive_roles.insert(c->role()->ID());
	c->concept()->accept(*this);
    }
}

void Ontology::NegativeStructuralTransformation::universal(const UniversalConcept *c) {
    if (not_seen(c)) {
	const ExistentialConcept* e = factory.existential(c->role(), factory.negation(c->concept()));
	ontology->nullary(Disjunction(c->negative(), e->positive()));
	e->accept(*positive);
    }
}

void Ontology::PositiveStructuralTransformation::universal(const UniversalConcept *c) {
    if (not_seen(c)) {
	const ExistentialConcept* e = factory.existential(c->role(), factory.negation(c->concept()));
	ontology->binary(c->positive(), e->negative(), Disjunction());
	if (negative->not_seen(e))
	    ontology->universal_axioms.insert(make_pair(make_pair(e->concept()->negative(), e->role()->ID()), e->negative()));
	c->concept()->accept(*this); //skip one
    }
}

void Ontology::subsumption(const Concept* c, const Concept* d) {
    if (c->type() == 'B' || d->type() == 'T')
	return;

    if (c->type() == 'T' && d->type() == 'U') {
	const UniversalConcept* u = (const UniversalConcept*) d;
	role_range.insert(make_pair(u->role()->ID(), Disjunction(u->concept()->positive())));
	u->concept()->accept(*pos_str);
	return;
    }
    c->accept(*neg_str);
    d->accept(*pos_str);
    unary(c->negative(), Disjunction(d->positive()));
}

void Ontology::disjoint(const Concept* c, const Concept* d) {
    c->accept(*neg_str);
    d->accept(*neg_str);
    binary(c->negative(), d->negative(), Disjunction());
}

Ontology::Ontology() {
    pos_str = new PositiveStructuralTransformation(this);
    neg_str = new NegativeStructuralTransformation(this);
    pos_str->negative = neg_str;
    neg_str->positive = pos_str;
}

Ontology::~Ontology() {
    delete pos_str;
    delete neg_str;
}

void Ontology::normalize() {
    hierarchy.closure();

    vector<pair<pair<ConceptID, RoleID>, ConceptID> > v; 
    FOREACH(a, universal_axioms)
	FOREACH(r, positive_roles)
	    if (hierarchy(*r, a->first.second))
		v.push_back(make_pair(make_pair(a->first.first, *r), a->second));
    FOREACH(a, v)
	universal_axioms.insert(*a);
}
