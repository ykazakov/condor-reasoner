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
#include <time.h>

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

bool PRINT_LOG = false;
bool VERBOSE = false;
bool OUTPUT = true;

bool SUBSET_OPT = true;
bool REMOVE_OPT = false;
bool TOP_PRESENT, TOP_OPT = false;
bool SECONDARY_OPT = false;

int goals_set = 0;

string write_disjunction(const Disjunction& d) {
    stringstream ss;
    FOREACH(x, d) {
	const Concept* c = factory.concept(*x);
	ss << " " << c->to_string();
	if (Concept::decompose(*x))
	    ss << "+";
	if (Concept::is_annotated(*x))
	    ss << "*";
    }
    return ss.str();
}

int time_interval(long long a, long long b) {
    return (b-a)*1000 / CLOCKS_PER_SEC;
}

int context_init_number = 0;
int axiom_init_number = 0;
int max_init_axioms = 0;
double total_init_length = 0;
int max_init_length = 0;

int context_succ_number = 0;
int axiom_succ_number = 0;
int max_succ_axioms = 0;
double total_succ_length = 0;
int max_succ_length = 0;

int link_number = 0;
int max_forward_links = 0;
int max_backward_links = 0;

int topush_number = 0;
int max_topush = 0;

Factory factory;
Ontology ontology;
Formatter formatter;

class Pusher {
    vector<pair<map<pair<ConceptID, RoleID>, ConceptID>::iterator, map<pair<ConceptID, RoleID>, ConceptID>::iterator> > bounds; 
    vector<map<pair<ConceptID, RoleID>, ConceptID>::iterator> v;
    int n, i;
    Disjunction d;

