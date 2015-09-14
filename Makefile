include Makefile.inc

BINS = cbftp cbftp-debug datafilecat datafilewrite

BINDIR = bin

all: ${BINS}

mkdirs:
	mkdir -p ${BINDIR}

sources:
	@cd src; ${MAKE}
	
cbftp: sources mkdirs
	g++ -o bin/cbftp $(FINALFLAGS) src/*.o src/ui/*.o src/ui/screens/*.o $(LINKFLAGS)
	
cbftp-debug: sources mkdirs
	cp misc/start_with_gdb.sh bin/cbftp-debug; chmod +x bin/cbftp-debug

datafilecat: src/crypto.cpp src/datafilecat.cpp mkdirs
	g++ -o bin/datafilecat ${FINALFLAGS} src/crypto.cpp src/datafilecat.cpp -lcrypto

datafilewrite: src/crypto.cpp src/datafilewrite.cpp mkdirs
	g++ -o bin/datafilewrite ${FINALFLAGS} src/crypto.cpp src/datafilewrite.cpp -lcrypto

linecount:
	find|grep -e '\.h$$' -e '\.cpp$$'|awk '{print $$1}'|xargs wc -l	

clean:
	@cd src; ${MAKE} clean
	rm -rf ${BINDIR}
        
