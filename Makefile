include Makefile.inc

.PHONY: src

BINDIR = bin

BINS = cbftp $(BINDIR)/cbftp-debug $(BINDIR)/datafilecat $(BINDIR)/datafilewrite

SRC = $(wildcard src/*.cpp src/ui/*.cpp src/ui/screens/*.cpp)

OBJS = $(wildcard $(SRC:%.cpp=%.o))

all: ${BINS}

$(BINDIR):
	mkdir -p $@

src:
	@+${MAKE} -C src
	
$(BINDIR)/cbftp: $(OBJS)
	${CXX} -o $(BINDIR)/cbftp $(FINALFLAGS) $(SRC:%.cpp=%.o) $(LINKFLAGS)
	
cbftp: src | $(BINDIR)
	@+${MAKE} --no-print-directory $(BINDIR)/cbftp
	
$(BINDIR)/cbftp-debug: misc/start_with_gdb.sh | $(BINDIR)
	cp misc/start_with_gdb.sh $@; chmod +x bin/cbftp-debug

$(BINDIR)/datafilecat: src/crypto.cpp src/tools/datafilecat.cpp Makefile.inc | $(BINDIR)
	${CXX} -o $@ ${FINALFLAGS} $(STATIC_SSL_INCLUDE) src/crypto.cpp src/tools/datafilecat.cpp $(SSL_LINKFLAGS)

$(BINDIR)/datafilewrite: src/crypto.cpp src/tools/datafilewrite.cpp Makefile.inc | $(BINDIR)
	${CXX} -o $@ ${FINALFLAGS} $(STATIC_SSL_INCLUDE) src/crypto.cpp src/tools/datafilewrite.cpp $(SSL_LINKFLAGS)

linecount:
	find|grep -e '\.h$$' -e '\.cpp$$'|awk '{print $$1}'|xargs wc -l	

clean:
	@+${MAKE} -C src clean
	@if test -d $(BINDIR); then rm -rf $(BINDIR); echo rm -rf $(BINDIR); fi