    void build() {
	set<ConceptID, Concept::DecomposeLess> s;
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

ConceptID split_ID[2];

class Context {
    public:
	static bool UNLINK;


	bool SPLIT;

	const Concept* core;
	ConceptID inexist;
	RoleID inrole;
	bool top;
	bool secondary;
	bool satisfiable;
	bool processing;

	unordered_multimap<ConceptID, Disjunction> axiom_index;
	set<Disjunction> topush;
	multiset<Disjunction, Disjunction::SizeLess> todo;
	set<pair<RoleID, Context*> > forward_links;
	set<Context*> backward_links;
	set<pair<RoleID, Disjunction> > universals;

	vector<const AtomicConcept*> super;

	int axioms;

	//  public:
	explicit Context(pair<RoleID, const Concept*>);
	~Context();
	void unlink();

	void add(const Disjunction& a); //things to consider before pushing into todo
	void link(RoleID r, Context* c);
	Context* secondary_context(RoleID r, const Concept* c);
	int process();
	void resolve_unary(const Disjunction& d, ConceptID head);
	void resolve_binary(const Disjunction& d, ConceptID head, Context*);
	void push(const Disjunction& d);
	bool not_occurs(const  Disjunction& ax);
	void remove_supersets(const Disjunction& ax);
};

bool Context::UNLINK = true;

tracker<pair<RoleID, const Concept*>, Context> context_tracker; //could try hash_tracker
map<ConceptID, tracker<pair<RoleID, const Concept*>, Context> > secondary_context_tracker;
vector<Context* > top_contexts;
vector<list<Context*> > all_contexts;
list<Context*> active;

Context* Context::secondary_context(RoleID r, const Concept* c) {
	ConceptID s = core->ID();
	return secondary_context_tracker[s](make_pair(r, c));
}

Context::Context(pair<RoleID, const Concept*> rc) : core(rc.second), inrole(rc.first), top(core->type() == 'T'), satisfiable(true), processing(false), secondary(false) { 
    todo.insert(Disjunction(Concept::concept_decompose(core)));
	if (!TOP_OPT && TOP_PRESENT)
		todo.insert(Disjunction(factory.top()->ID()));
    if (inrole) {
	inexist = Concept::concept_decompose(factory.existential(factory.role(inrole), core));
	if (top || !TOP_OPT)
	FOREACH(range, ontology.role_range)
	    if (ontology.hierarchy(inrole, range->first))
		todo.insert(range->second);
    }
    active.push_back(this);
    all_contexts[inrole].push_back(this);
    axioms = 0;

    if (inrole)
	context_succ_number++;
    else
	context_init_number++;


    SPLIT = (inrole == 0);
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
	    for (ConceptID *i = a.end(); i != a.begin(); ) {
		i--;
		if (Concept::is_annotated(*i)) 
			annot++;
		else
		    break;
	    }

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
void Context::link(RoleID r, Context* target) {
	forward_links.insert(make_pair(r, target));
	target->backward_links.insert(this);

	link_number++;
	max_forward_links = max(max_forward_links, (int) forward_links.size());
	max_backward_links = max(max_backward_links, (int) target->backward_links.size());

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
}

void Context::resolve_unary(const Disjunction& ax, ConceptID head) {
    EQRANGE(i, ontology.unary_axioms, head) 
		add(i->second.resolve(ax));
}

void Context::resolve_binary(const Disjunction& ax, ConceptID head, Context *con) {
	EQRANGE(i, ontology.binary_axioms, head) {
	    EQRANGE(j, con->axiom_index, Concept::clear_decompose(i->second.first)) 
		if (!Concept::decompose(i->second.first) || Concept::decompose(j->second.front())) 
	//	    if (inrole || !ax.has_annotated() || !j->second.has_annotated() || ax.back() == j->second.back())
			add(i->second.second.resolve(ax, j->second));
	}
}

void Context::push(const Disjunction& d) {
    FOREACH(source, backward_links)
	EQRANGE(j, (*source)->axiom_index, Concept::clear_decompose(inexist))
	if (j->second.front() == inexist)  
	    (*source)->add(d.resolve(j->second));
}

bool Context::not_occurs(const  Disjunction& ax) {
    if (SUBSET_OPT) {
	for (ConceptID* c = ax.begin(); c != ax.end(); c++)
	    EQRANGE(j, axiom_index, Concept::clear_decompose(*c)) {
		if (j->second.subset(ax)) {
		    return false;
		}
	    }
	/*
	   for (ConceptID* c = ax.begin(); c != ax.end(); c++)
	   EQRANGE(j, top_contexts[inrole]->axiom_index, Concept::clear_decompose(*c)) {
	   if (j->second.subset(ax)) {
	   return false;
	   }
	   }
	   */
	return true;
    }
    else {
	EQRANGE(j, axiom_index, Concept::clear_decompose(ax.front())) 
	    if (j->second == ax) 
		return false;
	return true;
    }
}

void Context::remove_supersets(const Disjunction& ax) {
	FOREACH(j, axiom_index)
		if (ax.subset(j->second))
			axiom_index.erase(j);
}


int Context::process() {
	//cerr << "process " << core->to_string() << endl;
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
				if (secondary)
					cout << "!";
				if (inrole)
					cout << factory.role(inrole)->to_string() << ",";
				cout  <<  core->to_string() << endl;
			}
			if (!inrole && !goals_set)
				formatter.unsatisfiable(core);
			//if this == top can end here
			push(Disjunction::bottom);
			unlink();

			axiom_index.clear();
			topush.clear();
			todo.clear();
			forward_links.clear();
			backward_links.clear();
			universals.clear();

			topush.insert(Disjunction::bottom);
		}

		else if (not_occurs(ax)) {
		    if (REMOVE_OPT)
			remove_supersets(ax);
		    ConceptID head = Concept::clear_decompose(ax.front());
		    const Concept* norm = factory.concept(head);
		    axiom_index.insert(make_pair(head, ax));


		    if (VERBOSE) {
			if (secondary)
			    cout << "!";
			if (inrole)
			    cout << factory.role(inrole)->to_string() << ",";
			cout	<< core->to_string() << " [= " << write_disjunction(ax) << endl;
		    }
		    if (inrole) {
			axiom_succ_number++;
			max_succ_axioms = max(max_succ_axioms, (int) axiom_index.size());
			total_succ_length += ax.size();
			max_succ_length = max(max_succ_length, (int) ax.size());
		    }
		    else {
			axiom_init_number++;
			max_init_axioms = max(max_init_axioms, (int) axiom_index.size());
			total_init_length += ax.size();
			max_init_length = max(max_init_length, (int) ax.size());
		    }


			if (Concept::is_annotated(head)) {

				if (inrole) {
					Pusher p(ax, inrole);
					do {
//						if (!top && p.disjunction().size() == 1 && p.disjunction().front() == Concept::clear_decompose(inexist))
//							continue;
						if (topush.find(p.disjunction()) == topush.end()) {
							topush.insert(p.disjunction());
    
							topush_number++;
							max_topush = max(max_topush, (int) topush.size());


							if (top) {
								FOREACH(c, all_contexts[inrole]) 
									(*c)->push(p.disjunction());
							}
							else 
								push(p.disjunction());
						}
					} while (p.next());

				}
				else {
				    if (goals_set) 
					super.push_back((const AtomicConcept*) norm);
				    else
					formatter.subsumption(core, (const AtomicConcept*) norm);
				}
			}

			else { 

				resolve_unary(ax, head);
				if (top) {
					FOREACH(c, all_contexts[inrole])
						(*c)->resolve_binary(ax, head, *c);
				}
				else {
					resolve_binary(ax, head, this);
					if (TOP_OPT) 
						resolve_binary(ax, head, top_contexts[inrole]);
				}

				if (Concept::decompose(ax.front())) {
					if (norm->type() == 'C')
						resolve_unary(ax, ax.front());

					if (norm->type() == 'D') {
					    if (ax.size() == 1 && SPLIT) {
						Disjunction d = ontology.unary_axioms.find(ax.front())->second;
						add(Disjunction(d.front(), split_ID[0]));
						add(Disjunction(split_ID[1]).resolve(d));
						SPLIT = false;
					    }
					    else 
						resolve_unary(ax, ax.front());
					}


					if (norm->type() == 'N' || norm->type() == 'U') {
						resolve_binary(ax, ax.front(), this);
						if (TOP_OPT)
							resolve_binary(ax, ax.front(), top_contexts[inrole]);
					}

					if (norm->type() == 'E') {
						const ExistentialConcept* e = (const ExistentialConcept*) norm;
						RoleID r = e->role()->ID();

						bool secondary_target = false;
						if (SECONDARY_OPT)
						FOREACH(u, universals)
							if (ontology.hierarchy(r, u->first))
								secondary_target = true;

						Context* target = (secondary_target) ? secondary_context(r, e->concept()) : context_tracker(make_pair(r, e->concept()));
						if (target->satisfiable) {
							link(r, target);
							if (TOP_OPT)
								FOREACH(i, top_contexts[r]->topush) 
									add(i->resolve(ax));
						}
						FOREACH(i, target->topush)  {
							add(i->resolve(ax));
						}
					}

					if (norm->type() == 'U') {
						resolve_unary(ax, ax.front()); // for transitivity

						const UniversalConcept *u = (const UniversalConcept*) norm;
						RoleID r = u->role()->ID();
						Disjunction d(Concept::concept_decompose(u->concept()), Concept::annotate(factory.negation(u->concept())->ID()));
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

							if (SECONDARY_OPT) {
								list<pair<Context*, Context*> > redirect;
								FOREACH(i, forward_links) 
									if (ontology.hierarchy(i->first, r)) {
										if (!i->second->secondary)
											redirect.push_back(make_pair(i->second, secondary_context(i->first, i->second->core)));
										else
											i->second->add(d);
									}

								FOREACH(c, redirect) {
									c->second->secondary = true;
									forward_links.erase(make_pair(c->second->inrole, c->first));
									c->first->backward_links.erase(this);
									link(c->second->inrole, c->second);
									//PUSH!

									if (TOP_OPT)
										FOREACH(i, top_contexts[inrole]->forward_links) 
											if (ontology.hierarchy(i->first, r)) 
												link(i->first, secondary_context(i->first, i->second->core));
									//PUSH!
								}
							}
							else {
								FOREACH(i, forward_links)
									if (ontology.hierarchy(i->first, r)) 
										i->second->add(d);
								if (TOP_OPT)
									FOREACH(i, top_contexts[inrole]->forward_links) 
										if (ontology.hierarchy(i->first, r)) 
											i->second->add(d);
							}
						}
					}
				}

				if (inrole) {
					if (ontology.universal_axioms.find(make_pair(head, inrole)) != ontology.universal_axioms.end()) 
						add(ax.annotate());
				}
				else {
					if (norm->type() == 'A') 
						add(ax.annotate());
				}
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
	secondary_context_tracker.clear();
    Context::UNLINK = true;
}

void set_top_contexts() {
	top_contexts.reserve(Role::number());

	top_contexts[0] = context_tracker(make_pair(0, factory.top()));
	FOREACH(r, ontology.positive_roles) 
		top_contexts[*r] = context_tracker(make_pair(*r, factory.top()));

	top_contexts[0]->process();
	FOREACH(r, ontology.positive_roles) 
		top_contexts[*r]->process();
}


int main(int argc, char* argv[]) {
    ifstream input;
    ofstream output;
    ifstream goals_file;
    int input_set = 0;
    int output_set = 0;

    for (int i = 1; i < argc; i++)  {

	if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
	    cout << endl;
	    cout << "This is an experimental version of the ConDOR reasoner\n"
		"for classification of SH ontologies. The input file must be\n"
		"in OWL2 functional-style syntax with at most one axiom per line." << endl;
	    cout << endl;
	    cout << "Usage: condor < inputfile > outputfile" << endl;
	    cout << "   or: condor -i inputfile -o outputfile" << endl;
	    cout << endl;
	    cout << "Arguments:" << endl;
	    cout << "-h  (--help): display this help" << endl;
	    cout << "-i  (--input): follow by the input file" << endl;
	    cout << "-n  (--nooutput): classify the ontology but suppress the output" << endl;
	    cout << "-o  (--output): follow by the output file" << endl;
	    cout << "-g  (--goals): follow by a file containing a list of classification goals" << endl;
//	    cout << "-l  (--log): write performance log into condor.log" << endl;
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

	if (strcmp(argv[i], "-g") == 0 || strcmp(argv[i], "--goals") == 0) {
	    if (++i < argc) {
		goals_file.open(argv[i]);
		if (!goals_file.is_open()) {
		    cerr << "Error opening goals file: " << argv[i] << endl;
		    return 0;
		}
		goals_set = i;
		OUTPUT = false;
		continue;
	    }
	    cerr << "Goals file expected after -g or --goals." << endl;
	    return 0;
	}

	if (strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--nooutput") == 0) {
	    OUTPUT = false;
	    continue;
	}

	//arguments for testing
	if (strcmp(argv[i], "-w") == 0 || strcmp(argv[i], "--write") == 0) {
	    VERBOSE = true;
	    OUTPUT = false;
	    continue;
	}

	if (strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--log") == 0) {
	    PRINT_LOG = true;
	    continue;
	}
	
	if (strcmp(argv[i], "-sub0") == 0) {
		SUBSET_OPT = false;
		continue;
	}
	if (strcmp(argv[i], "-sub1") == 0) {
		SUBSET_OPT = true;
		continue;
	}
	if (strcmp(argv[i], "-rem0") == 0) {
		REMOVE_OPT = false;
		continue;
	}
	if (strcmp(argv[i], "-rem1") == 0) {
		REMOVE_OPT = true;
		continue;
	}
	if (strcmp(argv[i], "-top0") == 0) {
		TOP_OPT = false;
		continue;
	}
	if (strcmp(argv[i], "-top1") == 0) {
		TOP_OPT = true;
		continue;
	}
	if (strcmp(argv[i], "-sc0") == 0) {
		SECONDARY_OPT = false;
		continue;
	}
	if (strcmp(argv[i], "-sc1") == 0) {
		SECONDARY_OPT = true;
		continue;
	}

	cerr << "Unrecognized argument. Use -h for help." << endl;
	return 0;
    }



    clock_t timepoint[4];
    timepoint[0] = clock();


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

    vector<const Concept*> goals;
    if (goals_set) {
	cerr << "READING GOALS from " << argv[goals_set] << endl;
	string line;
	while (getline(goals_file, line)) {
	    stringstream ss(line);
	    string s;
	    vector<const AtomicConcept*> a;
	    while (ss >> s) {
		a.push_back(factory.atomic(s));
		if (factory.atomic_tracker.was_new())
		    cerr << "Warning: " << s << " does not occur in the ontology." << endl;
	    }

	    if (a.size() == 1) 
		goals.push_back(a[0]);
	    if (a.size() > 1) {
		const DummyConcept* d = factory.dummy(line);
		FOREACH(x, a)
		    ontology.unary(d->ID(), Disjunction((*x)->ID()));
		goals.push_back(d);
	    }
	}
    }
    else {
	vector<const AtomicConcept*> a = factory.all_atomic_ordered();
	formatter.init(a);
	goals.reserve(a.size());
	FOREACH(x, a)
	    goals.push_back(*x);
    }

    timepoint[1] = clock();
    cerr << "CLASSIFICATION" << endl;

    ontology.normalize();

    /*
    cout << "UNARY" << endl;
    FOREACH(x, ontology.unary_axioms)
	cout << factory.concept(x->first)->to_string() << " -> " << write_disjunction(x->second) << endl;
    cout << "BINARY" << endl;
    FOREACH(x, ontology.binary_axioms)
	cout << factory.concept(x->first)->to_string() << " " << factory.concept(x->second.first)->to_string() << " -> " << write_disjunction(x->second.second) << endl;

    return 0;
    */

    split_ID[0] = Concept::maximal_ID();
    split_ID[1] = Concept::maximal_ID();
    factory.dummy(split_ID[0]);
    factory.dummy(split_ID[1]);
    ontology.binary_axioms.insert(make_pair(split_ID[0], make_pair(split_ID[1], Disjunction())));
    ontology.binary_axioms.insert(make_pair(split_ID[1], make_pair(split_ID[0], Disjunction())));



	TOP_PRESENT = (ontology.unary_axioms.find(factory.top()->ID()) != ontology.unary_axioms.end());
	TOP_OPT = TOP_OPT && TOP_PRESENT;

	all_contexts.resize(Role::number());
	if (TOP_OPT)
		set_top_contexts();

       int progress = 0;
       int total = goals.size();
       int percent = 1;
//    for (vector<const AtomicConcept*>::reverse_iterator a = atomics.rbegin();  a != atomics.rend(); a++) {
       FOREACH(a, goals) {
	progress++;
	while (progress*100 > total*percent) {
	    cerr << "\b\b\b" << percent << "%";
	    percent++;
	}

	Context now(make_pair(0, *a));
	while (!active.empty()) {
	    Context *c = active.front();
	    active.pop_front();
	    c->process();
	}

	if (goals_set) {
	    (output_set ? output : cout) << endl << (*a)->to_string() << endl;
	    if (now.satisfiable) {
		sort(now.super.begin(), now.super.end(), AtomicConcept::AlphaLess());
		FOREACH(x, now.super)
		    (output_set ? output : cout) << "  " << (*x)->to_string() << endl;
	    }
	    else
		(output_set ? output : cout) << "  owl:Nothing" << endl;
	}

	all_contexts[0].pop_back();
       }
       cerr << "\b\b\b100%" << endl;

       Context::UNLINK = false;
       clear();

    timepoint[2] = clock();
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

    timepoint[3] = clock();

	   if (PRINT_LOG) {
	       ofstream stats;
	       stats.open("condor.log");

	       stats << "Command:";
	       for (int i = 1; i < argc; i++)
		   stats << " " << argv[i];
	       stats << endl << endl;
	       stats << "Total time: " << time_interval(timepoint[0], timepoint[3]) << "ms" << endl;
	       stats << "Classification time: " << time_interval(timepoint[1], timepoint[2]) << "ms" << endl;
	       stats << endl;
	       stats << "Contexts: " << context_init_number << " " << context_succ_number << endl;;
	       stats << "Avg axioms: " << axiom_init_number / context_init_number << " " << axiom_succ_number / context_succ_number << endl;
	       stats << "Max axioms: " << max_init_axioms << " " << max_succ_axioms << endl;
	       stats << endl; 
	       stats << "Axioms: " << axiom_init_number << " " << axiom_succ_number << endl;;
	       stats << "Avg length: " << total_init_length / axiom_init_number << " " << total_succ_length / axiom_succ_number << endl;
	       stats << "Max length: " << max_init_length << " " << max_succ_length << endl;
	       stats << endl; 
	       stats << "Avg links: " << link_number / (context_init_number+context_succ_number) << endl;
	       stats << "Max forward links: " << max_forward_links << endl;
	       stats << "Max backward links: " << max_backward_links << endl;
	       stats << "Avg topush: " << topush_number / context_succ_number << endl;
	       stats << "Max topush: " << max_topush << endl;

	       stats.close();
	   }
}
