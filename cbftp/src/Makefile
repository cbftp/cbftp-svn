include ../Makefile.inc

OBJECTS = potentialelement.o scoreboardelement.o \
	siterace.o engine.o ftpconn.o potentiallistelement.o scoreboard.o \
	sitelogicmanager.o filelist.o globalcontext.o potentialtracker.o \
	sitemanager.o sitelogic.o file.o main.o race.o site.o transfermanager.o \
	rawbuffer.o crypto.o datafilehandler.o transfermonitor.o \
	sitelogicrequest.o sitelogicrequestready.o tickpoke.o tickpoketarget.o \
	connstatetracker.o delayedcommand.o remotecommandhandler.o eventreceiver.o \
	iomanager.o workmanager.o event.o datablockpool.o datablock.o \
	skiplist.o sizelocationtrack.o eventlog.o proxy.o proxymanager.o \
	proxysession.o recursivecommandlogic.o localstorage.o localdownload.o \
	externalfileviewing.o transferstatus.o transferjob.o timereference.o \
	skiplistitem.o pendingtransfer.o localfile.o localfilelist.o util.o \
	localupload.o encoding.o signalevents.o

all: ${OBJECTS}
	@cd ui; ${MAKE}
	
main.o:
potentialelement.o: 
scoreboardelement.o:
siterace.o:
engine.o:
ftpconn.o:
potentiallistelement.o: 
scoreboard.o:
sitelogicmanager.o:
filelist.o:
globalcontext.o:
potentialtracker.o: 
sitemanager.o:
sitelogic.o:
file.o:
race.o:
site.o:
transfermanager.o:
rawbuffer.o:
crypto.o:
datafilehandler.o:
sitelogicrequest.o:
sitelogicrequestready.o:
tickpoke.o:
tickpoketarget.o:
connstatetracker.o:
delayedcommand.o:
remotecommandhandler.o:
eventreceiver.o:
iomanager.o:
workmanager.o:
event.o:
datablockpool.o:
datablock.o:
transfermonitor.o:
skiplist.o:
sizelocationtrack.o:
eventlog.o:
proxy.o:
proxymanager.o:
proxysession.o:
recursivecommandlogic.o:
localstorage.o:
localtransfer.o:
externalfileviewing.o:
transferstatus.o:
transferjob.o:
timereference.o:
skiplistitem.o:
pendingtransfer.o:
localfile.o:
localfilelist.o:
util.o:
localupload.o:
encoding.o:
signalevents.o:

clean:
	rm -f ${OBJECTS}
	@cd ui; ${MAKE} clean        