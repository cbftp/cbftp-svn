include ../../Makefile.inc

OBJECTS = menuselectsite.o menuselectoption.o menuselectoptionelement.o \
    uiwindow.o textinputfield.o numinputarrow.o uicommand.o \
    menuselectoptionnumarrow.o menuselectoptiontextfield.o \
    menuselectoptioncheckbox.o legendwindow.o termint.o uifilelist.o uifile.o \
    ui.o menuselectoptiontextbutton.o focusablearea.o menuselectsiteelement.o \
    menusection.o menuselectoptioncontainer.o infowindow.o \
    textarrow.o menuselectoptiontextarrow.o commandhistory.o chardraw.o \
    menuselectadjustableline.o
	
all: ${OBJECTS}
	@cd screens; ${MAKE}

clean:
	rm -rf ${OBJECTS}
	@cd screens; ${MAKE} clean  
