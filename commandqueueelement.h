#pragma once

#include <semaphore.h>

class CommandQueueElement {
  private:
    int opcode;
    sem_t * donesem;
    void * arg1;
    void * arg2;
    void * arg3;
  public:
    CommandQueueElement(int);
    CommandQueueElement(int, sem_t *);
    CommandQueueElement(int, void *);
    CommandQueueElement(int, sem_t *, void *);
    CommandQueueElement(int, void *, void *);
    CommandQueueElement(int, sem_t *, void *, void *);
    CommandQueueElement(int, void *, void *, void *);
    CommandQueueElement(int, sem_t *, void *, void *, void*);
    int getOpcode();
    void * getArg1();
    void * getArg2();
    void * getArg3();
    sem_t * getDoneSem();
};
