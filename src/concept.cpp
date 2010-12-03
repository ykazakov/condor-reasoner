#include "concept.h"

const int offset = 10000000;

const ConceptID Concept::annotated_offset = 100*offset;
const ConceptID Concept::negative_offset = 25*offset;

ConceptID AtomicConcept::next_id = 1*offset;
ConceptID TopConcept::next_id = 0*offset;
ConceptID BottomConcept::next_id = 8*offset;
ConceptID NegationConcept::next_id = 7*offset;
ConceptID ConjunctionConcept::next_id = 6*offset;
ConceptID DisjunctionConcept::next_id = 5*offset;
ConceptID ExistentialConcept::next_id = 2*offset;
ConceptID UniversalConcept::next_id = 3*offset;

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
