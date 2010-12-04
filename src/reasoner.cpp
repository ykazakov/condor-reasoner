#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <set>
#include <map>
#include <vector>
#include <list>
#include <deque>
#include <queue>
#include <algorithm>

#include "header.h"
#include "tracker.h"
#include "role.h"
#include "concept.h"
#include "disjunction.h"
#include "factory.h"
#include "ontology.h"
#include "parser.h"
#include "formatter.h"

using namespace std;

int ORDERING = 1; // possible values are 0,1,2
bool VERBOSE = false;
bool OUTPUT = true;

string write_disjunction(const Disjunction& d) {
    stringstream ss;
    FOREACH(x, d) {
	ConceptID id = Concept::normalize(*x);
	const Concept* c = factory.concept(id);
	ss << " " << c->to_string();
	if (c->positive() == id)
	    ss << "+";
	else
	    ss << "-";
	if (Concept::annotated(*x))
	    ss << "*";
    }
    return ss.str();
}


Factory factory;
Ontology ontology;
Formatter formatter;

class Pusher {
    vector<pair<map<pair<ConceptID, RoleID>, ConceptID>::iterator, map<pair<ConceptID, RoleID>, ConceptID>::iterator> > bounds; 
    vector<map<pair<ConceptID, RoleID>, ConceptID>::iterator> v;
    int n, i;
    Disjunction d;

    void build() {
	set<ConceptID> s;
	for (int j = 0; j < n; j++)
	    s.insert(v[j]->second);
	d = Disjunction(s);
    }

    public:

    Pusher(const Disjunction& ax, RoleID r, int nn = -1) {
	if (nn == -1)
	    n = ax.size();
	else
	    n = nn;
	bounds.reserve(n);
	v.reserve(n);
	for (i = 0; i < n; i++) {
	    bounds.push_back(ontology.universal_axioms.equal_range(make_pair(Concept::normalize(ax[i]), r)));
	    if (bounds[i].first == bounds[i].second) {
		i = -1;
		cerr << "Internal error: axiom cannot be pushed" << endl;
		return;
	    }
	    v.push_back(bounds[i].first);
	}
	build();
    }

    bool next() {
	i--;
	while (i >= 0 && i != n)
	    if (v[i] == bounds[i].second) {
		v[i] = bounds[i].first;
		i++;
	    }
	    else {
		v[i]++;
		if (v[i] == bounds[i].second)
		    i--;
		else
		    i++;
	    }

	if (i == n) {
	    build();
	    return true;
	}
	else
	    return false;
    }

    Disjunction disjunction() {
	return d;
    }
};

class Context {
    public:
	static bool UNLINK;


	const Concept* core;
	ConceptID inexist;
	RoleID inrole;
	bool top;
	bool satisfiable;
	bool processing;

	unordered_multimap<ConceptID, Disjunction> axiom_index;
	unordered_multimap<ConceptID, Disjunction> occurs;
	set<Disjunction> topush;
	multiset<Disjunction, Disjunction::SizeLess> todo;
	set<pair<RoleID, Context*> > forward_links;
	set<Context*> backward_links;
	set<pair<RoleID, Disjunction> > universals;

	int axioms;

	//  public:
	explicit Context(pair<RoleID, ConceptID>);
	~Context();
	void unlink();

	void add(const Disjunction& a); //things to consider before pushing into todo
	void link(RoleID r, Context* c);
	int process();
	void resolve_unary(const Disjunction& d, ConceptID head);
	void resolve_binary(const Disjunction& d, ConceptID head, Context*);
	void push(const Disjunction& d);
	bool not_occurs(const  Disjunction& ax, bool add = true);
};

bool Context::UNLINK = true;

tracker<pair<RoleID, ConceptID>, Context> context_tracker; //could try hash_tracker
vector<Context* > top_contexts;
vector<list<Context*> > all_contexts;
list<Context*> active;

