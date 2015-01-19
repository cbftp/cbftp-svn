#pragma once

// simple smart pointer template class.
// suggested usage:
// Pointer<T> p(new T());

template <class T> class Pointer {
public:
  Pointer() :
    object(NULL),
    counter(NULL) {
  }
  explicit Pointer(T * t) :
    object(t),
    counter(new int) {
    *counter = 1;
  }
  Pointer(const Pointer & other) :
    object(other.object),
    counter(other.counter) {
    if (counter != NULL) {
      (*counter)++;
    }
  }
  ~Pointer() {
    decrease();
  }
  Pointer & operator=(const Pointer & other) {
    decrease();
    object = other.object;
    counter = other.counter;
    ++(*counter);
    return *this;
  }
  T * operator->() const {
    return object;
  }
  T & operator*() const {
    return *object;
  }
  T * get() const {
    return object;
  }
  bool operator!() const {
    return counter == NULL;
  }
  bool operator==(const Pointer & other) const {
    return object == other.object;
  }
  bool operator<(const Pointer & other) const {
    return object < other.object;
  }
  void reset() {
    decrease();
    object = NULL;
    counter = NULL;
  }
private:
  void decrease() {
    if (counter != NULL && !--(*counter)) {
      delete counter;
      delete object;
    }
  }
  T * object;
  int * counter;
};
