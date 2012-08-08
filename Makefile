CPPFLAGS=-g -O0 -DBUILDTIME="\"`date`\"" -DVERSION="\"svn:r`svn info|grep Revision|awk '{ print $$2 }'`\""
FINALFLAGS=-g -O0
LINKFLAGS=-g -lncurses -lpthread -lssl -lcrypto

BINS = clusterbomb datafilecat

OBJECTS = commandqueueelement.o ftpthreadcom.o potentialelement.o scoreboardelement.o \
	siterace.o transfer.o engine.o ftpthread.o potentiallistelement.o scoreboard.o \
	sitethreadmanager.o ui.o filelist.o globalcontext.o potentialtracker.o \
	sitemanager.o sitethread.o file.o main.o race.o site.o transfermanager.o \
	menuselectsite.o menuselectsiteelement.o menuselectoption.o \
	menuselectoptionelement.o rawbuffer.o uiwindow.o loginscreen.o mainscreen.o \
	editsitescreen.o confirmationscreen.o uicommunicator.o textinputfield.o \
	numinputarrow.o menuselectoptionnumarrow.o menuselectoptiontextfield.o \
	menuselectoptioncheckbox.o sitestatusscreen.o rawdatascreen.o crypto.o \
	datafilehandler.o newkeyscreen.o legendwindow.o termint.o browsescreen.o \
	sitethreadrequest.o sitethreadrequestready.o

all: ${BINS}
	
main.o:
commandqueueelement.o:
ftpthreadcom.o: commandqueueelement.h
potentialelement.o: 
scoreboardelement.o:
siterace.o:
transfer.o:
engine.o:
ftpthread.o:
potentiallistelement.o: 
scoreboard.o:
sitethreadmanager.o:
ui.o:
filelist.o:
globalcontext.o:
potentialtracker.o: 
sitemanager.o:
sitethread.o:
file.o:
race.o:
site.o:
transfermanager.o:
menuselectsite.o:
menuselectsiteelement.o:
rawbuffer.o:
uiwindow.o:
loginscreen.o:
mainscreen.o:
editsitescreen.o:
confirmationscreen.o:
uicommunicator.o:
textinputfield.o:
sitestatusscreen.o:
crypto.o:
datafilehandler.o:
newkeyscreen.o:
legendwindow.o:
termint.o:
browsescreen.o:
sitethreadrequest.o:
sitethreadrequestready.o:

clusterbomb: ${OBJECTS}
	g++ -g -o clusterbomb $(FINALFLAGS) $(OBJECTS) $(LINKFLAGS)
	
datafilecat: crypto.cpp datafilecat.cpp
	g++ -o datafilecat ${FINALFLAGS} -DNO_LOCAL_DEPS crypto.cpp datafilecat.cpp $(LINKFLAGS)
	
clean:
	rm -f ${BINS} $(OBJECTS)
        
