include Makefile.inc

.PHONY: src

BINDIR = bin

BINS = cbftp $(BINDIR)/cbftp-debug $(BINDIR)/datafilecat $(BINDIR)/datafilewrite

SRC = $(wildcard src/*.cpp)

all: ${BINS}

CORE_LIB = src/core/libcore.a
core:
	@+$(MAKE) -C src/core

EXT_LIB = src/ext/libext.a
ext:
	@+$(MAKE) -C src/ext

ifneq ($(UI_PATH),)
UI_DEP = $(wildcard $(UI_PATH)/*.a)
UI_LINK = -Wl,--whole-archive $(UI_DEP) -Wl,--no-whole-archive
ui:
	@+$(MAKE) -C $(UI_PATH)
endif

OBJS = $(wildcard $(SRC:%.cpp=%.o))


$(BINDIR):
	mkdir -p $@

src:
	@+${MAKE} -C src

ifneq ($(UI_PATH),)
cbftp: src ext core ui | $(BINDIR)
else
cbftp: src ext core | $(BINDIR)
endif
	@+${MAKE} --no-print-directory $(BINDIR)/cbftp

$(BINDIR)/cbftp: $(OBJS) $(UI_DEP) $(EXT_LIB) $(CORE_LIB)
	${CXX} -o $(BINDIR)/cbftp $(OPTFLAGS) $(SRC:%.cpp=%.o) $(UI_LINK) $(EXT_LIB) $(CORE_LIB) $(LINKFLAGS)
	
$(BINDIR)/cbftp-debug: misc/start_with_gdb.sh | $(BINDIR)
	cp misc/start_with_gdb.sh $@; chmod +x bin/cbftp-debug

$(BINDIR)/datafilecat: src/crypto.cpp src/tools/datafilecat.cpp Makefile.inc | $(BINDIR)
	${CXX} -o $@ ${OPTFLAGS} $(STATIC_SSL_INCLUDE) src/crypto.cpp src/tools/datafilecat.cpp $(SSL_LINKFLAGS)

$(BINDIR)/datafilewrite: src/crypto.cpp src/tools/datafilewrite.cpp Makefile.inc | $(BINDIR)
	${CXX} -o $@ ${OPTFLAGS} $(STATIC_SSL_INCLUDE) src/crypto.cpp src/tools/datafilewrite.cpp $(SSL_LINKFLAGS)

linecount:
	find|grep -e '\.h$$' -e '\.cpp$$'|awk '{print $$1}'|xargs wc -l	

clean:
ifneq ($(UI_PATH),)
	@+${MAKE} -C $(UI_PATH) clean
endif
	@+${MAKE} -C src clean
	@+$(MAKE) -C src/ext clean
	@+${MAKE} -C src/core clean
	@if test -d $(BINDIR); then rm -rf $(BINDIR); echo rm -rf $(BINDIR); fi
