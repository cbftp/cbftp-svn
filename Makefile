all:
	wc -l *.h *.cpp;
	g++ -O0 -m32 -lssl -lpthread -o clusterbomb main.cpp engine.cpp \
        ftpthread.cpp sitethread.cpp filelist.cpp scoreboard.cpp \
        sitemanager.cpp site.cpp file.cpp scoreboardelement.cpp race.cpp \
        siterace.cpp globalcontext.cpp sitethreadmanager.cpp \
        commandqueueelement.cpp transfer.cpp transfermanager.cpp \
        ftpthreadcom.cpp
clean:
	rm clusterbomb
