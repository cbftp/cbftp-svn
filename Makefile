CPPFLAGS=-m32 -DBUILDTIME="\"`date`\"" -DSVNREV="\"`svn info|grep Revision|awk '{ print $$2 }'`\"" -o build/
FINALFLAGS=-m32 -O0 -lncurses -lpthread -lssl
OBJECTS = commandqueueelement.o ftpthreadcom.o potentialelement.o scoreboardelement.o \
	siterace.o transfer.o engine.o ftpthread.o potentiallistelement.o scoreboard.o \
	sitethreadmanager.o ui.o filelist.o globalcontext.o potentialtracker.o \
	sitemanager.o sitethread.o file.o main.o race.o site.o transfermanager.o \
	menuselectsite.o menuselectsiteelement.o

all: $(OBJECTS)
	wc -l *.cpp *.h;
	g++ -o clusterbomb $(FINALFLAGS) $(OBJECTS)
	
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

clean:
	rm clusterbomb $(OBJECTS)
        
