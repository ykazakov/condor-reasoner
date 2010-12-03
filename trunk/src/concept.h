#ifndef CONCEPT_H_
#define CONCEPT_H_

#include <string>
#include <vector>

#include "role.h"

using namespace std;

typedef int ConceptID;

class ConceptVisitor;

class Concept {
  protected:
  static const ConceptID annotated_offset, negative_offset;
  ConceptID neg, pos;

  public:
  Concept();
  virtual ~Concept() = 0;

  ConceptID positive() const { return pos; }
  ConceptID negative() const { return neg; }

  virtual string to_string() const = 0;
  virtual char type() const = 0;
  virtual void accept (ConceptVisitor& visitor) const = 0;

  static ConceptID normalize(ConceptID id);
  static ConceptID annotate(ConceptID id);
  static bool plain(ConceptID id);
  static bool annotated(ConceptID id);
};

class AtomicConcept : public Concept {
  const string name;
  static ConceptID next_id;

  public:
  virtual ~AtomicConcept() {}
  explicit AtomicConcept(const string& name) : name(name) { pos = neg = next_id++; }

  virtual string to_string() const;
  virtual char type() const;
  virtual void accept(ConceptVisitor &visitor) const;

  struct AlphaLess {
    bool operator()(const AtomicConcept *rhs, const AtomicConcept *lhs) const {
      return rhs->name < lhs->name;
    }
  };
};

class TopConcept : public Concept {
  static ConceptID next_id;
  public:
  virtual ~TopConcept() {}
  explicit TopConcept() { pos = neg = next_id++; }
  virtual string to_string() const;
  virtual char type() const;
  virtual void accept(ConceptVisitor &visitor) const;
};

class BottomConcept : public Concept {
  static ConceptID next_id;
  public:
  virtual ~BottomConcept() {}
  explicit BottomConcept() { pos = neg = next_id++; }
  virtual string to_string() const;
  virtual char type() const;
  virtual void accept(ConceptVisitor &visitor) const;
};

class NegationConcept : public Concept {
  const Concept *c;
  static ConceptID next_id;

  public:
  virtual ~NegationConcept() {}
  explicit NegationConcept(const Concept *c) : c(c) { pos = neg = next_id++; }
  const Concept *concept() const { return c; };

  virtual string to_string() const;
  virtual char type() const;
  virtual void accept(ConceptVisitor &visitor) const;
};

class ConjunctionConcept : public Concept {
  vector<const Concept *> v;
  static ConceptID next_id;

  public:
  virtual ~ConjunctionConcept() {}
  explicit ConjunctionConcept(const vector<const Concept *> &v) : v(v) { pos = neg = next_id++; }
  const vector<const Concept *> &elements() const { return v; }

  virtual string to_string() const;
  virtual char type() const;
  virtual void accept(ConceptVisitor &visitor) const;
};

class DisjunctionConcept : public Concept {
  vector<const Concept *> v;
  static ConceptID next_id;

  public:
  virtual ~DisjunctionConcept() {}
  explicit DisjunctionConcept(const vector<const Concept *> &v) : v(v) { pos = neg = next_id++; }
  const vector<const Concept *> &elements() const { return v; }

  virtual string to_string() const;
  virtual char type() const;
  virtual void accept(ConceptVisitor &visitor) const;
};

class ExistentialConcept : public Concept {
  const Role *r;
  const Concept  *c;
  static ConceptID next_id;

  public:
  virtual ~ExistentialConcept() {}
  explicit ExistentialConcept(const pair<const Role *, const Concept *> &p) : r(p.first), c(p.second) { pos = next_id++; neg = pos + negative_offset; }
  const Role *role() const { return r; }
  const Concept *concept() const { return c; }

  virtual string to_string() const;
  virtual char type() const;
  virtual void accept(ConceptVisitor &visitor) const;
};

class UniversalConcept : public Concept {
  const Role *r;
  const Concept *c;
  static ConceptID next_id;

  public:
  virtual ~UniversalConcept() {}
  explicit UniversalConcept(const pair<const Role *, const Concept *> &p) : r(p.first), c(p.second) { pos = next_id++; neg = pos + negative_offset; }
  const Role *role() const { return r; }
  const Concept *concept() const { return c; }

  virtual string to_string() const;
  virtual char type() const;
  virtual void accept(ConceptVisitor &visitor) const;
};

class ConceptVisitor {
  virtual void atomic(const AtomicConcept *c) = 0;
  virtual void top(const TopConcept *c) = 0;
  virtual void bottom(const BottomConcept *c) = 0;
  virtual void negation(const NegationConcept *c) = 0;
  virtual void conjunction(const ConjunctionConcept *c) = 0;
  virtual void disjunction(const DisjunctionConcept *c) = 0;
  virtual void existential(const ExistentialConcept *c) = 0;
  virtual void universal(const UniversalConcept *c) = 0;

  public:
    virtual ~ConceptVisitor() = 0;

  friend class AtomicConcept;
  friend class TopConcept;
  friend class BottomConcept;
  friend class NegationConcept;
  friend class ConjunctionConcept;
  friend class DisjunctionConcept;
  friend class ExistentialConcept;
  friend class UniversalConcept;
};

inline ConceptID Concept::normalize(ConceptID id) {
  if (id < annotated_offset)
    return id;
  else
    return id-annotated_offset;
}

inline ConceptID Concept::annotate(ConceptID id) {
	if (id < annotated_offset)
		return id+annotated_offset;
	else
		return id;
}

inline bool Concept::plain(ConceptID id) {
  return (id < annotated_offset);
}

inline bool Concept::annotated(ConceptID id) {
  return (id >= annotated_offset);
}

#endif /* CONCEPT_H_ */
