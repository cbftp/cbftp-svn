include ../../../Makefile.inc

SRC = $(wildcard *.cpp)
OBJS = $(wildcard $(SRC:%.cpp=%.o)) $(wildcard $(SRC:%.cpp=%.d))

all: $(SRC:%.cpp=%.o)
	@#

ifeq ($(HAS_SVN), 0)
SVN_DIR := ../../../.svn
endif

loginscreen.o: loginscreen.cpp ../../../Makefile.inc ${SVN_DIR}
	${CXX}  ${CPPFLAGS}  -c -o loginscreen.o loginscreen.cpp -DBUILDTIME=${DATE} -DVERSION=${VERSION} -DBOXTAG=${BOXTAG}
	
clean:
	@if test $(firstword $(OBJS)); then rm -rf $(OBJS); echo rm -rf $(OBJS); fi

$(SRC:%.cpp=%.o): ../../../Makefile.inc

-include $(SRC:%.cpp=%.d)
