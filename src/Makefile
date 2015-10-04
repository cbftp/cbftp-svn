include ../Makefile.inc

SRC = $(wildcard *.cpp)
OBJS = $(wildcard $(SRC:%.cpp=%.o)) $(wildcard $(SRC:%.cpp=%.d))

all: $(SRC:%.cpp=%.o)
	@+${MAKE} -C ui

clean:
	@if test $(firstword $(OBJS)); then rm -rf $(OBJS); echo rm -rf $(OBJS); fi
	@+${MAKE} -C ui clean        

-include $(SRC:%.cpp=%.d)
