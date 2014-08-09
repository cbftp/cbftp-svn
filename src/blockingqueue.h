#pragma once

#include <pthread.h>
#include <semaphore.h>
#include <list>

template <class T> class BlockingQueue {
public:
  BlockingQueue() {
    sem_init(&content, 0, 0);
    pthread_mutex_init(&queuemutex, NULL);
  }
  void push(T t) {
    pthread_mutex_lock(&queuemutex);
    queue.push_back(t);
    pthread_mutex_unlock(&queuemutex);
    sem_post(&content);
  }
  T pop() {
    sem_wait(&content);
    pthread_mutex_lock(&queuemutex);
    T ret = queue.front();
    queue.pop_front();
    pthread_mutex_unlock(&queuemutex);
    return ret;
  }
  unsigned int size() const {
    pthread_mutex_lock(&queuemutex);
    unsigned int size = queue.size();
    pthread_mutex_unlock(&queuemutex);
    return size;
  }
private:
  std::list<T> queue;
  sem_t content;
  mutable pthread_mutex_t queuemutex;
};
