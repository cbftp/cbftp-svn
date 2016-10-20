include ../../Makefile.inc

SRC = $(wildcard *.cpp)
SCREENS_SRC = $(wildcard screens/*.cpp)
LIB = libui.a
CLEANFILES = $(wildcard $(SRC:%.cpp=%.o) $(SRC:%.cpp=%.d) $(LIB))

all: $(SRC:%.cpp=%.o)
	@+$(MAKE) -C screens
	@+$(MAKE) --no-print-directory $(LIB)

quiet:
	@#

$(LIB): $(SRC:%.cpp=%.o) $(SCREENS_SRC:%.cpp=%.o) | quiet
	ar rc $@ $^ && ranlib $@

clean:
	@if test $(firstword $(CLEANFILES)); then rm -rf $(CLEANFILES); echo rm -rf $(CLEANFILES); fi
	@+$(MAKE) -C screens $@

$(SRC:%.cpp=%.o): ../../Makefile.inc

-include $(SRC:%.cpp=%.d)
