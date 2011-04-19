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

#include <iostream>

#include "ontology.h"
#include "factory.h"

extern Factory factory;

void Ontology::nullary(const Disjunction& a) {
    unary(factory.top()->ID(), a);
}

void Ontology::unary(ConceptID c, const Disjunction& a) {
  unary_axioms.insert(make_pair(c, a));
}

void Ontology::binary(ConceptID c1, ConceptID c2, const Disjunction& a) {
  binary_axioms.insert(make_pair(c1, make_pair(c2, a)));
  binary_axioms.insert(make_pair(c2, make_pair(c1, a)));
  binary_count[c1]++;
  binary_count[c2]++;
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
    if (seen.find(c->ID()) == seen.end()) {
	seen.insert(c->ID());
	return true;
    }
    return false;
}
bool Ontology::PositiveStructuralTransformation::not_seen(const Concept* c) {
    if (seen.find(c->ID()) == seen.end()) {
	seen.insert(c->ID());
	return true;
    }
    return false;
}

void Ontology::NegativeStructuralTransformation::atomic(const AtomicConcept *c) {}
void Ontology::PositiveStructuralTransformation::atomic(const AtomicConcept *c) {}

void Ontology::NegativeStructuralTransformation::top(const TopConcept *c) {
    if (not_seen(c)) 
	ontology->nullary(Disjunction(c->ID())); 
}
void Ontology::PositiveStructuralTransformation::top(const TopConcept *c) {}

void Ontology::NegativeStructuralTransformation::bottom(const BottomConcept *c) {}
void Ontology::PositiveStructuralTransformation::bottom(const BottomConcept *c) { 
    if (not_seen(c))
	ontology->unary(c->ID(), Disjunction()); 
}

void Ontology::NegativeStructuralTransformation::negation(const NegationConcept *c) {
    if (not_seen(c)) {
	ontology->nullary(Disjunction(c->ID(), Concept::concept_decompose(c->concept())));
	c->concept()->accept(*positive);
    }
}
void Ontology::PositiveStructuralTransformation::negation(const NegationConcept *c) { 
    if (not_seen(c)) {
	ontology->binary(Concept::concept_decompose(c), c->concept()->ID(), Disjunction());
	c->concept()->accept(*negative);
    }
}

void Ontology::NegativeStructuralTransformation::conjunction(const ConjunctionConcept *c) {
    if (not_seen(c)) {
	vector<const Concept*> u, q(c->elements());
	set<ConceptID, Concept::DecomposeLess> s;
        for (int i = 0; i < q.size(); i++) {
	    if (q[i]->type() == 'C') {
		FOREACH(x, ((const ConjunctionConcept*) q[i])->elements())
		    q.push_back(*x);
	    } else if (q[i]->type() == 'N') {
		const NegationConcept* x = (const NegationConcept*) q[i];
		s.insert(Concept::concept_decompose(x->concept()));
		x->concept()->accept(*positive);
	    }
	    else if (q[i]->type() == 'U') {
		const UniversalConcept* y = (const UniversalConcept*) q[i];
		const ExistentialConcept* x = factory.existential(y->role(), factory.negation(y->concept()));
		s.insert(Concept::concept_decompose(x));
		x->accept(*positive);
	    }
	    else {
		u.push_back(q[i]);
		q[i]->accept(*this);
	    }
	}
	s.insert(c->ID());

	if (u.empty())
	    ontology->nullary(Disjunction(s));
	else if (u.size() == 1)
	    ontology->unary(u[0]->ID(), Disjunction(s));
	else {
	    const Concept *d = u[0];
	    vector<const Concept*> t;
	    t.push_back(d);
	    for (int i = 1; i < u.size()-1; i++) {
		t.push_back(u[i]);
		const Concept *conj = factory.conjunction(t);
		ontology->binary(d->ID(), u[i]->ID(), Disjunction(conj->ID()));
		d = conj;
	    }
	    ontology->binary(d->ID(), u.back()->ID(), Disjunction(s));
	}
    }
}
void Ontology::PositiveStructuralTransformation::conjunction(const ConjunctionConcept *c) {
    if (not_seen(c)) 
	for (vector<const Concept*>::const_iterator i = c->elements().begin(); i != c->elements().end(); i++) {
	    ontology->unary(Concept::concept_decompose(c), Disjunction(Concept::concept_decompose(*i)));
	    (*i)->accept(*this);
	}
}

void Ontology::NegativeStructuralTransformation::disjunction(const DisjunctionConcept* c) {
    if (not_seen(c))
	for (vector<const Concept*>::const_iterator i = c->elements().begin(); i != c->elements().end(); i++) {
	    ontology->unary((*i)->ID(), Disjunction(c->ID()));
	    (*i)->accept(*this);
	}
}
void Ontology::PositiveStructuralTransformation::disjunction(const DisjunctionConcept* c) {
    if (not_seen(c)) {
	ontology->unary(Concept::concept_decompose(c), Disjunction(c->elements(), true));
	for (vector<const Concept*>::const_iterator i = c->elements().begin(); i != c->elements().end(); i++) 
	    (*i)->accept(*this);
    }
}