Context::Context(pair<RoleID, ConceptID> rc) : core(factory.concept(rc.second)), inrole(rc.first), top(core->type() == 'T'), satisfiable(true), processing(false) { 
    todo.insert(Disjunction(rc.second));
    if (inrole) {
	inexist = factory.existential(factory.role(inrole), core)->positive();
	if (top)
	FOREACH(range, ontology.role_range)
	    if (ontology.hierarchy(inrole, range->first))
		todo.insert(range->second);
    }
    active.push_back(this);
    all_contexts[inrole].push_back(this);
    axioms = 0;
}

Context::~Context() { 
    if (UNLINK) 
	unlink(); 
}

void Context::unlink() {
    //unlink and free
    FOREACH(i, forward_links)
	i->second->backward_links.erase(this);
    FOREACH(i, backward_links)
	(*i)->forward_links.erase(make_pair(inrole, this));
}


void Context::add(const Disjunction& a) {
    if (satisfiable) {
	if (!inrole) {
	    int annot  = 0;
	    for (ConceptID *i = a.begin(); i != a.end(); i++)
		if (Concept::annotated(*i)) 
		    annot++;
		else
		    break;

	    if (annot > 1) {
		return;
	    }
	}


	todo.insert(a);
	if (!processing) {
	    if (top)
		active.push_front(this);
	    else
		active.push_back(this);
	}
    }
}

//remove duplicates here
void Context::link(RoleID r, Context* c) {
    forward_links.insert(make_pair(r, c));
    c->backward_links.insert(this);
}

void Context::resolve_unary(const Disjunction& ax, ConceptID head) {
    EQRANGE(i, ontology.unary_axioms, head)
	add(i->second.resolve(ax, head));
}

void Context::resolve_binary(const Disjunction& ax, ConceptID head, Context *con) {
    EQRANGE(i, ontology.binary_axioms, head)
	EQRANGE(j, con->axiom_index, i->second.first)
	add(i->second.second.resolve(ax, head, j->second, i->second.first));
}

void Context::push(const Disjunction& d) {
    FOREACH(source, backward_links)
	EQRANGE(j, (*source)->axiom_index, inexist)
	    (*source)->add(d.resolve(j->second, inexist));
}

bool Context::not_occurs(const  Disjunction& ax, bool add) {
    bool notyet = true;
    for (ConceptID* c = ax.begin(); c != ax.end() && notyet; c++)  {
	EQRANGE(j, occurs, *c) {
	    if (j->second.subset(ax)) {
		notyet = false;
		break;
	    }
	}
    }
    
    for (ConceptID* c = ax.begin(); c != ax.end() && notyet; c++)  {
	EQRANGE(j, top_contexts[inrole]->occurs, *c) {
	    if (j->second.subset(ax)) {
		notyet = false;
		break;
	    }
	}
    }

    if (add && notyet) {
	int i = 0;
	while (i < ax.size() && Concept::annotated(ax[i]))
	    i++;
	if (i == ax.size()) 
	    occurs.insert(make_pair(ax[0], ax));
	else 
	    occurs.insert(make_pair(ax[i], ax));
    }

    return notyet;
}

