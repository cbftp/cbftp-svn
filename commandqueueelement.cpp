#include "commandqueueelement.h"

CommandQueueElement::CommandQueueElement(int opcode) {
  this->opcode = opcode;
}

CommandQueueElement::CommandQueueElement(int opcode, sem_t * donesem) {
  sem_init(donesem, 0, 0);
  this->donesem = donesem;
  this->opcode = opcode;
}

CommandQueueElement::CommandQueueElement(int opcode, void * arg1) {
  this->opcode = opcode;
  this->arg1 = arg1;
}

CommandQueueElement::CommandQueueElement(int opcode, sem_t * donesem, void * arg1) {
  sem_init(donesem, 0, 0);
  this->donesem = donesem;
  this->opcode = opcode;
  this->arg1 = arg1;
}

CommandQueueElement::CommandQueueElement(int opcode, void * arg1, void * arg2) {
  this->opcode = opcode;
  this->arg1 = arg1;
  this->arg2 = arg2;
}

CommandQueueElement::CommandQueueElement(int opcode, sem_t * donesem, void * arg1, void * arg2) {
  sem_init(donesem, 0, 0);
  this->donesem = donesem;
  this->opcode = opcode;
  this->arg1 = arg1;
  this->arg2 = arg2;
}

CommandQueueElement::CommandQueueElement(int opcode, void * arg1, void * arg2, void * arg3) {
  this->opcode = opcode;
  this->arg1 = arg1;
  this->arg2 = arg2;
  this->arg3 = arg3;
}

CommandQueueElement::CommandQueueElement(int opcode, sem_t * donesem, void * arg1, void * arg2, void * arg3) {
  sem_init(donesem, 0, 0);
  this->donesem = donesem;
  this->opcode = opcode;
  this->arg1 = arg1;
  this->arg2 = arg2;
  this->arg3 = arg3;
}

int CommandQueueElement::getOpcode() {
  return opcode;
}

void * CommandQueueElement::getArg1() {
  return arg1;
}

void * CommandQueueElement::getArg2() {
  return arg2;
}

void * CommandQueueElement::getArg3() {
  return arg3;
}

sem_t * CommandQueueElement::getDoneSem() {
  return donesem;
}
