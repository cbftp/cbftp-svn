# uncomment and edit this line to link with a static openssl
#STATIC_SSL_PATH := /path/to/openssl

# relative path to the ui to use
UI_PATH := src/ui

ifneq ($(STATIC_SSL_PATH),)
STATIC_SSL_INCLUDE := -I$(STATIC_SSL_PATH)/include
STATIC_SSL_LINK := $(STATIC_SSL_PATH)/libssl.a $(STATIC_SSL_PATH)/libcrypto.a
SSL_LINKFLAGS := $(STATIC_SSL_LINK) -ldl
else
SSL_LINKFLAGS := -lssl -lcrypto
endif

HAS_SVN := ${shell svn info > /dev/null 2>&1; echo $$?}

#svntest:
ifeq ($(HAS_SVN), 0)
VERSION := "\"svn:r${shell svn info|grep Revision|awk '{ print $$2 }'}\""
else
VERSION := "\"unknown\""
endif

DATE := "\"${shell date}\""

BOXTAG := "\"cbftp\""

CXX=g++
OPTFLAGS=-g -O0
CPPFLAGS=$(OPTFLAGS) -Wall -MMD -MP $(STATIC_SSL_INCLUDE)
LINKFLAGS=$(SSL_LINKFLAGS) -lncursesw -lpthread 
