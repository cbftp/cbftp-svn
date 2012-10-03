include Makefile.inc

BINS = clusterbomb datafilecat

BINDIR = bin

all: ${BINS}

mkdirs:
	mkdir -p ${BINDIR}

sources:
	@cd src; ${MAKE}
	
clusterbomb: sources mkdirs
	g++ -g -o bin/clusterbomb $(FINALFLAGS) src/*.o src/ui/*.o $(LINKFLAGS)
	
datafilecat: src/crypto.cpp src/datafilecat.cpp sources mkdirs
	g++ -o bin/datafilecat ${FINALFLAGS} -DNO_LOCAL_DEPS src/crypto.cpp src/datafilecat.cpp $(LINKFLAGS)
	
clean:
	@cd src; ${MAKE} clean
	rm -rf ${BINDIR}
        
