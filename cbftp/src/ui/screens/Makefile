include ../../../Makefile.inc

OBJECTS = loginscreen.o mainscreen.o sitestatusscreen.o editsitescreen.o \
	confirmationscreen.o rawdatascreen.o newkeyscreen.o browsescreen.o \
	addsectionscreen.o globaloptionsscreen.o newracescreen.o \
	racestatusscreen.o scoreboardscreen.o rawcommandscreen.o skiplistscreen.o \
	changekeyscreen.o eventlogscreen.o editproxyscreen.o proxyoptionsscreen.o \
	viewfilescreen.o nukescreen.o fileviewersettingsscreen.o \
	selectsitesscreen.o transfersscreen.o transferjobstatusscreen.o \
	allracesscreen.o alltransferjobsscreen.o browsescreenselector.o \
	browsescreensite.o browsescreenlocal.o

all: ${OBJECTS}

loginscreen.o: loginscreen.cpp ../../../Makefile.inc
	g++  ${CPPFLAGS} -DBUILDTIME=${DATE} -DVERSION=${VERSION} -DBOXTAG=${BOXTAG} -c -o loginscreen.o loginscreen.cpp
clean:
	rm -rf ${OBJECTS}
