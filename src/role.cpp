#include <queue>
#include <cstring>

#include "role.h"

RoleID Role::next_id = 1;

Role::Role() : id(next_id++) {}
Role::~Role() {}

InverseRole::InverseRole(const Role* r) {
  inv = r;
}

InverseRole::~InverseRole() {}

string InverseRole::to_string() const {
  return "ObjectInverseOf(" + inv->to_string() + ")";
}

AtomicRole::AtomicRole(const string& name) : name(name) {
  inv = new InverseRole(this);
}

AtomicRole::~AtomicRole() {
  delete inv;
}

string AtomicRole::to_string() const {
  return name;
}

int Role::number() {
    return next_id;
}

void RoleHierarchy::add(const Role* r, const Role* s) {
  if (r != s) {
    direct.insert(make_pair(r->ID(), s->ID()));
    direct.insert(make_pair(r->inverse()->ID(), s->inverse()->ID()));
  }
}

//IDs must be in [1, next_id)
void RoleHierarchy::closure() {
  int n = Role::number();
  all = new bool* [n];
  all[0] = new bool [n*n];
  for (int i = 1; i < n; i++)
    all[i] = all[i-1]+n;
  memset(all[0], 0, n*n*sizeof(bool));

  queue<RoleID> q;
  for (RoleID r = 0; r < n; r++) {
    q.push(r);
    while (!q.empty()) {
      RoleID s = q.front();
      q.pop();
      if (!all[r][s]) {
	all[r][s] = true;
        pair<multimap<RoleID, RoleID>::iterator, multimap<RoleID, RoleID>::iterator> p = direct.equal_range(s);
	for (multimap<RoleID, RoleID>::iterator i = p.first; i != p.second; i++)
	  q.push(i->second);
      }
    }
  }
}

RoleHierarchy::RoleHierarchy() : closed(false), all(NULL) {}

RoleHierarchy::~RoleHierarchy() {
  if (closed) {
    delete[] all[0];
    delete[] all;
  }
}
