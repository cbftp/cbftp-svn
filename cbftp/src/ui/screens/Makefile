include ../../../Makefile.inc

SRC = $(wildcard *.cpp)
CLEANFILES = $(wildcard $(SRC:%.cpp=%.o) $(SRC:%.cpp=%.d))

all: $(SRC:%.cpp=%.o)
	@#

ifeq ($(HAS_SVN), 0)
SVN_DIR := $(SVN_ROOT_DIR)/.svn
endif

loginscreen.o: loginscreen.cpp ../../../Makefile.inc $(SVN_DIR)
	$(CXX)  $(CPPFLAGS)  -c -o loginscreen.o loginscreen.cpp -DBUILDTIME=$(DATE) -DVERSION=$(VERSION) -DBOXTAG=$(BOXTAG)
	
clean:
	@if test $(firstword $(CLEANFILES)); then rm -rf $(CLEANFILES); echo rm -rf $(CLEANFILES); fi

$(SRC:%.cpp=%.o): ../../../Makefile.inc

-include $(SRC:%.cpp=%.d)
