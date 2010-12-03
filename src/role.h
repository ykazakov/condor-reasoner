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
