#ifndef TRACKER_H_
#define TRACKER_H_

#include <map>
#include <vector>

#include "header.h"

using namespace std;

template<typename S, typename T>
class tracker {
  map<S, T*> m;
  bool last;

  public:
  tracker() : m(), last(false) {}
  ~tracker() { clear(); }
  
  void clear() {
    for (typename map<S, T*>::iterator i = m.begin(); i != m.end(); i++)
      delete i->second;
    m.clear();
  }

  T* operator()(const S& x) {
    if (m.find(x) == m.end()) {
      T* r = new T(x);
      m[x] = r;
      last = true;
      return r;
    }
    else {
      last = false;
      return m[x];
    }
  }

  bool was_new() {
    return last;
  }

  void erase(const S& x) {
      typename map<S, T*>::iterator i = m.find(x);
      delete i->second;
      m.erase(i);
  }

  vector<T*> get_range() {
    vector<T*> l;
    l.reserve(m.size());
    for (typename map<S, T*>::iterator i = m.begin(); i != m.end(); i++)
      l.push_back(i->second);
	return l;
  }
};

template<typename S, typename T>
class hash_tracker {
  unordered_map<S, T*> m;
  bool last;
  public:
  hash_tracker() : m(), last(false) {}
  ~hash_tracker() { clear(); }

  void clear() {
    for (typename unordered_map<S, T*>::iterator i = m.begin(); i != m.end(); i++)
      delete i->second;
    m.clear();
  }

  T* operator()(const S& x) {
    if (m.find(x) == m.end()) {
      T* r = new T(x);
      m[x] = r;
      last = true;
      return r;
    }
    else {
      last = false;
      return m[x];
    }
  }

  bool was_new() {
    return last;
  }

  void erase(const S& x) {
      typename unordered_map<S, T*>::iterator i = m.find(x);
      delete i->second;
      m.erase(i);
  }

  vector<T*> get_range() {
    vector<T*> l;
    l.reserve(m.size());
    for (typename unordered_map<S, T*>::iterator i = m.begin(); i != m.end(); i++)
      l.push_back(i->second);
	return l;
  }
};

#endif /* TRACKER_H_ */
