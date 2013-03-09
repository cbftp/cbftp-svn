include ../../Makefile.inc

OBJECTS = menuselectsite.o menuselectoption.o menuselectoptionelement.o \
    uiwindow.o uicommunicator.o textinputfield.o numinputarrow.o \
    menuselectoptionnumarrow.o menuselectoptiontextfield.o \
    menuselectoptioncheckbox.o legendwindow.o termint.o uifilelist.o uifile.o \
    ui.o menuselectoptiontextbutton.o focusablearea.o menuselectsiteelement.o \
    stringpair.o menusection.o menuselectoptioncontainer.o infowindow.o
	
all: ${OBJECTS}
	@cd screens; ${MAKE}

clean:
	rm -rf ${OBJECTS}
	@cd screens; ${MAKE} clean  