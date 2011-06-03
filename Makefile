CCFLAGS=-m32
LINKFLAGS=-O0 -lncurses -lpthread -lssl
OBJECTS = commandqueueelement.o ftpthreadcom.o potentialelement.o scoreboardelement.o \
	siterace.o transfer.o engine.o ftpthread.o potentiallistelement.o scoreboard.o \
	sitethreadmanager.o ui.o filelist.o globalcontext.o potentialtracker.o \
	sitemanager.o sitethread.o file.o main.o race.o site.o transfermanager.o

all: $(OBJECTS)
	g++ -o clusterbomb $(LINKFLAGS) $(OBJECTS)
	
main.o:
commandqueueelement.o:
ftpthreadcom.o: commandqueueelement.h

clean:
	rm clusterbomb
