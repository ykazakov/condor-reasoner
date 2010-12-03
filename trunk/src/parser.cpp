#include <iostream>
#include <cctype>

#include "parser.h"

bool starts_with(const string& s, const string& p) {
    for (int i = 0; i < p.size(); i++)
	if (i == s.size() || s[i] != p[i])
	    return false;
    return true;
}

void trim_left(string &s) {
    int i = 0;
    while (i < s.size() && s[i] == ' ')
	i++;
    s.erase(0, i);
}

//exception to throw when cannot process a line
class IgnoreLine {
  public:
    string message;
    bool verbose;
    IgnoreLine() : message(""), verbose(false) {}
    IgnoreLine(string m) : message(m), verbose(true) {};
};

class WrongArguments : public IgnoreLine {
  public:
    WrongArguments(string axiom) : IgnoreLine("Wrong number of arguments for " + axiom) {}
};

int Parser::brackets(const string &s) {
  int b = 0;
  for (int i = 0; i < s.size(); i++) {
    if (s[i] == '(')
      b++;
    if (s[i] == ')')
      b--;
  }
  return b;
}


vector<string> Parser::split(const string &s) {
  vector<string> r;

  int i = 0;
  while (i < s.size() && s[i] == ' ')
    i++;

  if (i == s.size())
    throw IgnoreLine("Internal error: empty string to split");

  int j = i;
  while (j < s.size() && s[j] != ' ' && s[j] != '(')
    j++;
  r.push_back(s.substr(i, j-i));

  while (j < s.size() && s[j] == ' ')
    j++;

  if (j < s.size()) {
    if (s[j] != '(')
      throw IgnoreLine("Unexpected character: " + s[j]);

    i = j+1;
    while (true) {
      while (i < s.size() && s[i] == ' ')
	i++;
      if (i == s.size())
	throw IgnoreLine("Unmatched bracket: " + s);
      if (s[i] == ')')
	break;
      j = i;
      int b = 0;
      while (j < s.size()) {
	if (b == 0 && (s[j] == ' ' || s[j] == ')'))
	  break;
	if (s[j] == '(')
	  b++;
	if (s[j] == ')')
	  b--;
	j++;
      }

      if (s[i] == '(')
	r.back() += s.substr(i, j-i);
      else
	r.push_back(s.substr(i, j-i));
      i = j;
    }
    if (r.size() == 1)
      throw IgnoreLine("Empty parantheses: " + s);
  }

  return r;
}


const Role *Parser::read_role(const string &s) {
  vector<string> v = split(s);
  if (v.size() > 1) {

      /*
    if (v[0] == "ObjectInverseOf") {
      if (v.size() != 2)
	throw WrongArguments("ObjectInverseOf");
      return read_role(v[1])->inverse();
    }

    else {
    */
      unsupported_constructor.insert(v[0]);
      throw IgnoreLine();
//    }
  }
  else
    return factory.role(v[0]);
}


const Concept *Parser::read_concept(const string &s) {
  vector<string> v = split(s);
  if (v.size() > 1) {

    if (v[0] == "ObjectComplementOf") {
      if (v.size() != 2)
	throw WrongArguments("ObjectComplementOf");
      return factory.negation(read_concept(v[1]));
    }

    else if (v[0] == "ObjectIntersectionOf") {
      vector<const Concept *> c;
      for (int i = 1; i < v.size(); i++)
	c.push_back(read_concept(v[i]));
      return factory.conjunction(c);
    }

    else if (v[0] == "ObjectUnionOf") {
      vector<const Concept *> c;
      for (int i = 1; i < v.size(); i++)
	c.push_back(read_concept(v[i]));
      return factory.disjunction(c);
    }

    else if (v[0] == "ObjectSomeValuesFrom") {
      if (v.size() != 3)
	throw WrongArguments("ObjectSomeValuesFrom");
      return factory.existential(read_role(v[1]), read_concept(v[2]));
    }

    else if (v[0] == "ObjectAllValuesFrom") {
      if (v.size() != 3)
	throw WrongArguments("ObjectAllValuesFrom");
      return factory.universal(read_role(v[1]), read_concept(v[2]));
    }

    else {
      unsupported_constructor.insert(v[0]);
      throw IgnoreLine();
    }
  }
  else {
    if (v[0] == "owl:Thing")
      return factory.top();
    if (v[0] == "owl:Nothing")
      return factory.bottom();

    return factory.atomic(v[0]);
  }
}

void Parser::concept_subsumption(const Concept *c, const Concept *d) {
  ontology.subsumption(c, d);
}

void Parser::disjoint_classes(const Concept *c, const Concept *d) {
    ontology.disjoint(c, d);
}

void Parser::role_inclusion(const Role *r, const Role *s) {
  ontology.hierarchy.add(r, s);
}

