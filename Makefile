include Makefile.inc

BINS = clusterbomb datafilecat datafilewrite

BINDIR = bin

all: ${BINS}

mkdirs:
	mkdir -p ${BINDIR}

sources:
	@cd src; ${MAKE}
	
clusterbomb: sources mkdirs
	g++ -o bin/clusterbomb $(FINALFLAGS) src/*.o src/ui/*.o src/ui/screens/*.o $(LINKFLAGS)
	
datafilecat: src/crypto.cpp src/datafilecat.cpp mkdirs
	g++ -o bin/datafilecat ${FINALFLAGS} -DNO_LOCAL_DEPS src/crypto.cpp src/datafilecat.cpp -lcrypto

datafilewrite: src/crypto.cpp src/datafilewrite.cpp mkdirs
	g++ -o bin/datafilewrite ${FINALFLAGS} -DNO_LOCAL_DEPS src/crypto.cpp src/datafilewrite.cpp -lcrypto

linecount:
	find|grep -e '\.h$$' -e '\.cpp$$'|awk '{print $$1}'|xargs wc -l	

clean:
	@cd src; ${MAKE} clean
	rm -rf ${BINDIR}
        
