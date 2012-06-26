CPPFLAGS=-m32 -O3 -DBUILDTIME="\"`date`\"" -DSVNREV="\"`svn info|grep Revision|awk '{ print $$2 }'`\""
FINALFLAGS=-m32 -O3 -lncurses -lpthread -lssl

BINS = clusterbomb datafilecat

OBJECTS = commandqueueelement.o ftpthreadcom.o potentialelement.o scoreboardelement.o \
	siterace.o transfer.o engine.o ftpthread.o potentiallistelement.o scoreboard.o \
	sitethreadmanager.o ui.o filelist.o globalcontext.o potentialtracker.o \
	sitemanager.o sitethread.o file.o main.o race.o site.o transfermanager.o \
	menuselectsite.o menuselectsiteelement.o menuselectoption.o \
	menuselectoptionelement.o rawbuffer.o uiwindow.o loginscreen.o mainscreen.o \
	editsitescreen.o confirmationscreen.o uiwindowcommand.o textinputfield.o \
	numinputarrow.o menuselectoptionnumarrow.o menuselectoptiontextfield.o \
	menuselectoptioncheckbox.o sitestatusscreen.o rawdatascreen.o crypto.o \
	datafilehandler.o newkeyscreen.o

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
uiwindowcommand.o:
textinputfield.o:
sitestatusscreen.o:
crypto.o:
datafilehandler.o:
newkeyscreen.o:

clusterbomb: ${OBJECTS}
	g++ -o clusterbomb $(FINALFLAGS) $(OBJECTS)
	
datafilecat: crypto.cpp datafilecat.cpp
	g++ -o datafilecat ${FINALFLAGS} -DNO_LOCAL_DEPS crypto.cpp datafilecat.cpp
	
clean:
	rm -f ${BINS} $(OBJECTS)
        
