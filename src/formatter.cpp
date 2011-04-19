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

#include <algorithm>
#include <iterator>

#include "formatter.h"

template<typename T, typename S> 
bool empty_intersection(const T& b1, const T& e1, const S& b2, const S& e2) {
    T i1 = b1;
    S i2 = b2;
    while (i1 != e1 && i2 != e2) {
	if (*i1 == *i2)
	    return false;
	if (*i1 < *i2)
	    i1++;
	else
	    i2++;
    }
    return true;
}

void Formatter::init(const vector<const AtomicConcept*>& ord) {
    consistent = true;
    concepts = ord;
    n = concepts.size();
    for (int i = 0; i < n; i++)
	order[concepts[i]->ID()] = i;
    super.resize(n);
}

/*
int unsat = 0;
int subsum = 0;
*/

void Formatter::unsatisfiable(const Concept* x) {
	if (x->type() == 'A')
		bot.insert(order[x->ID()]);
	else if (x->type() == 'T')
		consistent == false;
	else 
		cerr << "Internal error: unexpected concept type for formatter" << endl;
//    unsat++;
}

void Formatter::subsumption(const Concept* x, const AtomicConcept* y) {
	if (x->ID() == y->ID())
		return;
	if (x->type() == 'A') 
	    super[order[x->ID()]].push_back(order[y->ID()]);
	else if (x->type() == 'T')
		top.insert(y->ID());
	else
		cerr << "Internal error: Unexpected concept type for formatter" << endl;
//  subsum++;
}

void Formatter::write(ostream& output) {
    /*
    cerr << "unsatisfiable: " << unsat << endl;
    cerr << "subsumptions: " << subsum << endl;
    */
	output << "Ontology(" << endl;

	if (!consistent) {
		cerr << "The ontology is inconsistent." << endl;
		output << "EquivalentClasses(owl:Nothing owl:Thing)" << endl;
	}
	else {
		vector<bool> noneq(n, true);
		if (!bot.empty()) {
			output << "EquivalentClasses(owl:Nothing";
			for (set<int>::iterator i = bot.begin(); i != bot.end(); i++) {
				output << " " << concepts[*i]->to_string();
				noneq[*i] = false;
			}
			output << ")" << endl;
		}
		if (!top.empty()) {
			output << "EquivalentClasses(owl:Thing";
			for (set<int>::iterator i = top.begin(); i != top.end(); i++) {
				output << " " << concepts[*i]->to_string();
				noneq[*i] = false;
			}
			output << ")" << endl;
		}

		for (int i = 0; i < n; i++)
		    if (noneq[i]) {
			sort(super[i].begin(), super[i].end());
			super[i].erase(unique(super[i].begin(), super[i].end()), super[i].end());
		    }

		vector<int> v;
		for (int i = 0; i < n; i++)
		    if (noneq[i]) {
			v.clear();
			FOREACH(j, super[i])
			    if (binary_search(super[*j].begin(), super[*j].end(), i))
				v.push_back(*j);
			if (!v.empty()) {
			    output << "EquivalentClasses(" << concepts[i]->to_string();
			    FOREACH(j, v) {
				    output << " " << concepts[*j]->to_string();
				    noneq[*j] = false;
				}
			    output << ")" << endl;
			}
		    }

		vector<int> trans(n, -1);
		for (int i = 0; i < n; i++)
		    if (noneq[i]) {
			FOREACH(j, super[i])
			    if (noneq[*j] && trans[*j] != i)
				FOREACH(k, super[*j])
				    trans[*k] = i;
			FOREACH(j, super[i])
			    if (noneq[*j] && trans[*j] != i) 
				output << "SubClassOf(" << concepts[i]->to_string() << " " << concepts[*j]->to_string() << ")" << endl;
			}
	}
	output << ")" << endl;
}