int Context::process() {
    //	cerr << "process " << core->to_string() << endl;
    processing = true;

    while (!todo.empty()) {
	if (!satisfiable)
	    cerr << "Internal error: processing an unsatisfiable context" << endl;
	Disjunction ax = *todo.begin();
	todo.erase(todo.begin());
	if (ax.size() == 0) {
	    satisfiable = false;
	    if (VERBOSE) {
		cout << "UNSAT ";
	       if (inrole)
	   	  cout << factory.role(inrole)->to_string() << ",";
	      cout  <<  core->to_string() << endl;
	    }
	    if (!inrole)
		formatter.unsatisfiable(core);
	    //if this == top can end here
	    push(Disjunction::bottom);
	    unlink();

	    axiom_index.clear();
	    occurs.clear();
	    topush.clear();
	    todo.clear();
	    forward_links.clear();
	    backward_links.clear();
	    universals.clear();

	    topush.insert(Disjunction::bottom);
	}
	else if (not_occurs(ax)) { 

	    if (VERBOSE) {
		if (inrole)
		    cout << factory.role(inrole)->to_string() << ",";
		cout	<< core->to_string() << " [=" << write_disjunction(ax) << endl;
	    }

	    bool pushable = inrole;
	    for (ConceptID* head = ax.begin(); head != ax.end(); head++) {
		const Concept* norm = factory.concept(Concept::normalize(*head));
		bool positive = norm->positive() == Concept::normalize(*head);
		if (Concept::plain(*head)) {
		    axiom_index.insert(make_pair(*head, ax));

		    resolve_unary(ax, *head);

		    if (top) {
			FOREACH(c, all_contexts[inrole])
			    (*c)->resolve_binary(ax, *head, *c);
		    }
		    else {
			resolve_binary(ax, *head, this);
			resolve_binary(ax, *head, top_contexts[inrole]);
		    }

		    if (norm->type() == 'E' && positive) {
			const ExistentialConcept* e = (const ExistentialConcept*) norm;
			RoleID r = e->role()->ID();
			Context* target = context_tracker(make_pair(r, e->concept()->positive()));
			if (target->satisfiable) {
			    link(r, target);
			    if (top) {
				FOREACH(c, all_contexts[inrole])
				    FOREACH(i, (*c)->universals)
				    if (ontology.hierarchy(r, i->first))
					target->add(i->second);
			    }
			    else {
				FOREACH(i, universals)
				    if (ontology.hierarchy(r, i->first))
					target->add(i->second);
			    }
			    FOREACH(i, top_contexts[r]->topush)
				add(i->resolve(ax, *head));
			}
			FOREACH(i, target->topush)
			    add(i->resolve(ax, *head));
		    }

		    if (norm->type() == 'U' && positive) {
			const UniversalConcept *u = (const UniversalConcept*) norm;
			RoleID r = u->role()->ID();
			Disjunction d(u->concept()->positive(), Concept::annotate(factory.negation(u->concept())->negative()));

			if (top) {
			    FOREACH(c, all_contexts[inrole]) {
				(*c)->universals.insert(make_pair(r, d));
				FOREACH(i, (*c)->forward_links) 
				    if (ontology.hierarchy(i->first, r))
					i->second->add(d);
			    }
			}
			else {
			    universals.insert(make_pair(r, d));
			    FOREACH(i, forward_links) 
				if (ontology.hierarchy(i->first, r))
				    i->second->add(d);
			    FOREACH(i, top_contexts[inrole]->forward_links) 
				if (ontology.hierarchy(i->first, r))
				    i->second->add(d);
			}
		    }
		}

		if (inrole) {
		    if (ontology.universal_axioms.find(make_pair(Concept::normalize(*head), inrole)) == ontology.universal_axioms.end())  
			pushable = false;
		    if (!pushable && ORDERING > 0)
			break;
		}
		else {
		    if (norm->type() == 'A' && head == ax.begin()) {
			if (ax.size() == 1) 
			    formatter.subsumption(core, (const AtomicConcept*) norm);
		    }
		    else if (ORDERING > 0)
			break;
		}
	    }

	    if (pushable)  {
		Pusher p(ax, inrole);
		do {
		    topush.insert(p.disjunction());
		    if (top) {
			FOREACH(c, all_contexts[inrole]) 
			    (*c)->push(p.disjunction());
		    }
		    else 
			push(p.disjunction());
		} while (p.next());
	    }
	}
    }
    processing = false;
    return 0;
}

void clear() {
    Context::UNLINK = false;
    top_contexts.clear();
    all_contexts.clear();
    context_tracker.clear();
    Context::UNLINK = true;
}

