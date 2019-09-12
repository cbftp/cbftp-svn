include ../../../Makefile.inc

SRC = $(wildcard *.cpp)
CLEANFILES = $(wildcard $(SRC:%.cpp=%.o) $(SRC:%.cpp=%.d))

all: $(SRC:%.cpp=%.o)
	@#

clean:
	@if test $(firstword $(CLEANFILES)); then rm -rf $(CLEANFILES); echo rm -rf $(CLEANFILES); fi

$(SRC:%.cpp=%.o): ../../../Makefile.inc

-include $(SRC:%.cpp=%.d)