void Ontology::NegativeStructuralTransformation::existential(const ExistentialConcept *c) {
    if (not_seen(c)) {
	ontology->negative_existentials.insert(c);
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
	ontology->nullary(Disjunction(c->ID(), Concept::concept_decompose(e)));
	e->accept(*positive);
    }
}

void Ontology::PositiveStructuralTransformation::universal(const UniversalConcept *c) {
    if (not_seen(c)) {
	ontology->positive_universals.insert(c);
	c->concept()->accept(*this); //skip one
    }
}

void Ontology::subsumption(const Concept* c, const Concept* d) {
    if (c->type() == 'B' || d->type() == 'T')
	return;

    if (c->type() == 'T' && d->type() == 'U') {
	const UniversalConcept* u = (const UniversalConcept*) d;
	role_range.insert(make_pair(u->role()->ID(), Disjunction(Concept::concept_decompose(u->concept()))));
	u->concept()->accept(*pos_str);
	c->accept(*neg_str);
	return;
    }
    c->accept(*neg_str);
    d->accept(*pos_str);
    unary(c->ID(), Disjunction(Concept::concept_decompose(d)));
}

void Ontology::disjoint(const Concept* c, const Concept* d) {
    c->accept(*neg_str);
    d->accept(*neg_str);
    binary(c->ID(), d->ID(), Disjunction());
}

void Ontology::transitive_role(const Role* r) {
    transitive_roles.insert(r->ID());
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

    //reduce transitivity for universals
    set<const UniversalConcept *> s;   
    FOREACH(u, positive_universals) 
	FOREACH(i, transitive_roles)
	    if (hierarchy(*i, (*u)->role()->ID())) {
		const Role *t = factory.role(*i);
		const UniversalConcept *c = factory.universal(t, factory.universal(t, (*u)->concept()));
		if (t != (*u)->role())
		    unary(Concept::concept_decompose(*u), Disjunction(Concept::concept_decompose(c)));
		s.insert(c);
	    }
    FOREACH(u, s) {
	unary(Concept::concept_decompose((*u)->concept()), Disjunction(Concept::concept_decompose(*u)));
	positive_universals.insert(*u);
	positive_universals.insert((const UniversalConcept *) (*u)->concept());
    }

    //duals of universals
    FOREACH(u, positive_universals) {
	const ExistentialConcept *e = factory.existential((*u)->role(), factory.negation((*u)->concept()));
	negative_existentials.insert(e);
	binary(Concept::concept_decompose(*u), e->ID(), Disjunction());
    }
		    
    //reduce number of neighbours in binary_axioms
    map<ConceptID, ConceptID> dummy;
    FOREACH(i, binary_count)
	if (i->second > 100) {
	    ConceptID d = factory.dummy(Concept::minimal_ID())->ID();
	    dummy[i->first] = d;
	    EQRANGE(j, binary_axioms, i->first)
		unary(j->second.first, Disjunction(d, j->second.second)); 
	}
    FOREACH(i, binary_axioms)
	if (dummy.find(i->first) != dummy.end() || dummy.find(i->second.first) != dummy.end())
	    binary_axioms.erase(i);
    FOREACH(i, dummy) {
	binary_axioms.insert(make_pair(i->first, make_pair(i->second, Disjunction())));
	binary_axioms.insert(make_pair(i->second, make_pair(i->first, Disjunction())));
    }

    //unfold role hierarchy into existential axioms
    FOREACH(e, negative_existentials)
	FOREACH(r, positive_roles)
	    if (hierarchy(*r, (*e)->role()->ID())) 
		universal_axioms.insert(make_pair(make_pair((*e)->concept()->ID(), *r), (*e)->ID()));
		
    //reduce transitivity for existentials
    FOREACH(e, negative_existentials) {
	RoleID r = (*e)->role()->ID();
	if (transitive_roles.find(r) != transitive_roles.end()) 
	    FOREACH(s, positive_roles)
		if (hierarchy(*s, r))
		    universal_axioms.insert(make_pair(make_pair((*e)->ID(), *s), (*e)->ID()));

	FOREACH(t, transitive_roles)
	    if (*t != r && hierarchy(*t, r)) {
		ConceptID f = factory.existential(factory.role(*t), (*e)->concept())->ID();
		FOREACH(s, positive_roles)
		    if (hierarchy(*s, *t)) {
			universal_axioms.insert(make_pair(make_pair((*e)->concept()->ID(), *s), f));
			universal_axioms.insert(make_pair(make_pair(f, *s), f));
			universal_axioms.insert(make_pair(make_pair(f, *s), (*e)->ID()));
		    }
	    }
    }
}