void set_top_contexts() {
       top_contexts.reserve(Role::number());
       all_contexts.resize(Role::number());

       ConceptID top_id = factory.top()->positive();
       top_contexts[0] = context_tracker(make_pair(0, top_id));
       FOREACH(r, ontology.positive_roles) 
	   top_contexts[*r] = context_tracker(make_pair(*r, top_id));

       top_contexts[0]->process();
       FOREACH(r, ontology.positive_roles) 
	   top_contexts[*r]->process();
}


int main(int argc, char* argv[]) {
    ifstream input;
    ofstream output;
    int input_set = 0;
    int output_set = 0;

    for (int i = 1; i < argc; i++)  {

	if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
	    cout << endl;
	    cout << "This is an experimental version of the ConDOR reasoner\n"
		"for classification of ALCH ontologies. The input file must be\n"
		"in OWL2 functional-style syntax with at most one axiom per line." << endl;
	    cout << endl;
	    cout << "Usage: condor < inputfile > outputfile" << endl;
	    cout << "   or: condor -i inputfile -o outputfile" << endl;
	    cout << endl;
	    cout << "Arguments:" << endl;
	    cout << "-h  (--help): display this help" << endl;
	    cout << "-i  (--input): follow by the input file" << endl;
	    cout << "-n  (--nooutput): classify the ontology but suppress the output" << endl;
	    cout << "-o  (--output): follow by the output output file" << endl;
	    cout << "-v  (--version): print version number" << endl;
	    return 0;
	}

	if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--input") == 0) {
	    if (++i < argc) {
		input.open(argv[i]);
		if (!input.is_open()) {
		    cerr << "Error opening input file: " << argv[i] << endl;
		    return 0;
		}
		input_set = i;
		continue;
	    }
	    cerr << "Input file expected after -i or --input." << endl;
	    return 0;
	}

	if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0) {
	    if (++i < argc) {
		output.open(argv[i]);
		if (!output.is_open()) {
		    cerr << "Error opening output file: " << argv[i] << endl;
		    return 0;
		}
		output_set = i;
		continue;
	    }
	    cerr << "Output file expected after -o or --output." << endl;
	    return 0;
	}

	if (strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--nooutput") == 0) {
	    OUTPUT = false;
	    continue;
	}

	if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
	    cout << "ConDOR version 0.1.2" << endl;
	    return 0;
	}

	//arguments for testing
	if (strcmp(argv[i], "--verbose") == 0) {
	    VERBOSE = true;
	    OUTPUT = false;
	    continue;
	}

	if (argv[i][0] == '-') {
	    int x = argv[i][1]-'0';
	    if (x >= 0 && x <= 2) {
		ORDERING = x;
		continue;
	    }
	}

	cerr << "Unrecognized argument. Use -h for help." << endl;
	return 0;
    }




    cerr << "PARSING from ";
    Parser p;
    if (input_set) {
	cerr << argv[input_set] << endl;
	p.read(input);
	input.close();
    }
    else {
	cerr << "standard input" << endl;
	p.read();
    }

    ontology.normalize();
       cerr << "CLASSIFICATION" << endl;

       vector<const AtomicConcept*> atomics = factory.all_atomic_ordered();
       formatter.init(atomics);
       set_top_contexts();

       int progress = 0;
       int total = atomics.size();
       int percent = 1;
       FOREACH(a, atomics) {
	   if (++progress*100 > total*percent) {
	       cerr << "\b\b\b" << percent << "%";
	       percent++;
	   }

	   context_tracker(make_pair(0, (*a)->positive()));
	   while (!active.empty()) {
	       Context *c = active.front();
	       active.pop_front();
	       c->process();
	   }
	   all_contexts[0].pop_back();
	   context_tracker.erase(make_pair(0, (*a)->positive()));
       }
       cerr << "\b\b\b100%" << endl;

       Context::UNLINK = false;
       clear();

       if (OUTPUT) {
	   cerr << "OUTPUT to ";
	   if (output_set) {
	       cerr << argv[output_set] << endl;
	       formatter.write(output);
		 output.close();
	   }
	   else {
	       cerr << "standard output" << endl;
	       formatter.write();
	   }
       }
}
