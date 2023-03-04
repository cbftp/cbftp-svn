#pragma once

#include <cassert>
#include <list>
#include <unordered_map>

template <class T> class JobList {
public:
  typename std::list<T>::iterator begin() {
    return items.begin();
  }
  typename std::list<T>::const_iterator begin() const {
    return items.begin();
  }
  typename std::list<T>::iterator end() {
    return items.end();
  }
  typename std::list<T>::const_iterator end() const {
    return items.end();
  }
  T& front() {
    return items.front();
  }
  T& back() {
    return items.back();
  }
  void push_back(const T& t) {
    items.push_back(t);
    typename std::pair<typename std::unordered_map<unsigned int, typename std::list<T>::iterator>::iterator, bool> res = index.emplace(t->getId(), --items.end());
    assert(res.second);
  }
  bool empty() const {
    return items.empty();
  }
  typename std::list<T>::const_iterator find(unsigned int id) const {
    typename std::unordered_map<unsigned int, typename std::list<T>::iterator>::const_iterator it = index.find(id);
    if (it != index.end()) {
      return it->second;
    }
    return items.end();
  }
  typename std::list<T>::const_iterator find(const T& t) const {
    return find(t->getId());
  }
 typename std::list<T>::iterator find(unsigned int id) {
    typename std::unordered_map<unsigned int, typename std::list<T>::iterator>::iterator it = index.find(id);
    if (it != index.end()) {
      return it->second;
    }
    return items.end();
  }
  typename std::list<T>::iterator find(const T& t) {
    return find(t->getId());
  }
  bool contains(unsigned int id) {
    return index.find(id) != index.end();
  }
  bool contains(const T& t) {
    return contains(t->getId());
  }
  typename std::list<T>::iterator erase(unsigned int id) {
    typename std::unordered_map<unsigned int, typename std::list<T>::iterator>::iterator it = index.find(id);
    if (it != index.end()) {
      typename std::list<T>::iterator it2 = items.erase(it->second);
      index.erase(id);
      return it2;
    }
    return items.end();
  }
  typename std::list<T>::iterator erase(const T& t) {
    return erase(t->getId());
  }
  typename std::list<T>::iterator erase(typename std::list<T>::iterator it) {
    return erase((*it)->getId());
  }
  typename std::list<T>::iterator remove(const T& t) {
    return erase(t->getId());
  }
  void clear() {
    index.clear();
    items.clear();
  }
  size_t size() const {
    return index.size();
  }
private:
  std::unordered_map<unsigned int, typename std::list<T>::iterator> index;
  std::list<T> items;
};
