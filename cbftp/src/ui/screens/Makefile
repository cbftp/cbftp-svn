include ../../../Makefile.inc

SRC = $(wildcard *.cpp)
OBJS = $(wildcard $(SRC:%.cpp=%.o)) $(wildcard $(SRC:%.cpp=%.d))

all: $(SRC:%.cpp=%.o)
	@#

loginscreen.o: loginscreen.cpp ../../../Makefile.inc
	g++  ${CPPFLAGS} -DBUILDTIME=${DATE} -DVERSION=${VERSION} -DBOXTAG=${BOXTAG} -c -o loginscreen.o loginscreen.cpp
	
clean:
	@if test $(firstword $(OBJS)); then rm -rf $(OBJS); echo rm -rf $(OBJS); fi

-include $(SRC:%.cpp=%.d)
