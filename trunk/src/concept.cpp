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

#include "concept.h"

#include "sstream"

const int offset = 1000000;

const ConceptID Concept::annotated_mask = 1<<27;
const ConceptID Concept::decompose_mask = 1<<28;
const ConceptID Concept::normalize_mask = (1<<25)-1;

ConceptID Concept::max_id = 10*offset;
ConceptID Concept::min_id = offset-1;

ConceptID Concept::maximal_ID() {
    return max_id++;
}
ConceptID Concept::minimal_ID() {
    return min_id--;
}

ConceptID AtomicConcept::next_id = 9*offset;
ConceptID TopConcept::next_id = 8*offset;
ConceptID BottomConcept::next_id = 1*offset;
ConceptID NegationConcept::next_id = 2*offset;
ConceptID ConjunctionConcept::next_id = 3*offset;
ConceptID DisjunctionConcept::next_id = 4*offset;
ConceptID ExistentialConcept::next_id = 7*offset;
ConceptID UniversalConcept::next_id = 6*offset;

ConceptVisitor::~ConceptVisitor() {}

Concept::Concept() {}
Concept::~Concept() {}

void AtomicConcept::accept(ConceptVisitor &visitor) const { 
  visitor.atomic(this); 
}

void TopConcept::accept(ConceptVisitor &visitor) const { 
  visitor.top(this); 
}

void BottomConcept::accept(ConceptVisitor &visitor) const { 
  visitor.bottom(this); 
}

void NegationConcept::accept(ConceptVisitor &visitor) const { 
  visitor.negation(this); 
}

void ConjunctionConcept::accept(ConceptVisitor &visitor) const { 
  visitor.conjunction(this); 
}

void DisjunctionConcept::accept(ConceptVisitor &visitor) const { 
  visitor.disjunction(this); 
}

void ExistentialConcept::accept(ConceptVisitor &visitor) const { 
  visitor.existential(this); 
}

void UniversalConcept::accept(ConceptVisitor &visitor) const { 
  visitor.universal(this); 
}

void DummyConcept::accept(ConceptVisitor &visitor) const { 
  visitor.dummy(this); 
}

string AtomicConcept::to_string() const{ 
  return name; 
}

string TopConcept::to_string() const { 
  return "owl:Thing"; 
}

string BottomConcept::to_string() const { 
  return "owl:Nothing"; 
}

string NegationConcept::to_string() const { 
  return "ObjectComplementOf(" + c->to_string() + ")"; 
}

string ConjunctionConcept::to_string() const { 
  string r("ObjectIntersectionOf("+v[0]->to_string());
  for (unsigned int i = 1; i < v.size(); i++)
    r += " " + v[i]->to_string();
  return r+")";
}

string DisjunctionConcept::to_string() const { 
  string r("ObjectUnionOf("+v[0]->to_string());
  for (unsigned int i = 1; i < v.size(); i++)
    r += " " + v[i]->to_string();
  return r+")";
}

string ExistentialConcept::to_string() const { 
  return "ObjectSomeValuesFrom(" + r->to_string() + " " + c->to_string() + ")"; 
}

string UniversalConcept::to_string() const { 
  return "ObjectAllValuesFrom(" + r->to_string() + " " + c->to_string() + ")"; 
}

string DummyConcept::to_string() const {
    if (name != "")
	return name;

    stringstream ss;
    ss << "Dummy(" << ID() << ")";
    return ss.str();
}


char AtomicConcept::type() const{ 
  return 'A'; 
}

char TopConcept::type() const { 
  return 'T';
}

char BottomConcept::type() const { 
  return 'B';
}

char NegationConcept::type() const { 
  return 'N';
}

char ConjunctionConcept::type() const { 
  return 'C';
}

char DisjunctionConcept::type() const { 
  return 'D';
}

char ExistentialConcept::type() const { 
  return 'E';
}

char UniversalConcept::type() const { 
  return 'U';
}

char DummyConcept::type() const {
    return 'X';
}

ConceptID Concept::concept_decompose(const Concept* c) {
    char t = c->type();
    if (t == 'N' || t == 'C' || t == 'D' || t == 'E' || t == 'U')
	return mark_decompose(c->ID());
    else
	return c->ID();
}

