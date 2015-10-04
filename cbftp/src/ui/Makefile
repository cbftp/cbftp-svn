include ../../Makefile.inc

SRC = $(wildcard *.cpp)
OBJS = $(wildcard $(SRC:%.cpp=%.o)) $(wildcard $(SRC:%.cpp=%.d))

all: $(SRC:%.cpp=%.o)
	@+${MAKE} -C screens

clean:
	@if test $(firstword $(OBJS)); then rm -rf $(OBJS); echo rm -rf $(OBJS); fi
	@+${MAKE} -C screens clean  

-include $(SRC:%.cpp=%.d)
