include ../Makefile.inc

OBJECTS = commandqueueelement.o ftpthreadcom.o potentialelement.o scoreboardelement.o \
	siterace.o transfer.o engine.o ftpthread.o potentiallistelement.o scoreboard.o \
	sitethreadmanager.o filelist.o globalcontext.o potentialtracker.o \
	sitemanager.o sitethread.o file.o main.o race.o site.o transfermanager.o \
	rawbuffer.o crypto.o datafilehandler.o \
	sitethreadrequest.o sitethreadrequestready.o tickpoke.o tickpoketarget.o \
	connstatetracker.o delayedcommand.o

all: ${OBJECTS}
	@cd ui; ${MAKE}
	
main.o:
commandqueueelement.o:
ftpthreadcom.o:
potentialelement.o: 
scoreboardelement.o:
siterace.o:
transfer.o:
engine.o:
ftpthread.o:
potentiallistelement.o: 
scoreboard.o:
sitethreadmanager.o:
filelist.o:
globalcontext.o:
potentialtracker.o: 
sitemanager.o:
sitethread.o:
file.o:
race.o:
site.o:
transfermanager.o:
rawbuffer.o:
crypto.o:
datafilehandler.o:
sitethreadrequest.o:
sitethreadrequestready.o:
tickpoke.o:
tickpoketarget.o:
connstatetracker.o:
delayedcommand.o:

clean:
	rm -f ${OBJECTS}
	@cd ui; ${MAKE} clean        
