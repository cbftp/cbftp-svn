include ../../../Makefile.inc

OBJECTS = loginscreen.o mainscreen.o sitestatusscreen.o editsitescreen.o \
	confirmationscreen.o rawdatascreen.o newkeyscreen.o browsescreen.o \
	addsectionscreen.o globaloptionsscreen.o newracescreen.o \
	racestatusscreen.o scoreboardscreen.o

all: ${OBJECTS}

loginscreen.o:
	g++  ${CPPFLAGS} -DBUILDTIME=${DATE} -DVERSION=${VERSION} -c -o loginscreen.o loginscreen.cpp
clean:
	rm -rf ${OBJECTS}