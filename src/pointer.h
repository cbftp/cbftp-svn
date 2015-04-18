#pragma once

// simple smart pointer template class.
// suggested usage:
// Pointer<T> p(makePointer<T>(args));

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
    if (counter != NULL) {
      ++(*counter);
    }
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

template <typename T>
Pointer<T> makePointer() {
  return Pointer<T>(new T());
}

template <typename T, typename A1>
Pointer<T> makePointer(A1 a1) {
  return Pointer<T>(new T(a1));
}

template <typename T, typename A1, typename A2>
Pointer<T> makePointer(A1 a1, A2 a2) {
  return Pointer<T>(new T(a1, a2));
}

template <typename T, typename A1, typename A2, typename A3>
Pointer<T> makePointer(A1 a1, A2 a2, A3 a3) {
  return Pointer<T>(new T(a1, a2, a3));
}

template <typename T, typename A1, typename A2, typename A3, typename A4>
Pointer<T> makePointer(A1 a1, A2 a2, A3 a3, A4 a4) {
  return Pointer<T>(new T(a1, a2, a3, a4));
}

template <typename T, typename A1, typename A2, typename A3, typename A4,
typename A5>
Pointer<T> makePointer(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) {
  return Pointer<T>(new T(a1, a2, a3, a4, a5));
}

template <typename T, typename A1, typename A2, typename A3, typename A4,
typename A5, typename A6>
Pointer<T> makePointer(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6) {
  return Pointer<T>(new T(a1, a2, a3, a4, a5, a6));
}

template <typename T, typename A1, typename A2, typename A3, typename A4,
typename A5, typename A6, typename A7>
Pointer<T> makePointer(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7) {
  return Pointer<T>(new T(a1, a2, a3, a4, a5, a6, a7));
}

template <typename T, typename A1, typename A2, typename A3, typename A4,
typename A5, typename A6, typename A7, typename A8>
Pointer<T> makePointer(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8) {
  return Pointer<T>(new T(a1, a2, a3, a4, a5, a6, a7, a8));
}

template <typename T, typename A1, typename A2, typename A3, typename A4,
typename A5, typename A6, typename A7, typename A8, typename A9>
Pointer<T> makePointer(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8,
    A9 a9) {
  return Pointer<T>(new T(a1, a2, a3, a4, a5, a6, a7, a8, a9));
}