void Parser::read(istream &input) {
  string line;
  int lineno = 0;
  do  {
    getline(input, line); lineno++;
    trim_left(line);
  } while (input && !starts_with(line, "Ontology"));

  if (!input) {
      cerr << "Error: \"Ontology\" not found." << endl;
      exit(0);
  }
  int i = 0;
  while (i < line.size() && line[i] != '(')
      i++;
  if (i == line.size()) {
      cerr << "Error: \"Ontology\" must be followed by '(' on the same line." << endl;
      exit(0);
  }
  line = line.substr(i+1);
  trim_left(line);

//  int line_number = 0;

  while (input && !starts_with(line, ")")) {
    int b = brackets(line);
    int old_lineno = lineno;
    while (input && b) {
	if (b < 0) {
	    cerr << "Error: unmatched \')\' around line " << lineno << "." << endl;
	    exit(0);
	}

      string tmp;
      getline(input, tmp); lineno++;
      b += brackets(tmp);
      line += tmp;
    }
    if (!input) {
	cerr << "Error: unmatched \'(\' around line " << old_lineno << "." << endl;
	exit(0);
    }

//    cerr << ++line_number << endl;


    if (line == "" || line[0] == '<' || starts_with(line, "Annotation") || starts_with(line, "Declaration") || starts_with(line, "//"));	//ignore
    else {
      try {

	vector<string> v = split(line);

	if (v[0] == "SubClassOf") {
	  if (v.size() != 3)
	    throw WrongArguments("SubClassOf");
	  concept_subsumption(read_concept(v[1]), read_concept(v[2]));
	}

	else if (v[0] == "EquivalentClasses") {
	  if (v.size() < 3)
	    throw WrongArguments("EquivalentClasses");

	  vector<const Concept *> c;
	  for (int i = 1; i < v.size(); i++)
	    c.push_back(read_concept(v[i]));
	  for (int i = 1; i < c.size(); i++) {
	      concept_subsumption(c[0], c[i]);
	      concept_subsumption(c[i], c[0]);
	    }
	}

	else if (v[0] == "DisjointClasses") {
	  if (v.size() < 3)
	    throw WrongArguments("DisjointClasses");

	  vector<const Concept *> c;
	  for (int i = 1; i < v.size(); i++)
	    c.push_back(read_concept(v[i]));
	  for (int i = 0; i < c.size(); i++)
	    for (int j = 0; j < i; j++)
		disjoint_classes(c[i], c[j]);
	}

	else if (v[0] == "SubObjectPropertyOf") {
	  if (v.size() != 3)
	    throw WrongArguments("SubObjectPropertyOf");
	  const Role *r = read_role(v[1]);
	  const Role *s = read_role(v[2]);
	  role_inclusion(r, s);
	}

	else if (v[0] == "EquivalentObjectProperties") {
	  if (v.size() < 3)
	    throw WrongArguments("EquivalentObjectProperties");
	  vector<const Role *> r;
	  for (int i = 1; i < v.size(); i++)
	    r.push_back(read_role(v[i]));
	  for (int i = 0; i < r.size(); i++)
	    for (int j = 0; j < r.size(); j++)
	      role_inclusion(r[i], r[j]);
	}

	/*
	else if (v[0] == "InverseObjectProperties") {
	  if (v.size() != 3)
	    throw WrongArguments("InverseObjectProperties");
	  const Role *r = read_role(v[1]);
	  const Role *s = read_role(v[2]);
	  role_inclusion(r, s->inverse());
	  role_inclusion(s->inverse(), r);
	}
	*/

	else if (v[0] == "ObjectPropertyDomain") {
	    if (v.size() != 3)
		throw WrongArguments("ObjectPropertyDomain");
	    const Role* r = read_role(v[1]);
	    const Concept* c = read_concept(v[2]);
	    concept_subsumption(factory.existential(r, factory.top()), c);
	}

	else if (v[0] == "ObjectPropertyRange") {
	    if (v.size() != 3)
		throw WrongArguments("ObjectPropertyRange");
	    const Role* r = read_role(v[1]);
	    const Concept* c = read_concept(v[2]);
	    concept_subsumption(factory.top(), factory.universal(r, c));
	}

	else
	  unsupported_axiom.insert(v[0]);
      }
      catch (IgnoreLine exc) {
	if (exc.verbose)
	  cerr << exc.message << " around line " << lineno << ":" << endl << line << endl << endl;
      }
    }
    getline(input, line); lineno++;
    trim_left(line);
  }

  for (set<string>::iterator i = unsupported_axiom.begin(); i != unsupported_axiom.end(); i++)
    cerr << "Unsupported axiom: " << *i << endl;
  for (set<string>::iterator i = unsupported_constructor.begin(); i != unsupported_constructor.end(); i++)
    cerr << "Unsupported constructor: " << *i << endl;
}
