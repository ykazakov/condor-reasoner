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

#ifndef ROLE_H_
#define ROLE_H_

#include <string>
#include <map>

using namespace std;


typedef int RoleID;

class Role {
  protected:
  const RoleID id;
  const Role* inv;

  static RoleID next_id;

  public:
  Role();
  virtual ~Role() = 0;

  const Role* inverse() const { return inv; }
  RoleID ID() const { return id; }
  virtual string to_string() const = 0;

  static int number();
};

class InverseRole : public Role {
  public:
  InverseRole(const Role* r);
  virtual ~InverseRole();

  virtual string to_string() const;
};

class AtomicRole : public Role {
  const string name;

  public:
  AtomicRole(const string &name);
  virtual ~AtomicRole();

  virtual string to_string() const;
};



class RoleHierarchy {
  multimap<RoleID, RoleID> direct;
  bool** all;
  bool closed;

  public:
  RoleHierarchy();
  ~RoleHierarchy();


  void add(const Role* r, const Role* s);
  void closure();
  bool operator()(const Role* r, const Role* s) const {
    return all[r->ID()][s->ID()];
  }
  bool operator()(RoleID r, RoleID s) const {
	  return all[r][s];
  }
};

#endif /* ROLE_H_ */
