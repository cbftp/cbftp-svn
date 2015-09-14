HAS_SVN := ${shell svn info > /dev/null 2>&1; echo $$?}

#svntest:
ifeq ($(HAS_SVN), 0)
VERSION := "\"svn:r${shell svn info|grep Revision|awk '{ print $$2 }'}\""
else
VERSION := "\"unknown\""
endif

DATE := "\"${shell date}\""

BOXTAG := "\"cbftp\""

CPPFLAGS :=-g -Wall -O0
FINALFLAGS=-g -O0
LINKFLAGS=-lncursesw -lpthread -lssl -lcrypto

