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
  static const ConceptID annotated_mask;
  static const ConceptID decompose_mask;
  static const ConceptID normalize_mask;

  static ConceptID max_id, min_id;
  ConceptID id;

  public:
  Concept();
  virtual ~Concept() = 0;

  ConceptID ID() const { return id; }

  virtual string to_string() const = 0;
  virtual char type() const = 0;
  virtual void accept (ConceptVisitor& visitor) const = 0;

  static ConceptID maximal_ID();
  static ConceptID minimal_ID();

  static ConceptID normalize(ConceptID id);
  static ConceptID annotate(ConceptID id);
  static bool is_annotated(ConceptID id);
  static ConceptID clear_decompose(ConceptID id);
  static ConceptID mark_decompose(ConceptID id);
  static ConceptID concept_decompose(const Concept* c); 
  static bool decompose(ConceptID id);

  struct DecomposeLess {
      bool operator()(ConceptID lhs, ConceptID rhs) const {
	  return (clear_decompose(lhs)) < (clear_decompose(rhs));
      }
  };
};

class AtomicConcept : public Concept {
  const string name;
  static ConceptID next_id;

  public:
  virtual ~AtomicConcept() {}
  explicit AtomicConcept(const string& name) : name(name) { id = next_id++; }

  virtual string to_string() const;
  virtual char type() const;
  virtual void accept(ConceptVisitor &visitor) const;

  struct AlphaLess {
    bool operator()(const AtomicConcept *lhs, const AtomicConcept *rhs) const {
      return lhs->name < rhs->name;
    }
  };
};

class TopConcept : public Concept {
  static ConceptID next_id;
  public:
  virtual ~TopConcept() {}
  explicit TopConcept() { id = next_id++; }
  virtual string to_string() const;
  virtual char type() const;
  virtual void accept(ConceptVisitor &visitor) const;
};

class BottomConcept : public Concept {
  static ConceptID next_id;
  public:
  virtual ~BottomConcept() {}
  explicit BottomConcept() { id = next_id++; }
  virtual string to_string() const;
  virtual char type() const;
  virtual void accept(ConceptVisitor &visitor) const;
};

class NegationConcept : public Concept {
  const Concept *c;
  static ConceptID next_id;

  public:
  virtual ~NegationConcept() {}
  explicit NegationConcept(const Concept *c) : c(c) { id = next_id++; }
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
  explicit ConjunctionConcept(const vector<const Concept *> &v) : v(v) { id = next_id++; }
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
  explicit DisjunctionConcept(const vector<const Concept *> &v) : v(v) { id = next_id++; }
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
  explicit ExistentialConcept(const pair<const Role *, const Concept *> &p) : r(p.first), c(p.second) { id = next_id++; }
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
  explicit UniversalConcept(const pair<const Role *, const Concept *> &p) : r(p.first), c(p.second) { id = next_id++; }
  const Role *role() const { return r; }
  const Concept *concept() const { return c; }

  virtual string to_string() const;
  virtual char type() const;
  virtual void accept(ConceptVisitor &visitor) const;
};

class DummyConcept : public Concept {
   string name;

   public:
    virtual ~DummyConcept() {}
    explicit DummyConcept(pair<int, string> p) { id = p.first; name = p.second; }
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
  virtual void dummy(const DummyConcept *c) {}

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
  friend class DummyConcept;
};

inline ConceptID Concept::normalize(ConceptID id) {
    return (id & normalize_mask);
}

inline ConceptID Concept::annotate(ConceptID id) {
    return (id | annotated_mask);
}

inline bool Concept::is_annotated(ConceptID id) {
  return (id & annotated_mask) != 0;
}

inline ConceptID Concept::mark_decompose(ConceptID id) {
    return (id | decompose_mask);
}


inline ConceptID Concept::clear_decompose(ConceptID id) {
    return (id & ~decompose_mask);
}

inline bool Concept::decompose(ConceptID id) {
    return (id & decompose_mask) != 0;
}

#endif /* CONCEPT_H_ */
