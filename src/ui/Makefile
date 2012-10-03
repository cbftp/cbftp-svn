include ../../Makefile.inc

OBJECTS = menuselectsite.o menuselectoption.o menuselectoptionelement.o uiwindow.o loginscreen.o mainscreen.o \
	editsitescreen.o confirmationscreen.o uicommunicator.o textinputfield.o numinputarrow.o \
	menuselectoptionnumarrow.o menuselectoptiontextfield.o menuselectoptioncheckbox.o sitestatusscreen.o \
	rawdatascreen.o newkeyscreen.o legendwindow.o termint.o browsescreen.o uifilelist.o uifile.o ui.o \
	menuselectsiteelement.o

all: ${OBJECTS}

loginscreen.o:
	g++  ${CPPFLAGS} -DBUILDTIME=${DATE} -DVERSION=${VERSION} -c -o loginscreen.o loginscreen.cpp
clean:
	rm -rf ${OBJECTS}
